#!/usr/bin/env python3
"""Check protected primary fading-particle caller admission with original fragments."""
import argparse,hashlib,json,os,subprocess,sys,tempfile,traceback
from pathlib import Path
R=Path(__file__).resolve().parents[1];sys.path.insert(0,str(R/'tools'))
from coverage_evidence import write_report
P='research/runtime/broader-player-acceptance-results-20261110/'
parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--output',required=True);parser.add_argument('--report',required=True);args=parser.parse_args();out=(R/args.output).resolve();assert out.is_relative_to(R/'working')and not out.exists();assert args.report.startswith(P)and not(R/args.report).exists();out.mkdir(parents=True);scratch=Path(tempfile.mkdtemp(prefix='mnm-fading-admission-',dir='/dev/shm'));env=dict(os.environ,TMPDIR=str(scratch));names=['reconstruction/rendering/kind8_class.h','tools/test-fading-particle-admission.py','runtime/scene/fading_particle_admission.h','runtime/scene/world_producer_bypass.c','runtime/scene/world_trace.c','tests/fading-particle-admission-test.cpp','tests/kind8-sprite-admission-test.cpp','tests/sprite-binary-reference.cpp','runtime/scene/world_raster_callers.h','runtime/scene/kind8_sprite_admission.h','reconstruction/rendering/kind8_sprite_class.h','reconstruction/rendering/particle_sprite.hpp','reconstruction/rendering/kind8_random.hpp'];snap={n:(R/n).read_bytes()for n in names};q=dict(schema=1,kind='fading_particle_primary_admission_controls',success=False,original_game_runs=0,new_player_executions=0,scope='Exact fading class5c752c/method54a4a0 under the primary kind8 return. Actual unchanged original primary/method bodies with synthetic pixel tails; ten leaf/tag pairs, full method-byte mutations, wrong contexts, all shorter readable spans and inaccessible-page ends. This is not World205 operand replay or whole-player acceptance.',sources={n:hashlib.sha256(b).hexdigest()for n,b in snap.items()})
def run(label,cmd):
 print(label,flush=True)
 with(out/(label+'.log')).open('xb')as f:subprocess.run([str(x)for x in cmd],env=env,stdout=f,stderr=subprocess.STDOUT,check=True)
try:
 run('original-before',[R/'tools/original-manifest.sh','verify']);exe=R/'original/Arcane_Nocd/Chaos.exe';assert hashlib.sha256(exe.read_bytes()).hexdigest()=='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
 src=(R/'runtime/scene/world_producer_bypass.c').read_text();a=src.index('static int raster_caller(');b=src.index('static u32 bypass_hash',a);c=src.index('static int raster_batch_caller(');(scratch/'sprite-batch-caller.inc').write_text(src[a:b]+src[c:])
 obs=(R/'runtime/scene/world_trace.c').read_text();a=obs.index('static int kind8_supported(');b=obs.index('void world_begin(',a);(scratch/'sprite-observer-caller.inc').write_text(obs[a:b])
 for name,test in [('fading','fading-particle-admission-test.cpp'),('old-sprite','kind8-sprite-admission-test.cpp')]:
  run(name+'-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-fno-omit-frame-pointer','-fno-pie','-no-pie','-I'+str(scratch),R/'tests'/test,'-o',scratch/name]);run(name+'-run',[scratch/name,exe]);q[name]=json.loads((out/(name+'-run.log')).read_text());assert q[name]['success'];run(name+'-save-binary',['cp',scratch/name,out/name])
 assert q['fading']['context_pairs']==10 and q['fading']['original_primary_cases']==240 and q['old-sprite']['context_pairs']==15
 assert all((R/n).read_bytes()==body for n,body in snap.items());q.update(success=True,sources_stable=True)
except Exception as e:q['error']=repr(e);traceback.print_exc()
finally:
 try:run('original-after',[R/'tools/original-manifest.sh','verify']);q['original_manifest_verified_before_after']=True
 except Exception as e:q['original_after_error']=repr(e);q['success']=False
 q['artifacts']={}
 for f in out.iterdir():
  if f.is_file():n=args.report.removesuffix('.json')+'-'+f.name+'.bin';q['artifacts'][f.name]=dict(record=n,sha256=write_report(R,n,f.read_bytes()))
 q['operator']=dict(record=args.report.removesuffix('.json')+'-operator.bin',sha256=write_report(R,args.report.removesuffix('.json')+'-operator.bin',Path(__file__).read_bytes()))
 h=write_report(R,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=q['success'],sha256=h,error=q.get('error'),fading=q.get('fading'))),flush=True)
raise SystemExit(not q['success'])
