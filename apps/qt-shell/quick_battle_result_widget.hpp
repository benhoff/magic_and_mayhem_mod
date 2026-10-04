#pragma once
#include <QImage>
#include <QWidget>
#include <array>
class QLabel;
class QPushButton;
class QuickBattleResultWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Action { Spectate, Continue, Quit };
    Q_ENUM(Action)
    enum class PrimaryAction { None, Spectate, Continue };
    struct Player {
        bool active = false;
        QString name;
        QString portraitText;
        QString kills;
        QString deaths;
        QString handicapBonus;
        QString score;
    };
    struct Results {
        std::array<Player, 4> players{};
        PrimaryAction primaryAction = PrimaryAction::None;
        bool canQuit = true;
    };
    explicit QuickBattleResultWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    void setResults(const Results& results);
    void focusFirstAction();
    QRect contentRect() const;
signals:
    void actionRequested(QuickBattleResultWidget::Action action);
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    Results results_;
    QImage background_;
    std::array<QLabel*, 26> labels_{};
    std::array<QRect, 26> rectangles_{};
    std::array<int, 26> alignments_{};
    std::array<int, 26> fontSizes_{};
    std::array<QLabel*, 4> portraits_{};
    std::array<QRect, 4> portraitRectangles_{};
    std::array<QPushButton*, 3> buttons_{};
    std::array<QRect, 3> buttonRectangles_{};
};
