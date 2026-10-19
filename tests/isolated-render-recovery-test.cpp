#include "blit.hpp"
#include "surface_copy.hpp"
#include "gpu_production_queue.hpp"
#include <QGuiApplication>
#include <QElapsedTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <thread>
#include <chrono>
#include <iostream>
using namespace mnm::render;
namespace {
void require(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
constexpr PixelFormat rgb{16,{0xf800,0x7e0,31}};
void pixels(GlBlitter& g,SurfaceId id,const std::vector<std::uint32_t>& expected){require(g.read(id).pixels==expected,"Native state or exactly-once operation disagreed with oracle");}
}
int main(int argc,char** argv){
    qputenv("MNM_NATIVE_DEVICE_RECOVERY","1");qputenv("MNM_RENDER_PROCESS_TIMEOUT_MS","1000");
    QGuiApplication app(argc,argv);
    try{
        GlBlitter g;
        auto dst=g.create({2,2,{0,0,0,0}},rgb);
        auto frame=g.presentGpu(dst);auto frozen=frame.snapshot();
        const auto partial=g.allocate(2,2,rgb);g.update(partial,0,0,{1,1,{0xf800}});
        const auto indexed=g.create({2,2,{0,1,1,0}},{8,{}});g.setPalette(indexed,0,{{255,0,0},{0,255,0}});
        const auto indexFrame=g.presentGpu(indexed);const auto frozenPalette=indexFrame.snapshot();
        g.setClipper(dst,{true,std::vector<Rect>{{0,0,1,1}}});
        // This checkpoint must retain undefined regions, palette and clipper metadata.
        g.checkpointRecovery();
        QElapsedTimer elapsed;elapsed.start();
        for(unsigned mode=1;mode<=3;++mode){
            const std::array<QByteArray,4> names={"__GLX_VENDOR_LIBRARY_NAME","__EGL_VENDOR_LIBRARY_FILENAMES","MESA_LOADER_DRIVER_OVERRIDE","GALLIUM_DRIVER"};
            std::array<QByteArray,4> before;
            for(unsigned n=0;n<names.size();++n){before[n]=qgetenv(names[n].constData());if(mode==1)qputenv(names[n].constData(),"unavailable-test-vendor");}
            g.injectGraphicsFault(mode);
            // Every failure occurs after this non-idempotent GPU draw but before
            // its acknowledgment. A retry must not add the channels twice.
            g.additiveRect(dst,{0,0,2,2},{1,1,1});
            if(mode==1)for(unsigned n=0;n<names.size();++n){if(before[n].isNull())qunsetenv(names[n].constData());else qputenv(names[n].constData(),before[n]);}
            const auto expected=std::uint32_t(mode)*0x821u;
            pixels(g,dst,{expected,expected,expected,expected});
            require(g.stats().recoveryRebuilds==mode,"Worker failure did not replace process exactly once");
        }
        const auto faultMilliseconds=elapsed.elapsed();require(faultMilliseconds<8000,"Hung child blocked recovery deadline");
        require(frozen.raster().pixelColor(0,0)==QColor(Qt::black),"Immutable pre-fault frame changed");
        auto next=g.presentGpu(dst);require(frame.raster()==next.raster() && frame.raster()!=frozen.raster(),"Mutable frame identity not preserved");
        require(frozenPalette.raster().pixelColor(1,0)==QColor(Qt::green),"Frozen palette lost on checkpoint restart");
        bool undefined=false;try{g.read(partial);}catch(const std::runtime_error&){undefined=true;}
        require(undefined && g.stats().recoveryRebuilds==3,"Undefined contents were exposed or domain refusal retried");
        g.update(partial,1,0,{1,1,{0x7e0}});g.update(partial,0,1,{2,1,{31,65535}});pixels(g,partial,{0xf800,0x7e0,31,65535});
        auto source=g.create({2,2,{31,31,31,31}},rgb);
        auto clipped=g.surfaceCopy(source,dst,{SurfaceCopyApi::Blt,{0,0,2,2},{0,0,2,2},0});require(!clipped.hresult && clipped.pieces==1,"Clipper metadata lost");
        pixels(g,dst,{31,0x1863,0x1863,0x1863});
        // Compaction bounds replay storage and is itself replaceable from the
        // previous committed checkpoint plus journal.
        for(unsigned n=0;n<270;++n)g.update(dst,0,0,{1,1,{n}});
        g.injectGraphicsFault(1);auto copy=g.cloneSurface(dst);pixels(g,copy,{269,0x1863,0x1863,0x1863});
        require(g.stats().recoveryCheckpoints>=2,"Journal was not compacted");
        g.injectGraphicsFault(3);g.checkpointRecovery();pixels(g,copy,{269,0x1863,0x1863,0x1863});
        GlBlitter other;auto target=other.allocate(2,2,rgb);other.importGpu565(target,g.presentGpu(copy));pixels(other,target,{269,0x1863,0x1863,0x1863});
        auto imported=other.presentGpu(target).snapshot();other.injectGraphicsFault(2);other.importGpu565(target,imported);pixels(other,target,{269,0x1863,0x1863,0x1863});
        // Queue/application callbacks execute once; only their renderer call is
        // retried. Both existing immutable leases survive a producer fault.
        GpuProductionQueue queue(nullptr,nullptr);std::uint64_t callbacks=0;SurfaceId queueSurface=0;
        for(unsigned i=1;i<=3;++i){
            require(queue.submit(i,[&,i](GlBlitter& renderer){++callbacks;if(!queueSurface)queueSurface=renderer.create({1,1,{0}},rgb);if(i==2)renderer.injectGraphicsFault(1);renderer.additiveRect(queueSurface,{0,0,1,1},{1,0,0});return renderer.presentGpu(queueSurface);}),"Queue admission");
            GpuProductionQueue::Completion result;QElapsedTimer wait;wait.start();
            while(!queue.take(result)&&wait.elapsed()<5000)std::this_thread::sleep_for(std::chrono::milliseconds(1));
            require(result.frame.valid() && result.frame.raster().pixelColor(0,0).red()==int(i*8),"Producer continuation disagreed with oracle");
        }
        require(callbacks==3,"Application producer callback replayed");queue.join();
        const QJsonObject report{{"success",true},{"post_draw_faults",3},{"crash_recovery",true},{"hung_worker_recovery",true},{"graphics_error_recovery",true},{"software_vendor_override",true},{"fault_elapsed_ms",double(faultMilliseconds)},{"partial_validity",true},{"palette_and_clipper",true},{"immutable_and_mutable_frames",true},{"journal_compaction",true},{"checkpoint_failure_recovery",true},{"cross_owner_import",true},{"producer_callback_count",int(callbacks)},{"physical_hardware_reset",false}};
        std::cout<<QJsonDocument(report).toJson().constData();return 0;
    }catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
}
