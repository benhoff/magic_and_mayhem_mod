#include "../apps/qt-shell/acceptance_capture.hpp"
#include "../apps/qt-shell/resident_capture_path.hpp"
#include "../apps/qt-shell/resident_profile_capture.hpp"
#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <cstdio>
#include <stdexcept>

static void check(bool v,const char* why){if(!v)throw std::runtime_error(why);}
static QByteArray read(const QString& path){QFile f(path);check(f.open(QIODevice::ReadOnly),"read");return f.readAll();}
int main(int argc,char** argv){
    QCoreApplication app(argc,argv);QJsonObject report;QJsonArray checks;
    try{
        check(argc==2,"output argument");const QString root=QFileInfo(QString::fromLocal8Bit(argv[1])).absoluteFilePath();
        const QString worlds=root+"/worlds",captures=root+"/captures";
        check(QDir().mkpath(worlds)&&QDir().mkpath(captures),"directories");
        auto passed=[&](const char* name){checks.append(QString::fromLatin1(name));};
        const auto first=residentCapturePath(root+"/old/world-continuous.bin",{},worlds);
        const auto second=residentCapturePath(root+"/fresh/world-continuous.bin",{},worlds);
        check(first!=second&&first==residentCapturePath(root+"/old/world-continuous.bin",{},worlds),"epoch naming");
        std::vector<std::uint8_t> source(16,17),request(64,23);
        {ResidentProfileCapture a(first,1024*1024);check(a.enqueue(1,source,request),"first enqueue");a.finish();check(a.report()["complete"].toBool(),"first completion");}
        const auto original=read(first);
        source[0]=41;
        {ResidentProfileCapture a(second,1024*1024);check(a.enqueue(1,source,request),"fresh enqueue");a.finish();check(a.report()["complete"].toBool(),"fresh completion");}
        check(read(first)==original&&read(second)!=original,"old corpus changed");passed("distinct native writer epochs retain both complete corpora");
        {ResidentProfileCapture duplicate(first,1024*1024);duplicate.finish();check(!duplicate.report()["complete"].toBool()&&!duplicate.report()["error"].toString().isEmpty(),"duplicate was admitted");}
        check(read(first)==original,"duplicate overwrote old corpus");passed("legacy NewOnly duplicate remains a refusal without overwrite");
        check(residentCapturePath({},"legacy.bin",{})=="legacy.bin"&&residentCapturePath({}, {},{}).isEmpty(),"legacy path changed");passed("legacy file and disabled modes unchanged");
        for(int n=0;n<4;++n){bool refused=false;try{
            if(n==0)residentCapturePath(root+"/a","legacy.bin",worlds);
            if(n==1)residentCapturePath(root+"/a",{},"relative");
            if(n==2)residentCapturePath(root+"/a",{},root+"/missing");
            if(n==3)residentCapturePath("relative",{},worlds);
        }catch(const std::invalid_argument&){refused=true;}check(refused,"invalid path admitted");}
        passed("conflicting relative missing-directory and relative-channel configurations refuse");
        qputenv("MNM_ACCEPTANCE_CAPTURE_DIRECTORY",captures.toUtf8());qputenv("MNM_ACCEPTANCE_CAPTURE_ID","epoch-controls");
        qputenv("MNM_ACCEPTANCE_COMMAND_HISTORY","1");qputenv("MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY",worlds.toUtf8());
        {AcceptanceCapture a("commands");a.bindProducer(root+"/old/render-commands.bin",101,3);a.finish();}
        {AcceptanceCapture a("commands");a.bindProducer(root+"/fresh/render-commands.bin",202,3);a.finish();}
        check(QDir(captures).entryList({"*.producer.json"},QDir::Files).size()==2,"producer bindings absent");passed("actual command captures bind two distinct producer identities");
        {AcceptanceCapture a("commands");a.bindProducer(root+"/invalid/render-commands.bin",0,3);a.finish();}
        check(QDir(captures).entryList({"*.producer.json"},QDir::Files).size()==2,"invalid producer binding written");passed("invalid producer identity refuses capture");
        {AcceptanceCapture a("commands");a.bindProducer(root+"/duplicate/render-commands.bin",303,3);const auto files=QDir(captures).entryList({"*.producer.json"},QDir::Files);check(files.size()==3,"first duplicate binding absent");a.bindProducer(root+"/other/render-commands.bin",404,3);a.finish();}
        passed("duplicate producer binding refuses instead of replacing provenance");
        {AcceptanceCapture a("viewport");a.bindProducer(root+"/viewport",505,3);a.finish();}
        check(QDir(captures).entryList({"*.producer.json"},QDir::Files).size()==3,"viewport acquired a command binding");passed("viewport role cannot acquire command provenance");
        qunsetenv("MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY");
        {AcceptanceCapture a("commands");a.bindProducer(root+"/legacy",606,3);a.finish();}
        check(QDir(captures).entryList({"*.producer.json"},QDir::Files).size()==3,"legacy acquired binding");passed("legacy command capture creates no epoch sideband");
        report={{"success",true},{"checks",checks},{"first",first},{"second",second},{"capture_directory",captures}};
    }catch(const std::exception& e){report={{"success",false},{"error",e.what()},{"checks",checks}};}
    const auto json=QJsonDocument(report).toJson(QJsonDocument::Compact);std::printf("%s\n",json.constData());return report["success"].toBool()?0:1;
}
