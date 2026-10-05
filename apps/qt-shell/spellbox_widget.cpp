#include "spellbox_widget.hpp"
#include "menu_assets.hpp"
#include <QApplication>
#include <QDrag>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QMimeData>
#include <QMouseEvent>
#include <QPainter>
#include <QSet>
#include <QCursor>
#include <stdexcept>
namespace {
constexpr auto mime="application/x-mnm-spellbox-cell";
bool single(const QString& text,int limit,bool empty=false) {
    if((text.isEmpty()&&!empty)||text.size()>limit)return false;
    for(const auto c:text)if(c.unicode()<32||c.unicode()==127)return false;
    return true;
}
bool fail(QString* error,const QString& text){if(error)*error=text;return false;}
}
// Private cells retain QPushButton keyboard/accessibility behavior. The original
// artwork is a single frame, not a presumed normal/hover/pressed triplet.
class SpellboxCell final : public QPushButton {
public:
    SpellboxCell(SpellboxWidget* owner,bool item,int index):QPushButton(owner),owner_(owner),item_(item),index_(index){setAcceptDrops(true);setMouseTracking(true);setCursor(Qt::PointingHandCursor);installEventFilter(owner);}
    mnm::ui::MenuSpriteFrame frame;
    bool selected=false;
    int count=-1;
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);p.setRenderHint(QPainter::SmoothPixmapTransform);p.scale(width()/84.0,height()/86.0);p.setFont(mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Tooltip));
        const int shelfCount=item_?qMax(0,owner_->available(index_)-((lifted_||owner_->carriedItem_==index_)?1:0)):count;
        if(!isEnabled())p.setOpacity(.45);
        if(shelfCount==0){} // Hide artwork only when no copy remains on the shelf.
        else if(!frame.image.isNull())p.drawImage(-frame.origin,frame.image);
        else {p.setPen(QColor("#fff5d6"));p.drawText(QRect(2,4,80,77),Qt::AlignCenter|Qt::TextWordWrap,text());}
        p.setOpacity(1);if(selected||hasFocus()){p.setPen(QPen(selected?QColor("#ffdd88"):QColor("#fff5d6"),2));p.drawRoundedRect(QRect(1,1,81,83),6,6);}
        if(shelfCount>=0){p.setFont(mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Yellow));p.fillRect(QRect(59,65,24,20),QColor(20,12,8,210));p.setPen(Qt::white);p.drawText(QRect(59,65,24,20),Qt::AlignCenter,QString::number(shelfCount));}
    }
    void mousePressEvent(QMouseEvent* e) override {
        start_=e->position().toPoint();
        lifted_=item_&&e->button()==Qt::LeftButton&&owner_->available(index_)>0;
        pickedUp_=false;
        if(!item_&&e->button()==Qt::LeftButton&&owner_->carriedItem_<0){
            const int item=owner_->itemIndex(owner_->draft_.talismans[index_].itemId);
            if(item>=0){owner_->carry(item,index_);pickedUp_=true;owner_->pointerAt(owner_->mapFromGlobal(e->globalPosition().toPoint()));}
        }
        update();QPushButton::mousePressEvent(e);
    }
    void mouseReleaseEvent(QMouseEvent* e) override {
        if(pickedUp_&&e->button()==Qt::LeftButton){
            const QPoint point=owner_->mapFromGlobal(e->globalPosition().toPoint());
            bool placed=false;
            for(int t=0;t<owner_->talismans_.size();++t)if(t!=index_&&owner_->talismans_[t]->geometry().contains(point)){placed=owner_->placeCarried(t);break;}
            if(!placed)owner_->putDown();
            pickedUp_=false;setDown(false);
        }
        lifted_=false;update();QPushButton::mouseReleaseEvent(e);
    }
    void hideEvent(QHideEvent* e) override {lifted_=false;pickedUp_=false;QPushButton::hideEvent(e);}
    void mouseMoveEvent(QMouseEvent* e) override {
        if(!(e->buttons()&Qt::LeftButton)||(e->position().toPoint()-start_).manhattanLength()<QApplication::startDragDistance()){QPushButton::mouseMoveEvent(e);return;}
        const int item=pickedUp_?owner_->carriedItem_:item_?index_:owner_->itemIndex(owner_->draft_.talismans[index_].itemId);
        if(item<0 || (item_ && owner_->available(item)<1))return;
        if(!pickedUp_)owner_->carry(item,item_?-1:index_);
        const auto token=owner_->revision_;auto* data=new QMimeData;data->setData(mime,QByteArray::number(token));QDrag drag(this);drag.setMimeData(data);
        if(owner_->carriedArt_->pixmap().isNull()==false){drag.setPixmap(owner_->carriedArt_->pixmap());drag.setHotSpot({drag.pixmap().width()/2,drag.pixmap().height()/2});}
        owner_->carriedArt_->hide();setDown(false);drag.exec(Qt::MoveAction);
        lifted_=false;pickedUp_=false;update();owner_->putDown();
    }
    void dragEnterEvent(QDragEnterEvent* e) override {
        auto* source=dynamic_cast<SpellboxCell*>(e->source());
        if(source&&source->owner_==owner_&&e->mimeData()->data(mime)==QByteArray::number(owner_->revision_)){owner_->pointerAt(owner_->mapFromGlobal(mapToGlobal(e->position().toPoint())));e->acceptProposedAction();}
    }
    void dragMoveEvent(QDragMoveEvent* e) override {
        auto* source=dynamic_cast<SpellboxCell*>(e->source());
        if(source&&source->owner_==owner_&&e->mimeData()->data(mime)==QByteArray::number(owner_->revision_)){owner_->pointerAt(owner_->mapFromGlobal(mapToGlobal(e->position().toPoint())));owner_->carriedArt_->hide();e->acceptProposedAction();}
    }
    void dragLeaveEvent(QDragLeaveEvent* e) override {owner_->hoveredTalisman_=-1;owner_->populate();e->accept();}
    void dropEvent(QDropEvent* e) override {
        auto* source=dynamic_cast<SpellboxCell*>(e->source());
        if(source&&source->owner_==owner_&&e->mimeData()->data(mime)==QByteArray::number(owner_->revision_)){if(owner_->dropCell(item_,index_,source))e->acceptProposedAction();}
    }
private:
    friend class SpellboxWidget;
    SpellboxWidget* owner_;bool item_,lifted_=false,pickedUp_=false;int index_;QPoint start_;
};
SpellboxWidget::SpellboxWidget(QWidget* parent):QWidget(parent) {
    qApp->installEventFilter(this);setMouseTracking(true);setAcceptDrops(true);
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Portmanteau preview");
    const std::array<QString,5> names{"Assign","Remove","Preview","OK","Cancel"};
    for(int i=0;i<5;++i){actions_[i]=new QPushButton(names[i],this);actions_[i]->setObjectName("spellbox"+names[i]);actions_[i]->setStyleSheet("QPushButton {color:#fff5d6;background:rgba(20,12,8,175);border:1px solid #ac915a;} QPushButton:hover,QPushButton:focus {background:#51402a;} QPushButton:disabled {color:#888;}");actions_[i]->installEventFilter(this);}
    connect(actions_[0],&QPushButton::clicked,this,[this]{if(selectedItem_>=0&&selectedTalisman_>=0)assignItem(draft_.items[selectedItem_].id,draft_.talismans[selectedTalisman_].id);});
    connect(actions_[1],&QPushButton::clicked,this,[this]{if(selectedTalisman_>=0)removeItem(draft_.talismans[selectedTalisman_].id);});
    connect(actions_[2],&QPushButton::clicked,this,&SpellboxWidget::preview);connect(actions_[3],&QPushButton::clicked,this,&SpellboxWidget::accept);connect(actions_[4],&QPushButton::clicked,this,&SpellboxWidget::cancel);
    detail_=new QLabel(this);detail_->setObjectName("spellboxDetail");detail_->setTextFormat(Qt::PlainText);detail_->setWordWrap(true);detail_->setAlignment(Qt::AlignCenter);detail_->setStyleSheet("color:#fff5d6;background:rgba(20,12,8,175);");
    carriedArt_=new QLabel(this);carriedArt_->setObjectName("spellboxCarriedArtwork");carriedArt_->setAttribute(Qt::WA_TransparentForMouseEvents);carriedArt_->hide();
    for(int i=0;i<4;++i){recipeArt_[i]=new QLabel(this);recipeArt_[i]->setObjectName(QString("spellboxRecipe%1").arg(i));recipeArt_[i]->setAlignment(Qt::AlignCenter);recipeArt_[i]->setWordWrap(true);recipeArt_[i]->setStyleSheet("color:#fff5d6;background:transparent;");recipeArt_[i]->setMouseTracking(true);}
    detail_->hide();populate();
}
bool SpellboxWidget::valid(const Inventory& inv) {
    if(!single(inv.ownerId,128)||inv.items.size()>30||inv.talismans.size()>21)return false;
    QSet<QString> ids;std::array<int,3> counts{};QMap<QString,int> assigned;
    for(const auto& item:inv.items){
        if(!single(item.id,128)||!single(item.name,128)||ids.contains(item.id)||item.artworkIndex< -1||item.artworkIndex>22||item.quantity<1||item.quantity>999)return false;
        ids.insert(item.id);
        for(const auto& spell:item.spells)if(!single(spell.id,128,true)||!single(spell.name,128,true)||spell.id.isEmpty()!=spell.name.isEmpty()||spell.artworkIndex< -1||spell.artworkIndex>94||(spell.id.isEmpty()&&spell.artworkIndex!=-1))return false;
    }
    ids.clear();
    for(const auto& slot:inv.talismans){
        const int a=int(slot.alignment);if(!single(slot.id,128)||ids.contains(slot.id)||a<0||a>2||++counts[a]>7)return false;ids.insert(slot.id);
        if(!slot.itemId.isEmpty()){bool found=false;for(const auto& item:inv.items)if(item.id==slot.itemId){found=true;if(item.spells[a].id.isEmpty()||++assigned[item.id]>item.quantity)return false;}if(!found)return false;}
    }
    return true;
}
bool SpellboxWidget::setInventory(const Inventory& inv,QString* error) {
    if(!valid(inv))return fail(error,"Invalid supplied Spellbox inventory");
    putDown();accepted_=inv;draft_=inv;selectedItem_=inv.items.isEmpty()?-1:0;selectedTalisman_=inv.talismans.isEmpty()?-1:0;++revision_;populate();if(error)error->clear();return true;
}
bool SpellboxWidget::loadAssets(const QString& root,QString* error) {
    try {
        const auto fonts=mnm::ui::loadMenuFonts(root);
        auto background=mnm::ui::loadMenuImage(root,"Interface/SpellBox/800x600/Portmanteau.bmp",{800,600});
        auto items=mnm::ui::loadMenuSprites(root,"Interface/SpellBox/800x600/mitems.spr"),talismans=mnm::ui::loadMenuSprites(root,"Interface/SpellBox/800x600/Talisman.spr");
        if(items.size()!=23||talismans.size()!=95)throw std::runtime_error("Unexpected Spellbox sprite frame counts");
        for(const auto& f:items)if(f.image.isNull())throw std::runtime_error("Empty Spellbox item frame");
        for(const auto& f:talismans)if(f.image.isNull())throw std::runtime_error("Empty Spellbox talisman frame");
        const auto tips=mnm::ui::loadMenuLayout(root,"Interface/SpellBox","Spellboxtooltip.cfg");
        if(tips.value("HEADER").value("ValidConfig")!="TRUE")throw std::runtime_error("Invalid Spellbox tooltip configuration");
        std::array<QString,3> labels;for(int i=0;i<3;++i)labels[i]=mnm::ui::textLabel(tips,QString::number(4+i));
        mnm::ui::installMenuFonts(this,fonts);
        background_=std::move(background);itemSprites_=std::move(items);talismanSprites_=std::move(talismans);alignmentNames_=labels;populate();if(error)error->clear();return true;
    }catch(const std::exception& e){return fail(error,QString::fromUtf8(e.what()));}
}
int SpellboxWidget::itemIndex(const QString& id) const {for(int i=0;i<draft_.items.size();++i)if(draft_.items[i].id==id)return i;return -1;}
int SpellboxWidget::talismanIndex(const QString& id) const {for(int i=0;i<draft_.talismans.size();++i)if(draft_.talismans[i].id==id)return i;return -1;}
int SpellboxWidget::available(int index) const {int result=draft_.items[index].quantity;for(const auto& slot:draft_.talismans)if(slot.itemId==draft_.items[index].id)--result;return result;}
bool SpellboxWidget::assignItem(const QString& itemId,const QString& talismanId,QString* error) {
    const int i=itemIndex(itemId),t=talismanIndex(talismanId);
    if(i<0||t<0)return fail(error,"Unknown supplied item or talisman ID");
    const auto& item=draft_.items[i];auto& slot=draft_.talismans[t];
    if(item.spells[int(slot.alignment)].id.isEmpty())return fail(error,"No supplied spell for this alignment");
    if(slot.itemId!=itemId&&available(i)<1)return fail(error,"No unassigned copies of this item");
    slot.itemId=itemId;selectedItem_=i;selectedTalisman_=t;++revision_;carriedItem_=-1;carriedTalisman_=-1;hoveredTalisman_=-1;carriedArt_->hide();populate();if(error)error->clear();return true;
}
bool SpellboxWidget::removeItem(const QString& id,QString* error) {
    const int t=talismanIndex(id);if(t<0)return fail(error,"Unknown supplied talisman ID");
    draft_.talismans[t].itemId.clear();selectedTalisman_=t;++revision_;carriedItem_=-1;carriedTalisman_=-1;hoveredTalisman_=-1;carriedArt_->hide();populate();if(error)error->clear();return true;
}
SpellboxWidget::Request SpellboxWidget::draftRequest() const {
    Request result;result.ownerId=draft_.ownerId;for(const auto& slot:draft_.talismans){Assignment a;a.talismanId=slot.id;a.alignment=slot.alignment;a.itemId=slot.itemId;const int i=itemIndex(slot.itemId);if(i>=0)a.spellId=draft_.items[i].spells[int(slot.alignment)].id;result.assignments.push_back(a);}return result;
}
void SpellboxWidget::preview() {
    if(selectedTalisman_<0)return;
    const auto& slot=draft_.talismans[selectedTalisman_];int i=selectedItem_;if(i<0)i=itemIndex(slot.itemId);
    if(i<0)return;
    const auto& spell=draft_.items[i].spells[int(slot.alignment)];if(spell.id.isEmpty())return;
    emit spellPreviewRequested(Preview{draft_.ownerId,slot.id,draft_.items[i].id,spell.id,slot.alignment});
}
void SpellboxWidget::accept(){putDown();if(draft_.ownerId.isEmpty())return;accepted_=draft_;emit loadoutAccepted(draftRequest());}
void SpellboxWidget::cancel(){putDown();draft_=accepted_;++revision_;populate();emit cancelled();}
void SpellboxWidget::carry(int item,int sourceTalisman){
    if(item<0 || item>=draft_.items.size() || (sourceTalisman<0 && available(item)<1))return;
    if(sourceTalisman>=0){
        if(sourceTalisman>=draft_.talismans.size()||draft_.talismans[sourceTalisman].itemId!=draft_.items[item].id)return;
        draft_.talismans[sourceTalisman].itemId.clear();++revision_;
    }
    carriedItem_=item;carriedTalisman_=sourceTalisman;selectedItem_=item;hoveredTalisman_=-1;
    populate();pointerAt(mapFromGlobal(QCursor::pos()));
}
void SpellboxWidget::putDown(){carriedItem_=-1;carriedTalisman_=-1;hoveredTalisman_=-1;if(carriedArt_)carriedArt_->hide();populate();}
void SpellboxWidget::pointerAt(const QPoint& point){
    if(carriedItem_<0)return;
    int hovered=-1;for(int t=0;t<talismans_.size();++t)if(t!=carriedTalisman_&&talismans_[t]->geometry().contains(point)){hovered=t;break;}
    if(hoveredTalisman_!=hovered){hoveredTalisman_=hovered;populate();}
    const auto& item=draft_.items[carriedItem_];mnm::ui::MenuSpriteFrame frame;
    if(item.artworkIndex>=0&&!itemSprites_.isEmpty())frame=itemSprites_[item.artworkIndex];
    const int size=qMax(24,qRound(84*contentRect().width()/800.0));QPixmap image(size,size);image.fill(Qt::transparent);
    QPainter painter(&image);painter.scale(size/84.0,size/86.0);
    if(!frame.image.isNull())painter.drawImage(-frame.origin,frame.image);else {painter.setPen(Qt::white);painter.drawText(QRect(0,0,84,86),Qt::AlignCenter|Qt::TextWordWrap,item.name);}painter.end();
    carriedArt_->setPixmap(image);carriedArt_->setGeometry(point.x()-size/2,point.y()-size/2,size,size);carriedArt_->setVisible(rect().contains(point));carriedArt_->raise();
}
bool SpellboxWidget::placeCarried(int t){
    if(carriedItem_<0||t<0||t>=draft_.talismans.size())return false;
    const auto id=draft_.items[carriedItem_].id;const auto target=draft_.talismans[t].id;
    if(draft_.items[carriedItem_].spells[int(draft_.talismans[t].alignment)].id.isEmpty())return false;
    if(!assignItem(id,target))return false;
    putDown();return true;
}
void SpellboxWidget::returnCarried(){putDown();}
bool SpellboxWidget::dropCell(bool item,int index,SpellboxCell* source) {
    if(carriedItem_<0){const int i=source->item_?source->index_:itemIndex(draft_.talismans[source->index_].itemId);carry(i,source->item_?-1:source->index_);}
    if(carriedItem_<0)return false;
    if(item){returnCarried();return true;}
    return placeCarried(index);
}
void SpellboxWidget::populate() {
    auto sync=[this](QVector<SpellboxCell*>& cells,bool item,int count){while(cells.size()>count)delete cells.takeLast();while(cells.size()<count){const int i=cells.size();auto* cell=new SpellboxCell(this,item,i);cell->setObjectName(QString("spellbox%1%2").arg(item?"Item":"Talisman").arg(i));connect(cell,&QPushButton::clicked,this,[this,item,i]{if(item){selectedItem_=i;if(carriedItem_>=0){if(carriedTalisman_>=0)returnCarried();else if(carriedItem_==i)putDown();else carry(i);}else carry(i);}
            else {selectedTalisman_=i;if(carriedItem_>=0)placeCarried(i);else {const int assigned=itemIndex(draft_.talismans[i].itemId);if(assigned>=0)carry(assigned,i);}}populate();});cells.push_back(cell);cell->show();}};
    sync(items_,true,draft_.items.size());sync(talismans_,false,draft_.talismans.size());
    for(int i=0;i<items_.size();++i){const auto& item=draft_.items[i];auto* cell=items_[i];cell->setText(item.name);cell->setAccessibleName(item.name);cell->setToolTip(QString("%1 — %2 available of %3").arg(item.name).arg(available(i)).arg(item.quantity));cell->frame=item.artworkIndex>=0&&!itemSprites_.isEmpty()?itemSprites_[item.artworkIndex]:mnm::ui::MenuSpriteFrame{};cell->count=available(i)-(carriedItem_==i&&carriedTalisman_>=0?1:0);cell->selected=i==selectedItem_;cell->update();}
    for(int t=0;t<talismans_.size();++t){const auto& slot=draft_.talismans[t];auto* cell=talismans_[t];const int a=int(slot.alignment),i=t==hoveredTalisman_&&carriedItem_>=0&&!draft_.items[carriedItem_].spells[a].id.isEmpty()?carriedItem_:itemIndex(slot.itemId);const QString name=alignmentNames_[a]+(i>=0?" — "+draft_.items[i].spells[a].name:QString(" — empty"));cell->setText(name);cell->setAccessibleName(name);cell->setToolTip(name);const int art=i>=0?draft_.items[i].spells[a].artworkIndex:a;cell->frame=art>=0&&!talismanSprites_.isEmpty()?talismanSprites_[art]:mnm::ui::MenuSpriteFrame{};cell->selected=t==selectedTalisman_;cell->update();}
    const int detailTalisman=hoveredTalisman_>=0?hoveredTalisman_:selectedTalisman_;
    const bool slot=selectedTalisman_>=0,item=selectedItem_>=0;const int a=slot?int(draft_.talismans[selectedTalisman_].alignment):0;
    actions_[0]->setEnabled(slot&&item&&!draft_.items[selectedItem_].spells[a].id.isEmpty()&&(available(selectedItem_)>0||draft_.talismans[selectedTalisman_].itemId==draft_.items[selectedItem_].id));
    const int assigned=slot?itemIndex(draft_.talismans[selectedTalisman_].itemId):-1;
    actions_[1]->setEnabled(assigned>=0);const int previewItem=selectedItem_>=0?selectedItem_:assigned;actions_[2]->setEnabled(slot&&previewItem>=0&&!draft_.items[previewItem].spells[a].id.isEmpty());actions_[3]->setEnabled(!draft_.ownerId.isEmpty());
    QString detail="Select an item and a talisman; Assign prepares a local spell.";
    if(detailTalisman>=0&&previewItem>=0){const int detailAlignment=int(draft_.talismans[detailTalisman].alignment);const auto& source=draft_.items[previewItem];const auto& spell=source.spells[detailAlignment];detail=QString("%1 • %2\n%3").arg(source.name,alignmentNames_[detailAlignment],spell.id.isEmpty()?"No supplied spell":spell.name);}
    detail_->setText(detail);
    for(int r=0;r<4;++r){auto* label=recipeArt_[r];if(!label)continue;label->clear();QString text;mnm::ui::MenuSpriteFrame frame;
        if(selectedItem_>=0){const auto& source=draft_.items[selectedItem_];
            if(r==0){text=source.name;if(source.artworkIndex>=0&&!itemSprites_.isEmpty())frame=itemSprites_[source.artworkIndex];}
            else {const auto& spell=source.spells[r-1];text=alignmentNames_[r-1]+" — "+(spell.id.isEmpty()?QString("No supplied spell"):spell.name);if(spell.artworkIndex>=0&&!talismanSprites_.isEmpty())frame=talismanSprites_[spell.artworkIndex];}
        }
        label->setToolTip(text);label->setAccessibleName(text);
        if(!frame.image.isNull()){QPixmap image(84,86);image.fill(Qt::transparent);QPainter painter(&image);painter.drawImage(-frame.origin,frame.image);painter.end();label->setPixmap(image);label->setScaledContents(true);}else {label->setText(text);label->setScaledContents(false);}
    }
    arrange();update();
}
QRect SpellboxWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void SpellboxWidget::arrange() {
    const auto canvas=contentRect();const double scale=canvas.width()/800.0;auto place=[&](QWidget* w,QRect r){w->setGeometry(qRound(canvas.x()+r.x()*scale),qRound(canvas.y()+r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));auto f=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Tooltip,scale);w->setFont(f);};
    for(int i=0;i<5;++i){place(actions_[i],QRect(292+i*99,65,94,22));}
    place(detail_,QRect(292,41,490,39));
    for(int r=0;r<4;++r)if(recipeArt_[r])place(recipeArt_[r],QRect(305+r*118,5,56,57));
    std::array<int,3> rows{};for(int t=0;t<talismans_.size();++t){const int a=int(draft_.talismans[t].alignment);place(talismans_[t],QRect(198-a*94,7+84*rows[a]++,84,86));}
    for(int i=0;i<items_.size();++i)place(items_[i],QRect(290+(i%6)*83,93+(i/6)*102,84,86));
    QWidget* previous=nullptr;for(auto* c:talismans_){if(previous)QWidget::setTabOrder(previous,c);previous=c;}for(auto* c:items_){if(previous)QWidget::setTabOrder(previous,c);previous=c;}for(auto* b:actions_){if(previous)QWidget::setTabOrder(previous,b);previous=b;}
}
void SpellboxWidget::focusFirstControl(){if(!items_.isEmpty())items_.first()->setFocus(Qt::OtherFocusReason);else if(!talismans_.isEmpty())talismans_.first()->setFocus(Qt::OtherFocusReason);else actions_[4]->setFocus(Qt::OtherFocusReason);}
bool SpellboxWidget::eventFilter(QObject* watched,QEvent* event) {
    if(watched==this&&(event->type()==QEvent::DragEnter||event->type()==QEvent::DragMove||event->type()==QEvent::Drop)){
        auto* drop=static_cast<QDropEvent*>(event);auto* source=dynamic_cast<SpellboxCell*>(drop->source());
        const QPoint point=drop->position().toPoint();const auto canvas=contentRect();
        if(source&&source->owner_==this&&drop->mimeData()->data(mime)==QByteArray::number(revision_)&&carriedItem_>=0&&canvas.contains(point)&&point.x()>=canvas.x()+qRound(290*canvas.width()/800.0)){
            hoveredTalisman_=-1;populate();if(event->type()==QEvent::Drop)returnCarried();drop->acceptProposedAction();return true;
        }
    }
    if(watched==this&&event->type()==QEvent::Hide&&carriedItem_>=0)putDown();
    if(auto* widget=qobject_cast<QWidget*>(watched);widget&&isVisible()&&(widget==this||isAncestorOf(widget))){
        if(event->type()==QEvent::MouseMove&&carriedItem_>=0){auto* mouse=static_cast<QMouseEvent*>(event);pointerAt(mapFromGlobal(mouse->globalPosition().toPoint()));}
        if(event->type()==QEvent::MouseButtonRelease&&carriedItem_>=0){
            auto* mouse=static_cast<QMouseEvent*>(event);const QPoint point=mapFromGlobal(mouse->globalPosition().toPoint());const auto canvas=contentRect();
            if(mouse->button()==Qt::LeftButton&&canvas.contains(point)&&point.x()>=canvas.x()+qRound(290*canvas.width()/800.0))returnCarried();
        }
        if(event->type()==QEvent::MouseButtonPress&&carriedItem_>=0){auto* mouse=static_cast<QMouseEvent*>(event);if(mouse->button()==Qt::RightButton){putDown();return true;}
            const QPoint point=mapFromGlobal(mouse->globalPosition().toPoint());
            if(widget==this&&mouse->button()==Qt::LeftButton&&contentRect().contains(point)){const auto canvas=contentRect();if(point.x()>=canvas.x()+qRound(290*canvas.width()/800.0))returnCarried();else putDown();return true;}}
    }
    // The application filter also sees unrelated windows; only handle this screen.
    auto* target=qobject_cast<QWidget*>(watched);if(!target || (target!=this&&!isAncestorOf(target)))return false;
    if(event->type()==QEvent::KeyPress){auto* key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Escape){if(!key->isAutoRepeat()){if(carriedItem_>=0)putDown();else cancel();}return true;}if(key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter){if(!key->isAutoRepeat())if(auto* b=qobject_cast<QPushButton*>(watched))b->click();return true;}}
    return QWidget::eventFilter(watched,event);
}
void SpellboxWidget::keyPressEvent(QKeyEvent* key){if(key->key()==Qt::Key_Escape){if(!key->isAutoRepeat()){if(carriedItem_>=0)putDown();else cancel();}key->accept();}else QWidget::keyPressEvent(key);}
void SpellboxWidget::paintEvent(QPaintEvent*){QPainter p(this);p.fillRect(rect(),Qt::black);if(!background_.isNull())p.drawImage(contentRect(),background_);}
void SpellboxWidget::resizeEvent(QResizeEvent* e){QWidget::resizeEvent(e);arrange();}
