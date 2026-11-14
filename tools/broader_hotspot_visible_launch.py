"""Use the exact visible-subtree observer for only the declared hotspot route."""
from pathlib import Path
from broader_visible_world_launch import WorldBinding as BaseWorldBinding
from broader_visible_accessibility import VisibleMenus
from rendering_pilot import require

class WorldBinding(BaseWorldBinding):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        require(self.plan['family'] == 'hotspot', 'Hotspot observer requires the hotspot family')

    def patch(self, module):
        require(self.plan['family'] == 'hotspot', 'Hotspot observer requires the hotspot family')
        module = super().patch(module)
        if Path(getattr(module, '__file__', '')).name == 'run-rendering-menu-acceptance.py':
            module.AccessibleMenus = VisibleMenus
        return module

    def receipt(self):
        result = super().receipt()
        result['hotspot_accessibility_observation'] = dict(
            policy='visible-qt-descendants-v1', cached_observations=False,
            actions_changed=False, deadlines_changed=False,
            native_code_or_limits_changed=False,
            scope='The declared hotspot route uses the same live visible dictionaries, DFS order, geometry, focus and values. Hidden widget pages are pruned; hidden list descendants stay traversed. Original controller methods, actions, waits, capture limits and cursor COPY assertions remain exact. A complete fresh pilot is still required.')
        return result
