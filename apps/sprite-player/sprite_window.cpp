#include "sprite_window.hpp"
#include <QComboBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QImage>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

namespace mnm::ui {
SpriteWindow::SpriteWindow(const std::filesystem::path& root,const QString& tag,QWidget* parent)
    :QWidget(parent),player_(root) {
    setWindowTitle("Magic & Mayhem — Sprite tables");auto* layout=new QVBoxLayout(this);
    auto* files=new QHBoxLayout;path_=new QLineEdit(tag,this);path_->setObjectName("tagPath");
    path_->setPlaceholderText("TAG path relative to the installation");files->addWidget(path_);
    auto* browse=new QPushButton("Browse…",this);files->addWidget(browse);
    auto* open=new QPushButton("Open",this);open->setObjectName("open");files->addWidget(open);layout->addLayout(files);
    names_=new QComboBox(this);names_->setObjectName("names");layout->addWidget(names_);
    image_=new QLabel(this);image_->setObjectName("preview");image_->setFixedSize(sprites::previewWidth,sprites::previewHeight);
    layout->addWidget(image_,0,Qt::AlignHCenter);
    seek_=new QSlider(Qt::Horizontal,this);seek_->setObjectName("seek");layout->addWidget(seek_);
    auto* controls=new QHBoxLayout;pause_=new QPushButton("Pause",this);pause_->setObjectName("pause");pause_->setCheckable(true);
    reverse_=new QPushButton("Reverse",this);reverse_->setObjectName("reverse");reverse_->setCheckable(true);
    reload_=new QPushButton("Reload",this);reload_->setObjectName("reload");
    modes_=new QComboBox(this);modes_->setObjectName("mode");modes_->addItems({"Loop","Once","Ping-pong"});
    controls->addWidget(pause_);controls->addWidget(modes_);controls->addWidget(reverse_);controls->addWidget(reload_);layout->addLayout(controls);
    status_=new QLabel("Open a TAG sprite table",this);status_->setObjectName("status");status_->setWordWrap(true);layout->addWidget(status_);
    for(auto* w:std::vector<QWidget*>{names_,seek_,pause_,modes_,reverse_,reload_})w->setEnabled(false);
    connect(browse,&QPushButton::clicked,this,[this,root=std::filesystem::canonical(root)]{
        const auto file=QFileDialog::getOpenFileName(this,"Open sprite table",QString::fromStdString(root.string()),"Sprite tables (*.tag *.TAG)");
        if(!file.isEmpty())path_->setText(QString::fromStdString(std::filesystem::path(file.toStdString()).lexically_relative(root).generic_string()));
    });
    auto load=[this]{attempt([this]{player_.open(path_->text().toStdString());
        {QSignalBlocker a(modes_),b(reverse_),c(pause_);modes_->setCurrentIndex(0);reverse_->setChecked(false);pause_->setChecked(false);}
        names();for(auto* w:std::vector<QWidget*>{names_,seek_,pause_,modes_,reverse_,reload_})w->setEnabled(true);
    });};
    connect(open,&QPushButton::clicked,this,load);connect(path_,&QLineEdit::returnPressed,this,load);
    connect(names_,&QComboBox::currentIndexChanged,this,[this]{choose();});
    connect(modes_,&QComboBox::currentIndexChanged,this,[this]{choose();});
    connect(reverse_,&QPushButton::clicked,this,[this]{choose();});
    connect(pause_,&QPushButton::clicked,this,[this](bool paused){attempt([&]{player_.pause(paused);});});
    connect(seek_,&QSlider::valueChanged,this,[this](int value){attempt([&]{player_.seek(std::size_t(value));});});
    connect(reload_,&QPushButton::clicked,this,[this]{attempt([this]{player_.reload();names();});});
    timer_.setInterval(33);connect(&timer_,&QTimer::timeout,this,[this]{
        const auto elapsed=clock_.restart();
        if(!pause_->isEnabled() || pause_->isChecked() || player_.timeline().position().finished)return;
        attempt([&]{player_.advance(std::uint64_t(elapsed));});
    });
    clock_.start();timer_.start();if(!tag.isEmpty())load();
}
void SpriteWindow::attempt(const std::function<void()>& action) {
    try {action();refresh();}
    catch(const std::exception& e) {status_->setText(QString("Could not update sprite table: %1").arg(QString::fromUtf8(e.what())));}
    clock_.restart();
}
void SpriteWindow::names() {
    const auto selected=player_.timeline().checkpoint().clip->entries().front().name;
    QSignalBlocker block(names_);names_->clear();int index=0;
    for(const auto& group:player_.groups()) {
        const auto raw=QByteArray(reinterpret_cast<const char*>(group.name.data()),8);
        auto text=QString::fromLatin1(raw.split('\0').front()).toHtmlEscaped();
        names_->addItem(QString("%1 [%2] — %3 frames").arg(text,QString::fromLatin1(raw.toHex())).arg(group.count));
        if(group.name==selected)index=names_->count()-1;
    }
    names_->setCurrentIndex(index);
}
void SpriteWindow::choose() {
    attempt([this]{player_.select(std::size_t(names_->currentIndex()),sprites::TimelineMode(modes_->currentIndex()),reverse_->isChecked());});
}
void SpriteWindow::refresh() {
    const auto pixels=sprites::renderPreview(renderer_,player_.plan());
    QImage image(pixels.width,pixels.height,QImage::Format_RGB32);
    if(image.isNull())throw std::runtime_error("Preview image allocation failed");
    for(int y=0;y<pixels.height;++y)for(int x=0;x<pixels.width;++x) {
        const auto p=pixels.pixels[std::size_t(y)*pixels.width+x];
        const unsigned r=p>>11,g=(p>>5)&63,b=p&31;image.setPixel(x,y,qRgb((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2)));
    }
    image_->setPixmap(QPixmap::fromImage(image));const auto& timeline=player_.timeline();
    {QSignalBlocker block(seek_);seek_->setRange(0,int(timeline.duration()/100-1));seek_->setValue(int(timeline.position().scheduleEntry));}
    status_->setText(QString("%1 · %2 · frame %3%4").arg(QString::fromStdString(player_.tagPath()),QString::fromStdString(player_.spritePath()))
                     .arg(timeline.frame()).arg(timeline.position().finished?" · finished":""));
}
}
