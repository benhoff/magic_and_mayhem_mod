"""Query live target properties without describing every unrelated Qt control.

Full screenshots continue to use VisibleMenus.nodes. Target lookup traverses the
same visible subtree, including hidden list descendants, without cross-poll
caches. Every match is counted before ambiguity is rejected; physical actions
and their enabled, geometry and host-focus checks remain the controller's own.
"""
from broader_visible_accessibility import VisibleMenus

OBJECTS = {'Launch game':'launchGame','Quick Battle':'mainMenuAction2',
           'Preferences':'mainMenuAction3','Quit':'mainMenuAction4',
           'Create Single Player Game':'quickBattleAction2','Map':'singlePlayerMap'}

class TargetMenus(VisibleMenus):
    def targets(self, suffix, role=None):
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
            ident = None
            if visible and hasattr(node, 'get_accessible_id'):
                try:
                    ident = node.get_accessible_id()
                except Exception:
                    pass
            matched = ident is not None and ident.endswith('.' + suffix)
            node_role = node.get_role_name() if matched or not (visible or root) else None
            if matched and (role is None or node_role == role):
                component = node.get_component_iface()
                rect = None
                if component:
                    r = component.get_extents(self.a.CoordType.SCREEN)
                    rect = [r.x, r.y, r.width, r.height]
                item = dict(name=node.get_name(), role=node_role,
                            focused=state.contains(self.a.StateType.FOCUSED),
                            enabled=state.contains(self.a.StateType.ENABLED),
                            showing=True, rect=rect, accessible_id=ident)
                value = node.get_value_iface()
                if value:
                    item['value'] = dict(current=value.get_current_value(),
                                         minimum=value.get_minimum_value(),
                                         maximum=value.get_maximum_value())
                out.append((node, item))
            if visible or root or node_role == 'list':
                for i in range(node.get_child_count()-1, -1, -1):
                    child = node.get_child_at_index(i)
                    if child:
                        pending.append((child, False))
        self.last_target_observation_work = dict(visited=seen, matched=len(out))
        return out

    def find(self, name, role=None):
        suffix = OBJECTS.get(name)
        if suffix:
            found = self.targets(suffix, role)
            if found:
                if len(found) != 1:
                    raise ValueError('Accessible target missing/ambiguous: '+name+' '+str(role))
                return found[0]
        return super().find(name, role)
