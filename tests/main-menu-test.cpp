#include "main_menu_widget.hpp"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QKeyEvent>
#include <QPushButton>
#include <QTemporaryDir>
#include <cstdio>
#include <stdexcept>

static void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
static void write(const QString& path, const QByteArray& bytes) {
    QFile file(path);
    require(file.open(QIODevice::WriteOnly) && file.write(bytes) == bytes.size(), "fixture write");
}
int main(int argc, char** argv) {
    QApplication app(argc, argv);
    try {
        QTemporaryDir directory;
        require(directory.isValid(), "temporary fixture");
        const auto root = directory.path();
        require(QDir().mkpath(root + "/Interface/MainScreen/800x600") && QDir().mkpath(root + "/CFG"), "fixture directories");
        QImage image(800, 600, QImage::Format_RGB32);
        image.fill(QColor(64, 96, 128));
        require(image.save(root + "/Interface/MainScreen/800x600/Test 800-600.JPG", "JPG"), "fixture JPEG");
        QByteArray layout("[GLOBALS]\r\nBackgroundFile=\"Test\"\r\n");
        QByteArray strings("[STRINGS]\r\n");
        for (int i = 0; i < 6; ++i) {
            layout += QString("[TEXTBUTTON_%1]\r\nRect2=100,%2,700,%3 ; inline comment\r\nText=%4\r\n")
                .arg(i + 1).arg(260 + i * 45).arg(300 + i * 45).arg(10 + i).toLatin1();
            strings += QString("STR_%1=Fixture %2\r\n").arg(10 + i).arg(i).toLatin1();
        }
        layout += "[TEXT_1]\r\nRect2=600,565,790,590\r\n";
        const auto cfg = root + "/Interface/MainScreen/screen (MainMenu).cfg";
        write(cfg, layout);
        write(root + "/CFG/interface screens text.cfg", strings);
        MainMenuWidget menu;
        QString error;
        require(menu.loadAssets(root, &error) && error.isEmpty(), "load valid assets");
        menu.show();
        app.processEvents();
        auto button = [&](int i) { return menu.findChild<QPushButton*>(QString("mainMenuAction%1").arg(i)); };
        require(button(5)->isHidden(), "conditional button hidden");
        menu.setCommandLineBattleVisible(true);
        app.processEvents();
        require(button(5)->isVisible(), "conditional button shown");
        int count = 0;
        MainMenuWidget::Action action = MainMenuWidget::Action::Quit;
        QObject::connect(&menu, &MainMenuWidget::actionRequested, &menu, [&](MainMenuWidget::Action value) { action = value; ++count; });
        for (int i = 0; i < 6; ++i) {
            require(button(i)->text() == QString("Fixture %1").arg(i), "configured label lookup");
            button(i)->click();
            require(count == i + 1 && int(action) == i, "semantic signal mapping");
        }
        button(0)->setFocus();
        QKeyEvent down(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
        QKeyEvent up(QEvent::KeyRelease, Qt::Key_Space, Qt::NoModifier);
        QApplication::sendEvent(button(0), &down);
        QApplication::sendEvent(button(0), &up);
        require(count == 7 && action == MainMenuWidget::Action::NewGame, "keyboard activation");
        menu.resize(1200, 600);
        app.processEvents();
        require(menu.contentRect() == QRect(200, 0, 800, 600), "wide letterboxing");
        require(button(0)->geometry() == QRect(300, 260, 600, 40), "configured geometry with offset");
        auto captured = menu.grab().toImage();
        require(captured.pixelColor(20, 20) == QColor(Qt::black), "black letterbox");
        const auto color = captured.pixelColor(220, 20);
        require(qAbs(color.red() - 64) < 4 && qAbs(color.green() - 96) < 4 && qAbs(color.blue() - 128) < 4, "background presentation");
        menu.resize(400, 600);
        app.processEvents();
        require(menu.contentRect() == QRect(0, 150, 400, 300), "tall letterboxing");
        require(button(0)->geometry() == QRect(50, 280, 300, 20), "scaled geometry");
        write(cfg, QByteArray(layout).replace("100,260,700,300", "100,260,900,300"));
        require(!menu.loadAssets(root, &error) && !error.isEmpty(), "reject invalid rectangle");
        require(button(0)->text() == "Fixture 0" && button(0)->geometry() == QRect(50, 280, 300, 20), "transactional reload");
        write(cfg, layout);
        write(root + "/Interface/MainScreen/800x600/Test 800-600.JPG", "invalid JPEG");
        require(!menu.loadAssets(root, &error) && !error.isEmpty(), "reject corrupt image");
        require(!menu.loadAssets(root + "/missing", &error) && !error.isEmpty(), "missing installation diagnostic");
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Main menu test: %s\n", error.what());
        return 1;
    }
}
