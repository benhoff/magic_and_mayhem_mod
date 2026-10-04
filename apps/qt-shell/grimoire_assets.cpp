#include "grimoire_assets.hpp"
#include "asset_file.hpp"
#include <QRegularExpression>
#include <algorithm>
#include <stdexcept>
namespace mnm::ui {
namespace {
QString clean(const QByteArray& source) {
    const auto text=QString::fromLatin1(source);QString output;int depth=0;
    if (text.contains(QChar(0))) throw std::runtime_error("NUL in Grimoire text");
    for (int i=0;i<text.size();++i) {
        const auto c=text[i];
        if (c=='~' && i+1<text.size() && text[i+1]=='[') {++depth;++i;}
        else if (depth && c=='[') ++depth;
        else if (depth && c==']') --depth;
        else if (!depth || c=='\n') output+=c;
        if (depth>16) throw std::runtime_error("Grimoire comment nesting limit");
    }
    if (depth) throw std::runtime_error("Unterminated Grimoire comment");
    return output;
}
}
GrimoireBook loadGrimoireBook(const QString& root) {
    GrimoireBook book;book.layout=loadMenuLayout(root,"Interface/Grimoire","Grimoire.cfg");
    book.tooltips=loadMenuLayout(root,"Interface/Grimoire","Grimoiretooltip.cfg");
    if (book.layout.value("HEADER").value("ValidConfig")!="TRUE" || book.tooltips.value("HEADER").value("ValidConfig")!="TRUE")
        throw std::runtime_error("Invalid Grimoire configuration header");
    auto created=assets::AssetStore::create(std::filesystem::path(root.toStdString()));
    if (auto* error=std::get_if<assets::Error>(&created)) throw std::runtime_error(error->detail);
    const auto& store=std::get<assets::AssetStore>(created);
    auto opened=store.open("Interface/Grimoire/Grimoire.txt");
    if (auto* error=std::get_if<assets::Error>(&opened)) throw std::runtime_error(error->detail);
    auto bytes=assets::readWhole(*std::get<std::unique_ptr<assets::AssetFile>>(opened),256*1024);
    if (auto* error=std::get_if<assets::Error>(&bytes)) throw std::runtime_error(error->detail);
    const auto& data=std::get<std::vector<std::uint8_t>>(bytes);
    const auto text=clean(QByteArray(reinterpret_cast<const char*>(data.data()),qsizetype(data.size())));
    const QRegularExpression chapterRx("^~C([0-7])-([0-9]{3})-\"([^\"]{1,256})\"$"),sectionRx("^~S([0-9]{1,3})-([0-9]{3})-\"([^\"]{1,256})\"$"),levelRx("^~L([0-4])$"),dynamicRx("^~D\\s+\"([^\"]{1,128})\"\\s+.+$");
    book.chapters.resize(8);int chapter=-1,section=-1,level=-1,total=0;
    for (const auto& raw:text.split('\n')) {
        const auto line=raw.trimmed();if (line.isEmpty()) continue;
        auto match=chapterRx.match(line);
        if (match.hasMatch()) {
            chapter=match.captured(1).toInt();section=level=-1;
            if (!book.chapters[chapter].title.isEmpty()) throw std::runtime_error("Duplicate Grimoire chapter");
            book.chapters[chapter].title=match.captured(3);continue;
        }
        match=sectionRx.match(line);
        if (match.hasMatch()) {
            if (chapter<0 || ++total>512) throw std::runtime_error("Invalid Grimoire section order/count");
            const int id=match.captured(1).toInt();if (!id) throw std::runtime_error("Grimoire section zero is virtual contents");
            auto& entries=book.chapters[chapter].entries;
            for (const auto& entry:entries) if (entry.section==id) throw std::runtime_error("Duplicate Grimoire section");
            entries.push_back({id,match.captured(2).toInt(),match.captured(3),{}});section=entries.size()-1;level=-1;continue;
        }
        match=levelRx.match(line);
        if (match.hasMatch()) {
            if (chapter<0 || section<0) throw std::runtime_error("Grimoire level before section");
            level=match.captured(1).toInt();auto& levels=book.chapters[chapter].entries[section].levels;
            if (levels.contains(level)) throw std::runtime_error("Duplicate Grimoire knowledge level");
            levels[level].pages.push_back(QString());continue;
        }
        if (chapter<0 || section<0 || level<0) throw std::runtime_error("Grimoire text before level");
        auto& content=book.chapters[chapter].entries[section].levels[level];
        match=dynamicRx.match(line);
        if (match.hasMatch()) {if (content.dynamicLabels.size()>=64) throw std::runtime_error("Grimoire dynamic label limit");content.dynamicLabels.push_back(match.captured(1));continue;}
        if (line=="~NP") {if (content.pages.size()>=16) throw std::runtime_error("Grimoire page limit");content.pages.push_back(QString());continue;}
        if (line=="~NL" || line=="~NB") {content.pages.last()+="\n\n";continue;}
        if (line.contains('~')) throw std::runtime_error("Unsupported Grimoire directive");
        auto& page=content.pages.last();if (!page.isEmpty() && !page.endsWith('\n')) page+=' ';page+=line;
        if (page.size()>65536) throw std::runtime_error("Grimoire page text limit");
    }
    for (int i=0;i<8;++i) {
        auto& c=book.chapters[i];if (c.title.isEmpty() || c.entries.isEmpty()) throw std::runtime_error("Missing Grimoire chapter or entries");
        const auto sorted=book.layout.value("CHAPTERS").value(QString("C%1_SORTED").arg(i));
        if (sorted!="TRUE" && sorted!="FALSE") throw std::runtime_error("Invalid Grimoire sort flag");
        c.sorted=sorted=="TRUE";
        for (auto& e:c.entries) {if (e.levels.isEmpty()) throw std::runtime_error("Missing Grimoire knowledge level");for (auto& l:e.levels) for (auto& page:l.pages) page=page.trimmed();}
        if (c.sorted) std::stable_sort(c.entries.begin(),c.entries.end(),[](const auto& a,const auto& b){return a.title.compare(b.title,Qt::CaseInsensitive)<0;});
    }
    // Resolve a known file to locate the matched, case-correct installed page directory.
    auto resolver=assets::PathResolver::create(store.root());
    if (auto* error=std::get_if<assets::Error>(&resolver)) throw std::runtime_error(error->detail);
    const auto resolved=std::get<assets::PathResolver>(resolver).resolve("Interface/Grimoire/800x600/Backdrop.JPG");
    if (auto* error=std::get_if<assets::Error>(&resolved)) throw std::runtime_error(error->detail);
    const auto directory=std::get<assets::ResolvedAsset>(resolved).matchedPath.parent_path();
    const QRegularExpression artRx("^C([0-7])S([0-9]{2,3})P([1-2])_.*\\.JPG$",QRegularExpression::CaseInsensitiveOption);
    int files=0;
    for (const auto& item:std::filesystem::directory_iterator(directory)) {
        if (++files>512) throw std::runtime_error("Grimoire artwork directory limit");
        const auto name=QString::fromStdString(item.path().filename().string());const auto match=artRx.match(name);if (!match.hasMatch()) continue;
        const auto key=QString("%1/%2/%3").arg(match.captured(1).toInt()).arg(match.captured(2).toInt()).arg(match.captured(3).toInt());
        if (book.artwork.contains(key)) throw std::runtime_error("Ambiguous Grimoire artwork prefix");
        book.artwork[key]=name;
    }
    return book;
}
}
