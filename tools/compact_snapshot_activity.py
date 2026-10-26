from pathlib import Path
import os
def busy(path):
    expected=str(path)
    for proc in Path('/proc').iterdir():
        if not proc.name.isdecimal() or int(proc.name)==os.getpid():continue
        try:
            for fd in (proc/'fd').iterdir():
                try:value=os.readlink(fd).removesuffix(' (deleted)')
                except OSError:continue
                if value==expected:return True
            for line in (proc/'maps').read_text(errors='replace').splitlines():
                if line.split(None,5)[-1]==expected:return True
        except OSError:continue
    return False
