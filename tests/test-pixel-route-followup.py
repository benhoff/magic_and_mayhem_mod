import copy
from pathlib import Path
import sys
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from native_pixel_counters import ROUTES
from pixel_route_matrix import FAMILY_ROUTES
from pixel_route_followup import TARGETS, ALTERNATE, PHYSICAL, UNRESOLVED, summarize


def campaign(selected):
    families = {family: [dict(route=r, installed=1, native=int(family+'/'+r in selected), forwarded=0)
                         for r in routes] for family, routes in ROUTES.items()}
    return dict(success=True, sources_unchanged=True, inputs_stable=True, private_wine_stopped=True,
                original_manifest_verified_after=True, normal_quit_required=True,
                pixel_producers=dict(families=families, native=len(selected), forwarded=0, history_refused=False))


class Followup(unittest.TestCase):
    def setUp(self):
        self.physical = campaign(PHYSICAL)
        self.alternate = campaign(ALTERNATE)
        self.cpu = dict(verified=True, base_family=15, executable_bytes_unchanged=True,
                        dispatch_pointers_written_by_debugger=False, child_exit=0, override=0,
                        targets=[0x5947b2, 0x595b47, 0x5968a4, 0x597086])
        self.comparisons = {family: dict(success=True, physical_words=100,
                             route_coverage=[dict(route=r, native_by_format=[3, 2], changed_by_format=[2, 1])
                                             for r in routes]) for family, routes in FAMILY_ROUTES.items()}
        self.callers = dict(success=True, global_unreachable_proved=False,
                            routes=[dict(route=r) for r in UNRESOLVED])

    def result(self):
        return summarize(self.physical, self.alternate, self.cpu, self.comparisons, self.callers)

    def test_exact_cohort_and_union(self):
        self.assertEqual((len(TARGETS), len(PHYSICAL), len(ALTERNATE), len(UNRESOLVED)), (27, 15, 7, 5))
        result = self.result()
        self.assertEqual(result['denominator'], 43)
        self.assertEqual(len(result['live_targets']), 22)
        self.assertEqual(set(result['remaining_live']), set(UNRESOLVED))
        self.assertFalse(result['all_targets_live'])
        self.assertFalse(result['complete_replacement'])

    def test_fixture_calls_never_become_game_callers(self):
        self.comparisons['pixel']['route_coverage'][3]['native_by_format'] = [100000, 100000]
        row = next(r for r in self.result()['routes'] if r['route'] == 'pixel/pcx_words')
        self.assertEqual((row['physical_calls'], row['authored_cpu_calls']), (0, 0))
        self.assertFalse(row['live_executed'])

    def test_missing_real_route_rejected(self):
        self.physical['pixel_producers']['families']['pixel'][1]['native'] = 0
        self.physical['pixel_producers']['native'] -= 1
        with self.assertRaisesRegex(ValueError, 'did not execute'):
            self.result()

    def test_incomplete_campaign_provenance_rejected(self):
        for key in ('success', 'sources_unchanged', 'inputs_stable', 'private_wine_stopped', 'original_manifest_verified_after', 'normal_quit_required'):
            with self.subTest(key=key):
                old = self.physical[key]; self.physical[key] = False
                with self.assertRaises(ValueError):self.result()
                self.physical[key] = old

    def test_census_does_not_replace_ordinary_acceptance(self):
        self.physical['cpu_census_only'] = True
        with self.assertRaises(ValueError):self.result()

    def test_forwarding_or_history_refusal_rejected(self):
        self.alternate['pixel_producers']['history_refused'] = True
        with self.assertRaises(ValueError):self.result()
        self.alternate['pixel_producers']['history_refused'] = False
        self.alternate['pixel_producers']['families']['raster'][0]['forwarded'] = 1
        self.alternate['pixel_producers']['forwarded'] = 1
        with self.assertRaises(ValueError):self.result()

    def test_missing_native_pixel_change_in_either_format_rejected(self):
        for key in ('native_by_format', 'changed_by_format'):
            for at in (0, 1):
                with self.subTest(key=key, format=at):
                    old = copy.deepcopy(self.comparisons)
                    self.comparisons['raster']['route_coverage'][15][key][at] = 0
                    with self.assertRaises(ValueError):self.result()
                    self.comparisons = old

    def test_wrong_or_mutated_cpu_dispatch_rejected(self):
        for changes in (dict(base_family=6), dict(verified=False), dict(dispatch_pointers_written_by_debugger=True),
                        dict(executable_bytes_unchanged=False), dict(child_exit=1), dict(override=1),
                        dict(targets=[0x59521a, 0x59603e, 0x596490, 0x596cb8])):
            with self.subTest(changes=changes):
                old = self.cpu; self.cpu = old | changes
                with self.assertRaises(ValueError):self.result()
                self.cpu = old

    def test_static_absence_does_not_prove_unreachability(self):
        self.callers['global_unreachable_proved'] = True
        with self.assertRaises(ValueError):self.result()
        self.callers['global_unreachable_proved'] = False
        self.callers['routes'].pop()
        with self.assertRaises(ValueError):self.result()

    def test_actual_future_caller_may_close_a_gap(self):
        self.physical['pixel_producers']['families']['pixel'][3]['native'] = 1
        self.physical['pixel_producers']['native'] += 1
        self.assertNotIn('pixel/pcx_words', self.result()['remaining_live'])


if __name__ == '__main__':
    unittest.main()
