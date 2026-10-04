#pragma once
#include "menu_sprites.hpp"
#include <QMetaType>
#include <QWidget>
#include <array>
class QLabel;
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
private:
    static bool valid(const Campaign&);
    const Region* selected() const;
    void enter();void populate();void arrange();
    Campaign campaign_;
    std::array<QImage,3> maps_{};
    mnm::ui::MenuSpriteSheet flags_;
    std::array<QPushButton*,3> realms_{};
    QVector<mnm::ui::SpriteButton*> regions_;
    std::array<QPushButton*,4> auxiliary_{};
    mnm::ui::SpriteButton* enter_=nullptr;
    mnm::ui::SpriteButton* cancel_=nullptr;
    QLabel* heading_=nullptr;
};
Q_DECLARE_METATYPE(RealmViewerWidget::Request)
Q_DECLARE_METATYPE(RealmViewerWidget::Realm)
