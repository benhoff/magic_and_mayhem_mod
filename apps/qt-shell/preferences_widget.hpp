#pragma once
#include <QImage>
#include <QMetaType>
#include <QWidget>
#include <array>
class QButtonGroup;
class QLabel;
class QPushButton;
class QRadioButton;
class QSlider;
class PreferencesWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Resolution { High, Low };
    Q_ENUM(Resolution)
    enum class Animation { Full, Cut };
    Q_ENUM(Animation)
    enum class Speed { Fast, Medium, Slow };
    Q_ENUM(Speed)
    struct Settings {
        int musicLevel = -1500;
        int soundLevel = -1000;
        Resolution resolution = Resolution::High;
        Animation animation = Animation::Full;
        Speed dialogueSpeed = Speed::Medium;
        Speed gameSpeed = Speed::Medium;
        bool borderPicture = true;
        bool operator==(const Settings& other) const {
            return musicLevel==other.musicLevel && soundLevel==other.soundLevel && resolution==other.resolution &&
                animation==other.animation && dialogueSpeed==other.dialogueSpeed && gameSpeed==other.gameSpeed && borderPicture==other.borderPicture;
        }
    };
    struct ControlPolicy {
        std::array<int,2> minimum{0,-2500},maximum{15,0},step{1,50};
        std::array<bool,12> radios{};std::array<bool,2> sliders{};
        bool canApply=false,canCancel=false;
    };
    bool setEngineSettings(const Settings&,const ControlPolicy&,QString* error=nullptr);
    void setControlAvailability(const ControlPolicy&);
    explicit PreferencesWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    bool setSettings(const Settings& settings, QString* error = nullptr);
    Settings settings() const { return accepted_; }
    Settings draftSettings() const;
    QRect contentRect() const;
    void focusFirstControl();
signals:
    void settingsApplied(const PreferencesWidget::Settings& settings);
    void cancelled();
    void audioLevelChanged(int channel,int value);
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    bool validSettings(const Settings& settings, const std::array<int,2>& minimum, const std::array<int,2>& maximum) const;
    void populate(const Settings& settings);
    void apply();
    void cancel();
    void arrange();
    Settings accepted_;
    bool populating_=false,engineMode_=false;
    QImage background_;
    std::array<QLabel*,11> labels_{};
    std::array<QRect,11> labelRectangles_{};
    std::array<QRadioButton*,12> radios_{};
    std::array<QRect,12> radioRectangles_{};
    std::array<QButtonGroup*,5> groups_{};
    std::array<QSlider*,2> sliders_{};
    std::array<QRect,2> sliderRectangles_{};
    std::array<int,2> minimum_{-5000,-5000},maximum_{0,0};
    QPushButton* ok_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect okRectangle_,cancelRectangle_;
};
Q_DECLARE_METATYPE(PreferencesWidget::Settings)
