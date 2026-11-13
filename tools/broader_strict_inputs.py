"""Exact regular-file inventory of the strict source installation."""
import hashlib
from pathlib import Path
from rendering_pilot import require

def installation_inputs(game):
 game=Path(game);result={}
 for path in sorted(game.rglob('*')):
  if not path.is_file():continue
  require(not path.is_symlink(),'Strict source input symlink')
  with path.open('rb')as f:sha=hashlib.file_digest(f,'sha256').hexdigest()
  result[path.relative_to(game).as_posix()]=dict(sha256=sha,bytes=path.stat().st_size)
 require(result,'Empty strict source installation');return result
