#include "realm_viewer_widget.hpp"
#include "menu_assets.hpp"
#include <QFontMetrics>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QTimer>
#include <QShowEvent>
#include <QHideEvent>
#include <QLabel>
#include <QPainter>
#include <QSet>
#include <stdexcept>
namespace {
bool line(const QString& text,int limit){if(text.trimmed().isEmpty()||text.size()>limit)return false;for(const auto c:text)if(c.unicode()<32||c.unicode()==127)return false;return true;}
bool failure(QString* error,const QString& text){if(error)*error=text;return false;}
std::array<mnm::ui::MenuSpriteFrame,3> pair(const mnm::ui::MenuSpriteSheet& frames,int first){if(first<0||first+1>=frames.size()||frames[first].image.isNull()||frames[first+1].image.isNull())throw std::runtime_error("Missing Realm Viewer button pair");return {frames[first],frames[first+1],frames[first+1]};}
}
RealmViewerWidget::RealmViewerWidget(QWidget* parent):QWidget(parent) {
    setMouseTracking(true);animationTimer_=new QTimer(this);animationTimer_->setObjectName("realmViewerAnimationTimer");animationTimer_->setInterval(40);connect(animationTimer_,&QTimer::timeout,this,&RealmViewerWidget::advanceAnimation);
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Realm Viewer preview");
    auto button=[this](const QString& object,const QString& text){auto* b=new mnm::ui::SpriteButton(text,this);b->setObjectName(object);b->installEventFilter(this);b->setCursor(Qt::PointingHandCursor);b->setStyleSheet("QPushButton {color:#fff5d6;background:rgba(20,20,20,175);border:1px solid #ac915a;} QPushButton:checked,QPushButton:hover,QPushButton:focus {background:#51402a;} QPushButton:disabled {color:#888;}");return b;};
    const std::array<QString,3> names{"Celtic","Greek","Medieval"};for(int r=0;r<3;++r){realms_[r]=button(QString("realmViewerRealm%1").arg(r),names[r]);realms_[r]->setCheckable(true);connect(realms_[r],&QPushButton::clicked,this,[this,r]{selectRealm(Realm(r));});}
    const std::array<QString,5> labels{"Spellbox","Grimoire","Character","Options","Spell Research"};for(int a=0;a<5;++a){auxiliary_[a]=button(QString("realmViewerAuxiliary%1").arg(a),labels[a]);auxiliary_[a]->setAccessibleName(labels[a]);connect(auxiliary_[a],&QPushButton::clicked,this,[this,a]{emit auxiliaryRequested(AuxiliaryAction(a));});}
    enter_=button("realmViewerEnter","Open Region");cancel_=button("realmViewerCancel","Cancel");connect(enter_,&QPushButton::clicked,this,&RealmViewerWidget::enter);connect(cancel_,&QPushButton::clicked,this,&RealmViewerWidget::cancelled);
    heading_=new QLabel(this);heading_->setObjectName("realmViewerHeading");heading_->setTextFormat(Qt::PlainText);heading_->setAlignment(Qt::AlignCenter);heading_->setStyleSheet("color:#fff5d6;background:rgba(20,20,20,175);");populate();
}
bool RealmViewerWidget::valid(const Campaign& c) {
    const int active=int(c.realm);if(active<0||active>2||!c.realmAvailable[active]||!line(c.id,128)||!line(c.name,256))return false;
    QSet<QString> ids;const std::array<int,3> maximum{8,12,16};
    for(int r=0;r<3;++r){if(c.regions[r].size()>maximum[r])return false;QSet<int> art;bool selection=c.selectedRegionIds[r].isEmpty();for(const auto& region:c.regions[r]){
        if(!line(region.id,128)||!line(region.name,256)||ids.contains(region.id)||region.artworkNumber<1||region.artworkNumber>maximum[r]||art.contains(region.artworkNumber)||region.flagArtworkIndex< -1||region.flagArtworkIndex>88||region.flagPosition.x()<0||region.flagPosition.x()>752||region.flagPosition.y()<0||region.flagPosition.y()>548)return false;
        ids.insert(region.id);art.insert(region.artworkNumber);if(region.id==c.selectedRegionIds[r])selection=true;
    }if(!selection)return false;}
    return true;
}
bool RealmViewerWidget::setCampaign(const Campaign& campaign,QString* error) {
    if(!valid(campaign))return failure(error,"Invalid supplied Realm Viewer campaign");
    hoveredRegion_.clear();setToolTip({});unsetCursor();
    campaign_=campaign;for(int r=0;r<3;++r)if(campaign_.selectedRegionIds[r].isEmpty()&&!campaign_.regions[r].isEmpty())campaign_.selectedRegionIds[r]=campaign_.regions[r].first().id;
    populate();if(error)error->clear();return true;
}
bool RealmViewerWidget::loadAssets(const QString& root,QString* error) {
    try {
        const auto fonts=mnm::ui::loadMenuFonts(root);
        const std::array<QString,3> names{"Celtic","Greek","Medieval"};std::array<QImage,3> maps;
        for(int r=0;r<3;++r)maps[r]=mnm::ui::loadMenuImage(root,"Interface/RealmViewer/800x600/"+names[r]+"_Map.bmp",{800,600});
        auto flags=mnm::ui::loadMenuSprites(root,"Interface/RealmViewer/Generic/flags.spr");auto buttons=mnm::ui::loadMenuSprites(root,"Interface/RealmViewer/Generic/RlmBtn.spr");
        if(flags.size()!=89||buttons.size()!=12)throw std::runtime_error("Unexpected Realm Viewer sprite frame counts");
        for(const auto& frame:flags)if(frame.image.isNull())throw std::runtime_error("Empty Realm Viewer flag frame");
        auto visuals=mnm::ui::loadRealmViewerVisuals(root,flags);
        const auto close=pair(buttons,0),open=pair(buttons,2),spellbox=pair(buttons,4),grimoire=pair(buttons,8),research=pair(buttons,6);
        const auto shared=mnm::ui::loadMenuSprites(root,"Sprites/Buttons.spr");const auto character=mnm::ui::spriteStates(shared,0);
        const auto tips=mnm::ui::loadMenuLayout(root,"Interface/RealmViewer","realmviewtooltip.cfg");if(tips.value("HEADER").value("ValidConfig")!="TRUE")throw std::runtime_error("Invalid Realm Viewer tooltip configuration");
        std::array<QString,5> labels;labels[4]="Spell Research";for(int i=0;i<4;++i)labels[i]=mnm::ui::textLabel(tips,QString::number(i));
        mnm::ui::installMenuFonts(this,fonts);
        maps_=std::move(maps);flags_=std::move(flags);visuals_=std::move(visuals);animationTick_=0;hoveredRegion_.clear();setToolTip({});unsetCursor();cancel_->setSprites(close,{94,39});enter_->setSprites(open,{94,39});enter_->setAccessibleName("Open selected region");
        static_cast<mnm::ui::SpriteButton*>(auxiliary_[0])->setSprites(spellbox,{94,39});static_cast<mnm::ui::SpriteButton*>(auxiliary_[1])->setSprites(grimoire,{94,39});static_cast<mnm::ui::SpriteButton*>(auxiliary_[2])->setSprites(character,{60,60});
        static_cast<mnm::ui::SpriteButton*>(auxiliary_[4])->setSprites(research,{94,39});
        for(int i=0;i<5;++i){auxiliary_[i]->setToolTip(labels[i]);auxiliary_[i]->setAccessibleName(labels[i]);}populate();if(error)error->clear();return true;
    }catch(const std::exception& e){return failure(error,QString::fromUtf8(e.what()));}
}
const RealmViewerWidget::Region* RealmViewerWidget::selected() const {const int r=int(campaign_.realm);for(const auto& region:campaign_.regions[r])if(region.id==campaign_.selectedRegionIds[r])return &region;return nullptr;}
bool RealmViewerWidget::selectRegion(const QString& id,QString* error) {
    for(const auto& region:campaign_.regions[int(campaign_.realm)])if(region.id==id){const bool changed=campaign_.selectedRegionIds[int(campaign_.realm)]!=id;campaign_.selectedRegionIds[int(campaign_.realm)]=id;populate();if(error)error->clear();if(changed)emit selectionChanged(id);return true;}
    return failure(error,"Unknown region in selected realm");
}
bool RealmViewerWidget::selectRealm(Realm realm,QString* error) {
    const int r=int(realm);if(r<0||r>2||!campaign_.realmAvailable[r])return failure(error,"Unavailable supplied realm");
    const bool changed=realm!=campaign_.realm;campaign_.realm=realm;hoveredRegion_.clear();setToolTip({});unsetCursor();populate();if(error)error->clear();if(changed)emit realmChanged(realm);return true;
}
void RealmViewerWidget::enter(){const auto* region=selected();if(region&&region->available)emit regionRequested(Request{campaign_.id,region->id,campaign_.realm,region->artworkNumber});}
void RealmViewerWidget::populate() {
    const auto& model=campaign_.regions[int(campaign_.realm)];while(regions_.size()>model.size())delete regions_.takeLast();while(regions_.size()<model.size()){
        const int i=regions_.size();auto* b=new mnm::ui::SpriteButton({},this);b->setObjectName(QString("realmViewerRegion%1").arg(i));b->installEventFilter(this);b->setCursor(Qt::PointingHandCursor);b->setStyleSheet(mnm::ui::menuButtonStyle());connect(b,&QPushButton::clicked,this,[this,i]{selectRegion(campaign_.regions[int(campaign_.realm)][i].id);});regions_.push_back(b);b->show();
    }
    for(int i=0;i<regions_.size();++i){const auto& region=model[i];auto* b=regions_[i];b->setText(region.name);b->setAccessibleName(region.name+(region.available?"":" — unavailable"));b->setToolTip(b->accessibleName());}
    for(int r=0;r<3;++r){realms_[r]->setEnabled(campaign_.realmAvailable[r]);realms_[r]->setChecked(r==int(campaign_.realm));}
    const auto* region=selected();enter_->setEnabled(region&&region->available);heading_->setToolTip(region?region->name:campaign_.name);heading_->setAccessibleName(region?region->name:campaign_.name);
    for(int a=0;a<5;++a){auxiliary_[a]->setEnabled(!campaign_.id.isEmpty()&&campaign_.auxiliaryAvailable[a]);}
    refreshFlags();arrange();update();
}
QRect RealmViewerWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void RealmViewerWidget::arrange() {
    const auto canvas=contentRect();const double s=canvas.width()/800.0;auto place=[&](QWidget* widget,const QRect& r){widget->setGeometry(qRound(canvas.x()+r.x()*s),qRound(canvas.y()+r.y()*s),qRound(r.width()*s),qRound(r.height()*s));auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Tooltip,s);widget->setFont(font);};
    for(int r=0;r<3;++r){place(realms_[r],{530+90*r,10,84,39});}
    place(heading_,{665,550,125,39});const auto* region=selected();heading_->setText(QFontMetrics(heading_->font()).elidedText(region?region->name:campaign_.name,Qt::ElideRight,qMax(1,heading_->width()-12)));
    const auto& model=campaign_.regions[int(campaign_.realm)];for(int i=0;i<regions_.size();++i)place(regions_[i],QRect(model[i].flagPosition,QSize(48,52)));
    place(cancel_,{10,550,94,39});place(enter_,{110,550,94,39});place(auxiliary_[0],{210,550,94,39});place(auxiliary_[4],{310,550,94,39});place(auxiliary_[1],{410,550,94,39});place(auxiliary_[2],{510,530,60,60});place(auxiliary_[3],{575,550,84,39});
    QWidget* previous=nullptr;for(auto* b:realms_){if(previous)QWidget::setTabOrder(previous,b);previous=b;}for(auto* b:regions_){QWidget::setTabOrder(previous,b);previous=b;}QWidget::setTabOrder(previous,enter_);previous=enter_;for(auto* b:auxiliary_){QWidget::setTabOrder(previous,b);previous=b;}QWidget::setTabOrder(previous,cancel_);
}
void RealmViewerWidget::focusFirstControl(){const auto& model=campaign_.regions[int(campaign_.realm)];for(int i=0;i<model.size();++i)if(model[i].id==campaign_.selectedRegionIds[int(campaign_.realm)]){regions_[i]->setFocus(Qt::OtherFocusReason);return;}cancel_->setFocus(Qt::OtherFocusReason);}
bool RealmViewerWidget::eventFilter(QObject* watched,QEvent* event) {
    auto* b=qobject_cast<QPushButton*>(watched);const int index=regions_.indexOf(static_cast<mnm::ui::SpriteButton*>(b));
    if(event->type()==QEvent::MouseButtonDblClick&&index>=0){selectRegion(campaign_.regions[int(campaign_.realm)][index].id);enter();return true;}
    if(event->type()==QEvent::KeyPress){auto* key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Escape){if(!key->isAutoRepeat())emit cancelled();return true;}if(key->key()==Qt::Key_Enter||key->key()==Qt::Key_Return){if(!key->isAutoRepeat()){if(index>=0){selectRegion(campaign_.regions[int(campaign_.realm)][index].id);enter();}else if(b)b->click();}return true;}}
    return QWidget::eventFilter(watched,event);
}
void RealmViewerWidget::keyPressEvent(QKeyEvent* key){if(key->key()==Qt::Key_Escape){if(!key->isAutoRepeat())emit cancelled();key->accept();}else QWidget::keyPressEvent(key);}
QString RealmViewerWidget::regionAt(const QPoint& point) const {
    const auto canvas=contentRect();if(!canvas.contains(point)||canvas.width()<1||canvas.height()<1)return {};
    const QPoint map(int(qint64(point.x()-canvas.x())*800/canvas.width()),int(qint64(point.y()-canvas.y())*600/canvas.height()));
    const int realm=int(campaign_.realm);const auto& shapes=visuals_.regions[realm];
    // Stable artwork order resolves the small shared silhouette edges.
    for(int art=1;art<=shapes.size();++art)if(shapes[art-1].contains(map))for(const auto& region:campaign_.regions[realm])if(region.artworkNumber==art)return region.id;
    return {};
}
void RealmViewerWidget::refreshFlags(){
    const auto& model=campaign_.regions[int(campaign_.realm)];
    for(int i=0;i<regions_.size();++i){const auto& region=model[i];if(region.flagArtworkIndex>=0&&!flags_.isEmpty()){
        // Only the supplied green flag (frame zero) opts into sequence zero.
        // Other caller-selected sprite ordinals retain their explicit identity.
        auto f=region.flagArtworkIndex==0&&!visuals_.animations.isEmpty()?mnm::ui::realmFlagFrame(visuals_.animations[0],animationTick_):flags_[region.flagArtworkIndex];
        // ANI positions are relative to a foot anchor; the accessible flag
        // control has a top-left origin and a padded 48x52 logical canvas.
        if(region.flagArtworkIndex==0&&!visuals_.animations.isEmpty())f.origin-=QPoint(2,48);
        regions_[i]->setSprites({f,f,f},{48,52});
    }else regions_[i]->clearSprites();}
}
void RealmViewerWidget::advanceAnimation(){++animationTick_;refreshFlags();update();}
void RealmViewerWidget::mouseMoveEvent(QMouseEvent* e){
    const auto id=regionAt(e->position().toPoint());if(id!=hoveredRegion_){hoveredRegion_=id;QString tip;for(const auto& region:campaign_.regions[int(campaign_.realm)])if(region.id==id)tip=region.name+(region.available?"":" — unavailable");setToolTip(tip);setCursor(id.isEmpty()?Qt::ArrowCursor:Qt::PointingHandCursor);update();}QWidget::mouseMoveEvent(e);
}
void RealmViewerWidget::mousePressEvent(QMouseEvent* e){if(e->button()==Qt::LeftButton){const auto id=regionAt(e->position().toPoint());if(!id.isEmpty()){selectRegion(id);focusFirstControl();e->accept();return;}}QWidget::mousePressEvent(e);}
void RealmViewerWidget::mouseDoubleClickEvent(QMouseEvent* e){if(e->button()==Qt::LeftButton){const auto id=regionAt(e->position().toPoint());if(!id.isEmpty()){selectRegion(id);enter();e->accept();return;}}QWidget::mouseDoubleClickEvent(e);}
void RealmViewerWidget::leaveEvent(QEvent* e){hoveredRegion_.clear();setToolTip({});unsetCursor();update();QWidget::leaveEvent(e);}
void RealmViewerWidget::showEvent(QShowEvent* e){QWidget::showEvent(e);animationTimer_->start();}
void RealmViewerWidget::hideEvent(QHideEvent* e){animationTimer_->stop();QWidget::hideEvent(e);}
void RealmViewerWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);p.fillRect(rect(),Qt::black);const auto canvas=contentRect();const int realm=int(campaign_.realm);const auto& map=maps_[realm];if(!map.isNull())p.drawImage(canvas,map);
    p.save();p.setClipRect(canvas);p.translate(canvas.topLeft());p.scale(canvas.width()/800.0,canvas.height()/600.0);
    const auto& model=campaign_.regions[realm];const auto& shapes=visuals_.regions[realm];
    for(const auto& region:model)if(region.artworkNumber<=shapes.size()&&(region.id==campaign_.selectedRegionIds[realm]||region.id==hoveredRegion_)){const auto& shape=shapes[region.artworkNumber-1];p.drawImage(shape.borderRect.topLeft(),shape.border);}
    // Campaign travel/character position is not supplied by the preview model.
    // Selecting a region must not invent an endlessly walking character.
    p.restore();
    for(int i=0;i<model.size();++i){if(shapes.isEmpty()&&model[i].id==campaign_.selectedRegionIds[realm]){p.setPen(QPen(QColor("#ffdd88"),2));p.drawRect(regions_[i]->geometry().adjusted(-2,-2,2,2));}if(!model[i].available){p.setPen(Qt::white);p.drawLine(regions_[i]->geometry().topLeft(),regions_[i]->geometry().bottomRight());}}
}
void RealmViewerWidget::resizeEvent(QResizeEvent* event){QWidget::resizeEvent(event);arrange();}
