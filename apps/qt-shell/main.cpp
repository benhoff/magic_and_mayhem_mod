#include "window_host.hpp"
#include "gl_viewport.hpp"
#include "frame_stream.hpp"
#include "blit.hpp"
#include <QSurfaceFormat>
#include <QUuid>
#include <memory>
#include <QApplication>
#include <QCloseEvent>
#include <QCommandLineParser>
#include <QDir>
#include <QDockWidget>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QLabel>
#include <QMainWindow>
#include <QPlainTextEdit>
#include <QProcess>
#include <QProcessEnvironment>
#include <QPushButton>
#include <QStatusBar>
#include <QTimer>
#include <QToolBar>
#include <QVBoxLayout>
#include <QWindow>
#include <cstdio>
#include <exception>

class Shell final:public QMainWindow {
public:
    explicit Shell(QString repository,bool opengl=false,bool captureDraws=false):repo_(std::move(repository)),opengl_(opengl),captureDraws_(captureDraws) {
        setWindowTitle("Magic & Mayhem Workshop");resize(1100,850);
        viewport_=new QWidget(this);layout_=new QVBoxLayout(viewport_);
        layout_->setContentsMargins(0,0,0,0);viewport_->setMinimumSize(800,600);
        placeholder_=new QLabel("Launch the game to open its viewport.",viewport_);
        placeholder_->setAlignment(Qt::AlignCenter);layout_->addWidget(placeholder_);setCentralWidget(viewport_);
        if(opengl_){gl_=new GlViewport(viewport_);layout_->addWidget(gl_);gl_->hide();}
        auto* toolbar=addToolBar("Game");toolbar->setMovable(false);
        launch_=new QPushButton("Launch game",this);toolbar->addWidget(launch_);
        check_=new QPushButton("Check installation",this);toolbar->addWidget(check_);
        detach_=new QPushButton("Detach game",this);toolbar->addWidget(detach_);detach_->setEnabled(false);
        retry_=new QPushButton("Attach game",this);toolbar->addWidget(retry_);retry_->setEnabled(false);
        if(opengl_){detach_->hide();retry_->hide();}
        auto* dock=new QDockWidget("Launch log",this);log_=new QPlainTextEdit(dock);
        log_->setReadOnly(true);log_->setMaximumBlockCount(2000);dock->setWidget(log_);addDockWidget(Qt::BottomDockWidgetArea,dock);
        process_.setProcessChannelMode(QProcess::MergedChannels);
        connect(launch_,&QPushButton::clicked,this,[this]{start(false);});
        connect(check_,&QPushButton::clicked,this,[this]{start(true);});
        connect(detach_,&QPushButton::clicked,this,[this]{detach();statusBar()->showMessage("Game detached; use Attach game to restore the viewport.");});
        connect(retry_,&QPushButton::clicked,this,[this]{elapsed_.restart();poll_.start();});
        connect(&process_,&QProcess::readyReadStandardOutput,this,[this]{
            auto cursor=log_->textCursor();cursor.movePosition(QTextCursor::End);
            cursor.insertText(QString::fromLocal8Bit(process_.readAllStandardOutput()));log_->setTextCursor(cursor);
        });
        connect(&process_,&QProcess::errorOccurred,this,[this](QProcess::ProcessError error){
            statusBar()->showMessage("Launcher error: "+process_.errorString());
            if(error==QProcess::FailedToStart)finished();
        });
        connect(&process_,qOverload<int,QProcess::ExitStatus>(&QProcess::finished),this,[this](int code,QProcess::ExitStatus){
            finished();statusBar()->showMessage(QString("Launcher exited with status %1.").arg(code));
        });
        poll_.setInterval(250);connect(&poll_,&QTimer::timeout,this,[this]{discover();});
        frames_.setInterval(16);connect(&frames_,&QTimer::timeout,this,[this]{
            if(!stream_)return;
            if(!gl_->error().isEmpty()){frames_.stop();statusBar()->showMessage("OpenGL initialization failed: "+gl_->error());return;}
            auto frame=stream_->nextFrame();
            if(!frame.isNull()){gl_->setFrame(std::move(frame));placeholder_->hide();gl_->show();
                statusBar()->showMessage("OpenGL presentation active. Use the separate game window for input.");}
            else if(stream_->status()==2 || stream_->status()==3)
                statusBar()->showMessage("Frame capture rejected a surface; check rendering evidence before replacing the native viewport.");
        });
        if(!host_.available() && !opengl_){
            launch_->setEnabled(false);placeholder_->setText("Game embedding requires an X11 session or XWayland.\nStart this shell with QT_QPA_PLATFORM=xcb.");
            statusBar()->showMessage("Viewport unavailable on this display backend.");
        }else statusBar()->showMessage("Ready. The game starts only when you click Launch game.");
    }
    bool attach(xcb_window_t id){
        if(foreign_ || !host_.exists(id))return false;
        auto* window=QWindow::fromWinId(WId(id));
        if(!window)return false;
        container_=QWidget::createWindowContainer(window,viewport_);
        foreign_=window;windowId_=id;container_->setFocusPolicy(Qt::StrongFocus);
        layout_->addWidget(container_);placeholder_->hide();container_->show();
        poll_.stop();detach_->setEnabled(true);retry_->setEnabled(false);
        statusBar()->showMessage("Game attached. Click the viewport to focus game input.");return true;
    }
    void detach(){
        poll_.stop();
        if(!foreign_)return;
        if(host_.exists(windowId_)){foreign_->setParent(nullptr);foreign_->show();}
        layout_->removeWidget(container_);delete container_;container_=nullptr;foreign_=nullptr;windowId_=0;
        placeholder_->show();detach_->setEnabled(false);
        retry_->setEnabled(process_.state()!=QProcess::NotRunning && !checking_);
    }
    WindowHost& host(){return host_;}
protected:
    void closeEvent(QCloseEvent* event) override {
        if(process_.state()!=QProcess::NotRunning){
            statusBar()->showMessage("Exit the game and wait for the launcher to finish before closing the shell.");event->ignore();return;
        }
        detach();event->accept();
    }
private:
    void start(bool check){
        if(process_.state()!=QProcess::NotRunning)return;
        const QString launcher=QDir(repo_).filePath(opengl_ && !check?"tools/run-opengl-game.py":"tools/run-game.sh");
        if(!QFileInfo(launcher).isExecutable()){statusBar()->showMessage("Cannot find tools/run-game.sh. Set --repo to the repository directory.");return;}
        checking_=check;excluded_=host_.windows();launch_->setEnabled(false);check_->setEnabled(false);
        auto env=QProcessEnvironment::systemEnvironment();
        // An inherited custom runner could bypass the windowed Wine launch.
        env.remove("MNM_RUNNER");env.insert("WINEDEBUG",env.value("WINEDEBUG","fixme-all"));
        process_.setProcessEnvironment(env);process_.setWorkingDirectory(repo_);
        QStringList arguments{check?"check":"launch","--no-gamescope","--window-size","800x600","--prefix",QDir(repo_).filePath("working/wineprefix-x86_64")};
        log_->appendPlainText(check?"Checking installation…":"Starting game…");
        if(opengl_ && !check){
            const QString directory=QDir(repo_).filePath("working/runtime/render");
            QDir().mkpath(directory);const QString path=directory+"/frame-"+QUuid::createUuid().toString(QUuid::WithoutBraces)+".bin";
            stream_=std::make_unique<FrameStream>();
            if(!stream_->create(path)){finished();statusBar()->showMessage(stream_->error());return;}
            arguments={"--stream",path};frames_.start();
            if(captureDraws_)arguments.append("--capture-draws");
            gl_->hide();placeholder_->show();
            placeholder_->setText("Waiting for the first DirectDraw frame…");
        }
        process_.start(launcher,arguments);
        if(!check && !opengl_){elapsed_.restart();poll_.start();statusBar()->showMessage("Waiting for the game window…");}
    }
    void discover(){
        const auto candidates=host_.desktops(excluded_);
        if(candidates.size()==1 && attach(candidates.front()))return;
        if(candidates.size()>1){poll_.stop();retry_->setEnabled(true);statusBar()->showMessage("Multiple new game desktops found. Close extra desktops, then click Attach game.");return;}
        if(elapsed_.elapsed()>30000){poll_.stop();retry_->setEnabled(true);statusBar()->showMessage("Window not found yet. Check the launch log, then click Attach game.");}
    }
    void finished(){
        detach();poll_.stop();frames_.stop();retry_->setEnabled(false);checking_=false;
        launch_->setEnabled(opengl_ || host_.available());check_->setEnabled(true);
    }
    QString repo_;bool opengl_=false,captureDraws_=false;GlViewport* gl_=nullptr;std::unique_ptr<FrameStream> stream_;QTimer frames_;WindowHost host_;QProcess process_;QTimer poll_;QElapsedTimer elapsed_;
    QSet<xcb_window_t> excluded_;bool checking_=false;
    QWidget* viewport_=nullptr;QVBoxLayout* layout_=nullptr;QLabel* placeholder_=nullptr;
    QWidget* container_=nullptr;QWindow* foreign_=nullptr;xcb_window_t windowId_=0;
    QPushButton *launch_=nullptr,*check_=nullptr,*detach_=nullptr,*retry_=nullptr;
    QPlainTextEdit* log_=nullptr;
};

int main(int argc,char** argv){
    // Help remains available from terminals without a graphical display.
    for(int i=1;i<argc;++i)if(QString::fromLocal8Bit(argv[i])=="--help" || QString::fromLocal8Bit(argv[i])=="-h"){
        std::printf("Usage: mnm-qt-shell [--repo DIRECTORY] [--renderer opengl|native]\n"
                    "  --capture-draws       Record a small blit when launching with OpenGL\n"
                    "  --smoke-test          Open and close the shell without launching a game\n"
                    "  --opengl-test         Check texture presentation with known pixels\n"
                    "  --surface-demo        Show persistent renderer surfaces and palette cycling\n"
                    "  --surface-test        Verify rendered surfaces through Qt framebuffer readback\n"
                    "  --stream-test FILE    Check a synthetic frame stream through OpenGL\n"
                    "  --embedding-test      Check an external fixture window\n");return 0;
    }
    QSurfaceFormat format;format.setVersion(3,3);format.setProfile(QSurfaceFormat::CoreProfile);QSurfaceFormat::setDefaultFormat(format);
    QApplication app(argc,argv);QCoreApplication::setApplicationName("mnm-qt-shell");
    QCommandLineParser parser;parser.setApplicationDescription("Magic & Mayhem Qt development shell");parser.addHelpOption();
    parser.addOption({"renderer","Presentation backend: opengl or native.","backend","opengl"});
    parser.addOption({"capture-draws","Record bounded drawing evidence when the game is launched."});
    parser.addOption({"opengl-test","Test OpenGL texture presentation with known pixels."});
    parser.addOption({"surface-demo","Show native renderer surfaces and palette cycling without launching the game."});
    parser.addOption({"surface-test","Test persistent native surfaces through Qt presentation."});
    parser.addOption({"stream-test","Verify a bridge stream through the OpenGL viewport.","file"});
    parser.addOption({"repo","Repository directory.","directory",QDir::currentPath()});
    parser.addOption({"smoke-test","Open the shell briefly without launching the game."});
    parser.addOption({"embedding-test","Test an external fixture window; does not run the game."});
    parser.addOption({"fixture-window","Internal external-window fixture."});parser.process(app);
    if(parser.isSet("fixture-window")){
        QWidget fixture;fixture.setWindowTitle("MagicMayhem");fixture.resize(800,600);
        auto* layout=new QVBoxLayout(&fixture);layout->addWidget(new QLabel("External viewport fixture",&fixture));fixture.show();
        QTimer::singleShot(10000,&app,&QCoreApplication::quit);return app.exec();
    }
    if(parser.isSet("opengl-test") || parser.isSet("stream-test") || parser.isSet("surface-test") || parser.isSet("surface-demo")){
        FrameStream stream;QImage image;
        std::unique_ptr<mnm::render::GlBlitter> renderer;mnm::render::SurfaceId surface=0;
        if(parser.isSet("surface-test") || parser.isSet("surface-demo")){
            try {
                renderer=std::make_unique<mnm::render::GlBlitter>();
                const mnm::render::PixelFormat format{8,{}};
                surface=renderer->create({4,2,{3,3,3,3,3,3,3,3}},format);
                const auto sprite=renderer->create({2,1,{1,1}},format);
                renderer->update(surface,0,0,{2,1,{0,0}});renderer->update(surface,0,1,{2,1,{2,2}});
                renderer->copy(sprite,surface,{0,0,2,1},2,0);renderer->destroy(sprite);
                renderer->setPalette(surface,0,{{255,255,0},{0,255,0},{0,0,255},{255,255,255}});
                renderer->setPalette(surface,0,{{255,0,0}});image=renderer->present(surface);
            }catch(const std::exception& error){std::fprintf(stderr,"Renderer failed: %s\n",error.what());return 8;}
        }else if(parser.isSet("stream-test")){
            if(!stream.open(parser.value("stream-test")))return 4;
            image=stream.nextFrame();if(image.isNull())return 5;
        }else{
            image=QImage(4,2,QImage::Format_RGBA8888);
            for(int y=0;y<2;++y)for(int x=0;x<4;++x)
                image.setPixelColor(x,y,y?(x<2?Qt::blue:Qt::white):(x<2?Qt::red:Qt::green));
        }
        GlViewport viewport;viewport.resize(640,480);viewport.setFrame(image);viewport.show();
        QTimer paletteTimer;
        if(parser.isSet("surface-demo")){
            viewport.setWindowTitle("Magic & Mayhem — native surface palette demo");
            bool yellow=false;
            QObject::connect(&paletteTimer,&QTimer::timeout,&app,[&]{
                try {yellow=!yellow;renderer->setPalette(surface,0,{{255,std::uint8_t(yellow?255:0),0}});
                    viewport.setFrame(renderer->present(surface));}
                catch(const std::exception& error){std::fprintf(stderr,"Renderer failed: %s\n",error.what());app.exit(8);}
            });paletteTimer.start(500);return app.exec();
        }
        QTimer::singleShot(500,&app,[&]{
            if(!viewport.ready()){std::fprintf(stderr,"OpenGL failed: %s\n",qPrintable(viewport.error()));app.exit(6);return;}
            auto actual=viewport.grabFramebuffer();
            bool ok=true;
            for(int y=0;y<2;++y)for(int x=0;x<2;++x){
                const QColor expected=y?(x?Qt::white:Qt::blue):(x?Qt::green:Qt::red);
                if(actual.pixelColor(actual.width()/2+(x?80:-80),actual.height()/2+(y?40:-40))!=expected)ok=false;
            }
            if(actual.pixelColor(actual.width()/2,10)!=QColor(Qt::black))ok=false;
            app.exit(ok?0:7);
        });return app.exec();
    }
    const auto renderer=parser.value("renderer");
    if(renderer!="opengl" && renderer!="native")parser.showHelp(2);
    if(parser.isSet("capture-draws") && renderer!="opengl")parser.showHelp(2);
    Shell shell(QDir(parser.value("repo")).absolutePath(),renderer=="opengl" && !parser.isSet("embedding-test"),parser.isSet("capture-draws"));shell.show();
    if(parser.isSet("smoke-test"))QTimer::singleShot(100,&app,&QCoreApplication::quit);
    if(parser.isSet("embedding-test")){
        if(!shell.host().available())return 2;
        QProcess fixture;fixture.start(QCoreApplication::applicationFilePath(),{"--fixture-window"});
        QTimer timer;QElapsedTimer elapsed;elapsed.start();
        QObject::connect(&timer,&QTimer::timeout,&app,[&]{
            const auto windows=shell.host().desktops({});
            if(windows.size()==1){
                const auto window=windows.front();
                const bool attached=shell.attach(window);
                app.processEvents();
                const bool embedded=shell.host().descendantOf(window,xcb_window_t(shell.winId()));
                shell.detach();app.processEvents();
                const bool detached=shell.host().exists(window) && !shell.host().descendantOf(window,xcb_window_t(shell.winId()));
                fixture.terminate();fixture.waitForFinished(1000);
                app.exit(attached && embedded && detached?0:1);
            }else if(elapsed.elapsed()>8000){fixture.kill();fixture.waitForFinished(1000);app.exit(3);}
        });
        timer.start(100);return app.exec();
    }
    return app.exec();
}
