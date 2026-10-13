#include "realm_viewer_widget.hpp"
#include "region_entry_widget.hpp"
#include "live_character_controller.hpp"
#include "character-wire-fixture.hpp"
#include "realm-viewer-fixtures.hpp"
#include "campaign-menu-assets-fixture.hpp"
#include <QApplication>
#include <QKeyEvent>
#include <QMap>
#include <QPushButton>
#include <QRadioButton>
#include <QStackedWidget>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonObject>
#include <cstdio>

static void check(bool ok,const char* why){CharacterWireFixture::check(ok,why);}
class CampaignFixture final {
public:
    CampaignFixture():session_("/unused"){
        check(assets_.isValid(),"fixture asset root");realm_viewer_fixture::create(assets_.path());campaign_menu_fixture_assets::create(assets_.path());
        stack_.setWindowTitle("Magic & Mayhem — synthetic campaign menu fixture");stack_.resize(800,600);
        realm_=new RealmViewerWidget(&stack_);entry_=new RegionEntryWidget(&stack_);character_=new CharacterScreenWidget(&stack_);
        stack_.addWidget(realm_);stack_.addWidget(entry_);stack_.addWidget(character_);
        QString error;check(realm_->loadAssets(assets_.path(),&error)&&entry_->loadAssets(assets_.path(),&error)&&character_->loadAssets(assets_.path(),&error),"synthetic menu assets");
        textControls(realm_);textControls(entry_);textControls(character_);
        RealmViewerWidget::Campaign model;model.id="fixture-campaign";model.name="Fixture campaign";
        model.auxiliaryAvailable={false,false,true,false,false};
        for(int r=0;r<3;++r)for(int n=0;n<2;++n){
            RealmViewerWidget::Region region;region.id=QString("fixture-%1-%2").arg(r).arg(n);
            region.name=QString("Fixture region %1.%2").arg(r+1).arg(n+1);region.artworkNumber=n+1;
            region.flagPosition={125+n*80,180+r*70};region.flagArtworkIndex=-1;
            region.auxiliaryAvailable={false,false,true};model.regions[r].append(region);
        }
        check(realm_->setCampaign(model,&error),"fixture campaign");
        session_.characterMenusEnabled=true;peer_.attach(session_);
        controller_=std::make_unique<LiveCharacterController>(session_,*character_);
        session_.stateChanged=[this](const auto& state){
            check(controller_->present(state),"production Character presentation");
            if(state.screen==20){stack_.setCurrentWidget(character_);if(state.ready)character_->focusFirstControl();}
        };
        session_.originalViewportRequested=[this]{
            ++returns_;MenuBridge::State outside;check(controller_->present(outside),"reset departed Character");
            if(parent_==4){stack_.setCurrentWidget(realm_);button(realm_,"realmViewerAuxiliary2")->setFocus();}
            else {stack_.setCurrentWidget(entry_);button(entry_,"regionEntryAuxiliary2")->setFocus();}
        };
        session_.failed=[this](const auto& reason){failure_=reason;};
        QObject::connect(realm_,&RealmViewerWidget::auxiliaryRequested,&stack_,[this](auto action){
            if(action==RealmViewerWidget::AuxiliaryAction::Character)openCharacter(4);
        });
        QObject::connect(realm_,&RealmViewerWidget::regionRequested,&stack_,[this](const auto& request){
            const auto model=realm_->campaign();const int r=int(request.realm);
            for(const auto& source:model.regions[r])if(source.id==request.regionId){
                RegionEntryWidget::Region target;target.id=source.id;target.name=source.name;
                target.artworkRealm=RegionEntryWidget::Realm(r);target.artworkNumber=source.artworkNumber;
                target.enterAvailable=false;target.auxiliaryAvailable=source.auxiliaryAvailable;
                target.difficulty=difficulties_.value(source.id,RegionEntryWidget::Difficulty::Initiate);
                check(entry_->setRegion(target),"fixture Region projection");textControls(entry_);stack_.setCurrentWidget(entry_);entry_->focusFirstControl();return;
            }
            check(false,"unmapped fixture Region");
        });
        QObject::connect(entry_,&RegionEntryWidget::auxiliaryRequested,&stack_,[this](auto action){
            if(action==RegionEntryWidget::AuxiliaryAction::Character)openCharacter(18);
        });
        QObject::connect(entry_,&RegionEntryWidget::difficultyChanged,&stack_,[this](auto difficulty){
            difficulties_[entry_->region().id]=difficulty;
        });
        QObject::connect(entry_,&RegionEntryWidget::cancelled,&stack_,[this]{
            stack_.setCurrentWidget(realm_);realm_->focusFirstControl();
        });
        QObject::connect(realm_,&RealmViewerWidget::cancelled,&stack_,[this]{stack_.close();});
        stack_.setCurrentWidget(realm_);stack_.show();QApplication::processEvents();realm_->focusFirstControl();
    }
    static QPushButton* button(QWidget* widget,const QString& name){
        auto* result=widget->findChild<QPushButton*>(name);check(result,"fixture button");return result;
    }
    static void textControls(QWidget* widget){
        // Placeholder SPR frames are single pixels. Use the widgets' text
        // fallback in this development fixture so each action stays readable.
        for(auto* b:widget->findChildren<QPushButton*>())
            if(auto* sprite=dynamic_cast<mnm::ui::SpriteButton*>(b))sprite->clearSprites();
    }
    void openCharacter(quint32 parent){parent_=parent;peer_.begin(parent);LiveMenuSessionTestAccess::poll(session_);}
    void step(){peer_.acknowledge();LiveMenuSessionTestAccess::poll(session_);check(failure_.isEmpty(),"fixture session fallback");QApplication::processEvents();}
    void interactive(){
        timer_.setInterval(150);QObject::connect(&timer_,&QTimer::timeout,&stack_,[this]{step();});timer_.start();
    }
    void verify(){
        auto click=[&](QWidget* widget,const QString& name){button(widget,name)->click();QApplication::processEvents();};
        click(realm_,"realmViewerRealm1");click(realm_,"realmViewerRegion1");
        const auto selected=realm_->campaign().selectedRegionIds;
        click(realm_,"realmViewerAuxiliary2");check(stack_.currentWidget()==character_,"Realm to actual Character controller");
        click(character_,"characterAdjustment2");check(!character_->isEnabled()&&character_->draftRequest().values[1]==160,"Realm purchase waits for engine peer");
        step();check(character_->draftRequest().values[1]==180,"Realm purchase applied");
        click(character_,"characterCancel");step();
        check(stack_.currentWidget()==realm_&&realm_->campaign().selectedRegionIds==selected&&realm_->campaign().realm==RealmViewerWidget::Realm::Greek&&button(realm_,"realmViewerAuxiliary2")->hasFocus(),"Cancel restores caller,realm,selection and focus");
        click(realm_,"realmViewerAuxiliary2");check(character_->draftRequest().values[1]==160,"Realm Cancel reopen rollback");
        click(character_,"characterAdjustment2");step();click(character_,"characterOk");step();
        check(stack_.currentWidget()==realm_,"Realm OK return");click(realm_,"realmViewerAuxiliary2");
        check(character_->draftRequest().values[1]==180&&character_->draftRequest().remainingExperience==39,"Realm OK reopen baseline");
        click(character_,"characterCancel");step();click(realm_,"realmViewerEnter");
        check(stack_.currentWidget()==entry_&&entry_->region().id=="fixture-1-1"&&!button(entry_,"regionEntryEnter")->isEnabled(),"selected Region entry without gameplay");
        entry_->findChild<QRadioButton*>("regionEntryDifficulty3")->click();
        click(entry_,"regionEntryAuxiliary2");check(stack_.currentWidget()==character_&&peer_.word(MNM_MENU_V18_CHARACTER)==18,"Region caller wire identity");
        click(character_,"characterAdjustment4");step();click(character_,"characterCancel");step();
        check(stack_.currentWidget()==entry_&&entry_->region().difficulty==RegionEntryWidget::Difficulty::Wizard&&button(entry_,"regionEntryAuxiliary2")->hasFocus(),"Region Cancel restores caller,difficulty and focus");
        click(entry_,"regionEntryAuxiliary2");check(character_->draftRequest().values[2]==4,"Region Cancel rollback");
        click(character_,"characterAdjustment4");step();click(character_,"characterOk");step();
        click(entry_,"regionEntryAuxiliary2");check(character_->draftRequest().values[2]==5&&character_->draftRequest().values[1]==180&&character_->draftRequest().remainingExperience==28,"cross-caller committed baseline");
        QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);QApplication::sendEvent(character_,&escape);step();
        click(entry_,"regionEntryCancel");check(stack_.currentWidget()==realm_&&realm_->campaign().selectedRegionIds==selected&&button(realm_,"realmViewerRegion1")->hasFocus(),"Region returns selected Realm flag");
        click(realm_,"realmViewerEnter");check(entry_->region().difficulty==RegionEntryWidget::Difficulty::Wizard,"Region difficulty survives auxiliary and reopen");click(entry_,"regionEntryCancel");
        auto locked=realm_->campaign();locked.auxiliaryAvailable[2]=false;check(realm_->setCampaign(locked),"locked snapshot");
        const auto sequence=peer_.word(28);click(realm_,"realmViewerAuxiliary2");
        check(stack_.currentWidget()==realm_&&!button(realm_,"realmViewerAuxiliary2")->isEnabled()&&peer_.word(28)==sequence,"unavailable Character cannot submit");
        check(returns_==6,"six acknowledged Character parent returns");
    }
    bool capture(const QString& path){return stack_.grab().save(path);}
private:
    QTemporaryDir assets_;
    CharacterWireFixture peer_;
    LiveMenuSession session_;
    QStackedWidget stack_;
    RealmViewerWidget* realm_=nullptr;
    RegionEntryWidget* entry_=nullptr;
    CharacterScreenWidget* character_=nullptr;
    std::unique_ptr<LiveCharacterController> controller_;
    QMap<QString,RegionEntryWidget::Difficulty> difficulties_;
    QTimer timer_;
    QString failure_;
    quint32 parent_=4;
    int returns_=0;
};
int main(int argc,char** argv){
    QApplication app(argc,argv);
    try{
        CampaignFixture fixture;
        if(app.arguments().contains("--interactive")){fixture.interactive();return app.exec();}
        fixture.verify();
        const auto args=app.arguments();const int at=args.indexOf("--capture");
        if(at>=0){check(at+1<args.size(),"capture path");check(fixture.capture(args[at+1]),"fixture screenshot");}
        QJsonObject report{{"success",true},{"character_parent_returns",6},{"callers",2},
            {"synthetic_campaign",true},{"live_validated",false},{"original_callbacks_executed",false}};
        std::puts(QJsonDocument(report).toJson(QJsonDocument::Compact).constData());return 0;
    }catch(const std::exception& error){std::fprintf(stderr,"Campaign fixture: %s\n",error.what());return 1;}
}
