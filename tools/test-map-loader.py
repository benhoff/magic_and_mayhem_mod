#!/usr/bin/env python3
"""All installed MAP payloads and selected original cell checks, offline only."""
import argparse,hashlib,importlib.util,json,os,struct,subprocess,tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser();parser.add_argument('binary',type=Path);parser.add_argument('--sanitized',type=Path);args=parser.parse_args()
    parent=REPO/'working/tests/map-loader';parent.mkdir(parents=True,exist_ok=True);out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120);(out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe';assert sha(exe)==HASH
        sourcePaths=[REPO/p for p in ['assets/map.cpp','assets/map.hpp','assets/map_inspect_main.cpp','assets/packed_container.cpp','assets/persistence_internal.hpp','assets/persistence.hpp','tests/map-cell-reference.cpp','tests/map-loader-test.cpp','tools/test-map-loader.py','tools/decode-cfg.py']]
        binaries=[args.binary.resolve()]+([args.sanitized.resolve()] if args.sanitized else [])
        root=REPO/'working/game-clean';files=sorted(p for p in root.rglob('*') if p.suffix.lower()=='.map');assert len(files)==683
        inputs={p:sha(p) for p in sourcePaths+binaries+[exe]+files}
        spec=importlib.util.spec_from_file_location('map_codec',REPO/'tools/decode-cfg.py');codec=importlib.util.module_from_spec(spec);spec.loader.exec_module(codec)
        fixtures=[];rows=[]
        for i,p in enumerate(files):
            meta,raw=codec.decode(p.read_bytes());header=struct.unpack_from('<19I',raw);assert header[0]==6 and header[4]==header[1]*header[2] and header[5]==header[4]*header[3] and len(raw)==76+12*header[5]
            fixture=out/f'decoded-{i:03}.bin';fixture.write_bytes(raw);fixtures.append(fixture)
            rows.append({'path':str(p.relative_to(root)),'width':header[1],'height':header[2],'layers':header[3],'cells':header[5],'metadata':list(header[6:]),'decoded_sha256':hashlib.sha256(raw).hexdigest()})
        for i,binary in enumerate(binaries):
            env=dict(os.environ);env['ASAN_OPTIONS']='detect_leaks=0'
            run=subprocess.run([str(binary),str(root),*[row['path'] for row in rows]],capture_output=True,text=True,env=env,timeout=180);(out/f'native-{i}.stdout').write_text(run.stdout);(out/f'native-{i}.stderr').write_text(run.stderr);run.check_returncode()
            actual=[json.loads(line) for line in run.stdout.splitlines()];assert actual==rows
        helper=out/'reference';command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-ffunction-sections','-Wl,--gc-sections','-I'+str(REPO/'assets'),str(REPO/'tests/map-cell-reference.cpp'),str(REPO/'assets/map.cpp'),str(REPO/'assets/packed_container.cpp'),'-o',str(helper)]
        run=subprocess.run(command,capture_output=True,text=True,timeout=60);(out/'compile.log').write_text(run.stdout+run.stderr);run.check_returncode()
        for name,a,z in [('reader',0x4eeaf0,0x4eefba),('cell_check',0x4f13f0,0x4f1890),('grid_binding',0x4ee680,0x4ee940)]:
            run=subprocess.run(['objdump','-d','-Mintel',f'--start-address={a}',f'--stop-address={z}',str(exe)],capture_output=True,text=True,check=True);(out/f'{name}.asm').write_text(run.stdout)
        run=subprocess.run([str(helper),str(exe),*map(str,fixtures)],capture_output=True,text=True,timeout=180);(out/'reference.stdout').write_text(run.stdout);(out/'reference.stderr').write_text(run.stderr);run.check_returncode();original=json.loads(run.stdout)
        assert original['cells']==sum(row['cells'] for row in rows) and original['checks']==2*original['cells']
        assert all(sha(p)==h for p,h in inputs.items())
        report={'all_match':True,'live_validated':False,'files':len(rows),'cells':original['cells'],'original_selected_checks':original['checks'],'native_builds':len(binaries),'executable_sha256':HASH,'helper_sha256':sha(helper),'source_and_input_sha256':{str(p.relative_to(REPO)):h for p,h in inputs.items()},'maps':rows,'original_scope':'Mode-1 extent-1 occupancy check in 0x4f13f0, using installed decoded cells, row/layer offsets and native coordinate access; full MAP loader and runtime initialization statically traced only'}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print({k:report[k] for k in ['files','cells','original_selected_checks','native_builds']},flush=True)
    finally:verify('after')
if __name__=='__main__':main()
