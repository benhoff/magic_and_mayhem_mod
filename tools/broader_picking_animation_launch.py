"""Bind only the authored cell target; preserve the frozen controller's checks."""
from functools import partial
import hashlib
from pathlib import Path

from broader_picking_animation_target import load_index, projected_targets, POLICY
from coverage_evidence import Reader
from rendering_picking_resize import targets as original_targets
from rendering_pilot import require


def binding_class(base):
    class AnimationTargetBinding(base):
        def __init__(self, *args, **kwargs):
            super().__init__(*args, **kwargs)
            require(self.plan['family'] == 'picking', 'ANI target binding requires picking family')
            self.target_modules = set()
            self.animation_review = self.plan['picking_animation_review']
            with Reader(self.root) as reader:
                review = reader.json(self.animation_review['record'])
                installation = {}
                for name in review['assets']:
                    raw = (self.root/'working/game-nocd'/name).read_bytes()
                    installation[name] = dict(sha256=hashlib.sha256(raw).hexdigest())
                self.animation_index = load_index(reader, self.animation_review, installation)
            self.animation_targets = partial(projected_targets, index=self.animation_index)

        def patch(self, module):
            module = super().patch(module)
            if Path(getattr(module, '__file__', '/')).name == 'run-rendering-picking-closeout.py':
                if id(module) not in self.target_modules:
                    require(module.targets is original_targets, 'Unexpected picking target implementation')
                    module.targets = self.animation_targets
                    self.target_modules.add(id(module))
                else:
                    require(module.targets is self.animation_targets, 'Picking target binding changed')
            return module

        def receipt(self):
            result = super().receipt()
            require(len(self.target_modules) == 1, 'Missing or duplicate picking controller binding')
            # This policy intentionally changes cell coordinates. The physical
            # motion, object target and all readiness/retirement checks stay exact.
            result['game_actions_changed'] = True
            result['picking_animation_target'] = dict(policy=POLICY,review=self.animation_review,
                bound_controllers=1,cell_target_policy_changed=True,object_target_unchanged=True,
                physical_delivery_unchanged=True,quiet_source_samples=3,
                exact_displayed_witnesses=3,readiness_seconds=8,
                normal_retirement_required=True,hit_feedback_search=False)
            return result
    return AnimationTargetBinding
