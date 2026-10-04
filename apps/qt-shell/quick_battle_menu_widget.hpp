#pragma once
#include <QImage>
#include <QWidget>
#include <array>

class QPushButton;
class QLabel;
class QuickBattleMenuWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Action { CreateMultiplayer, JoinMultiplayer, CreateSinglePlayer, Cancel };
    Q_ENUM(Action)
    explicit QuickBattleMenuWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& installationRoot, QString* error = nullptr);
    QRect contentRect() const;
    void focusFirstAction();
signals:
    void actionRequested(QuickBattleMenuWidget::Action action);
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    QImage background_;
    QLabel* heading_ = nullptr;
    QRect headingRectangle_{100, 75, 600, 50};
    std::array<QPushButton*, 4> buttons_{};
    std::array<QRect, 4> rectangles_{{{200,200,400,50}, {200,275,400,50}, {200,350,400,50}, {280,475,240,50}}};
};
