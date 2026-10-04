#pragma once
#include <QImage>
#include <QWidget>
#include <array>

class QPushButton;
class QLabel;

// Native presentation only. The controller owns engine actions and transitions.
class MainMenuWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Action { NewGame, LoadGame, QuickBattle, Preferences, Quit, CommandLineBattle };
    Q_ENUM(Action)
    explicit MainMenuWidget(QWidget* parent = nullptr);
    // Transactional, bounded reads through AssetStore; failure retains prior UI.
    bool loadAssets(const QString& installationRoot, QString* error = nullptr);
    void setCommandLineBattleVisible(bool visible);
    void setVersionText(const QString& text);
    QRect contentRect() const;
signals:
    void actionRequested(MainMenuWidget::Action action);
protected:
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    void arrange();
    QImage background_;
    std::array<QPushButton*, 6> buttons_{};
    std::array<QRect, 6> rectangles_{};
    QRect versionRectangle_{600, 565, 190, 25};
    QLabel* version_ = nullptr;
};
