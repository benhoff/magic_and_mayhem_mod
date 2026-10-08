#!/usr/bin/env python3
"""Exercise read-only kind8 diagnostics through the actual PE32 queue hook."""
import hashlib,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
from coverage_claims import behavior_contract,required_sources
ROOT=Path(__file__).resolve().parents[1]
def main():
    parent=ROOT/'working/tests/kind8';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True);capture=out/'capture';capture.mkdir()
    r=json.loads((ROOT/'research/runtime/coverage/register.json').read_text());b=next(b for b in r['behaviors'] if b['id']=='RS.world-kind8-virtual')
    claims=[{'behavior':b['id'],'contract_sha256':behavior_contract(b,{b['id']:b for b in r['builds']})}]
    paths=required_sources(b)|{str(p.relative_to(ROOT)) for p in (ROOT/'runtime/scene').glob('*.[chS]')}
    sources={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in sorted(paths)};(out/'prospective.json').write_text(json.dumps({'claims':claims,'sources':sources},indent=2)+'\n')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')};env.update(WINEPREFIX=str(ROOT/'working/tests/scene-selftest-wine'),WINEDEBUG='-all',MNM_SCENE_DIR='Z:'+str(capture).replace('/','\\'),MNM_SCENE_SAMPLES='1',MNM_KIND8_QUEUES='2')
    def run(name,cmd):
        with (out/(name+'.log')).open('x') as f:subprocess.run([str(c) for c in cmd],cwd=ROOT,env=env,stdout=f,stderr=subprocess.STDOUT,timeout=180,check=True)
    report={'success':False,'claims':claims,'sources':sources,'scope':'Synthetic object/vtable reads, malformed diagnostic rejection, finite cap and actual observer forwarding; no original raster semantics.'}
    try:
        run('build',['python3',ROOT/'tools/build-scene-observer.py','--selftest']);build=ROOT/'working/build/scene-observer-selftest'
        run('compile',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-c',ROOT/'tests/kind8-observer-test.c','-o',out/'fixture.obj'])
        run('link',['lld-link','/machine:x86','/entry:start','/subsystem:console','/nodefaultlib','/safeseh:no','/timestamp:0','/out:'+str(out/'fixture.exe'),out/'fixture.obj',build/'scene.lib',build/'kernel32.lib'])
        (out/'MnmScene.dll').write_bytes((build/'MnmScene.dll').read_bytes());run('execute',['wine',out/'fixture.exe'])
        spec=importlib.util.spec_from_file_location('kind8',ROOT/'tools/inspect-kind8.py');inspector=importlib.util.module_from_spec(spec);spec.loader.exec_module(inspector)
        paths=sorted(capture.glob('kind8-*.bin'));assert len(paths)==2
        decoded=[inspector.decode(p.read_bytes()) for p in paths];assert [r['status'] for r in decoded[0]['rows']]==[0,1,2];assert decoded[0]['rows'][0]['method']==0x12345678;assert decoded[0]['rows'][0]['y']==-4
        assert decoded[1]['captured']==512 and decoded[1]['total']==599 and decoded[1]['truncated']
        malformed=[];raw=paths[0].read_bytes()
        for offset,value in [(8,2),(12,64),(16,len(raw)+1),(20,0),(24,513),(28,1),(32,1),(36,0),(76,1),(80+4,3),(80+16,5),(80+20,1),(80+24+24,9)]:
            bad=bytearray(raw);struct.pack_into('<I',bad,offset,value);malformed.append(bytes(bad))
        malformed.extend([raw[:-1],b'BADMAGIC'+raw[8:]])
        for bad in malformed:
            try:inspector.decode(bad)
            except ValueError:pass
            else:raise AssertionError('Malformed kind8 diagnostics admitted')
        assert all(hashlib.sha256((ROOT/p).read_bytes()).hexdigest()==h for p,h in sources.items())
        report.update(success=True,sources_stable=True,forwarding=True,malformed_refusals=len(malformed),captures=decoded)
    finally:(out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(out/'report.json',flush=True)
if __name__=='__main__':main()
