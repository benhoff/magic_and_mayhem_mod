"""Bounded constant-address search across decoded direct control-flow edges.

This is a candidate finder, not exhaustive alias analysis or unreachability.
Unknown values merge to unknown. Calls discard all register facts. Implicit
bulk writers remain visible even when their destination/count is unknown.
"""
from collections import deque
import re
from alternate_canvas_lifecycle import constant, REGS, SLOTS

def successors(row, rows):
 m,o=row['mnemonic'],row['operand'];following=row['address']+row['size']
 if m.startswith(('ret','iret')) or m in {'int3','hlt','ud2'}:return []
 target=constant(o,{}) if m.startswith(('j','loop')) else None
 result=[]
 if target in rows:result.append(target)
 if m!='jmp' and following in rows:result.append(following)
 return list(dict.fromkeys(result))

def transfer(row, incoming):
 facts=dict(incoming);m=row['mnemonic'];args=[a.strip() for a in row['operand'].split(',')];dest=args[0]
 if m.startswith(('call','rep')) or m in {'int','sysenter','cpuid','popal','popad'}:return {}
 if m.startswith(('lods','movs','stos','scas','cmps')):
  for r in ('eax','ecx','esi','edi'):facts.pop(r,None)
  return facts
 if m in {'mul','div','idiv','cdq','cwd','cwde','cbw'} or (m=='imul' and len(args)==1):
  facts.pop('eax',None);facts.pop('edx',None)
 if m=='leave':facts.pop('ebp',None);facts.pop('esp',None)
 if m.startswith('loop'):facts.pop('ecx',None)
 if m in {'push','pop','pushfd','popfd'}:facts.pop('esp',None)
 if dest in REGS and m not in {'cmp','test','push','bt'}:
  value=None
  if m=='mov' and len(args)==2 and '[' not in args[1]:value=constant(args[1],incoming)
  elif m=='lea' and len(args)==2:
   mem=re.search(r'\[([^]]+)\]',args[1]);value=constant(mem[1],incoming) if mem else None
  elif m in {'add','sub'} and len(args)==2:value=constant(dest+('+' if m=='add' else '-')+args[1],incoming)
  elif m=='xor' and len(args)==2 and dest==args[1]:value=0
  elif m=='inc':value=constant(dest+'+1',incoming)
  elif m=='dec':value=constant(dest+'-1',incoming)
  facts.pop(dest,None)
  if value is not None:facts[dest]=value
 elif re.fullmatch(r'[abcd][lh]|[abcd]x|[sd]i|[sb]p',dest) and m not in {'cmp','test','push','bt'}:
  parent={'al':'eax','ah':'eax','ax':'eax','bl':'ebx','bh':'ebx','bx':'ebx','cl':'ecx','ch':'ecx','cx':'ecx','dl':'edx','dh':'edx','dx':'edx','si':'esi','di':'edi','sp':'esp','bp':'ebp'}[dest];facts.pop(parent,None)
 if m=='xchg' and len(args)==2:facts.pop(args[1],None)
 # ESP-derived memory, stack contents and calls have no modeled values.
 facts.pop('esp',None)
 return facts

def inspect(instructions, roots=()):
 rows={r['address']:r for r in instructions};edges={a:successors(r,rows) for a,r in rows.items()};pred={a:[] for a in rows}
 for a,targets in edges.items():
  for t in targets:pred[t].append(a)
 external=set(roots)
 for r in rows.values():
  if r['mnemonic']=='call':
   target=constant(r['operand'],{})
   if target in rows:external.add(target)
 states={a:{} for a in rows};outs={a:{} for a in rows};work=deque(sorted(rows));queued=set(work);visits=0
 while work:
  a=work.popleft();queued.remove(a);visits+=1
  if visits>len(rows)*64:raise ValueError('Constant frontier failed to converge within bounded work')
  incoming={}
  if pred[a] and a not in external:
   incoming=dict(outs[pred[a][0]])
   for previous in pred[a][1:]:incoming={k:v for k,v in incoming.items() if outs[previous].get(k)==v}
  states[a]=incoming;out=transfer(rows[a],incoming)
  if out!=outs[a]:
   outs[a]=out
   for target in edges[a]:
    if target not in queued:queued.add(target);work.append(target)
 writes={'mov','movd','movq','movups','movaps','movdqu','movdqa','add','sub','and','or','xor','inc','dec','not','neg','shl','shr','sar','sal','adc','sbb','xchg','xadd','cmpxchg','cmpxchg8b','fst','fstp','fist','fistp','fisttp','pop','bts','btr','btc'}
 candidates=[];escapes=[];bulk=[];unresolved=0;branch_carried=0
 for a,row in sorted(rows.items()):
  m=row['mnemonic'];args=[s.strip() for s in row['operand'].split(',')];facts=states[a];mem=re.search(r'\[([^]]+)\]',args[0]);write=m in writes or m.startswith('set')
  if mem and write:
   dest=constant(mem[1],facts)
   widths={'zmmword':64,'ymmword':32,'xmmword':16,'oword':16,'tbyte':10,'qword':8,'fword':6,'dword':4,'word':2,'byte':1}
   width=next((size for prefix,size in widths.items() if args[0].startswith(prefix+' ')),1)
   if dest is None:unresolved+=1
   elif any(dest<s+4 and s<dest+width for s in SLOTS):
    candidates.append(dict(row,destination=dest,width=width,registers=facts))
  if 'stos' in m or 'movs' in m:
   # Distinguish SSE movss/movsd explicit operands from implicit string stores.
   if len(args)==1 or m.startswith('rep') or ('es:[edi]' in row['operand'] and '[' in args[0]):
    dest=facts.get('edi');count=facts.get('ecx') if m.startswith('rep') else 1
    bulk.append(dict(row,destination=dest,count=count,direction_unknown=True))
  if m=='push' and not mem:
   value=constant(args[0],facts)
   if value in SLOTS:escapes.append(dict(row,pointer=value,registers=facts))
  if m=='mov' and len(args)==2 and '[' not in args[1]:
   value=constant(args[1],facts)
   if value in SLOTS and args[0] not in REGS:escapes.append(dict(row,pointer=value,registers=facts))
  if facts and any(rows[p]['mnemonic'].startswith(('j','loop')) for p in pred[a]):branch_carried+=1
 return dict(decoded_instructions=len(rows),direct_edges=sum(map(len,edges.values())),fixed_point_visits=visits,branch_entries_with_constants=branch_carried,overlapping_writes=candidates,escaped_pointer_addresses=escapes,unresolved_computed_stores=unresolved,bulk_writers=bulk,external_entry_roots=len(external),all_writers_closed=False,original_owner_recovered=False,global_unreachability_proved=False)
