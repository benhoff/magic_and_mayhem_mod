"""Full asset scans are diagnostic; executable and wire admission is separate."""
import os


def full_asset_verification():
    return os.environ.get('MNM_VERIFY_ASSETS', '1') != '0'


def verify_staged_assets(metadata):
    enabled = metadata.get('full_asset_verification', True)
    if type(enabled) is not bool:
        raise ValueError('Invalid full asset verification policy')
    return enabled
