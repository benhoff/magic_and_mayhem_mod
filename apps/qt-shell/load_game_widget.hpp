#pragma once
#include <QImage>
#include <QVector>
#include <QWidget>
class QLabel;
class QListWidget;
class QLineEdit;
class QPushButton;
class LoadGameWidget final : public QWidget {
    Q_OBJECT
public:
    struct Save { QString id; QString fileName; };
    explicit LoadGameWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    bool setSaves(const QVector<Save>& saves, const QString& selectedId = QString(), QString* error = nullptr);
    QString selectedSaveId() const;
    QRect contentRect() const;
    void focusSelection();
signals:
    void loadRequested(const QString& id);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    void confirmSelection();
    void selectFileName(const QString& name);
    QImage background_;
    QLabel* heading_ = nullptr;
    QListWidget* saves_ = nullptr;
    QLineEdit* fileName_ = nullptr;
    QPushButton* load_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QRect headingRectangle_, listRectangle_, editRectangle_, loadRectangle_, cancelRectangle_;
};
