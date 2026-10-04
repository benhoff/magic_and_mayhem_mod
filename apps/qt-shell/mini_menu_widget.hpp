#pragma once
#include <QImage>
#include <QWidget>
#include <array>

class QPushButton;
class MiniMenuWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Mode { Campaign, Battle };
    Q_ENUM(Mode)
    enum class Action { LoadGame, SaveGame, Preferences, QuitGame, QuitBattle, Cancel };
    Q_ENUM(Action)
    explicit MiniMenuWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& installationRoot, QString* error = nullptr);
    void setMode(Mode mode);
    Mode mode() const { return mode_; }
    QRect contentRect() const;
    void focusFirstAction();
signals:
    void actionRequested(MiniMenuWidget::Action action);
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    Mode mode_ = Mode::Campaign;
    QImage background_;
    std::array<QPushButton*, 8> buttons_{};
    std::array<QRect, 8> rectangles_{};
};
