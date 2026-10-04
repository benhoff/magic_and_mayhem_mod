#pragma once
#include <QImage>
#include <QWidget>
#include <array>
class QLabel;
class QPushButton;

class BattleResultWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Outcome { Victory, Defeat };
    Q_ENUM(Outcome)
    // Display values only; interpretation and engine ownership stay outside UI.
    struct Reward { QString achievement; QString points; };
    struct Results {
        std::array<Reward, 7> rewards{};
        QString title;
        QString totalPoints;
        QString maximumPoints;
        QString summary;
        QString rating;
    };
    explicit BattleResultWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, Outcome outcome, QString* error = nullptr);
    void setResults(const Results& results);
    Outcome outcome() const { return outcome_; }
    QRect contentRect() const;
    void focusContinue();
signals:
    void continueRequested();
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    Results results_;
    Outcome outcome_ = Outcome::Victory;
    QImage background_;
    std::array<QLabel*, 21> labels_{};
    std::array<QRect, 21> rectangles_{};
    std::array<int, 21> alignments_{};
    std::array<int, 21> fontSizes_{};
    QLabel* title_ = nullptr;
    QString ratingLabel_;
    QPushButton* continue_ = nullptr;
    QRect continueRect_;
};
