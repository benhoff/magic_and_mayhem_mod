#!/usr/bin/env python3
"""Bounded live glyph shadow/takeover and independent original capture replay."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile
from glyph_live_capture import inspect,decode
from glyph_backend_launch import authorize
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--claims',required=True,type=Path);args=parser.parse_args()
    claims=json.loads(args.claims.read_text());sources={p:sha(ROOT/p) for p in claims['sources']}
    if sources!=claims['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/glyph-live';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=claims['claims'],sources=sources,experiment=str(out.relative_to(ROOT)),original_sha256='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168',scope='Fixed-build masked supported glyph calls under observed live controls. Full shadow completion compare plus retained takeover inputs versus independently executed unchanged original; scoped partial glyph replacement only. Other drawing/layout original, no whole-game equivalence or performance claim.')
    commands=[]
    def run(label,argv,env=None,timeout=300):
        result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,env=env,timeout=timeout);log=out/(label+'.log');log.write_text(result.stdout+result.stderr);commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=result.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)));result.check_returncode();return result.stdout
    executable=ROOT/'working/game-nocd/Chaos.exe'
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);assert sha(executable)==report['original_sha256']
        objects=[]
        for name in ['reconstruction/rendering/glyph_backend.c','reconstruction/rendering/glyph_backend_state.c','reconstruction/rendering/glyph_backend_x87.S','tests/glyph-backend-state-probe.S']:
            obj=out/(Path(name).stem+'.o');run(obj.stem,['gcc','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/name,'-o',obj]);objects.append(obj)
        reference=out/'glyph-live-reference';run('reference-build',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',ROOT/'tests/glyph-live-reference.cpp',*objects,'-o',reference])
        run('admission-dll',['python3',ROOT/'tools/build-scene-observer.py','--selftest'])
        build=ROOT/'working/build/scene-observer-selftest';definition=out/'glyph.def';definition.write_text('LIBRARY MnmScene.dll\nEXPORTS\nGlyphAdmissionForTest@0\n')
        run('admission-import',['llvm-dlltool','-m','i386','-D','MnmScene.dll','-d',definition,'-l',out/'glyph.lib','--kill-at'])
        run('admission-build',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-Wall','-Wextra','-Werror','-c',ROOT/'tests/glyph-live-admission.c','-o',out/'admission.obj'])
        run('admission-link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0',f'/out:{build/"glyph-admission.exe"}',out/'admission.obj',out/'glyph.lib',build/'kernel32.lib'])
        admission_prefix=out/'admission-prefix';run('admission-prefix-copy',['cp','-a','--reflink=auto',(ROOT/'working/wineprefix-x86_64').resolve(),admission_prefix]);env={**os.environ,'WINEDEBUG':'-all','WINEPREFIX':str(admission_prefix)}
        run('admission-wine',['xvfb-run','-a','wine',build/'glyph-admission.exe'],env=env)
        subprocess.run(['wineserver','-k'],env=env,check=False,timeout=10);run('admission-prefix-stopped',['wineserver','-w'],env=env)
        report['virtual_query_admission']=dict(success=True,branches=['read/write committed extent','read-only second region','guard second region','no-access second region','zero extent/address','wrapping extent','overlap and adjacent identity ranges'])
        cohorts={};report['cohorts']=cohorts
        for mode in ('shadow','takeover'):
            output=run(mode+'-capture',['xvfb-run','-a','-s','-screen 0 1280x1024x24','python3',ROOT/'tools/capture-scene-game.py','--glyph-backend',mode,'--samples','4','--skip-window-screenshot','--claims',args.claims],env={**os.environ,'LIBGL_ALWAYS_SOFTWARE':'1'})
            candidates=[Path(line) for line in output.splitlines() if line.startswith(str(ROOT/'working/experiments/scene-observer/run-')) and Path(line).is_dir()]
            if len(candidates)!=1:raise ValueError('Missing capture run path')
            capture=candidates[0];data=json.loads((capture/'report.json').read_text());assert data['success']
            cohort=inspect(capture/'glyph-backend',mode);samples=sorted((capture/'glyph-backend').glob('glyph-?.bin'))
            for p in samples:decode(p)
            cohort.update(experiment=str(capture.relative_to(ROOT)),capture_report_sha256=sha(capture/'report.json'),independent_replay=run(mode+'-original-replay',[reference,executable,*samples]).strip(),inputs={str(p.relative_to(ROOT)):sha(p) for p in samples})
            cohorts[mode]=cohort
        # Exercise both public staging paths; this is launch authorization,
        # not a claim of a full interactive Qt gameplay journey.
        from glyph_backend_launch import sha as stage_sha
        launch_env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};launch_env['MNM_GLYPH_BACKEND']='takeover'
        launch_checks={}
        frame=out/'public-frame.bin'
        import sys
        sys.path.insert(0,str(ROOT/'protocols/python'))
        from mnm_protocols import frame_v1
        with frame.open('xb') as f:f.write(frame_v1.initial_header());f.truncate(frame_v1.SIZE)
        for label,argv,prefix in [('menu',['python3',ROOT/'tools/prepare-menu-observer.py','--actions','--native-render'],'Evidence directory: '),('normal',['python3',ROOT/'tools/run-opengl-game.py','--stream',frame,'--skip-movies','--stage-only'],'Render experiment: ')]:
            output=run(label+'-public-stage',argv,env=launch_env)
            roots=[Path(line.removeprefix(prefix)) for line in output.splitlines() if line.startswith(prefix)]
            if len(roots)!=1:raise ValueError('Public staging path missing')
            root=roots[0];metadata=json.loads((root/'manifest.json').read_text());authorized=dict(launch_env);authorize(root,metadata,authorized)
            if metadata['glyph_backend_mode']!='takeover' or authorized['MNM_GLYPH_BACKEND']!='takeover' or stage_sha(root/'game/Chaos.exe')!=metadata['staged_sha256']:raise ValueError('Public launch staging mismatch')
            refusals=0
            for changed,environment in [(dict(metadata,glyph_backend_mode='bad'),dict(launch_env)),(dict(metadata,glyph_directory=str(out)),dict(launch_env)),(dict(metadata,glyph_dll_sha256='0'*64),dict(launch_env)),(metadata,dict(launch_env,MNM_WORD_SPRITES='takeover'))]:
                try:authorize(root,changed,environment)
                except ValueError:refusals+=1
            if refusals!=4:raise ValueError('Public authorization guard accepted an invalid request')
            launch_checks[label]=dict(experiment=str(root.relative_to(ROOT)),manifest_sha256=stage_sha(root/'manifest.json'),glyph_dll_sha256=metadata['glyph_dll_sha256'],mode='takeover',authorization_refusals=refusals,interactive_qt_gameplay_validated=False)
        report['public_staging']=launch_checks
        report.update(success=True,live_observation=True,original_pixels_compared=True,live_replacement=True,original_work_bypassed=True)
    except Exception as error:report['error']=repr(error)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        report['inputs_stable']=sha(executable)==report['original_sha256'] and all(sha(ROOT/p)==h for cohort in report.get('cohorts',{}).values() for p,h in cohort['inputs'].items())
        report['original_manifest_verified_before_after']=len([c for c in commands if c['label'] in ('manifest-before','manifest-after') and c['returncode']==0])==2
        report['commands']=commands;report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items());report['success']=report['success'] and report['sources_stable'] and report['inputs_stable'] and report['original_manifest_verified_before_after'] and all(c['returncode']==0 for c in commands)
        report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file()}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
