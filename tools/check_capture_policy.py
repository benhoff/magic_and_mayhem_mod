"""Separate bounded semantic diagnostics from full rendering evidence."""
import subprocess


def policy(mode='diagnostic', deadline=None):
    if mode not in ('diagnostic', 'full'):
        raise ValueError('Unknown evidence mode')
    seconds = (120 if mode == 'diagnostic' else 900) if deadline is None else deadline
    if type(seconds) is not int or not 15 <= seconds <= 900:
        raise ValueError('Deadline must be an integer from 15 to 900 seconds')
    return dict(mode=mode, deadline_seconds=seconds, full_capture=mode == 'full',
                pixel_comparison_eligible=mode == 'full',
                scope='Full continuous rendering evidence' if mode == 'full' else
                'Bounded semantic/observer diagnostic; no pixel comparison or nominal acceptance')


def require_full(acquired):
    # Legacy acquisitions predate the explicit selector and always captured.
    if acquired.get('configuration', {}).get('evidence_mode', 'full') != 'full':
        raise ValueError('Lightweight diagnostic has no full rendering evidence; rerun with --evidence-mode full')
    if acquired.get('kind') == 'public_native_campaign_lightweight_diagnostic':
        raise ValueError('Lightweight diagnostic is ineligible for rendering comparison')


def stop_private_prefix(prefix,environment):
    """Stop only the caller's exclusively owned prefix after its player exited."""
    env=dict(environment,WINEPREFIX=str(prefix),WINEDEBUG='-all')
    forced=False
    try:
        subprocess.run(['wineserver','-w'],env=env,check=True,timeout=5,capture_output=True)
    except subprocess.TimeoutExpired:
        subprocess.run(['wineserver','-k'],env=env,check=True,timeout=5,capture_output=True)
        subprocess.run(['wineserver','-w'],env=env,check=True,timeout=10,capture_output=True)
        forced=True
    return dict(stopped=True,forced=forced,prefix=str(prefix))
