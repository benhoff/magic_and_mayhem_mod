#!/usr/bin/env python3
"""Bounded owned forward animation, placement and attachment renewal (SQLite)."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import struct
import subprocess

from coverage_claims import behavior_contract, required_sources
from coverage_evidence import write_report

ROOT = Path(__file__).resolve().parents[1]
SELECTED = ['AN.forward', 'AN.placement', 'AN.attachment']
BUILD = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def ani(groups):
    records, offsets = [], [0]
    words = [0, 1, 0x7fffffff, 0x80000000, 0xffffffff]
    for group in groups:
        for op, arg in group:
            records.append(struct.pack('<Ii9I', op, arg, *(words[i % 5] for i in range(9))))
        offsets.append(len(records))
    size = 44 + 4*len(offsets) + 44*len(records)
    return (b'ANI\0' + struct.pack('<5I', size, len(records), 5, 0, len(offsets))
            + b'fixture.spr\0'.ljust(20, b'\0')
            + struct.pack('<'+'I'*len(offsets), *offsets) + b''.join(records))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--asset-root', type=Path, required=True)
    args = parser.parse_args()
    out, report_path = args.output.resolve(), args.report.absolute()
    if (out.exists() or not out.is_relative_to(ROOT/'working/tests') or report_path.exists()
            or not report_path.is_relative_to(ROOT/'research/runtime')):
        parser.error('Require new checkout-local execution and research report paths')
    out.mkdir(parents=True)
    (out/'.retain').write_text('Retain bounded animation execution inputs, logs and failures.\n')
    (out/'tmp').mkdir()
    env = dict(os.environ, TMPDIR=str(out/'tmp'), ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',
               UBSAN_OPTIONS='halt_on_error=1:print_stacktrace=1')
    register = json.loads((ROOT/'research/runtime/coverage/register.json').read_bytes())
    behaviors = {b['id']: b for b in register['behaviors']}
    builds = {b['id']: b for b in register['builds']}
    model = ['reconstruction/animation/'+p+'.cpp' for p in ['no_cd','placement','body_placement','attachment']]
    refs = {
        'forward': ['tests/animation-binary-reference.cpp', model[0]],
        'placement': ['tests/animation-placement-reference.cpp', model[1]],
        'body': ['tests/animation-body-placement-reference.cpp', *model[:3]],
        'attachment': ['tests/animation-attachment-reference.cpp', model[0], model[1], model[3]],
    }
    flags = ['g++','-std=c++17','-Wall','-Wextra','-Iassets','-Ireconstruction/animation']
    paths = set().union(*(required_sources(behaviors[b]) for b in SELECTED))
    paths.update(['tools/test-rendering-twentyfirst-three.py','tests/animation-owned-boundaries-test.cpp'])
    deps = subprocess.check_output(flags+['-MM']+sorted(set(sum(refs.values(), [])+model)), cwd=ROOT, env=env, text=True)
    (out/'dependencies.txt').write_text(deps)
    for token in shlex.split(deps.replace('\\\n',' ')):
        path = ROOT/token
        if path.is_file(): paths.add(path.resolve().relative_to(ROOT).as_posix())
    sources = {p:sha(ROOT/p) for p in sorted(paths)}
    claims = [dict(behavior=b,contract_sha256=behavior_contract(behaviors[b],builds)) for b in SELECTED]
    report = dict(schema=1,success=False,sources=sources,claims=claims,original_claims=claims,
                  original_sha256=BUILD,inputs={},commands=[],controls={},comparisons={},live_replacement=False,
                  scope='Selected owned offline forward controller, record/body placement and mode-one '
                  'attachment lifecycle. Safe bounded synthetic and installed ANI original comparisons; '
                  'native DWORD/restore/refusal/ownership guards remain native policy. No reverse-mode, '
                  'file-loader, whole-scene, global cadence, entity lifetime, live integration/replacement or FPS claim.')

    def run(label, argv, structured=False):
        command = [str(a) for a in argv]
        result = subprocess.run(command,cwd=ROOT,env=env,capture_output=True,timeout=300)
        log = out/(label+'.log'); log.write_bytes(result.stdout+b'\n'+result.stderr)
        report['commands'].append(dict(label=label,argv=command,exit_code=result.returncode,log_sha256=sha(log)))
        result.check_returncode()
        return json.loads(result.stdout) if structured else None

    before = False
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']); before=True
        exe = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(exe)!=BUILD: raise ValueError('Unsupported original executable')
        installed = args.asset_root.resolve()/'Sprites/effects2.ani'
        report['inputs']={str(p):sha(p) for p in [exe,installed]}
        for mode, options in [('release',['-DCMAKE_BUILD_TYPE=Release']),
                              ('sanitized',['-DCMAKE_BUILD_TYPE=Debug',
                               '-DCMAKE_CXX_FLAGS=-fsanitize=address,undefined -fno-sanitize-recover=all -fno-pie',
                               '-DCMAKE_EXE_LINKER_FLAGS=-fsanitize=address,undefined -no-pie'])]:
            build=out/mode
            run(mode+'-configure',['cmake','-S',ROOT/'reconstruction/animation','-B',build,*options])
            run(mode+'-build',['cmake','--build',build,'-j2'])
            run(mode+'-ctest',['ctest','--test-dir',build,'--output-on-failure'])
            report['controls'][mode]=dict(success=True,ctests=5)
        for label, sources_ in refs.items():
            run(label+'-build',flags+['-m32','-O2','-fno-pie','-no-pie']+sources_+['-o',out/label])
        # Every branch dispatch is bounded; looping cases encounter a sprite each turn.
        groups=[]
        for delay in [0,1,2,7]:
            for repeats in [0,1,3]:
                groups.append([(1,delay),(2,repeats),(0,7),(5,-7),(0,8),(3,-3),(6,-1)])
        groups += [[(0,0),(0,1),(4,-2),(0,2),(6,-1)],
                   [(5,-2147483648),(8,123),(0,3),(5,2147483647),(0,4),(6,-1)],
                   [(6,-1)],[(1,-1),(0,5),(6,-1)]]
        synthetic=out/'forward.ani'; synthetic.write_bytes(ani(groups))
        states=0
        for i in range(len(groups)):
            rows=run('forward-'+str(i),[out/'forward',exe,synthetic,i,96,47],True); states+=len(rows)
        switch=out/'switch.ani'; switch.write_bytes(ani([[(0,7),(1,2),(0,8),(4,-3),(6,-1)],
                                                       [(0,17),(1,4),(0,18),(4,-3),(6,-1)]]))
        for tick in [1,2,3,4,7,13]:
            rows=run('switch-'+str(tick),[out/'forward',exe,switch,0,96,47,1,tick],True);states+=len(rows)
        report['comparisons']['forward']=dict(cases=len(groups)+6,states=states,all_match=True)
        placement=out/'placement.ani'
        # Cross product of signed DWORD metadata endpoints in all three offset pairs.
        words=[0,1,0x7fffffff,0x80000000,0xffffffff]
        payload=bytearray(ani([[(0,1)]*25+[(6,-1)]])); base=52
        for i,(x,y) in enumerate(( (x,y) for x in words for y in words )):
            for slot in [0,5,7]: struct.pack_into('<II',payload,base+i*44+8+slot*4,x,y)
        placement.write_bytes(payload)
        report['comparisons']['placement']=run('placement-original',[out/'placement',exe,placement],True)
        report['comparisons']['placement-installed']=run('placement-installed',[out/'placement',exe,installed],True)
        report['comparisons']['body']=run('body-original',[out/'body',exe],True)
        attached=out/'attachment.ani'
        attached.write_bytes(ani([[(1,i%3),(0,i),(5,-7),(0,i+1),(6,-1)] for i in range(48)]))
        attachment_states=0; attachment_cases=0
        for source in [attached,installed]:
            for asset in ([0,2,12] if source==attached else [2]):
                for facing in range(8):
                    value=run(f'attachment-{attachment_cases}',[out/'attachment',exe,source,40,asset,facing,64],True)
                    if not value['all_match']: raise ValueError('Attachment mismatch')
                    attachment_cases+=1;attachment_states+=len(value['rows'])
        report['comparisons']['attachment']=dict(cases=attachment_cases,states=attachment_states,all_match=True,
                                                    same_mode_noop=True,remove_and_reenter=True)
        report['derived_inputs']={p.name:sha(p) for p in [synthetic,switch,placement,attached]}
        report['native_all_passed']=len(report['controls'])==2
        report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items())
        report['inputs_stable']=all(sha(Path(p))==h for p,h in report['inputs'].items())
        if not report['sources_stable'] or not report['inputs_stable']: raise ValueError('Inputs changed')
        report['success']=True
    finally:
        try:
            run('original-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after']=before
        except BaseException:
            report['success']=False
            raise
        finally:
            write_report(ROOT,report_path,(json.dumps(report,indent=2)+'\n').encode())
    print(json.dumps(dict(success=report['success'],controls=report['controls'],comparisons=report['comparisons']),indent=2))


if __name__=='__main__': main()
