#pragma once
#include <QImage>
#include "menu_sprites.hpp"
#include <QMetaType>
#include <QWidget>
#include <QVector>
#include <array>
class QLabel;
class QPushButton;
class QSlider;
class QPlainTextEdit;
class QLineEdit;
class MultiplayerLobbyWidget final : public QWidget {
    Q_OBJECT
public:
    // Native presentation order follows visible rows, not recovered engine fields.
    enum class Rule { Mana, Health, MagicItems, SelectionTime, LawTalismans,
        NeutralTalismans, ChaosTalismans, GameTime, Lives, PlacesOfPower,
        ManaSprites, Artefacts, ControlLimit };
    Q_ENUM(Rule)
    enum class Mode { Host, Join };
    Q_ENUM(Mode)
    struct Message { QString sender, text; };
    struct Player {
        bool active = false;
        QString name, portraitId, portraitText, colourId, colourText;
        int handicap = 0;
        int portraitIndex = -1, colourIndex = -1; // Explicit artwork selections; -1 retains text.
    };
    struct Lobby {
        QString sessionId, gameName, mapId, mapName;
        int localSlot = 0;
        bool localReady = false;
        std::array<Player,4> players{};
        std::array<int,13> values{50,100,0,30,0,0,0,0,1,0,0,0,0};
    };
    explicit MultiplayerLobbyWidget(Mode mode, QWidget* parent = nullptr);
    Mode mode() const { return mode_; }
    bool setMessages(const QVector<Message>& messages, QString* error = nullptr);
    bool appendMessage(const Message& message, QString* error = nullptr);
    QVector<Message> messages() const { return messages_; }
    void clearChat();
    bool loadAssets(const QString& root, QString* error = nullptr);
    bool setLobby(const Lobby& setup, QString* error = nullptr);
    Lobby lobby() const;
    QRect contentRect() const;
    void focusFirstControl();
signals:
    void startRequested(const MultiplayerLobbyWidget::Lobby& setup);
    void readyRequested(bool ready);
    void chatRequested(const QString& text);
    void mapRequested();
    void playerChangeRequested(int slot);
    void colourChangeRequested(int slot);
    void playerRemovalRequested(int slot);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void keyPressEvent(QKeyEvent*) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
private:
    bool validLobby(const Lobby&, const std::array<int,17>& minimum,
                    const std::array<int,17>& maximum, const std::array<int,17>& step) const;
    void populate();
    void start();
    void arrange();
    const Mode mode_;
    Lobby lobby_;
    QVector<Message> messages_;
    QPlainTextEdit* chat_ = nullptr;
    QLineEdit* composer_ = nullptr;
    QRect chatRectangle_, composerRectangle_;
    void sendChat();
    void displayMessages();
    QImage background_;
    mnm::ui::MenuSpriteSheet sprites_;
    QString noPlayer_ = "No Player";
    std::array<QLabel*,40> labels_{};
    std::array<QRect,40> labelRectangles_{};
    std::array<QSlider*,17> sliders_{};
    std::array<QRect,17> sliderRectangles_{};
    std::array<int,17> minimum_{50,100,0,30,0,0,0,0,1,0,0,0,0,0,0,0,0};
    std::array<int,17> maximum_{200,800,21,3000,7,7,7,240,20,15,50,50,30,50,50,50,50};
    std::array<int,17> step_{10,20,1,10,1,1,1,5,1,1,1,5,1,5,5,5,5};
    std::array<QPushButton*,4> portraits_{}, colours_{};
    std::array<QRect,4> portraitRectangles_{},colourRectangles_{};
    std::array<QPushButton*,3> remove_{};
    std::array<QRect,3> removeRectangles_{};
    QPushButton* cancel_ = nullptr;
    QPushButton* start_ = nullptr;
    QPushButton* map_ = nullptr;
    QRect cancelRectangle_,startRectangle_,mapRectangle_;
};
Q_DECLARE_METATYPE(MultiplayerLobbyWidget::Lobby)
