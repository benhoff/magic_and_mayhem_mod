"""Use exact live visible-subtree observations only for the bounded caller case."""
from pathlib import Path
from broader_command_history_launch import WorldBinding as HistoryBinding
from broader_visible_accessibility import VisibleMenus

class WorldBinding(HistoryBinding):
    def patch(self, module):
        module = super().patch(module)
        if self.plan['family'] == 'caller' and Path(getattr(module, '__file__', '')).name == 'run-rendering-menu-acceptance.py':
            module.AccessibleMenus = VisibleMenus
        return module

    def receipt(self):
        result = super().receipt()
        if self.plan['family'] == 'caller':
            result['accessibility_observation'] = dict(policy='visible-qt-descendants-v1',
                cached_observations=False, actions_changed=False, deadlines_changed=False,
                native_code_or_limits_changed=False,
                scope='Visit the application root and every showing Qt descendant, plus hidden list descendants. Hidden Qt widget pages hide descendants; hidden lists remain traversed. Exact visible dictionaries and order are compared against the legacy traversal on native previews and the live shell before any original acquisition.')
        return result
