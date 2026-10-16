#!/usr/bin/env python3
"""Independent GPU rectangle/history pixels and strict input refusal controls."""
import argparse,copy,hashlib,json,os,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
def sha(data):return hashlib.sha256(data).hexdigest()
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--presenter',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing historical report overwrite')
 names=['apps/drawing-session/main.cpp','apps/drawing-session/recorded_composition.hpp','tools/test-recorded-drawing-composition.py','renderer/blit.cpp','renderer/blit.hpp','apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp']
 sources={n:sha((ROOT/n).read_bytes()) for n in names};binary=a.presenter.resolve();binary_sha=sha(binary.read_bytes())
 parent=ROOT/'working/tests/recorded-drawing-composition';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));(run/'.retain').write_text('Independent recorded GPU composition controls.\n');print(run,flush=True)
 w,h=6,4;base=[0x1234+i for i in range(w*h)];patch=[31,0xf800,0x7e0,31]
 def raw(words):return struct.pack('<'+'H'*len(words),*words)
 source_bytes=[raw(base),raw(patch)]
 source_rows=[dict(id=i+1,path=f'source-{i+1}.565',sha256=sha(b),width=w if i==0 else 2,height=h if i==0 else 2) for i,b in enumerate(source_bytes)]
 def blit(index,kind,source,view,src,dst,key=None):return dict(index=index,kind='copy',source_kind=kind,source=source,view_rect=view,source_rect=src,destination_rect=dst,key=key)
 frames=[blit(1,'snapshot',1,[0,0,w,h],[0,0,w,h],[0,0,w,h]),blit(2,'snapshot',2,[0,0,2,2],[0,0,2,2],[2,1,4,3],31),blit(3,'history',1,[2,1,4,3],[2,1,4,3],[2,1,4,3]),dict(index=4,kind='fill',colour=0x7e0),blit(5,'snapshot',2,[0,0,2,2],[0,0,2,2],[0,0,2,2])]
 plan=dict(schema=1,width=w,height=h,sources=source_rows,frames=frames)
 changed=base.copy();changed[1*w+3]=0xf800;changed[2*w+2]=0x7e0;last=[0x7e0]*(w*h);last[:2]=patch[:2];last[w:w+2]=patch[2:]
 expected=[base,changed,base,[0x7e0]*(w*h),last];checks=[];env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',LP_NUM_THREADS='8',QT_SCALE_FACTOR='1')
 def execute(label,config,valid=False):
  owned=run/(label+'-inputs');owned.mkdir()
  for i,b in enumerate(source_bytes,1):(owned/f'source-{i}.565').write_bytes(b)
  (owned/'presentation.json').write_text(json.dumps(config)+'\n');out=run/(label+'-outputs')
  command=['xvfb-run','-a','-s','-screen 0 1024x768x24',str(binary),str(owned),str(out),'--recorded-composition']
  result=subprocess.run(command,env=env,capture_output=True,timeout=60);(run/(label+'.log')).write_bytes(result.stdout+result.stderr)
  if (result.returncode==0)!=valid:raise ValueError(label+' unexpected result; inspect '+str(run/(label+'.log')))
  report=json.loads((out/'presentation-report.json').read_text()) if out.exists() else None
  if valid:
   assert report['success'] and report['gpu_composition'] and report['physical_presentation'] and not report['remaining_surfaces'] and not report['qimage_uploads'] and not report['original_oracles_read']
   assert report['plan_sha256']==sha((owned/'presentation.json').read_bytes()) and len(report['frames'])==len(expected)
   for index,words in enumerate(expected):
    assert (out/f'frame-{index:06d}.565').read_bytes()==raw(words),'Independent GPU word mismatch'
    physical=(out/f'frame-{index:06d}.rgba').read_bytes();assert len(physical)==w*h*4
    actual=[]
    for word in words:
     r,g,b=word>>11,(word>>5)&63,word&31;actual.extend((r<<3|r>>2,g<<2|g>>4,b<<3|b>>2,255))
    assert physical==bytes(actual),'Independent complete physical pixel mismatch'
  elif report:assert report['success'] is False
  checks.append(dict(name=label,success=True,exit_code=result.returncode))
 execute('keyed-copy-history-fill-and-physical',plan,True)
 for label,change in [
  ('wrong-hash',lambda q:q['sources'][0].update(sha256='0'*64)),
  ('oracle-field',lambda q:q['frames'][0].update(oracle_sha256='a'*64)),
  ('future-history',lambda q:q['frames'][2].update(source=3)),
  ('reordered-index',lambda q:q['frames'][1].update(index=3)),
  ('cropped-view',lambda q:q['frames'][2].update(view_rect=[3,1,4,3])),
  ('scaled-copy',lambda q:q['frames'][1].update(destination_rect=[0,0,3,3])),
  ('unsafe-path',lambda q:q['sources'][0].update(path='../outside.565')),
  ('fractional-number',lambda q:q['frames'][3].update(colour=1.5)),
  ('undefined-first-frame',lambda q:q['frames'][0].update(source_rect=[0,0,1,1],destination_rect=[0,0,1,1])),
 ]:
  q=copy.deepcopy(plan);change(q);execute(label,q)
 assert all(sha((ROOT/n).read_bytes())==h for n,h in sources.items()) and sha(binary.read_bytes())==binary_sha
 report=dict(schema=1,success=True,sources=sources,sources_stable=True,binary_sha256=binary_sha,controls=checks,complete_frames=5,pixels_compared=5*w*h,original_execution=False,scope='Owned synthetic GPU copy/key/history/fill plus every physical X11 RGB565 pixel. No original-game journey or real hardware assertion.',artifacts=str(run.relative_to(ROOT)))
 a.report.parent.mkdir(parents=True,exist_ok=True)
 with a.report.open('x') as out:json.dump(report,out,indent=2);out.write('\n')
 print(a.report)
if __name__=='__main__':main()
