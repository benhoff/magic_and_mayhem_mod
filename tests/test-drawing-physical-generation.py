import hashlib
from pathlib import Path
import sys
import unittest
sys.path.insert(0, str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_physical_generation import validate_capture


def sample(ordinal=3):
    body = bytes([7, 8, 9, 255])*12
    word = 0x4d4e0000 | ordinal
    marker = b''.join(bytes([255 if (word >> bit) & 1 else 0])*3+b'\xff' for bit in range(32))
    marker = b''.join(marker[i:i+4]*4 for i in range(0, len(marker), 4))
    sha = lambda data: hashlib.sha256(data).hexdigest()
    item = dict(physical_ordinal=ordinal, capture_height=35,
                marker_path=f'frame-{ordinal-1:06}-marker.rgba',
                marker_sha256=sha(marker), capture_sha256=sha(body+marker))
    return item, body, marker


class Generation(unittest.TestCase):
    def test_same_capture(self):
        self.assertTrue(validate_capture(*sample(), 4, 3, 3))

    def test_stale_capture_cannot_be_relabelled(self):
        item, body, marker = sample(2)
        item.update(physical_ordinal=3, marker_path='frame-000002-marker.rgba')
        with self.assertRaises(ValueError):validate_capture(item, body, marker, 4, 3, 3)

    def test_current_marker_from_another_image_does_not_bind_game_bytes(self):
        item, body, marker = sample()
        with self.assertRaises(ValueError):validate_capture(item, b'\x00'+body[1:], marker, 4, 3, 3)

    def test_corrupted_marker_with_updated_hash_still_refuses(self):
        item, body, marker = sample();marker = bytes([marker[0]^255])+marker[1:]
        item['marker_sha256'] = hashlib.sha256(marker).hexdigest()
        item['capture_sha256'] = hashlib.sha256(body+marker).hexdigest()
        with self.assertRaises(ValueError):validate_capture(item, body, marker, 4, 3, 3)

    def test_missing_receipt_wrong_extent_ordinal_or_filename(self):
        for key, value in [('physical_ordinal', True), ('capture_height', 3),
                           ('marker_path', '../other.rgba'), ('capture_sha256', '')]:
            item, body, marker = sample();item[key] = value
            with self.assertRaises(ValueError):validate_capture(item, body, marker, 4, 3, 3)
        with self.assertRaises(ValueError):validate_capture(*sample(), 4, 3, 0)
        with self.assertRaises(ValueError):validate_capture(*sample(), 4, 4, 3)


if __name__ == '__main__':unittest.main()
