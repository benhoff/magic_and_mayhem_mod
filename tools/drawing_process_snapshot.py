"""Read-only graphics-module census of processes in one private Wine prefix."""
from pathlib import Path
def snapshot_processes(experiment):
 root=Path(experiment).resolve();prefix=str(root/'wineprefix').encode();rows=[]
 for entry in Path('/proc').iterdir():
  if not entry.name.isdigit():continue
  try:
   variables=dict(part.split(b'=',1) for part in (entry/'environ').read_bytes().split(b'\0') if b'=' in part)
   if variables.get(b'WINEPREFIX')!=prefix:continue
   command=(entry/'cmdline').read_bytes().replace(b'\0',b' ').decode(errors='replace')
   maps=(entry/'maps').read_text();paths=sorted({line.split()[-1] for line in maps.splitlines() if '/' in line})
   graphics=[path for path in paths if any(name in path.lower() for name in ('egl','glx','opengl','vulkan','/dri/','wined3d','ddraw','delivery','winex11'))]
   game=[path for path in paths if path.startswith(str(root/'game')+'/')]
   rows.append(dict(pid=int(entry.name),command=command,graphics_modules=graphics,staged_game_modules=game,observer_environment={key.decode():variables.get(key,b'').decode(errors='replace') for key in (b'LD_PRELOAD',b'MNM_DRAW_X11_DIRECTORY')}))
  except (OSError,PermissionError):continue
 return dict(schema=1,scope='Private-prefix process/module presence only; no call reachability,stable addresses or source/presentation causal binding.',processes=rows,complete_process_discovery=False)
