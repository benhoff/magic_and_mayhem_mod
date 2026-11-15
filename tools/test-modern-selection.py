#!/usr/bin/env python3
"""Bounded default selection checks, including original Region/projection bytes."""
import argparse
import hashlib
import json
import platform
from pathlib import Path
import shlex
import subprocess

from coverage_claims import behavior_contract, required_sources, scenario_contract
from coverage_evidence import write_report
from rts_controls_launch import ROOT, HASH, stage, authorize, module


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--report', required=True)
    a = p.parse_args()
    out = a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working') or (ROOT/a.report).exists():
        p.error('Fresh owned working output and research report required')
    out.mkdir(parents=True)
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behavior = next(b for b in register['behaviors'] if b['id']=='NP.modern-selection-defaults')
    scenario = next(s for s in register['scenarios'] if s['id']=='input.modern-selection-bounded')
    sources = {n: hashlib.sha256((ROOT/n).read_bytes()).hexdigest() for n in sorted(required_sources(behavior))}
    report = dict(schema=1, success=False, sources=sources, commands=[],
        claims=[dict(behavior=behavior['id'], contract_sha256=behavior_contract(behavior,{}),
                     scenarios={scenario['id']:scenario_contract(scenario)})],
        scope='Native/ASan/UBSan and freestanding i386 policy; configured synthetic adapter; unchanged original Region and projection with explicit selection/UI service peers; guarded PE32 import/config staging; mocked native launch orchestration. No full live game, natural input timing, Quick Battle or multiplayer claim.',
        live_playtest=False, original_game_executed=False, original_executable_sha256=HASH)

    def run(label, command, guest=False):
        command = list(map(str,command)); log=out/(label+'.log')
        actual = command
        if guest and platform.machine()=='aarch64':
            script=out/(label+'.sh')
            script.write_text('set -eu\nexec > '+shlex.quote(str(log))+' 2>&1\nexec '+
                              shlex.join(['FEXInterpreter',*command])+'\n')
            actual=['muvm','--','/bin/bash',str(script)]
            result=subprocess.run(actual,cwd=ROOT,capture_output=True,timeout=180)
            (out/(label+'-launcher.log')).write_bytes(result.stdout+result.stderr)
        else:
            with log.open('xb') as stream:
                result=subprocess.run(actual,cwd=ROOT,stdout=stream,stderr=subprocess.STDOUT,timeout=180)
        report['commands'].append(dict(label=label,command=command,actual_command=actual,returncode=result.returncode))
        if result.returncode:
            raise ValueError(label+' failed: '+(log.read_text() if log.exists() else 'no log'))
        return log.read_text()

    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify'])
        raw=(ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes()
        if hashlib.sha256(raw).hexdigest()!=HASH:
            raise ValueError('Unsupported original executable')
        run('compiler-version',['clang','--version'])
        flags=['clang','-O1','-g','-std=c11','-Wall','-Wextra','-Werror']
        for name,extra in [('native',[]),('sanitized',['-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            run(name+'-build',[*flags,*extra,ROOT/'tests/rts-controls-test.c',ROOT/'game/input/rts_controls.c','-o',out/name])
            report[name]=json.loads(run(name,[out/name]))
        dll=module('selection_build','tools/build-rts-controls.py').build(out/'pe32')
        run('pe32-imports',['llvm-readobj','--file-headers','--coff-imports',dll])
        # Freestanding i386 avoids requiring x86 libc development packages on ARM.
        cross=['clang','--target=i386-linux-gnu','-DMNM_RTS_FREESTANDING32','-O1','-std=c11',
               '-ffreestanding','-fno-builtin','-fno-stack-protector','-fdeclspec',
               '-Wall','-Wextra','-Werror','-Wno-ignored-attributes','-Wno-unused-function',
               '-fno-pie','-nostdlib','-static','-fuse-ld=lld','-Wl,-e,_start,--image-base=0x10000000',
               '-I',out/'pe32']
        run('policy32-build',[*cross,ROOT/'tests/rts-controls-test.c',ROOT/'game/input/rts_controls.c','-o',out/'policy32'])
        report['policy32']=json.loads(run('policy32',[out/'policy32'],guest=True))
        offsets=module('selection_offsets','tools/export-menu-support.py')
        parts=[];report['original_fragments']=[]
        for name,start,end in [('region',0x4fdad0,0x4ff1ca),('project',0x4f80f0,0x4f8224)]:
            at=offsets.image_offset(raw,start,end-start);data=raw[at:at+end-start]
            parts.append('static const unsigned char original_'+name+'[]={'+','.join(str(x) for x in data)+'};\n')
            report['original_fragments'].append(dict(name=name,start=start,end=end,sha256=hashlib.sha256(data).hexdigest()))
        (out/'pe32/original-selection.h').write_text(''.join(parts))
        for label,extra in [('adapter',[]),('original-adapter',['-DMNM_RTS_ORIGINAL_SELECTION'])]:
            run(label+'-build',[*cross,*extra,ROOT/'tests/rts-controls-bridge-test.c',ROOT/'game/input/rts_controls.c','-o',out/label])
            report[label]=json.loads(run(label,[out/label],guest=True))
        run('config',['python3','-B',ROOT/'tests/test-rts-controls-config.py'])
        run('launcher',['python3','-B',ROOT/'tests/test-native-player-launcher.py'])
        # Selection-only staging never changes scroll preferences. The resulting
        # DLL/configuration must authorize in exactly the selection profile.
        staging=out/'staging';(staging/'game/CFG').mkdir(parents=True)
        prefs=b'[MOUSE]\nMouseScrollSpeed=17\n'
        (staging/'game/CFG/prefs.cfg').write_bytes(prefs)
        patched,metadata=stage(staging,raw,raw,True,mode='selection')
        env={};authorize(staging,metadata,env)
        assert env['MNM_RTS_CONTROLS_ACTIVE']=='selection'
        assert (staging/'game/CFG/prefs.cfg').read_bytes()==prefs
        assert metadata['rts_controls_preferences']==[]
        from rts_controls_source import original_sections_match
        assert original_sections_match(raw,patched)
        report['staging']=dict(success=True,mode='selection',preferences_unchanged=True,
            original_sections_unchanged=True,dll_sha256=metadata['rts_controls_dll_sha256'])
        report['success']=True
    except Exception as error:
        report['error']=repr(error)
    finally:
        try:
            run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=True
        except Exception as error:
            report.update(success=False,manifest_error=repr(error))
        report['sources_stable']=all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        # Preserve logs, exact tested executables and generated fixture inputs in
        # SQLite. Compiler objects are explicitly reproducible temporary output.
        report['artifacts']={}
        for path in sorted(out.rglob('*')):
            if not path.is_file() or path.suffix in ('.obj','.lib'):
                continue
            name=a.report.removesuffix('.json')+'-artifact-'+hashlib.sha256(str(path.relative_to(out)).encode()).hexdigest()[:16]+'.bin'
            payload=path.read_bytes();sha=write_report(ROOT,name,payload)
            report['artifacts'][str(path.relative_to(out))]=dict(record=name,sha256=sha,bytes=len(payload))
        write_report(ROOT,a.report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps({k:report.get(k) for k in ('success','error','native','sanitized','policy32','adapter','original-adapter','staging')}),flush=True)
    return not report['success']


if __name__=='__main__':
    raise SystemExit(main())
