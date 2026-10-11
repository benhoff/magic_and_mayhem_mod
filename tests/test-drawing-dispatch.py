#!/usr/bin/env python3
"""Branch slices retain both conditional arms, shared exits and opaque calls."""
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from drawing_census import instructions
from drawing_dispatch import branch


class BranchTests(unittest.TestCase):
    def test_conditional_both_arms_and_shared_exit(self):
        code = instructions('  1000: 74 05  je 0x1007\n'
                            '  1002: e8 f9 0f 00 00  call 0x2000\n'
                            '  1007: eb 07  jmp 0x1010\n')
        result = branch(code, 0x1000, {0x1010})
        self.assertEqual([r['site'] for r in result['calls']], [0x1002])
        self.assertEqual(result['shared_exits'], [0x1010])

    def test_indirect_call_retains_operand_without_target_guess(self):
        result = branch(instructions('  1000: ff d0  call eax\n  1002: c3  ret\n'), 0x1000, set())
        self.assertIsNone(result['calls'][0]['target'])
        self.assertEqual(result['calls'][0]['operand'], 'eax')

    def test_missing_instruction_and_indirect_jump_refuse(self):
        for assembly in ['  1000: 90  nop\n', '  1000: ff e0  jmp eax\n']:
            with self.assertRaises(ValueError): branch(instructions(assembly), 0x1000, set())

    def test_cycle_terminates_and_exit_only_slice_has_no_work(self):
        result = branch(instructions('  1000: eb fe  jmp 0x1000\n'), 0x1000, set())
        self.assertEqual(len(result['instructions']), 1)
        self.assertEqual(branch({}, 0x1010, {0x1010})['instructions'], [])


if __name__ == '__main__': unittest.main()
