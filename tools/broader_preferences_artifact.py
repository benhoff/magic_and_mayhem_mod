"""Resolve exact recorded session CFG identities without opening live paths."""
from pathlib import Path
from retained_artifacts import read_artifact
from rendering_pilot import require

def preferences_artifact(reader,acquired,root):
 root=Path(root);session=Path(acquired['session'])
 require(root.is_absolute()and session.is_absolute()and '..'not in session.parts,'Invalid recorded session path')
 relative=session.relative_to(root)
 require(len(relative.parts)>1 and relative.parts[0]=='working','Recorded session outside owned working tree')
 names={str(session/'game/CFG/prefs.cfg'),str(relative/'game/CFG/prefs.cfg')}
 found=[v for v in acquired['artifacts']if v['path']in names]
 require(len(found)==1,'Missing/ambiguous exact session preferences artifact')
 return read_artifact(reader,acquired,found[0]['path'],max_bytes=1024**2,label='exact recorded session preferences')
