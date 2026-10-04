#pragma once
#include <QImage>
#include <QMetaType>
#include <QWidget>
#include <array>
class QButtonGroup;
class QLabel;
class QLineEdit;
class QPushButton;
class QRadioButton;
class MultiplayerSetupWidget final : public QWidget {
    Q_OBJECT
public:
    enum class Mode { Join, Create };
    Q_ENUM(Mode)
    enum class Transport { IpxSpx, TcpIp, NullModem };
    Q_ENUM(Transport)
    struct Form {
        QString gameName;
        QString userName;
        Transport transport = Transport::TcpIp;
    };
    struct Request {
        Mode mode = Mode::Join;
        Transport transport = Transport::TcpIp;
        QString gameName;
        QString userName;
    };
    explicit MultiplayerSetupWidget(Mode mode, QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    bool setForm(const Form& form, QString* error = nullptr);
    Form form() const;
    Mode mode() const { return mode_; }
    QRect contentRect() const;
    void focusFirstField();
signals:
    void requestSubmitted(const MultiplayerSetupWidget::Request& request);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void updateAvailability();
    void submit();
    void arrange();
    const Mode mode_;
    QImage background_;
    std::array<QLabel*,4> labels_{};
    std::array<QRect,4> labelRectangles_{};
    std::array<QLineEdit*,2> edits_{};
    std::array<QRect,2> editRectangles_{};
    std::array<QRadioButton*,3> radios_{};
    std::array<QRect,3> radioRectangles_{};
    QButtonGroup* transports_ = nullptr;
    QPushButton* ok_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect okRectangle_,cancelRectangle_;
};
Q_DECLARE_METATYPE(MultiplayerSetupWidget::Request)
