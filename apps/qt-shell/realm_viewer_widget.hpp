#pragma once
#include "menu_sprites.hpp"
#include "realm_viewer_visuals.hpp"
#include <QMetaType>
#include <QWidget>
#include <array>
#include <optional>
class QLabel;
class QTimer;
class RealmViewerWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Realm { Celtic, Greek, Medieval };
    Q_ENUM(Realm)
    enum class AuxiliaryAction { Spellbox, Grimoire, Character, Options };
    Q_ENUM(AuxiliaryAction)
    struct Region {
        QString id,name;int artworkNumber=1;QPoint flagPosition;int flagArtworkIndex=0;
        bool available=true;std::array<bool,3> auxiliaryAvailable{true,true,true};
    };
    struct Campaign {
        QString id,name;
        std::array<QVector<Region>,3> regions;
        std::array<bool,3> realmAvailable{true,true,true};
        std::array<bool,4> auxiliaryAvailable{true,true,true,true};
        Realm realm=Realm::Celtic;
        std::array<QString,3> selectedRegionIds{};
    };
    struct Request {QString campaignId,regionId;Realm realm=Realm::Celtic;int artworkNumber=1;};
    explicit RealmViewerWidget(QWidget* parent=nullptr);
    bool loadAssets(const QString& root,QString* error=nullptr);
    bool setCampaign(const Campaign&,QString* error=nullptr);
    Campaign campaign() const {return campaign_;}
    bool selectRegion(const QString& id,QString* error=nullptr);
    bool selectRealm(Realm realm,QString* error=nullptr);
    QRect contentRect() const;
    QString regionAt(const QPoint& widgetPoint) const;
    // One bounded native presentation tick; also permits deterministic tests.
    void advanceAnimation();
    std::optional<QPoint> pathPreviewPosition() const;
    void focusFirstControl();
signals:
    void regionRequested(const RealmViewerWidget::Request&);
    void selectionChanged(const QString& regionId);
    void realmChanged(RealmViewerWidget::Realm);
    void auxiliaryRequested(RealmViewerWidget::AuxiliaryAction);
    void cancelled();
protected:
    bool eventFilter(QObject*,QEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseDoubleClickEvent(QMouseEvent*) override;
    void leaveEvent(QEvent*) override;
    void showEvent(QShowEvent*) override;
    void hideEvent(QHideEvent*) override;
private:
    static bool valid(const Campaign&);
    const Region* selected() const;
    void enter();void populate();void arrange();
    void refreshFlags();
    Campaign campaign_;
    std::array<QImage,3> maps_{};
    mnm::ui::MenuSpriteSheet flags_;
    mnm::ui::RealmViewerVisuals visuals_;
    QTimer* animationTimer_=nullptr;
    quint64 animationTick_=0,pathTick_=0;
    QString hoveredRegion_;
    std::array<QPushButton*,3> realms_{};
    QVector<mnm::ui::SpriteButton*> regions_;
    std::array<QPushButton*,4> auxiliary_{};
    mnm::ui::SpriteButton* enter_=nullptr;
    mnm::ui::SpriteButton* cancel_=nullptr;
    QLabel* heading_=nullptr;
};
Q_DECLARE_METATYPE(RealmViewerWidget::Request)
Q_DECLARE_METATYPE(RealmViewerWidget::Realm)
