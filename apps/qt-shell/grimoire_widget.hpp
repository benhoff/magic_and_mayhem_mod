#pragma once
#include "grimoire_assets.hpp"
#include "menu_sprites.hpp"
#include <QWidget>
class QLabel;
class QListWidget;
class QTextBrowser;
class QPushButton;
class GrimoireWidget final : public QWidget {
    Q_OBJECT
public:
    struct Location {int chapter=0,section=0,level=0,page=0;bool illustration=false;};
    explicit GrimoireWidget(QWidget* parent=nullptr);
    bool loadAssets(const QString& root,QString* error=nullptr);
    bool setLocation(const Location&,QString* error=nullptr);
    Location location() const {return location_;}
    const mnm::ui::GrimoireBook& book() const {return book_;}
    QRect contentRect() const;
    void focusFirstControl();
signals:
    void closed();
    void pageChanged(const GrimoireWidget::Location&);
    void navigationFailed(const QString&);
protected:
    bool eventFilter(QObject*,QEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void paintEvent(QPaintEvent*) override;
private:
    struct Spread {QImage left,right;QRect leftTitle,rightText,leftText;QString title,text;};
    static const mnm::ui::GrimoireEntry* entry(const mnm::ui::GrimoireBook&,const Location&);
    static bool valid(const mnm::ui::GrimoireBook&,const Location&);
    static Spread prepare(const QString&,const mnm::ui::GrimoireBook&,const Location&);
    QVector<Location> sequence() const;
    void turn(int direction);
    void selectChapter(int chapter);
    void populate();
    void arrange();
    QString root_;
    mnm::ui::GrimoireBook book_;
    Location location_;
    Spread spread_;
    QImage backdrop_;
    mnm::ui::MenuSpriteSheet icons_,turns_;
    std::array<QPushButton*,16> tabs_{};
    std::array<QRect,16> tabRects_{};
    QPushButton *previous_=nullptr,*next_=nullptr,*close_=nullptr,*contents_=nullptr,*artwork_=nullptr;
    QRect previousRect_,nextRect_,closeRect_;
    QLabel* title_=nullptr;
    QListWidget* entries_=nullptr;
    QTextBrowser* text_=nullptr;
};
Q_DECLARE_METATYPE(GrimoireWidget::Location)
