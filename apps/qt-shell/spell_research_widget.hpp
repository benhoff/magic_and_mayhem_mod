#pragma once
#include "grimoire_assets.hpp"
#include "menu_sprites.hpp"
#include <QWidget>
class QLabel;
class QListWidget;
class QLineEdit;
class QTextBrowser;
class SpellResearchWidget final:public QWidget {
    Q_OBJECT
public:
    struct Spell {QString id,name,description;bool available=true,learned=false;};
    struct Catalog {QString ownerId;QVector<Spell> spells;QString selectedSpellId;};
    struct Request {QString ownerId,spellId;};
    explicit SpellResearchWidget(QWidget* parent=nullptr);
    bool loadAssets(const QString& root,QString* error=nullptr);
    bool setCatalog(const Catalog&,QString* error=nullptr);
    Catalog catalog() const {return catalog_;}
    bool selectSpell(const QString& id,QString* error=nullptr);
    QRect contentRect() const;
    void focusFirstControl();
    static Catalog installedPreviewCatalog(const QString& root);
 signals:
    void researchRequested(const SpellResearchWidget::Request&);
    void selectedSpellChanged(const QString& id);
    void cancelled();
 protected:
    bool eventFilter(QObject*,QEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
 private:
    const Spell* selected() const;
    void populate();void updateSelection();void arrange();void research();
    Catalog catalog_;
    QImage backdrop_;mnm::ui::GrimoirePageArt left_,right_;
    QRect leftArea_{89,47,282,489},rightArea_{430,47,282,489};
    QLabel* heading_=nullptr;QLabel* title_=nullptr;QLabel* state_=nullptr;
    QLineEdit* filter_=nullptr;QListWidget* spells_=nullptr;QTextBrowser* description_=nullptr;
    QPushButton* research_=nullptr;mnm::ui::SpriteButton* close_=nullptr;
};
Q_DECLARE_METATYPE(SpellResearchWidget::Request)
