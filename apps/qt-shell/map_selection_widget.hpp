#pragma once
#include <QImage>
#include <QVector>
#include <QWidget>
class QLabel;
class QListWidget;
class QPushButton;
class MapSelectionWidget final : public QWidget {
    Q_OBJECT
public:
    struct Map { QString id; QString name; };
    explicit MapSelectionWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    // Caller-owned identifiers; no file enumeration or path interpretation here.
    bool setMaps(const QVector<Map>& maps, const QString& selectedId = QString(), QString* error = nullptr);
    QString selectedMapId() const;
    QRect contentRect() const;
    void focusSelection();
signals:
    void mapSelected(const QString& id);
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
    QListWidget* maps_ = nullptr;
    QPushButton* ok_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect headingRectangle_, listRectangle_, okRectangle_, cancelRectangle_;
};
