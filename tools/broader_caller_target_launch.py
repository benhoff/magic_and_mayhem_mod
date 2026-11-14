"""Bind current caller-only target observations without changing game actions."""
import ast,inspect,textwrap,time
from pathlib import Path
from broader_visible_world_launch import WorldBinding as BaseWorldBinding
from broader_caller_target_accessibility import TargetMenus
from rendering_pilot import require

def target_click_id(self, name):
    nodes = [v for _, v in self.access.targets(name) if v.get('accessible_id', '').endswith('.' + name)]
    require(len(nodes) == 1 and nodes[0]['enabled'], 'Missing/disabled physical control: ' + name)
    self.actions.append(dict(action='click', object=name, host_ns=time.monotonic_ns(),
                             physical=self.input.click(nodes[0]['rect'])))

def verify_click_body(original):
    old = ast.parse(textwrap.dedent(inspect.getsource(original))).body[0]
    new = ast.parse(textwrap.dedent(inspect.getsource(target_click_id))).body[0]
    class Restore(ast.NodeTransformer):
        changed = 0
        def visit_Call(self,node):
            if isinstance(node.func,ast.Attribute) and node.func.attr == 'targets':
                require(ast.dump(node,include_attributes=False) == ast.dump(ast.parse('self.access.targets(name)',mode='eval').body,include_attributes=False),'Unexpected target query')
                self.changed += 1
                return ast.parse('self.access.nodes()',mode='eval').body
            return self.generic_visit(node)
    restore = Restore(); new = restore.visit(new);new.name = old.name
    require(restore.changed == 1 and ast.dump(new,include_attributes=False) == ast.dump(old,include_attributes=False),'Physical click controller changed beyond target observation')
    return True

class WorldBinding(BaseWorldBinding):
    def __init__(self,*args,**kwargs):
        super().__init__(*args,**kwargs)
        require(self.plan['family'] == 'caller','Target observer requires caller family')
        self.target_menu_bindings = self.target_click_bindings = 0
        self.target_bound_modules = set()
    def patch(self,module):
        module = super().patch(module)
        name = Path(getattr(module,'__file__','')).name
        # Parent traversal can recursively revisit this module before its outer
        # patch call returns. Inspect membership after that traversal completes.
        if id(module) in self.target_bound_modules:
            if name == 'run-rendering-menu-acceptance.py':
                module.AccessibleMenus = TargetMenus
            return module
        if name == 'run-rendering-menu-acceptance.py':
            require(issubclass(module.AccessibleMenus,TargetMenus.__bases__[0]),'Unexpected existing caller menu observer')
            module.AccessibleMenus = TargetMenus;self.target_menu_bindings += 1
            self.target_bound_modules.add(id(module))
        if name == 'run-rendering-camera-acceptance.py':
            base = module.CameraJourney;verify_click_body(base.click_id)
            module.CameraJourney = type('TargetCameraJourney',(base,),{'click_id':target_click_id})
            self.target_click_bindings += 1
            self.target_bound_modules.add(id(module))
        return module
    def receipt(self):
        result = super().receipt()
        require(self.target_menu_bindings == 2 and self.target_click_bindings == 1,'Missing or extra target-observer module instance')
        result['caller_target_observation'] = dict(policy='live-qt-target-properties-v1',menu_bindings=2,click_bindings=1,
            cached_observations=False,game_actions_changed=False,response_deadlines_changed=False,native_code_or_limits_changed=False,
            full_screenshots_unchanged=True,physical_click_body_AST_verified=True,
            scope='Traverse the same live visible Qt tree including hidden list descendants; describe only matching controls for physical action lookup. Full screenshot observations stay unchanged. All target multiplicity,enabled,role,geometry and focus checks remain. This reduces diagnostic observer work; it neither extends the caller recorder cap nor accepts an incomplete stream.')
        return result
