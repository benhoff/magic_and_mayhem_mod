#pragma once
#include "menu_sprites.hpp"
#include <QMetaType>
#include <QWidget>
#include <array>
class QLabel;
class SpellboxCell;
class SpellboxWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Alignment { Law, Neutral, Chaos };
    Q_ENUM(Alignment)
    struct Spell { QString id,name; int artworkIndex=-1; };
    struct Item { QString id,name; int artworkIndex=-1,quantity=1; std::array<Spell,3> spells{}; };
    struct Talisman { QString id; Alignment alignment=Alignment::Law; QString itemId; };
    struct Inventory { QString ownerId; QVector<Item> items; QVector<Talisman> talismans; };
    struct Assignment { QString talismanId,itemId,spellId; Alignment alignment=Alignment::Law; };
    struct Request { QString ownerId; QVector<Assignment> assignments; };
    struct Preview { QString ownerId,talismanId,itemId,spellId; Alignment alignment=Alignment::Law; };
    explicit SpellboxWidget(QWidget* parent=nullptr);
    bool loadAssets(const QString& root,QString* error=nullptr);
    bool setInventory(const Inventory&,QString* error=nullptr);
    Inventory inventory() const { return accepted_; }
    Request draftRequest() const;
    bool assignItem(const QString& itemId,const QString& talismanId,QString* error=nullptr);
    bool removeItem(const QString& talismanId,QString* error=nullptr);
    QRect contentRect() const;
    void focusFirstControl();
signals:
    void loadoutAccepted(const SpellboxWidget::Request& request);
    void spellPreviewRequested(const SpellboxWidget::Preview& request);
    void cancelled();
protected:
    bool eventFilter(QObject*,QEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    friend class SpellboxCell;
    static bool valid(const Inventory&);
    int itemIndex(const QString&) const;
    int talismanIndex(const QString&) const;
    int available(int item) const;
    void populate();
    void arrange();
    void preview();
    void accept();
    void cancel();
    bool dropCell(bool item,int index,SpellboxCell* source);
    Inventory accepted_,draft_;
    int selectedItem_=-1,selectedTalisman_=-1;
    quint64 revision_=0;
    QImage background_;
    mnm::ui::MenuSpriteSheet itemSprites_,talismanSprites_;
    std::array<QString,3> alignmentNames_{"Talisman Of Law","Talisman Of Neutrality","Talisman Of Chaos"};
    QVector<SpellboxCell*> items_,talismans_;
    std::array<QPushButton*,5> actions_{};
    QLabel* detail_=nullptr;
};
Q_DECLARE_METATYPE(SpellboxWidget::Request)
Q_DECLARE_METATYPE(SpellboxWidget::Preview)
