#!/usr/bin/env python3
"""Execute scoped controls and bind EP04 progress to its coverage contract."""
import argparse,hashlib,json,os,shlex,subprocess,sys,struct,zlib
from pathlib import Path
from coverage_evidence import Reader,write_report
from coverage_claims import behavior_contract,scenario_contract,required_sources
from rendering_normal_acceptance import progress,PREFIX
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
def digest(path):
    with Path(path).open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--progress',required=True);p.add_argument('--input-preparation',required=True);p.add_argument('--input-restoration',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    if not a.report.startswith(PREFIX) or (ROOT/a.report).exists():p.error('New owned immutable report required')
    out=ROOT/'working/pilot'/(Path(a.report).stem+'-controls');out.mkdir(parents=True,exist_ok=False)
    with Reader(ROOT) as reader:
        recorded=reader.json(a.progress)
        joined=progress(reader.json('docs/native-rendering-closure.json'),reader,[r['pair'] for r in recorded['executions']],[r['reference'] for r in recorded.get('negative_executions',[])])
        require(joined=={k:v for k,v in recorded.items() if k!='sources'}|{'sources':joined['sources']},'Progress snapshot differs from independently re-admitted requirements')
        sources=dict(recorded['sources']);checks=[]
        require(all(n.startswith(PREFIX) and '..' not in Path(n).parts for n in (a.input_preparation,a.input_restoration)),'Unsafe input transition receipt')
        prepared,restored=[reader.json(n) for n in (a.input_preparation,a.input_restoration)]
        require(all(r['kind']=='native_normal_input_transition' and r['success'] is True and r['sources_stable'] is True and r['original_manifest_verified_before_after'] is True for r in (prepared,restored)),'Incomplete input transition')
        require(prepared['operation']=='prepare' and prepared['setting']==dict(section='VIDEO',IsHighRes='FALSE') and restored['operation']=='restore' and restored['preparation']==dict(record=a.input_preparation,sha256=reader.digest(a.input_preparation)),'Wrong input transition identity')
        admitted=[r['pair']['acquisition'] for r in recorded['executions']]
        require(prepared['acquisition'] in admitted and restored['acquisition'] in admitted,'Input transitions do not bind the admitted sessions')
        high,low=[reader.json(r['acquisition']['record']) for r in (prepared,restored)]
        require(high['scenario']==low['scenario']=='normal-storage','Wrong input consumers')
        changes={r['path']:r for r in prepared['changes']}
        require(set(changes)=={r['path'] for r in prepared['before']} and restored['restored']==prepared['before'],'Input restoration differs')
        for row in prepared['before']:
            name=row['path'];require(name in {'CFG/prefs.cfg','CFG/Encrypted/prefs.cfg'},'Unsafe preference path')
            require(reader.digest(row['record'])==row['sha256']==changes[name]['before_sha256']==high['installation_inputs'][name]['sha256']==digest(ROOT/'working/game-nocd'/name),'High preference/restoration differs')
            require(changes[name]['after_sha256']==low['installation_inputs'][name]['sha256'],'Low input was not the recorded transition')
        require({n:v for n,v in high['installation_inputs'].items() if n not in changes}=={n:v for n,v in low['installation_inputs'].items() if n not in changes},'Unreviewed input changes beyond display preference')
        for row in (prepared,restored):
            for name,h in row['sources'].items():
                require(name not in sources or sources[name]==h,'Input/player source conflict');sources[name]=h
        input_transitions=dict(verified=True,preparation=dict(record=a.input_preparation,sha256=reader.digest(a.input_preparation)),restoration=dict(record=a.input_restoration,sha256=reader.digest(a.input_restoration)),changed_paths=sorted(changes))

        tests=['tests/test-rendering-normal-acceptance.py','tests/test-native-render-storage.py','tests/test-rendering-pilot.py','tests/test-rendering-pilot-ledger.py']
        for name in [*tests,'tools/review-rendering-normal.py']:
            sources[name]=digest(ROOT/name)
        def run(label,cmd,env=None,expected=0,expected_text=None):
            result=subprocess.run([str(v) for v in cmd],cwd=ROOT,env=env,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=90)
            raw=result.stdout
            h=write_report(ROOT,str(Path(a.report).with_suffix(''))+'-'+label+'.bin',raw)
            checks.append(dict(name=label,returncode=result.returncode,sha256=h,bytes=len(raw)))
            require(result.returncode==expected and (expected_text is None or expected_text.encode() in raw),'Executed control failed: '+label)
            return raw
        for name in tests:run(Path(name).stem,[sys.executable,'-B',name])
        for name in ('tests/native-panel-boundary-test.c','runtime/menu/native_panel_boundary.h'):
            sources[name]=digest(ROOT/name)
        panel=ROOT/'working/build/native-panel-boundary-test'
        run('panel-boundary-build',['gcc','-std=c11','-O2','-Wall','-Wextra','-Werror',ROOT/'tests/native-panel-boundary-test.c','-o',panel])
        run('panel-boundary', [panel])
        build=(ROOT/'working/build/qt-shell').resolve();binary=build/'viewport-presentation-test'
        run('campaign-preferences',[build/'menu-campaign-preferences-test'],dict(os.environ,QT_QPA_PLATFORM='offscreen'))
        for dependency in [*(build/'CMakeFiles/viewport-presentation-test.dir').rglob('*.o.d'),*(build/'CMakeFiles/menu-campaign-preferences-test.dir').rglob('*.o.d')]:
            obj=Path(str(dependency)[:-2]);require(obj.is_file(),'Missing control object')
            for word in shlex.split(dependency.read_text().replace('\\\n',' ')):
                path=Path(word)
                if not path.is_absolute():path=build/path
                path=path.resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    require(path.stat().st_mtime_ns<=obj.stat().st_mtime_ns,'Unbuilt control source: '+str(path))
                    sources[path.relative_to(ROOT).as_posix()]=digest(path)
        require('tests/viewport-presentation-test.cpp' in sources,'Control compiler closure missing')
        for scale in ('1','1.5'):
            raw=run('viewport-dpr-'+scale,['xvfb-run','-a','-s','-screen 0 1920x1440x24',binary],dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',QT_SCALE_FACTOR=scale))
            result=json.loads(raw.decode().splitlines()[-1]);require(result['success'] is True and result['fullscreen_restore'] is True and result['logical_input_preserved'] is True and result['empty_frame_controls'] is True,'Presentation controls incomplete')
            checks[-1]['result']=result
        # Exercise the real parser with deliberately repackaged owned records.
        # These are controls, not a claim that natural source chunks occurred.
        from resident_capture import records
        first=reader.json(recorded['executions'][0]['pair']['acquisition']['record'])
        source=Path(first['temporary_root'])/'worlds.bin'
        generation,chunk,stream,request=next(records(source))
        require(generation==1 and stream[:8]==b'MNMPRO03','Initial owned producer prefix missing')
        packed=struct.pack('>I',len(stream))+zlib.compress(stream,1)
        history=build/'mnm-acceptance-world-history'
        world_source=next(producers for generation,chunk,producers,request in records(source) if not chunk)
        at=64;entry=None
        while at<len(world_source):
            size,sequence,kind=struct.unpack_from('<III',world_source,at)
            if kind==11:
                entry=bytearray(world_source[at:at+size]);struct.pack_into('<I',entry,4,1);break
            at+=size
        require(entry is not None,'No retained World entry for source-only guard control')
        world_only=world_source[:64]+entry
        for label,chunk,reserved,error in [('chunk-order',2,0,'Source chunk identity'),('chunk-reserved',1,1,'Reserved source header'),('chunk-world-refusal',1,0,'World/failure in source chunk')]:
            payload=world_only if label=='chunk-world-refusal' else stream
            body=struct.pack('>I',len(payload))+zlib.compress(payload,1)
            raw=b'MNMPROF3'+struct.pack('<QQIIII',1,chunk,len(payload),0,len(body),reserved)+body
            path=out/(label+'.bin');path.write_bytes(raw)
            h=write_report(ROOT,str(Path(a.report).with_suffix(''))+'-'+label+'-input.bin',raw)
            run(label,[history,path,Path(first['session'])/'game',ROOT/'working/original-reference',ROOT/'original/Arcane_Nocd/Chaos.exe',out/(label+'-reference')],expected=1,expected_text=error)
            checks[-1]['input_sha256']=h
        require(struct.unpack_from('<I',stream,72)[0]==1,'Initial source operation is not CREATE')
        create=stream[:64+struct.unpack_from('<I',stream,64)[0]]
        packed=struct.pack('>I',len(create))+zlib.compress(create,1)
        raw=b'MNMPROF3'+struct.pack('<QQIIII',1,1,len(create),0,len(packed),0)+packed
        path=out/'chunk-create-only.bin';path.write_bytes(raw)
        h=write_report(ROOT,str(Path(a.report).with_suffix(''))+'-chunk-create-only-input.bin',raw)
        run('chunk-create-only',[history,path,Path(first['session'])/'game',ROOT/'working/original-reference',ROOT/'original/Arcane_Nocd/Chaos.exe',out/'chunk-create-only-reference'],expected=1,expected_text='Empty original history')
        checks[-1]['input_sha256']=h
        sources['tools/resident_capture.py']=digest(ROOT/'tools/resident_capture.py')
        require(all(reader.digest(n)==h for n,h in sources.items()),'Source changed during scoped controls')
        register=reader.json('research/runtime/coverage/register.json');scenario=next(s for s in register['scenarios'] if s['id']=='RN31.normal')
        behaviors=[b for b in register['behaviors'] if b['id'] in scenario['behaviors']]
        require(len(behaviors)==3,'Expected three bounded rendering/handoff contracts')
        for behavior in behaviors:
            require(required_sources(behavior)<=set(sources),'Missing reviewed compiler/control inputs: '+str(required_sources(behavior)-set(sources)))
        builds={b['id']:b for b in register['builds']}
        report=dict(schema=1,kind='scoped_normal_rendering_execution_join',success=True,sources=sources,checks=checks,presentation_binary_sha256=digest(binary),
          original_source_sha256=builds['nocd-40209ca7']['source_sha256'],original_manifest_verified_before_after=True,
          progress=dict(record=a.progress,sha256=reader.digest(a.progress)),input_transitions=input_transitions,passed_requirements=joined['passed_requirements'],pending_requirements=joined['pending_requirements'],decisions=joined['decisions'],negative_executions=joined['negative_executions'],
          claims=[dict(behavior=b['id'],contract_sha256=behavior_contract(b,builds),scenarios={scenario['id']:scenario_contract(scenario)}) for b in behaviors],scope=scenario['scope'])
        h=write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode());print(json.dumps(dict(success=True,passed=joined['passed_requirements'],checks=len(checks),record=a.report,sha256=h)))
if __name__=='__main__':main()
