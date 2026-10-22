import importlib.util
from pathlib import Path
import struct
import unittest
spec=importlib.util.spec_from_file_location('alias',Path(__file__).resolve().parents[1]/'tools/original_canvas_alias.py')
alias=importlib.util.module_from_spec(spec);spec.loader.exec_module(alias)


class AliasCompleteness(unittest.TestCase):
    def test_incomplete_or_changed_observations_fail(self):
        rows=[];phase=0
        for first in (0,1,3):
            for second in (0,1,3):
                for locked in (0,1):
                    rows.extend([(60,phase,0,0,first,second),(61,phase,0,0,1,locked),
                                 (62,phase,1,0,0,48),(62,phase,2,0,0,48)])
                    if locked:rows.append((63,phase,2,0,0,0))
                    for id,held in ((1,first),(2,second)):
                        rows.extend((64,phase,id,count,0,0) for count in range(held,-1,-1))
                    rows.extend([(65,phase,0,0,first+1,second+1),(66,phase,0,0,0,0)]);phase+=1
        rows.append((99,99,0,0,18,0))
        def pack(rs):return b''.join(struct.pack('<6I',*row) for row in rs)
        result=alias.analyze(pack(rows));self.assertEqual(result['externally_retained_credits_drained'],48)
        for index in range(len(rows)):
            with self.assertRaises(ValueError):alias.analyze(pack(rows[:index]+rows[index+1:]))
        for index,row in enumerate(rows):
            changed=list(rows);changed[index]=row[:3]+(row[3]^1,)+row[4:]
            with self.assertRaises(ValueError):alias.analyze(pack(changed))
        with self.assertRaises(ValueError):alias.analyze(pack(rows)+b'x')


if __name__=='__main__':unittest.main()
