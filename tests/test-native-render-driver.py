from pathlib import Path
import sys
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from native_render_driver import require_nvidia


class Driver(unittest.TestCase):
    def test_retained_hardware_identities_admit(self):
        names=['NVIDIA TITAN RTX/PCIe/SSE2','zink Vulkan 1.4(NVIDIA TITAN RTX (NVIDIA_PROPRIETARY))']
        self.assertTrue(require_nvidia(names)['verified'])

    def test_missing_unknown_or_one_intermediate_software_context_refuses(self):
        for drivers in ([],None,'NVIDIA',[None],[''],['unknown'],['Intel'],['NVIDIA software renderer'],['NVIDIA','llvmpipe','NVIDIA'],['NVIDIA lavapipe'],['NVIDIA softpipe']):
            with self.subTest(drivers=drivers),self.assertRaises(ValueError):require_nvidia(drivers)


if __name__=='__main__':unittest.main()
