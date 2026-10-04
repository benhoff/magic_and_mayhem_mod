#include "realm_viewer_widget.hpp"
#include "realm_viewer_assets.hpp"
#include "realm-viewer-fixtures.hpp"
#include <QApplication>
#include <QMouseEvent>
#include <QTemporaryDir>
#include <QTimer>
#include <cstdio>
static void require(bool value,const char* message){if(!value)throw std::runtime_error(message);}
static RealmViewerWidget::Campaign campaign(const QString& root){
    RealmViewerWidget::Campaign c;c.id="visual-preview";c.name="Campaign";const auto catalog=mnm::ui::loadRealmCatalog(root);
    for(int r=0;r<3;++r)for(const auto& item:catalog[r])c.regions[r].push_back({QString("%1-%2").arg(r).arg(item.artworkNumber),item.name,item.artworkNumber,item.flagPosition,0,true,{true,true,true}});
    return c;
}
static void click(QWidget& widget,QPoint point,bool doubleClick=false){
    QMouseEvent event(doubleClick?QEvent::MouseButtonDblClick:QEvent::MouseButtonPress,QPointF(point),QPointF(widget.mapToGlobal(point)),Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&widget,&event);
}
int main(int argc,char** argv){QApplication app(argc,argv);try{
    QTemporaryDir fixture;require(fixture.isValid(),"temporary fixture");realm_viewer_fixture::create(fixture.path());
    auto flags=mnm::ui::loadMenuSprites(fixture.path(),"Interface/RealmViewer/Generic/flags.spr");
    auto visuals=mnm::ui::loadRealmViewerVisuals(fixture.path(),flags);const auto& shape=visuals.regions[0][0];
    require(shape.contains({125,300})&&!shape.contains({133,308})&&!shape.contains({124,300}),"binary mask edges and transparent hole");
    require(qAlpha(shape.border.pixel(133-shape.borderRect.x(),308-shape.borderRect.y()))==0,"blue colour key did not become transparent");
    const auto& clip=visuals.animations[0];require(clip.frames.size()==2&&clip.loop&&clip.duration==4,"ANI delay/loop compilation");
    require(mnm::ui::realmFlagFrame(clip,0).image==flags[0].image&&mnm::ui::realmFlagFrame(clip,1).image==flags[0].image&&mnm::ui::realmFlagFrame(clip,2).image==flags[1].image&&mnm::ui::realmFlagFrame(clip,4).image==flags[0].image,"ANI frame holds and looping");
    RealmViewerWidget widget;QString error;require(widget.loadAssets(fixture.path(),&error),qPrintable(error));auto model=campaign(fixture.path());require(widget.setCampaign(model,&error),qPrintable(error));
    require(widget.regionAt({125,300})=="0-1"&&widget.regionAt({133,308}).isEmpty()&&widget.regionAt({124,300}).isEmpty(),"actual silhouette selection");
    int changed=0,requests=0;QObject::connect(&widget,&RealmViewerWidget::selectionChanged,&widget,[&](const auto&){++changed;});QObject::connect(&widget,&RealmViewerWidget::regionRequested,&widget,[&](const auto&){++requests;});
    QMouseEvent hover(QEvent::MouseMove,QPointF(150,300),QPointF(150,300),Qt::NoButton,Qt::NoButton,Qt::NoModifier);QApplication::sendEvent(&widget,&hover);require(widget.toolTip()=="Celtic region 2"&&widget.cursor().shape()==Qt::PointingHandCursor,"map hover name and cursor");
    click(widget,{150,300});require(widget.campaign().selectedRegionIds[0]=="0-2"&&changed==1&&requests==0,"map click selects without opening");
    click(widget,{124,300});require(changed==1,"background click selected a rectangle");
    click(widget,{150,300},true);require(requests==1,"map double click opens selected region");
    model.regions[0][1].available=false;require(widget.setCampaign(model,&error),qPrintable(error));click(widget,{150,300},true);require(requests==1,"unavailable map activation guard");
    require(widget.setCampaign(campaign(fixture.path()),&error),qPrintable(error));
    for(int t=0;t<8;++t){const auto image=widget.grab().toImage();require(image.pixelColor(125,200)==QColor(80,100,120)&&image.pixelColor(135,210)==QColor(80,100,120),"idle selection invented a moving character on the FP route");widget.advanceAnimation();}
    widget.show();app.processEvents();auto* timer=widget.findChild<QTimer*>("realmViewerAnimationTimer");require(timer&&timer->isActive(),"visible animation timer");widget.hide();require(!timer->isActive(),"hidden menu animation timer stopped");
    widget.resize(1200,600);widget.show();app.processEvents();require(widget.regionAt({325,300})=="0-1"&&widget.regionAt({125,300}).isEmpty(),"letterbox mapping");widget.resize(400,300);app.processEvents();require(widget.regionAt({63,150})=="0-1"&&widget.regionAt({62,150}).isEmpty(),"scaled shape mapping");widget.hide();
    // Bad visual reloads preserve the complete existing screen and model.
    realm_viewer_fixture::write(fixture.path()+"/Interface/RealmViewer/800x600/Celtic_Border_01.pcx","bad PCX");require(!widget.loadAssets(fixture.path(),&error)&&widget.regionAt({63,150})=="0-1","PCX reload rollback");realm_viewer_fixture::create(fixture.path());
    auto broken=realm_viewer_fixture::ani();realm_viewer_fixture::put(broken,44+52*4+44+4,999);realm_viewer_fixture::write(fixture.path()+"/Interface/RealmViewer/Generic/flags.ani",broken);require(!widget.loadAssets(fixture.path(),&error)&&widget.regionAt({63,150})=="0-1","ANI reload rollback");realm_viewer_fixture::create(fixture.path());
    auto badPath=realm_viewer_fixture::fp(100,100);realm_viewer_fixture::put(badPath,116,800);realm_viewer_fixture::write(fixture.path()+"/Interface/RealmViewer/Generic/Celtic_FlagPath_01.FP",badPath);require(!widget.loadAssets(fixture.path(),&error),"path canvas bounds");
    if(argc==2){const QString root=QString::fromLocal8Bit(argv[1]);auto installedFlags=mnm::ui::loadMenuSprites(root,"Interface/RealmViewer/Generic/flags.spr");const auto installed=mnm::ui::loadRealmViewerVisuals(root,installedFlags);require(installed.animations[0].frames.size()==11&&installed.animations[50].frames.size()==12,"installed ANI animation roles");
        for(const auto& frame:installed.animations[0].frames)require(QRect(0,0,48,52).contains(QRect(QPoint(2,48)-frame.sprite.origin,frame.sprite.image.size())),"animated flag clipped outside accessible cell");
        RealmViewerWidget viewer;require(viewer.loadAssets(root,&error),qPrintable(error));require(viewer.setCampaign(campaign(root),&error),qPrintable(error));
        int count=0;for(int r=0;r<3;++r){require(viewer.selectRealm(RealmViewerWidget::Realm(r),&error),qPrintable(error));
            for(const auto& region:viewer.campaign().regions[r]){const auto& visual=installed.regions[r][region.artworkNumber-1];QPoint inside(-1,-1);for(int y=visual.maskRect.top();y<=visual.maskRect.bottom()&&inside.x()<0;++y)for(int x=visual.maskRect.left();x<=visual.maskRect.right();++x)if(viewer.regionAt({x,y})==region.id){inside={x,y};break;}require(inside.x()>=0,"installed region has no selectable silhouette pixel");
                click(viewer,inside);require(viewer.campaign().selectedRegionIds[r]==region.id,"installed region shape selects correct ID");
                for(const auto size:{QSize(1200,600),QSize(400,300)}){viewer.resize(size);viewer.show();app.processEvents();const auto canvas=viewer.contentRect();const QRect bounds=QRect(canvas.x()+visual.maskRect.x()*canvas.width()/800,canvas.y()+visual.maskRect.y()*canvas.height()/600,(visual.maskRect.width()+2)*canvas.width()/800+2,(visual.maskRect.height()+2)*canvas.height()/600+2).intersected(canvas);
                    // At half size, a one-pixel edge can disappear. Find a hit
                    // in the scaled mask instead of assuming rounded-edge parity.
                    bool found=false;for(int y=bounds.top();y<=bounds.bottom()&&!found;++y)for(int x=bounds.left();x<=bounds.right();++x)if(viewer.regionAt({x,y})==region.id){found=true;break;}require(found,"scaled installed shape became unselectable");}
                viewer.resize(800,600);app.processEvents();++count;
            }
            require(viewer.selectRegion(viewer.campaign().regions[r][0].id,&error),qPrintable(error));for(int t=0;t<30;++t)viewer.advanceAnimation();
            require(QDir().mkpath("working/tests/realm-visuals"),"capture dir");require(viewer.grab().save(QString("working/tests/realm-visuals/realm-%1.png").arg(r)),"installed visual capture");
        }
        require(count==36,"installed region count");std::puts("All 36 installed silhouettes selectable at native, half and letterboxed sizes; flag animation captured");
    }
    std::puts("Realm shape holes/keying, ANI timing/loops, idle route stability, availability, timer lifetime and reload rollback passed");return 0;
}catch(const std::exception& e){std::fprintf(stderr,"Realm visual test: %s\n",e.what());return 1;}}
