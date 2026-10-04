#pragma once
#include <QImage>
#include <QMetaType>
#include <QVector>
#include <QWidget>
#include <array>
class QLabel;
class QProgressBar;
class QPushButton;
class CharacterScreenWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Attribute { Mana, Health, ControlLimit, LawTalismans, NeutralTalismans, ChaosTalismans };
    Q_ENUM(Attribute)
    struct Stat {
        int value = 0;
        // Caller-supplied cost for each successive purchase from this snapshot.
        QVector<int> upgradeCosts;
        bool upgradeAvailable = true;
    };
    struct Character {
        QString id, name, portraitText, rating;
        int experiencePoints = 0;
        std::array<Stat,6> stats{};
    };
    struct Request {
        QString characterId;
        int remainingExperience = 0;
        std::array<int,6> values{}, purchasedIncrements{};
    };
    explicit CharacterScreenWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    bool setCharacter(const Character& character, QString* error = nullptr);
    Character character() const { return accepted_; }
    Request draftRequest() const;
    QRect contentRect() const;
    void focusFirstControl();
signals:
    void characterAccepted(const CharacterScreenWidget::Request& request);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    bool validCharacter(const Character&, const std::array<int,6>& minimum,
        const std::array<int,6>& maximum, const std::array<int,6>& increment) const;
    void change(int attribute, int direction);
    void accept();
    void cancel();
    void populate();
    void arrange();
    Character accepted_;
    std::array<int,6> purchased_{};
    std::array<int,6> minimum_{},maximum_{200,800,40,7,7,7},increment_{5,20,1,1,1,1};
    QImage background_;
    std::array<QLabel*,17> labels_{};
    std::array<QRect,17> labelRectangles_{};
    QLabel* portrait_ = nullptr;
    std::array<QProgressBar*,3> bars_{};
    std::array<QRect,3> barRectangles_{};
    std::array<QLabel*,3> talismans_{};
    std::array<QRect,3> talismanRectangles_{};
    std::array<QPushButton*,12> buttons_{};
    std::array<QRect,12> buttonRectangles_{};
    QPushButton* ok_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect okRectangle_,cancelRectangle_;
};
Q_DECLARE_METATYPE(CharacterScreenWidget::Request)
