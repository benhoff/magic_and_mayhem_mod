#!/usr/bin/env python3
"""Capture unchanged original Restore/retry control flow with explicit COM faults."""
import argparse,gzip,hashlib,json,struct,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
BUILD_HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
LOST=0x887601c2;BUSY=0x887601ae;FAIL=0x80004005

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def cases():
    rows=[]
    schedules=[('success',[0]),('lost-success',[LOST,0]),('busy-success',[BUSY,0]),('error-success',[FAIL,0]),('two-losses',[LOST,LOST,0]),('busy-lost-success',[BUSY,LOST,0]),('lost-busy-success',[LOST,BUSY,0])]
    for entry in [0x58bac0,0x58bc10,0x58c4a0,0x58c8a0]:
        for fast in ([0] if entry in [0x58bac0,0x58bc10] else [0,1]):
            for no_wait in ([0] if entry in [0x58bac0,0x58bc10] else [0,1]):
                for key_enabled in [0,1]:
                    for reload_bits in [0,1,2,3]:
                        for rs,rd in [(0,0),(FAIL,0),(0,FAIL),(FAIL,FAIL)]:
                            for key_result in [0,FAIL]:
                                for label,draws in schedules:
                                    rows.append(dict(id=len(rows),entry=entry,fast=fast,no_wait=no_wait,key_enabled=key_enabled,reload_bits=reload_bits,source_restore=rs,destination_restore=rd,key_result=key_result,color=0x12345678,draw_results=draws,label=label))
    return rows

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--executable',type=Path,default=ROOT/'working/game-nocd/Chaos.exe');p.add_argument('--report',type=Path,required=True);p.add_argument('--fixture-dir',type=Path,default=ROOT/'tests/fixtures/surfaces');a=p.parse_args();corpus=a.fixture_dir/'original-retry.json.gz';catalog=a.fixture_dir/'original-retry-corpus.json'
    if any(x.exists() for x in [a.report,corpus,catalog]):p.error('Refusing retained evidence overwrite')
    parent=ROOT/'working/tests/surface-retry';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    names=['tests/surface-retry-reference.cpp','tools/capture-original-surface-retry.py'];sources={n:sha(ROOT/n) for n in names}
    def verify(label):
        r=subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120);(run/(label+'.log')).write_text(r.stdout+r.stderr);r.check_returncode()
    verify('original-before')
    try:
        if sha(a.executable)!=BUILD_HASH:raise ValueError('Unsupported original executable')
        binary=run/'original-reference';subprocess.run(['g++','-m32','-std=c++17','-O2','-fno-pie','-no-pie','-Wall','-Wextra','-Werror',str(ROOT/names[0]),'-o',str(binary)],check=True)
        inputs=cases();file=run/'inputs.txt';file.write_text(''.join(' '.join(str(v) for v in [*(c[k] for k in ['id','entry','fast','no_wait','key_enabled','reload_bits','source_restore','destination_restore','key_result','color']),len(c['draw_results']),*c['draw_results']])+'\n' for c in inputs))
        r=subprocess.run([str(binary),str(a.executable.resolve()),str(file)],capture_output=True,text=True,timeout=30);(run/'outputs.json').write_text(r.stdout);(run/'stderr.log').write_text(r.stderr);r.check_returncode();outputs=json.loads(r.stdout)
        assert len(outputs)==len(inputs) and all(c['id']==o['id'] for c,o in zip(inputs,outputs))
        assert sha(a.executable)==BUILD_HASH and all(sha(ROOT/n)==h for n,h in sources.items())
        a.fixture_dir.mkdir(parents=True,exist_ok=True);raw=json.dumps([dict(input=c,original=o) for c,o in zip(inputs,outputs)],separators=(',',':')).encode();corpus.write_bytes(gzip.compress(raw,mtime=0))
        scope='Unchanged original wrappers58bac0/58bc10/58c4a0/58c8a0 in private i386 PE mapping; scripted COM draw/Restore/key outcomes and bounded reload/UI observers capture actual wrapper scheduling. No real driver/lost allocation/Restore pixels/palette, asset reload, Wine, live or Windows equivalence.'
        meta=dict(schema=1,original_sha256=BUILD_HASH,path=corpus.name,sha256=sha(corpus),raw_sha256=hashlib.sha256(raw).hexdigest(),cases=len(inputs),oracle='unchanged_original_wrapper_scripted_com_faults',scope=scope);catalog.write_text(json.dumps(meta,indent=2)+'\n');sources.update({str(x.relative_to(ROOT)):sha(x) for x in [corpus,catalog]})
        report=dict(schema=1,success=True,original_sha256=BUILD_HASH,sources=sources,cases=len(inputs),scope=scope,scripted_driver_results=True,real_driver_executed=False,wine_executed=False,original_instruction_bytes_patched=False,artifacts=str(run.relative_to(ROOT)),binary_sha256=sha(binary),inputs_sha256=sha(file),outputs_sha256=sha(run/'outputs.json'))
    finally:verify('original-after')
    report['original_manifest_verified_before_after']=True
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps(dict(success=True,cases=len(inputs))))
if __name__=='__main__':main()
