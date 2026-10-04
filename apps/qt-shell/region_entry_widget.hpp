#pragma once
#include <QImage>
#include <QMetaType>
#include <QWidget>
#include <array>
class QButtonGroup;
class QLabel;
class QPushButton;
class QRadioButton;
class RegionEntryWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Difficulty { Initiate, Apprentice, Adept, Wizard };
    Q_ENUM(Difficulty)
    enum class Realm { Celtic, Greek, Medieval };
    Q_ENUM(Realm)
    enum class AuxiliaryAction { Grimoire, Spellbox, Character };
    Q_ENUM(AuxiliaryAction)
    struct Region {
        QString id, name;
        Realm artworkRealm = Realm::Celtic;
        int artworkNumber = 1;
        Difficulty difficulty = Difficulty::Initiate;
        bool enterAvailable = true;
        std::array<bool,3> auxiliaryAvailable{true,true,true};
    };
    struct Request { QString regionId; Difficulty difficulty = Difficulty::Initiate; };
    explicit RegionEntryWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    // Artwork selection is independent of the caller-owned region ID.
    // Once loaded, this changes model/art transactionally using the same root.
    bool setRegion(const Region& region, QString* error = nullptr);
    Region region() const { return region_; }
    QRect contentRect() const;
    void focusFirstControl();
signals:
    void enterRequested(const RegionEntryWidget::Request& request);
    void auxiliaryRequested(RegionEntryWidget::AuxiliaryAction action);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    static bool validRegion(const Region& region);
    bool load(const QString& root, const Region& region, QString* error);
    void populate();
    void enter();
    void arrange();
    Region region_;
    QString assetRoot_;
    QString defaultHeading_ = "Current Region";
    QImage background_;
    QLabel* heading_ = nullptr;
    QRect headingRectangle_;
    QButtonGroup* difficulties_ = nullptr;
    std::array<QRadioButton*,4> radios_{};
    std::array<QRect,4> radioRectangles_{};
    std::array<QPushButton*,3> auxiliary_{};
    std::array<QRect,3> auxiliaryRectangles_{};
    QPushButton* enter_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect enterRectangle_,cancelRectangle_;
};
Q_DECLARE_METATYPE(RegionEntryWidget::Request)
