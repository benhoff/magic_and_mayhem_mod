#pragma once
#include <QImage>
#include <QVector>
#include <QWidget>
class QLabel;
class QListWidget;
class QPushButton;
class MultiplayerGameSelectionWidget final : public QWidget {
    Q_OBJECT
public:
    struct Session { QString id; QString name; };
    explicit MultiplayerGameSelectionWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    // Caller-owned identifiers; no session discovery or engine handles here.
    bool setSessions(const QVector<Session>& sessions, const QString& selectedId = QString(), QString* error = nullptr);
    QString selectedSessionId() const;
    QRect contentRect() const;
    void focusSelection();
signals:
    void sessionSelected(const QString& id);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    void confirmSelection();
    QImage background_;
    QLabel* heading_ = nullptr;
    QListWidget* sessions_ = nullptr;
    QPushButton* ok_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect headingRectangle_, listRectangle_, okRectangle_, cancelRectangle_;
};
