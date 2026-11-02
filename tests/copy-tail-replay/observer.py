"""Generate an offline-only observer; preimage mismatches refuse the build."""
from pathlib import Path
import sys
root = Path(sys.argv[1]).resolve()
sys.path.insert(0, str(root/'tools'))
from copy_tail_replay import offline_observer
Path(sys.argv[2]).write_text(offline_observer((root/'renderer/command_copy_diagnostics.cpp').read_text()))
