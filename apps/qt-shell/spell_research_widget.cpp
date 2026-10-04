#include "spell_research_widget.hpp"
#include <QKeyEvent>
#include <QFontMetrics>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QTextBrowser>
#include <QPainter>
#include <QSignalBlocker>
#include <QSet>
#include <stdexcept>
namespace {
bool line(const QString& value,int maximum){if(value.trimmed().isEmpty()||value.size()>maximum)return false;for(auto c:value)if(c.unicode()<32||c.unicode()==127)return false;return true;}
bool fail(QString* error,const QString& message){if(error)*error=message;return false;}
QRect largest(const mnm::ui::GrimoirePageArt& page){QRect result;for(const auto& area:page.areas)if(area.width()*area.height()>result.width()*result.height())result=area;return result;}
}
SpellResearchWidget::SpellResearchWidget(QWidget* parent):QWidget(parent){
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Spell Research preview");
    heading_=new QLabel("Spell Research",this);heading_->setObjectName("spellResearchHeading");title_=new QLabel(this);title_->setObjectName("spellResearchTitle");state_=new QLabel(this);state_->setObjectName("spellResearchState");
    for(auto* label:{heading_,title_,state_}){label->setTextFormat(Qt::PlainText);label->setStyleSheet("color:#3e2313;background:transparent;");label->setAlignment(Qt::AlignLeft|Qt::AlignVCenter);}
    filter_=new QLineEdit(this);filter_->setObjectName("spellResearchFilter");filter_->setPlaceholderText("Find a spell");filter_->setAccessibleName("Find a spell");filter_->setMaxLength(128);filter_->setStyleSheet("color:#3e2313;background:transparent;border:1px solid #ac915a;");
    spells_=new QListWidget(this);spells_->setObjectName("spellResearchSpells");spells_->setAccessibleName("Spells");spells_->setStyleSheet("QListWidget {color:#3e2313;background:transparent;border:0;} QListWidget::item:selected {color:#fff5d6;background:#51402a;}");
    description_=new QTextBrowser(this);description_->setObjectName("spellResearchDescription");description_->setAccessibleName("Spell description");description_->setOpenLinks(false);description_->setOpenExternalLinks(false);description_->setStyleSheet("QTextBrowser {color:#3e2313;background:transparent;border:0;}");
    const QString scrollStyle="QScrollBar:vertical {background:rgba(62,35,19,30);width:12px;margin:0;} QScrollBar::handle:vertical {background:#8a6b40;min-height:16px;border:1px solid #ac915a;} QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical {height:0;} QScrollBar::add-page:vertical,QScrollBar::sub-page:vertical {background:transparent;}";
    spells_->setStyleSheet(spells_->styleSheet()+scrollStyle);description_->setStyleSheet(description_->styleSheet()+scrollStyle);
    research_=new QPushButton("Research",this);research_->setObjectName("spellResearchRequest");research_->setStyleSheet(mnm::ui::menuButtonStyle());close_=new mnm::ui::SpriteButton("Close",this);close_->setObjectName("spellResearchClose");close_->setAccessibleName("Close Spell Research");close_->setStyleSheet(mnm::ui::menuButtonStyle());
    for(auto* w:QVector<QWidget*>{filter_,spells_,description_,research_,close_})w->installEventFilter(this);
    connect(filter_,&QLineEdit::textChanged,this,[this]{populate();});
    connect(spells_,&QListWidget::currentItemChanged,this,[this](QListWidgetItem* item){if(item)selectSpell(item->data(Qt::UserRole).toString());});
    connect(research_,&QPushButton::clicked,this,&SpellResearchWidget::research);connect(close_,&QPushButton::clicked,this,&SpellResearchWidget::cancelled);populate();
}
SpellResearchWidget::Catalog SpellResearchWidget::installedPreviewCatalog(const QString& root){
    const auto book=mnm::ui::loadGrimoireBook(root);Catalog catalog;catalog.ownerId="sample-character";
    for(const auto& entry:book.chapters[5].entries){QString text;const auto& level=entry.levels.first();for(const auto& page:level.pages){if(!text.isEmpty())text+="\n\n";text+=page;}catalog.spells.push_back({QString("grimoire-spell-%1").arg(entry.section),entry.title,text,true,false});}
    return catalog;
}
bool SpellResearchWidget::loadAssets(const QString& root,QString* error){try{
    const auto fonts=mnm::ui::loadMenuFonts(root);auto backdrop=mnm::ui::loadMenuImage(root,"Interface/Grimoire/800x600/Backdrop.JPG",{800,600});
    auto left=mnm::ui::loadGrimoirePageArt(root,"GenericL01.JPG"),right=mnm::ui::loadGrimoirePageArt(root,"GenericR01.JPG");const auto leftArea=largest(left),rightArea=largest(right).translated(400,0);
    if(leftArea.width()<160||rightArea.width()<160||leftArea.height()<300||rightArea.height()<300)throw std::runtime_error("Spell Research needs usable Grimoire text regions");
    const auto icons=mnm::ui::loadMenuSprites(root,"Interface/Grimoire/800x600/icons.spr");if(icons.size()!=34)throw std::runtime_error("Unexpected Spell Research close sprite catalog");
    const auto close=std::array<mnm::ui::MenuSpriteFrame,3>{icons[0],icons[1],icons[1]};for(const auto& frame:close)if(frame.image.isNull())throw std::runtime_error("Empty Spell Research close sprite");
    mnm::ui::installMenuFonts(this,fonts);backdrop_=std::move(backdrop);left_=std::move(left);right_=std::move(right);leftArea_=leftArea;rightArea_=rightArea;close_->setSprites(close,{27,116});arrange();update();if(error)error->clear();return true;
}catch(const std::exception& e){return fail(error,QString::fromUtf8(e.what()));}}
bool SpellResearchWidget::setCatalog(const Catalog& catalog,QString* error){
    if(!line(catalog.ownerId,128)||catalog.spells.size()>256)return fail(error,"Invalid Spell Research owner/catalog size");
    QSet<QString> ids;qsizetype textBytes=0;bool selection=catalog.selectedSpellId.isEmpty();
    for(const auto& spell:catalog.spells){if(!line(spell.id,128)||!line(spell.name,256)||ids.contains(spell.id)||spell.description.size()>65536||spell.description.contains(QChar(0)))return fail(error,"Invalid or duplicate supplied research spell");textBytes+=spell.description.size();if(textBytes>1024*1024)return fail(error,"Spell Research text limit");ids.insert(spell.id);if(spell.id==catalog.selectedSpellId)selection=true;}
    if(!selection)return fail(error,"Unknown selected research spell");
    catalog_=catalog;if(catalog_.selectedSpellId.isEmpty()&&!catalog_.spells.isEmpty())catalog_.selectedSpellId=catalog_.spells.first().id;
    {QSignalBlocker blocker(filter_);filter_->clear();}populate();if(error)error->clear();return true;
}
const SpellResearchWidget::Spell* SpellResearchWidget::selected() const {for(const auto& spell:catalog_.spells)if(spell.id==catalog_.selectedSpellId)return &spell;return nullptr;}
bool SpellResearchWidget::selectSpell(const QString& id,QString* error){
    for(const auto& spell:catalog_.spells)if(spell.id==id){const bool changed=catalog_.selectedSpellId!=id;catalog_.selectedSpellId=id;{QSignalBlocker blocker(spells_);for(int i=0;i<spells_->count();++i)if(spells_->item(i)->data(Qt::UserRole).toString()==id)spells_->setCurrentRow(i);}updateSelection();arrange();if(error)error->clear();if(changed)emit selectedSpellChanged(id);return true;}
    return fail(error,"Unknown supplied research spell");
}
void SpellResearchWidget::populate(){
    const QSignalBlocker blocker(spells_);spells_->clear();for(const auto& spell:catalog_.spells)if(spell.name.contains(filter_->text(),Qt::CaseInsensitive)){auto* item=new QListWidgetItem(spell.name,spells_);item->setData(Qt::UserRole,spell.id);item->setToolTip(spell.name);if(spell.id==catalog_.selectedSpellId)spells_->setCurrentItem(item);}
    updateSelection();arrange();
}
void SpellResearchWidget::updateSelection(){
    const auto* spell=selected();const bool visible=spell&&spell->name.contains(filter_->text(),Qt::CaseInsensitive);
    title_->setText(visible?spell->name:QString());description_->setPlainText(visible?spell->description:QString());state_->setText(visible?(spell->learned?"Learned":spell->available?"Available for research":"Unavailable"):"No spell selected");research_->setEnabled(visible&&spell->available&&!spell->learned);title_->setToolTip(title_->text());
}
void SpellResearchWidget::research(){const auto* spell=selected();if(spell&&research_->isEnabled())emit researchRequested({catalog_.ownerId,spell->id});}
QRect SpellResearchWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void SpellResearchWidget::arrange(){
    const auto canvas=contentRect();const double scale=canvas.width()/800.0;auto place=[&](QWidget* w,const QRect& r,mnm::ui::MenuFontRole role){w->setGeometry(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));w->setFont(mnm::ui::menuFont(this,role,scale));};
    using Role=mnm::ui::MenuFontRole;const auto l=leftArea_,r=rightArea_;
    place(heading_,{l.x(),l.y(),l.width(),40},Role::Heading);place(filter_,{l.x(),l.y()+45,l.width(),32},Role::Body);place(spells_,{l.x(),l.y()+87,l.width(),l.height()-87},Role::Body);
    place(title_,{r.x(),r.y(),r.width(),40},Role::Heading);auto fitted=title_->font();while(fitted.pixelSize()>qMax(6,qRound(10*scale))&&QFontMetrics(fitted).horizontalAdvance(title_->text())>title_->width())fitted.setPixelSize(fitted.pixelSize()-1);title_->setFont(fitted);place(state_,{r.x(),r.y()+45,r.width(),26},Role::Tooltip);place(description_,{r.x(),r.y()+77,r.width(),r.height()-122},Role::Body);place(research_,{r.x(),r.bottom()-38,r.width(),38},Role::Heading);place(close_,{773,238,27,116},Role::Tooltip);
    QWidget::setTabOrder(filter_,spells_);QWidget::setTabOrder(spells_,description_);QWidget::setTabOrder(description_,research_);QWidget::setTabOrder(research_,close_);
}
void SpellResearchWidget::focusFirstControl(){filter_->setFocus(Qt::OtherFocusReason);}
bool SpellResearchWidget::eventFilter(QObject* watched,QEvent* event){if(event->type()==QEvent::KeyPress){auto* key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Escape){if(!key->isAutoRepeat())emit cancelled();return true;}if(key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter){if(!key->isAutoRepeat()){if(watched==close_)emit cancelled();else research();}return true;}}return QWidget::eventFilter(watched,event);}
void SpellResearchWidget::keyPressEvent(QKeyEvent* key){if(key->key()==Qt::Key_Escape){if(!key->isAutoRepeat())emit cancelled();key->accept();}else QWidget::keyPressEvent(key);}
void SpellResearchWidget::paintEvent(QPaintEvent*){QPainter painter(this);painter.fillRect(rect(),Qt::black);const auto canvas=contentRect();if(!backdrop_.isNull())painter.drawImage(canvas,backdrop_);painter.translate(canvas.topLeft());painter.scale(canvas.width()/800.0,canvas.height()/600.0);if(!left_.image.isNull())painter.drawImage(0,0,left_.image);if(!right_.image.isNull())painter.drawImage(400,0,right_.image);}
void SpellResearchWidget::resizeEvent(QResizeEvent* e){QWidget::resizeEvent(e);arrange();}
