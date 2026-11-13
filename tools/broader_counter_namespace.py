"""Fresh cohort counters supplement, never replace, the global charged ledger."""
from pathlib import Path
from rendering_pilot import require

def prospective_bindings(root,previous):
 root=Path(root);result={}
 require(previous and isinstance(previous,dict),'Missing previous counter declaration')
 for key,value in previous.items():
  require(isinstance(key,str)and Path(key).is_absolute()and Path(key).is_relative_to(root/'working'),'Unknown legacy counter key')
  require(isinstance(value,str)and value.startswith('working/broader-player/bp16/')and '..'not in Path(value).parts and Path(value).suffix=='.sqlite3','Unknown historical counter namespace')
  new=value.replace('/bp16/','/bp20/',1);target=root/new
  require(new!=value and all(not Path(str(target)+suffix).exists()and not Path(str(target)+suffix).is_symlink()for suffix in('','-wal','-shm','-journal')),'Never reuse or truncate an existing prospective counter')
  result[key]=new
 require(len(set(result.values()))==len(result),'Aliased new counters')
 return result
