#include "menu_sprites.hpp"
#include "asset_file.hpp"
#include "sprite_loader.hpp"
#include <QPainter>
#include <QStyleOptionButton>
#include <stdexcept>
namespace mnm::ui {
MenuSpriteSheet loadMenuSprites(const QString& root,const QString& path) {
    auto created=assets::AssetStore::create(std::filesystem::path(root.toStdString()));
    if (auto* error=std::get_if<assets::Error>(&created)) throw std::runtime_error(error->detail);
    auto opened=std::get<assets::AssetStore>(created).open(path.toStdString());
    if (auto* error=std::get_if<assets::Error>(&opened)) throw std::runtime_error(path.toStdString()+": "+error->detail);
    assets::SpriteLimits limits;limits.inputBytes=8*1024*1024;limits.frames=128;limits.width=128;limits.height=128;
    limits.pixels=1024*1024;limits.decodedBytes=4*1024*1024;limits.scannedBytes=8*1024*1024;
    auto result=assets::loadSprite(*std::get<std::unique_ptr<assets::AssetFile>>(opened),limits);
    if (auto* error=std::get_if<assets::SpriteError>(&result)) throw std::runtime_error(path.toStdString()+": "+error->detail);
    const auto& sprite=std::get<assets::Sprite>(result);MenuSpriteSheet frames;
    for (const auto& frame:sprite.frames) {
        if (frame.originX<-128 || frame.originX>128 || frame.originY<-128 || frame.originY>128)
            throw std::runtime_error("Menu sprite origin outside native presentation limit");
        MenuSpriteFrame converted;converted.origin=QPoint(frame.originX,frame.originY);
        if (!frame.empty()) {
            converted.image=QImage(int(frame.width),int(frame.height),QImage::Format_ARGB32);
            if (converted.image.isNull()) throw std::runtime_error("Menu sprite allocation failed");
            for (unsigned y=0;y<frame.height;++y) {
                auto* row=reinterpret_cast<QRgb*>(converted.image.scanLine(int(y)));
                for (unsigned x=0;x<frame.width;++x) {
                    const auto n=std::size_t(y)*frame.width+x;
                    if (!frame.opaqueMask[n]) {row[x]=0;continue;}
                    if (sprite.storage==assets::SpriteStorage::rgb565) {
                        const auto word=std::get<std::vector<std::uint16_t>>(frame.pixels)[n];
                        // Replicate the high bits to the low bits when expanding packed RGB565.
                        const unsigned r=word>>11,g=(word>>5)&63,b=word&31;
                        row[x]=qRgb((r<<3)|(r>>2),(g<<2)|(g>>4),(b<<3)|(b>>2));
                    } else {
                        const auto index=std::get<std::vector<std::uint8_t>>(frame.pixels)[n];
                        const auto rgb=sprite.palettes[*frame.paletteIndex][index];row[x]=qRgb(rgb.red,rgb.green,rgb.blue);
                    }
                }
            }
        }
        frames.push_back(std::move(converted));
    }
    return frames;
}
std::array<MenuSpriteFrame,3> spriteStates(const MenuSpriteSheet& frames,int first) {
    if (first<0 || first>frames.size()-3) throw std::runtime_error("Missing menu sprite states");
    std::array<MenuSpriteFrame,3> result{frames[first],frames[first+1],frames[first+2]};
    for (const auto& frame:result) if (frame.image.isNull()) throw std::runtime_error("Empty menu sprite state");
    return result;
}
SpriteButton::SpriteButton(const QString& text,QWidget* parent):QPushButton(text,parent) {setAttribute(Qt::WA_Hover);}
void SpriteButton::setSprites(const std::array<MenuSpriteFrame,3>& frames,const QSize& canvas) {frames_=frames;canvas_=canvas;update();}
void SpriteButton::clearSprites() {frames_={};canvas_=QSize();update();}
void SpriteButton::paintEvent(QPaintEvent* event) {
    if (frames_[0].image.isNull() || canvas_.isEmpty()) {QPushButton::paintEvent(event);return;}
    QStyleOptionButton option;initStyleOption(&option);
    const int state=isEnabled()?(isDown()?2:((option.state&QStyle::State_MouseOver)?1:0)):0;
    const auto& frame=frames_[state];QPainter painter(this);painter.setRenderHint(QPainter::SmoothPixmapTransform);
    painter.save();painter.scale(double(width())/canvas_.width(),double(height())/canvas_.height());
    if (!isEnabled()) painter.setOpacity(.55);
    painter.drawImage(-frame.origin,frame.image);painter.restore();
    if (hasFocus()) {painter.setPen(QColor("#fff5d6"));painter.drawRect(rect().adjusted(0,0,-1,-1));}
}
void TalismanBar::setSprites(const MenuSpriteFrame& active,const MenuSpriteFrame& inactive) {active_=active;inactive_=inactive;update();}
void TalismanBar::setCounts(int value,int maximum) {
    if (maximum<1 || maximum>32 || value<0 || value>maximum) throw std::runtime_error("Invalid menu talisman counts");
    value_=value;maximum_=maximum;setText(QString("%1 / %2").arg(value).arg(maximum));update();}
void TalismanBar::paintEvent(QPaintEvent* event) {
    if (active_.image.isNull() || inactive_.image.isNull()) {QLabel::paintEvent(event);return;}
    QPainter painter(this);painter.setRenderHint(QPainter::SmoothPixmapTransform);painter.scale(width()/350.0,height()/50.0);
    for (int i=0;i<maximum_;++i) {
        const auto& frame=i<value_?active_:inactive_;
        painter.drawImage(QPointF(i*350.0/maximum_-frame.origin.x(),-frame.origin.y()),frame.image);
    }
}
}
