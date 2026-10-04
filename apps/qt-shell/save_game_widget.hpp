#pragma once
#include <QImage>
#include <QPointer>
#include <QVector>
#include <QWidget>
class QMessageBox;
class QLabel;
class QListWidget;
class QLineEdit;
class QPushButton;
class SaveGameWidget final : public QWidget {
    Q_OBJECT
public:
    struct Save { QString id; QString fileName; };
    explicit SaveGameWidget(QWidget* parent = nullptr);
    bool loadAssets(const QString& root, QString* error = nullptr);
    bool setSaves(const QVector<Save>& saves, const QString& selectedId = QString(), QString* error = nullptr);
    QString selectedSaveId() const;
    QRect contentRect() const;
    void focusSelection();
signals:
    void saveRequested(const QString& fileName, const QString& existingId);
    void deleteRequested(const QString& id);
    void cancelled();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void hideEvent(QHideEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
private:
    void arrange();
    void requestSave();
    void requestDelete();
    void confirmAction(bool deleting);
    void discardConfirmation();
    void updateActions();
    void selectFileName(const QString& name);
    QImage background_;
    QLabel* heading_ = nullptr;
    QListWidget* saves_ = nullptr;
    QLineEdit* fileName_ = nullptr;
    QPushButton* save_ = nullptr;
    QPushButton* cancel_ = nullptr;
    QPushButton* delete_ = nullptr;
    QPointer<QMessageBox> confirmation_;
    quint64 revision_ = 0;
    QRect headingRectangle_, listRectangle_, editRectangle_, saveRectangle_, cancelRectangle_, deleteRectangle_;
};
