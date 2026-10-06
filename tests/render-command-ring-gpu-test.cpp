#include "commands.hpp"
#include "gl_viewport.hpp"
#include "../protocols/include/mnm/render_command_ring.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <QtEndian>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <vector>
namespace {
void check(bool ok,const char* why){if(!ok)throw std::runtime_error(why);}
void word(QByteArray& b,unsigned n){auto at=b.size();b.resize(at+4);qToLittleEndian<quint32>(n,b.data()+at);}
void record(QByteArray& b,unsigned sequence,unsigned op,std::initializer_list<unsigned> fields,const QByteArray& data={}){
    word(b,op);word(b,sequence);word(b,unsigned(fields.size()*4+data.size()));for(auto f:fields)word(b,f);b+=data;
}
QByteArray pixels(unsigned color){QByteArray b(1024*512*2,0);for(qsizetype at=0;at<b.size();at+=2)qToLittleEndian<quint16>(color,b.data()+at);return b;}
}
int main(int argc,char** argv){
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);QApplication app(argc,argv);
    try{
        GlViewport viewport;viewport.setMinimumSize(1,1);viewport.resize(128,64);viewport.show();app.processEvents();check(viewport.ready(),"viewport ready");
        std::vector<uint32_t> map(MNM_RENDER_COMMANDS_V2_SIZE/4);std::memcpy(map.data(),MNM_RENDER_COMMANDS_V2_MAGIC,8);map[2]=2;map[3]=MNM_RENDER_COMMANDS_V2_SIZE;map[4]=123;
        mnm_ring_reader reader{};mnm_ring_writer writer{};check(mnm_ring_reader_bind(&reader,map.data(),MNM_RENDER_COMMANDS_V2_SIZE),"reader bind");check(mnm_ring_writer_bind(&writer,map.data(),MNM_RENDER_COMMANDS_V2_SIZE),"writer bind");
        QByteArray stream("MNMCMD01");word(stream,1);word(stream,16);unsigned seq=0;
        record(stream,++seq,1,{1,1024,512,16,0xf800,0x7e0,0x1f},pixels(0xf800));record(stream,++seq,6,{1});
        for(unsigned color:{0x7e0u,0x1fu,0xffffu}){record(stream,++seq,2,{1,0,0,1024,512},pixels(color));record(stream,++seq,6,{1});}
        record(stream,++seq,7,{1});record(stream,++seq,8,{});
        mnm::render::CommandDecoder decoder;mnm::render::GlBlitter renderer(viewport.context());unsigned frames=0,full=0;
        const QRgb expected[]={qRgb(255,0,0),qRgb(0,255,0),qRgb(0,0,255),qRgb(255,255,255)};
        mnm::render::CommandConsumer consumer(renderer,[&](auto frame){
            viewport.setGpuFrame(std::move(frame));viewport.repaint();auto image=viewport.grabFramebuffer();check(frames<4,"extra frame");
            for(int y=0;y<image.height();++y)for(int x=0;x<image.width();++x){
                check(image.pixel(x,y)==expected[frames],"independent complete display frame");
            }
            ++frames;
        },{mnm::render::CommandDiagnostics::Skip,false});
        QByteArray owned(MNM_RENDER_COMMANDS_V2_POLL_BYTES,0);qsizetype at=0;bool ended=false;
        while(!ended || reader.consumed!=writer.published){
            while(at<stream.size()){
                auto n=unsigned(std::min<qsizetype>(65531,stream.size()-at));int result=mnm_ring_write(&writer,stream.constData()+at,n);check(result>=0,"writer refusal");
                if(!result){++full;break;}at+=n;
            }
            if(at==stream.size() && !ended){check(mnm_ring_end(&writer),"writer END");ended=true;}
            const auto n=mnm_ring_read(&reader,owned.data(),owned.size());check(n>=0,"reader refusal");
            auto commands=decoder.append(owned.first(n));consumer.submit(commands.data(),commands.size());
        }
        decoder.finish();consumer.finish();const auto& result=consumer.result();
        check(frames==4 && full>0 && writer.published>4*MNM_RENDER_COMMANDS_V2_CAPACITY,"four wrapped frames and backpressure");
        check(result.liveSurfaces==0 && result.stats.nativeReadbacks==0 && result.stats.rgbaReadbacks==0 && viewport.imageUploads()==0,"ordinary GPU counters/cleanup");
        viewport.setGpuFrame({});std::printf("{\"success\":true,\"frames\":%u,\"bytes\":%u,\"full_retries\":%u,\"ordinary_readbacks\":0,\"viewport_uploads\":0}\n",frames,writer.published,full);return 0;
    }catch(const std::exception& e){std::fprintf(stderr,"ring GPU test failed: %s\n",e.what());return 1;}
}
