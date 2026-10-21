"""Owned instruction bytes distinguish entry decoding, data and indirect flow."""
import sys
from pathlib import Path
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from original_x86 import Decoder, PE32


class EntryDecoder(unittest.TestCase):
    def test_control_flow_and_pointer_addressing(self):
        decoder = Decoder()
        try:
            fixtures = [
                ('e8fbffffff', 'call', '0x1000', 5),
                ('ff248584115000', 'jmp', 'dword ptr [eax*4 + 0x501184]', 7),
                ('c20c00', 'ret', '0xc', 3),
                ('a1681f6e00', 'mov', 'eax, dword ptr [0x6e1f68]', 5),
                ('899f58e56d00', 'mov', 'dword ptr [edi + 0x6de558], ebx', 6),
                ('890424', 'mov', 'dword ptr [esp], eax', 3),
            ]
            for raw, mnemonic, operands, size in fixtures:
                with self.subTest(raw=raw):
                    row = decoder.decode(bytes.fromhex(raw), 0x1000)
                    self.assertEqual((row['mnemonic'], row['operand'], row['size']),
                                     (mnemonic, operands, size))
            with self.assertRaises(ValueError): decoder.decode(b'\x0f', 0x1000)
        finally:
            decoder.close()

    def test_invalid_image_refuses_before_decoding(self):
        for raw in (b'', bytes(512), b'MZ' + bytes(510)):
            with self.assertRaises(ValueError): PE32(raw)


if __name__ == '__main__': unittest.main()
