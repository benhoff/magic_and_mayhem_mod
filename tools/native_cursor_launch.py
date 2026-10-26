"""Select the pinned owning cursor adapter for native graphics sessions."""
from pathlib import Path


def stage(root, native_draw):
    if type(native_draw) is not bool:
        raise ValueError('Invalid native cursor policy')
    return dict(native_cursor_ownership=native_draw,
                native_cursor_journal=str(Path(root)/'native-cursor-lifetime.bin') if native_draw else None)


def authorize(root, metadata, environment):
    environment.pop('MNM_NATIVE_CURSOR_OWNERSHIP', None)
    environment.pop('MNM_NATIVE_CURSOR_LOG', None)
    enabled = metadata.get('native_cursor_ownership', False)
    if type(enabled) is not bool or enabled != metadata.get('native_draw', False):
        raise ValueError('Native graphics requires owning cursor staging')
    if not enabled:
        return
    journal = Path(root)/'native-cursor-lifetime.bin'
    if metadata.get('native_cursor_journal') != str(journal) or journal.exists() or journal.is_symlink():
        raise ValueError('Stale or redirected cursor lifetime journal')
    environment['MNM_NATIVE_CURSOR_OWNERSHIP'] = '2'
    environment['MNM_NATIVE_CURSOR_LOG'] = 'Z:'+str(journal).replace('/', '\\')
