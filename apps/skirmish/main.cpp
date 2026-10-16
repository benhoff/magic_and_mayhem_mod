#include "window.hpp"
#include <QApplication>
#include <QCommandLineParser>
#include <iostream>
#ifdef MNM_SKIRMISH_WINDOW_TEST
#include "../../tests/skirmish-window-driver.hpp"
#endif
#ifdef MNM_CAMPAIGN_WINDOW_TEST
#include "../../tests/campaign-window-driver.hpp"
#endif
int main(int argc, char **argv) try {
#if defined(MNM_SKIRMISH_WINDOW_TEST) || defined(MNM_CAMPAIGN_WINDOW_TEST)
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeDialogs);
#endif
    QApplication app(argc, argv);
    QCommandLineParser p;
    p.addHelpOption();
    p.addOption({"campaign", "Start a new native campaign"});
    p.addOption({"load-campaign", "Load a native campaign checkpoint", "file"});
    p.addOption({"load", "Load a native skirmish checkpoint", "file"});
#if defined(MNM_SKIRMISH_WINDOW_TEST) || defined(MNM_CAMPAIGN_WINDOW_TEST)
    p.addOption({"test-script", "Bounded test-only window script", "file"});
    p.addOption({"test-output", "New test-only output directory", "directory"});
#endif
    p.process(app);
    mnm::skirmish::Controller c;
    if (int(p.isSet("load")) + int(p.isSet("campaign")) + int(p.isSet("load-campaign")) > 1)
        throw std::invalid_argument("Choose one startup mode");
    if (p.isSet("campaign"))
        c.newCampaign();
    if (p.isSet("load-campaign"))
        c.loadCampaign(p.value("load-campaign").toStdString());
    if (p.isSet("load"))
        c.load(p.value("load").toStdString());
    mnm::skirmish::Window window(c);
    window.show();
#ifdef MNM_SKIRMISH_WINDOW_TEST
    auto driver =
        mnm::skirmish::test::start(p.value("test-script"), p.value("test-output"), window, c);
#endif
#ifdef MNM_CAMPAIGN_WINDOW_TEST
    mnm::skirmish::test::campaignWindow(p.value("test-script"), p.value("test-output"), window, c);
#endif
    return app.exec();
} catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
}
