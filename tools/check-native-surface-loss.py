#!/usr/bin/env python3
"""Retain source-bound real GL target-loss, worker and existing recovery controls."""
import argparse,hashlib,io,json,os,subprocess,tarfile
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_surface_loss import linked_sources
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/surface-loss-restore-results-20261109/'

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned control report required')
    build=ROOT/'working/build/qt-shell';out=ROOT/'working/surface-loss/controls'/Path(a.report).stem
    out.mkdir(parents=True,exist_ok=False);(out/'.retain').touch()
    tests=[('renderer-target',build/'renderer/render-surface-loss-test'),('worker-target',build/'native-surface-loss-worker-test'),
           ('gui-target',build/'native-gui-surface-loss-test'),
           ('terminal-restart',build/'resident-terminal-restart-test'),('ordinary-worker',build/'native-render-worker-test'),
           ('quiescent-context',build/'renderer/render-context-recovery-test')]
    def put(label,raw):
        record=a.report.removesuffix('.json')+'-'+label
        return dict(record=record,sha256=write_report(ROOT,record,raw),bytes=len(raw))
    with Reader(ROOT) as reader:
        reg=reader.json('research/runtime/coverage/register.json');behavior=next(b for b in reg['behaviors'] if b['id']=='NP.surface-loss-restore-acceptance')
        names=required_sources(behavior)|{'tools/check-native-surface-loss.py'}
        for _,binary in tests:names|=linked_sources(binary.parent,ROOT,binary.name)
        sources={n:reader.digest(n) for n in sorted(names)}
        claims=[dict(behavior=behavior['id'],contract_sha256=behavior_contract(behavior,{b['id']:b for b in reg['builds']}),
            scenarios={s['id']:scenario_contract(s) for s in reg['scenarios'] if behavior['id'] in s['behaviors']})]
    result=dict(schema=1,kind='native_surface_loss_controls',success=False,original_game_runs=0,
        sources=sources,claims=claims,commands=[],physical_hardware_reset=False,newly_passed_requirements=0)
    phase=io.BytesIO();unique=[]
    with tarfile.open(fileobj=phase,mode='w:gz') as archive:
        for name,digest in sources.items():
            base=subprocess.run(['git','show','HEAD:'+name],cwd=ROOT,capture_output=True)
            if base.returncode==0 and hashlib.sha256(base.stdout).hexdigest()==digest:continue
            raw=(ROOT/name).read_bytes();require(hashlib.sha256(raw).hexdigest()==digest,'Control source snapshot drift')
            entry=tarfile.TarInfo(name);entry.size=len(raw);entry.mode=(ROOT/name).stat().st_mode&0o777
            archive.addfile(entry,io.BytesIO(raw));unique.append(name)
    result['source_phase']=dict(**put('unique-sources.tar.gz.bin',phase.getvalue()),members=unique,
        revision=subprocess.check_output(['git','rev-parse','HEAD'],cwd=ROOT,text=True).strip())
    result['compiler']=subprocess.check_output(['g++','--version'],text=True).splitlines()[0]
    environment=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',__GLX_VENDOR_LIBRARY_NAME='mesa',
        __EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/50_mesa.json')
    for key in ('MNM_SURFACE_LOSS_TEST','MNM_NATIVE_DEVICE_RECOVERY','MNM_ACCEPTANCE_CAPTURE_DIRECTORY','MNM_ACCEPTANCE_CAPTURE_ID'):environment.pop(key,None)
    try:
        for label,binary in tests:
            require(binary.is_file(),'Missing built control: '+str(binary))
            raw=binary.read_bytes();artifact=put(label+'.bin',raw)
            selected=dict(environment)
            if label=='ordinary-worker':selected['MNM_NATIVE_FRAME_TRACE']='1'
            run=subprocess.run(['xvfb-run','-a',str(binary)],cwd=ROOT,env=selected,capture_output=True,timeout=120)
            log=run.stdout+run.stderr;(out/(label+'.log')).write_bytes(log)
            row=dict(label=label,returncode=run.returncode,binary=artifact,log=put(label+'.log',log),frame_trace=label=='ordinary-worker');result['commands'].append(row)
            require(hashlib.sha256(binary.read_bytes()).hexdigest()==artifact['sha256'],'Tested executable changed')
            require(run.returncode==0,'Control failed: '+label)
            if label=='quiescent-context':
                require(run.stdout.strip()==b'Native contexts rebuilt twice; pixels, palettes, clipping, partial validity and outstanding frame leases preserved',
                        'Missing context-recovery completion')
                row['result']=dict(success=True,context_rebuilds=2,completion='exact native test message and zero exit')
            else:
                values=[json.loads(line) for line in run.stdout.splitlines() if line.startswith(b'{')]
                require(values and values[-1].get('success') is True,'Missing control success: '+label);row['result']=values[-1]
        run=subprocess.run(['python3','-B','tests/test-rendering-surface-loss.py'],cwd=ROOT,capture_output=True,timeout=30)
        result['reader_control']=dict(returncode=run.returncode,log=put('reader.log',run.stdout+run.stderr));require(run.returncode==0,'Surface-loss reader control failed')
        result['success']=True
    except Exception as error:result['error']=str(error)
    with Reader(ROOT) as reader:result['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
    result['success']&=result['sources_stable']
    result['retained_setup_logs']=[]
    for f in sorted((ROOT/'working/surface-loss').glob('*.log')):
        if f.name==Path(a.report).stem+'.log':continue
        result['retained_setup_logs'].append(dict(path=f.relative_to(ROOT).as_posix(),**put('setup-'+f.name+'.bin',f.read_bytes())))
    result['early_diagnostic_limitations']='Initial exploratory compiler/reader and generic worker-refusal logs are retained. Earlier overwritten exploratory executables were not captured and earn no engine validation. Current binaries above and complete source phase are retained.'
    sha=write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps(dict(success=result['success'],record=a.report,sha256=sha,error=result.get('error'))),flush=True)
    return int(not result['success'])
if __name__=='__main__':raise SystemExit(main())
