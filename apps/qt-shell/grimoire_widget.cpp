#include "grimoire_widget.hpp"
#include <QFontMetrics>
#include <QKeyEvent>
#include <QLabel>
#include <QListWidget>
#include <QPainter>
#include <QSignalBlocker>
#include <QTextBrowser>
#include <stdexcept>
namespace {
using Page=mnm::ui::GrimoirePageArt;
Page pageArt(const QString& root,const QString& name){return mnm::ui::loadGrimoirePageArt(root,name);}
QRect largest(const Page& page) {QRect result;for(const auto& area:page.areas)if(area.width()*area.height()>result.width()*result.height())result=area;return result;}
int coordinate(const mnm::ui::Sections& cfg,const QString& section,const QString& key,int maximum) {
    bool ok=false;const int value=cfg.value(section).value(key).toInt(&ok);
    if(!ok || value<0 || value>maximum)throw std::runtime_error("Invalid Grimoire control coordinate");
    return value;
}
QRect positioned(const mnm::ui::Sections& cfg,const QString& section,const QSize& size) {
    const int x=coordinate(cfg,section,"800x600_X",800-size.width()),y=coordinate(cfg,section,"800x600_Y",600-size.height());return {x,y,size.width(),size.height()};
}
bool same(const GrimoireWidget::Location& a,const GrimoireWidget::Location& b) {return a.chapter==b.chapter&&a.section==b.section&&a.level==b.level&&a.page==b.page;}
}
GrimoireWidget::GrimoireWidget(QWidget* parent):QWidget(parent) {
    resize(800,600);setMinimumSize(320,240);setWindowTitle("Magic & Mayhem — Grimoire preview");
    auto button=[&](const QString& name,const QString& text){auto* b=new mnm::ui::SpriteButton(text,this);b->setObjectName(name);b->setStyleSheet(mnm::ui::menuButtonStyle());b->setCursor(Qt::PointingHandCursor);b->installEventFilter(this);return b;};
    for(int i=0;i<16;++i){tabs_[i]=button(QString("grimoireTab%1").arg(i),QString::number(i%8+1));connect(tabs_[i],&QPushButton::clicked,this,[this,i]{selectChapter(i%8);});}
    previous_=button("grimoirePrevious","Previous");next_=button("grimoireNext","Next");close_=button("grimoireClose","Close");contents_=button("grimoireContents","Contents");artwork_=button("grimoireArtwork","Artwork");
    previous_->setAccessibleName("Previous page");next_->setAccessibleName("Next page");close_->setAccessibleName("Close Grimoire");
    connect(previous_,&QPushButton::clicked,this,[this]{turn(-1);});connect(next_,&QPushButton::clicked,this,[this]{turn(1);});connect(close_,&QPushButton::clicked,this,&GrimoireWidget::closed);connect(contents_,&QPushButton::clicked,this,[this]{selectChapter(location_.chapter);});
    connect(artwork_,&QPushButton::clicked,this,[this]{auto target=location_;target.illustration=!target.illustration;QString error;if(!setLocation(target,&error))emit navigationFailed(error);});
    title_=new QLabel(this);title_->setObjectName("grimoireTitle");title_->setTextFormat(Qt::PlainText);title_->setAlignment(Qt::AlignCenter);title_->setStyleSheet("color:#3e2313;background:transparent;");title_->setAttribute(Qt::WA_TransparentForMouseEvents);
    entries_=new QListWidget(this);entries_->setObjectName("grimoireEntries");entries_->installEventFilter(this);entries_->setStyleSheet("QListWidget {color:#3e2313;background:transparent;border:0;} QListWidget::item:selected {color:#fff5d6;background:#51402a;}");
    connect(entries_,&QListWidget::itemActivated,this,[this](QListWidgetItem* item){const int section=item->data(Qt::UserRole).toInt();Location target{location_.chapter,section,0,0};if(const auto* e=entry(book_,target)){target.level=e->levels.firstKey();QString error;if(!setLocation(target,&error))emit navigationFailed(error);}});
    text_=new QTextBrowser(this);text_->setObjectName("grimoireText");text_->installEventFilter(this);text_->setOpenExternalLinks(false);text_->setOpenLinks(false);text_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);text_->setStyleSheet("QTextBrowser {color:#3e2313;background:transparent;border:0;}");
    for(int i=0;i<15;++i){QWidget::setTabOrder(tabs_[i],tabs_[i+1]);}
    QWidget::setTabOrder(tabs_[15],entries_);QWidget::setTabOrder(entries_,text_);QWidget::setTabOrder(text_,previous_);QWidget::setTabOrder(previous_,next_);QWidget::setTabOrder(next_,contents_);QWidget::setTabOrder(contents_,artwork_);QWidget::setTabOrder(artwork_,close_);
}
const mnm::ui::GrimoireEntry* GrimoireWidget::entry(const mnm::ui::GrimoireBook& book,const Location& where) {
    if(where.chapter<0 || where.chapter>=book.chapters.size())return nullptr;
    for(const auto& e:book.chapters[where.chapter].entries){if(e.section==where.section)return &e;}
    return nullptr;
}
bool GrimoireWidget::valid(const mnm::ui::GrimoireBook& book,const Location& where) {
    if(where.chapter<0 || where.chapter>=book.chapters.size() || where.page<0 || where.level<0 || where.level>4)return false;
    if(where.section==0)return where.page==0&&where.level==0&&!where.illustration;
    if(where.illustration&&!book.artwork.contains(QString("%1/%2/2").arg(where.chapter).arg(where.section)))return false;
    const auto* e=entry(book,where);return e&&e->levels.contains(where.level)&&where.page<e->levels[where.level].pages.size();
}
GrimoireWidget::Spread GrimoireWidget::prepare(const QString& root,const mnm::ui::GrimoireBook& book,const Location& where) {
    if(!valid(book,where))throw std::runtime_error("Invalid Grimoire location");
    const auto key=QString("%1/%2/").arg(where.chapter).arg(where.section);
    const auto leftName=book.artwork.value(key+"1","GenericL01.JPG"),rightName=book.artwork.value(key+"2","GenericR01.JPG");
    auto left=pageArt(root,leftName),right=pageArt(root,rightName);auto leftArea=largest(left),rightArea=largest(right);
    if(!where.illustration && leftArea.height()<80 && rightArea.height()<80){right=pageArt(root,"GenericR01.JPG");rightArea=largest(right);}
    Spread result;result.left=left.image;result.right=right.image;
    const auto* e=entry(book,where);result.title=e?e->title:book.chapters[where.chapter].title;
    // Reserve a heading in the upper text area; prefer a separate short header mask.
    result.leftTitle=leftArea.isEmpty()?QRect(89,47,282,46):QRect(leftArea.x(),leftArea.y(),leftArea.width(),qMin(46,leftArea.height()));
    for(const auto& area:left.areas)if(area.height()<80){result.leftTitle=area;break;}
    if(leftArea.intersects(result.leftTitle))leftArea.setTop(result.leftTitle.bottom()+8);
    result.leftText=leftArea;result.rightText=rightArea.translated(400,0);
    if(!e)result.text=QString("%1\n\n%2 entries\n\nSelect an entry from the contents.").arg(result.title).arg(book.chapters[where.chapter].entries.size());
    else {
        const auto& level=e->levels[where.level];result.text=level.pages[where.page];
        if(!level.dynamicLabels.isEmpty()){result.text+="\n\n";for(const auto& label:level.dynamicLabels)result.text+=label+": —\n";}
    }
    if(result.rightText.height()<80)result.rightText=QRect();
    if(result.leftText.height()<80)result.leftText=QRect();
    if(!where.illustration && ((!e && result.leftText.isEmpty()) || (result.leftText.isEmpty() && result.rightText.isEmpty())))throw std::runtime_error("Grimoire artwork has no usable text region");
    return result;
}
bool GrimoireWidget::loadAssets(const QString& root,QString* error) {
    if(error)error->clear();
    try {
        const auto fonts=mnm::ui::loadMenuFonts(root);
        auto book=mnm::ui::loadGrimoireBook(root);const auto spread=prepare(root,book,location_);
        const auto backdrop=mnm::ui::loadMenuImage(root,"Interface/Grimoire/800x600/Backdrop.JPG",QSize(800,600));
        const auto icons=mnm::ui::loadMenuSprites(root,"Interface/Grimoire/800x600/icons.spr"),turns=mnm::ui::loadMenuSprites(root,"Interface/Grimoire/Generic/PageTurns.spr");
        if(icons.size()!=34 || turns.size()!=4)throw std::runtime_error("Unexpected Grimoire sprite catalog");
        for(const auto& frame:icons){if(frame.image.isNull())throw std::runtime_error("Empty Grimoire icon");}
        for(const auto& frame:turns){if(frame.image.isNull())throw std::runtime_error("Empty Grimoire page turn");}
        const auto previous=positioned(book.layout,"PAGETURN_LEFT",QSize(45,35)),next=positioned(book.layout,"PAGETURN_RIGHT",QSize(45,35)),close=positioned(book.layout,"CLOSE_GRIMOIRE",QSize(27,116));
        std::array<QRect,16> rectangles;
        for(int i=0;i<16;++i){const auto prefix=QString("800x600_%1%2").arg(i%8+1).arg(i<8?"L":"R");const auto size=icons[2+(i%8)*4+(i<8?0:2)].image.size();rectangles[i]={coordinate(book.layout,"TABS",prefix+"X",800-size.width()),coordinate(book.layout,"TABS",prefix+"Y",600-size.height()),size.width(),size.height()};}
        // Validate every supplied tooltip before replacing an existing book/location.
        for(int i=0;i<9;++i)(void)mnm::ui::textLabel(book.tooltips,QString::number(i));
        mnm::ui::installMenuFonts(this,fonts);
        book_=std::move(book);spread_=spread;backdrop_=backdrop;icons_=icons;turns_=turns;root_=root;tabRects_=rectangles;previousRect_=previous;nextRect_=next;closeRect_=close;
        static_cast<mnm::ui::SpriteButton*>(previous_)->setSprites({turns[0],turns[1],turns[1]},previous.size());static_cast<mnm::ui::SpriteButton*>(next_)->setSprites({turns[2],turns[3],turns[3]},next.size());static_cast<mnm::ui::SpriteButton*>(close_)->setSprites({icons[0],icons[1],icons[1]},close.size());close_->setToolTip(mnm::ui::textLabel(book_.tooltips,"0"));
        populate();arrange();update();return true;
    }catch(const std::exception& failure){if(error)*error=QString::fromUtf8(failure.what());return false;}
}
bool GrimoireWidget::setLocation(const Location& where,QString* error) {
    if(error)error->clear();
    try{auto spread=prepare(root_,book_,where);location_=where;spread_=std::move(spread);populate();arrange();update();emit pageChanged(location_);return true;}catch(const std::exception& failure){if(error)*error=QString::fromUtf8(failure.what());return false;}
}
QVector<GrimoireWidget::Location> GrimoireWidget::sequence() const {
    QVector<Location> result;
    for(int c=0;c<book_.chapters.size();++c){result.push_back({c,0,0,0});for(const auto& e:book_.chapters[c].entries){const int level=e.section==location_.section&&c==location_.chapter?location_.level:e.levels.firstKey();for(int p=0;p<e.levels[level].pages.size();++p)result.push_back({c,e.section,level,p});}}
    return result;
}
void GrimoireWidget::turn(int direction) {const auto order=sequence();for(int i=0;i<order.size();++i)if(same(order[i],location_)){const int next=i+direction;if(next>=0&&next<order.size()){QString error;if(!setLocation(order[next],&error))emit navigationFailed(error);}return;}}
void GrimoireWidget::selectChapter(int chapter) {QString error;if(!setLocation({chapter,0,0,0},&error))emit navigationFailed(error);else focusFirstControl();}
void GrimoireWidget::populate() {
    if(book_.chapters.isEmpty())return;
    title_->setText(spread_.title);text_->setPlainText(spread_.text);text_->moveCursor(QTextCursor::Start);
    {QSignalBlocker blocker(entries_);entries_->clear();for(const auto& e:book_.chapters[location_.chapter].entries){auto* item=new QListWidgetItem(e.title,entries_);item->setData(Qt::UserRole,e.section);}if(entries_->count())entries_->setCurrentRow(0);}
    entries_->setVisible(location_.section==0);text_->setVisible(!location_.illustration);contents_->setEnabled(location_.section!=0);
    artwork_->setEnabled(location_.section!=0&&book_.artwork.contains(QString("%1/%2/2").arg(location_.chapter).arg(location_.section)));artwork_->setText(location_.illustration?"Read":"Artwork");
    for(int i=0;i<16;++i){const int n=2+(i%8)*4+(i<8?0:2);const bool selected=i%8==location_.chapter;static_cast<mnm::ui::SpriteButton*>(tabs_[i])->setSprites({icons_[n+(selected?1:0)],icons_[n+1],icons_[n+1]},tabRects_[i].size());tabs_[i]->setAccessibleName(book_.chapters[i%8].title);tabs_[i]->setToolTip(mnm::ui::textLabel(book_.tooltips,QString::number(i%8+1)));}
    const auto order=sequence();for(int i=0;i<order.size();++i)if(same(order[i],location_)){previous_->setEnabled(i>0);next_->setEnabled(i+1<order.size());break;}
}
void GrimoireWidget::focusFirstControl() {if(location_.section==0)entries_->setFocus(Qt::OtherFocusReason);else if(location_.illustration)artwork_->setFocus(Qt::OtherFocusReason);else text_->setFocus(Qt::OtherFocusReason);}
QRect GrimoireWidget::contentRect() const {return mnm::ui::menuContentRect(size());}
void GrimoireWidget::arrange() {
    const auto canvas=contentRect();const double scale=canvas.width()/800.0;auto map=[&](const QRect& r){return QRect(canvas.x()+qRound(r.x()*scale),canvas.y()+qRound(r.y()*scale),qRound(r.width()*scale),qRound(r.height()*scale));};
    for(int i=0;i<16;++i){tabs_[i]->setGeometry(map(tabRects_[i]));}
    previous_->setGeometry(map(previousRect_));next_->setGeometry(map(nextRect_));close_->setGeometry(map(closeRect_));contents_->setGeometry(map(QRect(350,535,100,30)));artwork_->setGeometry(map(QRect(475,535,100,30)));
    title_->setGeometry(map(spread_.leftTitle));entries_->setGeometry(map(spread_.leftText));text_->setGeometry(map(spread_.rightText.isEmpty()?spread_.leftText:spread_.rightText));
    auto font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Body,scale);entries_->setFont(font);text_->setFont(font);contents_->setFont(mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Tooltip,scale));artwork_->setFont(contents_->font());font=mnm::ui::menuFont(this,mnm::ui::MenuFontRole::Heading,scale);while(font.pixelSize()>10&&QFontMetrics(font).horizontalAdvance(title_->text())>title_->width())font.setPixelSize(font.pixelSize()-1);title_->setFont(font);
}
void GrimoireWidget::resizeEvent(QResizeEvent* event){QWidget::resizeEvent(event);arrange();}
bool GrimoireWidget::eventFilter(QObject* watched,QEvent* event) {
    if(event->type()==QEvent::KeyPress){auto* key=static_cast<QKeyEvent*>(event);if(key->key()==Qt::Key_Escape){key->accept();if(!key->isAutoRepeat())emit closed();return true;}if((key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter)&&watched==entries_){key->accept();if(!key->isAutoRepeat()&&entries_->currentItem())emit entries_->itemActivated(entries_->currentItem());return true;}if((key->key()==Qt::Key_Return||key->key()==Qt::Key_Enter)&&qobject_cast<QPushButton*>(watched)){key->accept();if(!key->isAutoRepeat())qobject_cast<QPushButton*>(watched)->click();return true;}if(key->key()==Qt::Key_PageDown||key->key()==Qt::Key_PageUp){key->accept();if(!key->isAutoRepeat())turn(key->key()==Qt::Key_PageDown?1:-1);return true;}}
    return QWidget::eventFilter(watched,event);
}
void GrimoireWidget::keyPressEvent(QKeyEvent* event){if(event->key()==Qt::Key_Escape){event->accept();if(!event->isAutoRepeat())emit closed();return;}if(event->key()==Qt::Key_Left||event->key()==Qt::Key_Right){event->accept();if(!event->isAutoRepeat())turn(event->key()==Qt::Key_Right?1:-1);return;}QWidget::keyPressEvent(event);}
void GrimoireWidget::paintEvent(QPaintEvent*){QPainter painter(this);painter.fillRect(rect(),Qt::black);painter.setRenderHint(QPainter::SmoothPixmapTransform);const auto canvas=contentRect();if(!backdrop_.isNull()){painter.drawImage(canvas,backdrop_);painter.drawImage(QRect(canvas.x(),canvas.y(),canvas.width()/2,canvas.height()),spread_.left);painter.drawImage(QRect(canvas.x()+canvas.width()/2,canvas.y(),canvas.width()-canvas.width()/2,canvas.height()),spread_.right);}}
