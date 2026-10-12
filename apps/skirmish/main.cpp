#include "window.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <iostream>
#ifdef MNM_SKIRMISH_WINDOW_TEST
#include "../../tests/skirmish-window-driver.hpp"
#endif
int main(int argc, char **argv) try {
#ifdef MNM_SKIRMISH_WINDOW_TEST
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif
    QApplication app(argc, argv);
    QCommandLineParser p;
    p.addHelpOption();
    p.addOption({"load", "Load a native skirmish checkpoint", "file"});
#ifdef MNM_SKIRMISH_WINDOW_TEST
    p.addOption({"test-script", "Bounded test-only window script", "file"});
    p.addOption({"test-output", "New test-only output directory", "directory"});
#endif
    p.process(app);
    mnm::skirmish::Controller c;
    if (p.isSet("load"))
        c.load(p.value("load").toStdString());
    mnm::skirmish::Window window(c);
    window.show();
#ifdef MNM_SKIRMISH_WINDOW_TEST
    auto driver =
        mnm::skirmish::test::start(p.value("test-script"), p.value("test-output"), window, c);
#endif
    return app.exec();
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
