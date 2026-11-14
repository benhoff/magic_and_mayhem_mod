"""Read preserved failures without granting them positive evidence credit."""
from rendering_pilot import require


def checked_negative(reader, reference):
    require(reader.digest(reference['record']) == reference['sha256'],
            'Retained negative input identity changed')
    value = reader.json(reference['record'])
    require(isinstance(value, dict) and value.get('success') is False,
            'Expected an explicitly failed retained input')
    return value
