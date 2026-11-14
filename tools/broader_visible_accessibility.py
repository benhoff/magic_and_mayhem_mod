"""Observe the visible Qt subtree without visiting hidden page descendants.

Application roots are traversed even when they lack SHOWING. Hidden widget pages in these admitted Qt
layouts hide descendants. Hidden lists are traversed: Qt combo popups can
report SHOWING list items even while the list itself lacks that state. Visible nodes keep the legacy
DFS order, live state, geometry and value queries. Nothing is cached across polls.
"""
from rendering_menu_accessibility import AccessibleMenus

class VisibleMenus(AccessibleMenus):
    def nodes(self):
        desktop = self.a.get_desktop(0)
        apps = []
        for i in range(desktop.get_child_count()):
            app = desktop.get_child_at_index(i)
            if app and app.get_process_id() == self.pid:
                apps.append(app)
        if len(apps) > 1:
            raise ValueError('Ambiguous accessibility process')
        out, pending, seen = [], [(n, True) for n in apps], 0
        while pending:
            node, root = pending.pop()
            seen += 1
            if seen > 4096:
                raise ValueError('Accessibility tree budget')
            state = node.get_state_set()
            visible = state.contains(self.a.StateType.SHOWING)
            if visible:
                component = node.get_component_iface()
                rect = None
                if component:
                    r = component.get_extents(self.a.CoordType.SCREEN)
                    rect = [r.x, r.y, r.width, r.height]
                item = dict(name=node.get_name(), role=node.get_role_name(),
                    focused=state.contains(self.a.StateType.FOCUSED),
                    enabled=state.contains(self.a.StateType.ENABLED), showing=True, rect=rect)
                if hasattr(node, 'get_accessible_id'):
                    try:
                        item['accessible_id'] = node.get_accessible_id()
                    except Exception:
                        pass
                value = node.get_value_iface()
                if value:
                    item['value'] = dict(current=value.get_current_value(),
                        minimum=value.get_minimum_value(), maximum=value.get_maximum_value())
                out.append((node, item))
            if visible or root or node.get_role_name() == 'list':
                for i in range(node.get_child_count()-1, -1, -1):
                    child = node.get_child_at_index(i)
                    if child:
                        pending.append((child, False))
        self.last_observation_work = dict(visited=seen, visible=len(out))
        return out
