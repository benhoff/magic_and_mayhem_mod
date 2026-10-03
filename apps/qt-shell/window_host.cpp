#include "window_host.hpp"
#include <QGuiApplication>
#include <cstdlib>

WindowHost::WindowHost() {
    if(QGuiApplication::platformName()!=QStringLiteral("xcb"))return;
    int screen=0;connection_=xcb_connect(nullptr,&screen);
    if(xcb_connection_has_error(connection_))return;
    auto screens=xcb_setup_roots_iterator(xcb_get_setup(connection_));
    while(screen-- && screens.rem)xcb_screen_next(&screens);
    if(!screens.rem)return;
    root_=screens.data->root;
    constexpr char name[]="_NET_WM_NAME";
    auto* atom=xcb_intern_atom_reply(connection_,xcb_intern_atom(connection_,0,sizeof(name)-1,name),nullptr);
    if(atom){netName_=atom->atom;std::free(atom);}
}
WindowHost::~WindowHost(){if(connection_)xcb_disconnect(connection_);}
bool WindowHost::available() const{return root_ && !xcb_connection_has_error(connection_);}
QSet<xcb_window_t> WindowHost::windows() const {
    QSet<xcb_window_t> result;
    if(!available())return result;
    QVector<xcb_window_t> pending{root_};
    for(int depth=0;depth<5 && !pending.empty();++depth){
        QVector<xcb_window_t> next;
        for(auto window:pending){
            auto* tree=xcb_query_tree_reply(connection_,xcb_query_tree(connection_,window),nullptr);
            if(!tree)continue;
            const auto* children=xcb_query_tree_children(tree);
            for(int i=0;i<xcb_query_tree_children_length(tree) && result.size()<4096;++i)
                if(!result.contains(children[i])){result.insert(children[i]);next.append(children[i]);}
            std::free(tree);
        }
        pending=next;
    }
    return result;
}
QString WindowHost::title(xcb_window_t window) const {
    for(auto atom:{netName_,xcb_atom_t(XCB_ATOM_WM_NAME)}){
        if(!atom)continue;
        auto* property=xcb_get_property_reply(connection_,xcb_get_property(connection_,0,window,atom,XCB_GET_PROPERTY_TYPE_ANY,0,1024),nullptr);
        if(!property)continue;
        QString result;
        if(property->format==8)result=QString::fromUtf8(static_cast<const char*>(xcb_get_property_value(property)),xcb_get_property_value_length(property));
        std::free(property);
        if(!result.isEmpty())return result;
    }
    return {};
}
QVector<xcb_window_t> WindowHost::desktops(const QSet<xcb_window_t>& excluded) const {
    QVector<xcb_window_t> result;
    for(auto window:windows()){
        if(excluded.contains(window))continue;
        const auto name=title(window);
        if(name!=QStringLiteral("MagicMayhem") && !name.startsWith(QStringLiteral("MagicMayhem - ")))continue;
        auto* attributes=xcb_get_window_attributes_reply(connection_,xcb_get_window_attributes(connection_,window),nullptr);
        if(attributes && attributes->map_state==XCB_MAP_STATE_VIEWABLE)result.append(window);
        std::free(attributes);
    }
    return result;
}
bool WindowHost::exists(xcb_window_t window) const {
    if(!available() || !window)return false;
    auto* tree=xcb_query_tree_reply(connection_,xcb_query_tree(connection_,window),nullptr);
    const bool present=tree;std::free(tree);return present;
}
bool WindowHost::descendantOf(xcb_window_t window,xcb_window_t ancestor) const {
    if(!available())return false;
    for(int depth=0;window && depth<16;++depth){
        if(window==ancestor)return true;
        auto* tree=xcb_query_tree_reply(connection_,xcb_query_tree(connection_,window),nullptr);
        if(!tree)return false;
        window=tree->parent;std::free(tree);
    }
    return false;
}
