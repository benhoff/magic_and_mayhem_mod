#!/usr/bin/env python3
"""Run prospectively bound controls policy/adapter and isolated staging checks."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shlex
import subprocess
from rts_controls_config import ROOT
from rts_controls_launch import HASH, stage, authorize, module
from rts_controls_source import original_sections_match, prepare_source


def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--claims',type=Path,required=True)
    p.add_argument('--output',type=Path,required=True)
    a = p.parse_args()
    out = a.output.resolve()
    if out.exists() or not out.is_relative_to(ROOT/'working'):
        raise ValueError('Require a fresh output under working/')
    declaration = json.loads(a.claims.read_text())
    sources = declaration['sources']
    if any(sha(ROOT/name) != digest for name,digest in sources.items()):
        raise ValueError('Prospective source changed')
    out.mkdir(parents=True)
    report = {'success':False,'sources':sources,'claims':declaration['claims'],
              'scope':'Owned native controls policy and synthetic32 adapter/API/service peers; production PE32 build and pinned-source disposable import/config staging. No original command execution, live input, original equivalence or replacement claim.',
              'live_playtest':False,'commands':[]}
    def run(label, argv):
        result = subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,text=True,
                                timeout=180,env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1'))
        log = out/(label+'.log');log.write_text(result.stdout+result.stderr)
        report['commands'].append({'label':label,'argv':list(map(str,argv)),
                                   'returncode':result.returncode,'log':str(log.relative_to(ROOT)),
                                   'sha256':sha(log)})
        result.check_returncode()
        return result.stdout
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify'])
        for label, flags in [('policy64',[]),('policy32',['-m32']),
                             ('policy-sanitized',['-fsanitize=address,undefined','-fno-omit-frame-pointer'])]:
            run(label+'-build',['clang','-O1','-g','-std=c11','-Wall','-Wextra','-Werror',
                *flags,'-MMD','-MF',out/(label+'.d'),ROOT/'tests/rts-controls-test.c',
                ROOT/'game/input/rts_controls.c','-o',out/label])
            report[label] = json.loads(run(label,[out/label]))
        build = module('rts_test_build','tools/build-rts-controls.py')
        dll = build.build(out/'pe32')
        run('pe32-imports',['llvm-readobj','--file-headers','--coff-imports',dll])
        run('adapter-build',['clang','-m32','-O1','-g','-std=c11','-fdeclspec',
            '-Wno-ignored-attributes','-Wall','-Wextra','-Werror','-fno-pie','-no-pie',
            '-I',out/'pe32','-MMD','-MF',out/'adapter.d',ROOT/'tests/rts-controls-bridge-test.c',
            ROOT/'game/input/rts_controls.c','-o',out/'adapter'])
        report['adapter'] = json.loads(run('adapter',[out/'adapter']))
        run('configuration',['python3','-B',ROOT/'tests/test-rts-controls-config.py'])
        # Guard/copy/import/config admission consumes one immutable executable,
        # but does not load or run it. Preferences are authored synthetic input.
        original = ROOT/'original/Arcane_Nocd/Chaos.exe'
        if sha(original) != HASH:
            raise ValueError('Unsupported immutable source')
        raw = original.read_bytes()
        staged = out/'staging'
        (staged/'game/CFG/Encrypted').mkdir(parents=True)
        prefs = b'[HEADER]\nValidConfig=TRUE\n[MOUSE]\nMouseScrollSpeed=32\n[SOUND]\nCDMusicEnabled=TRUE\n'
        encoder = module('rts_test_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(ROOT)
        (staged/'game/CFG/prefs.cfg').write_bytes(prefs)
        (staged/'game/CFG/Encrypted/prefs.cfg').write_bytes(encoder.encode(prefs,decoder)[0])
        custom = json.loads((ROOT/'patches/rts-controls.json').read_text())
        custom['mouse_scroll_speed']=64
        config = out/'custom.json';config.write_text(json.dumps(custom)+'\n')
        changed, metadata = stage(staged,raw,raw,True,config)
        environment = {'MNM_RTS_CONTROLS_ACTIVE':'inherited'}
        authorize(staged,metadata,environment)
        if changed == raw or environment.get('MNM_RTS_CONTROLS_ACTIVE') != '1':
            raise ValueError('Adapter staging/admission failed')
        if not original_sections_match(raw,changed):
            raise ValueError('Import extension should be eligible for isolated normalization')
        damaged=bytearray(changed);damaged[4096]^=1
        if original_sections_match(raw,bytes(damaged)):
            raise ValueError('Modified original code was admitted for normalization')
        (staged/'game/Chaos.exe').write_bytes(changed)
        source=prepare_source(staged/'game')
        if source == staged/'game' or sha(source/'Chaos.exe') != HASH or sha(staged/'game/Chaos.exe') != hashlib.sha256(changed).hexdigest():
            raise ValueError('Isolated source normalization changed incoming installation')
        report['source_preparation']={'success':True,'source_unchanged':True,'canonical_copy':str(source.relative_to(ROOT))}
        for path, packed in [('CFG/prefs.cfg',False),('CFG/Encrypted/prefs.cfg',True)]:
            value = (staged/'game'/path).read_bytes()
            actual = decoder.decode(value)[1] if packed else value
            if actual != prefs.replace(b'MouseScrollSpeed=32',b'MouseScrollSpeed=64'):
                raise ValueError('Staged preference edit changed other settings')
        offsets = module('rts_test_offsets','tools/export-menu-support.py')
        pe = int.from_bytes(raw[60:64],'little')
        section_at = pe+24+int.from_bytes(raw[pe+20:pe+22],'little')
        reviewed_sections = []
        for i in range(int.from_bytes(raw[pe+6:pe+8],'little')):
            row = raw[section_at+i*40:section_at+(i+1)*40]
            # Retain every original executable section; import extension is data.
            if int.from_bytes(row[36:40],'little')&0x20000000:
                size = int.from_bytes(row[16:20],'little');at=int.from_bytes(row[20:24],'little')
                if changed[at:at+size] != raw[at:at+size]:
                    raise ValueError('Staging changed original executable code')
                reviewed_sections.append(row[:8].rstrip(b'\0').decode('ascii'))
        report['staging'] = {'success':True,'original_sha256':HASH,
            'staged_sha256':hashlib.sha256(changed).hexdigest(),'unchanged_executable_sections':reviewed_sections,
            'preferences_round_trip':True,'dll_sha256':metadata['rts_controls_dll_sha256']}
        closure = set()
        for dependency in out.glob('*.d'):
            for name in shlex.split(dependency.read_text().replace('\\\n',' ').split(':',1)[1]):
                path = Path(name).resolve()
                if path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    closure.add(str(path.relative_to(ROOT)))
        if closure - sources.keys():
            raise ValueError('Undeclared compiler dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources'] = sorted(closure)
        report['success'] = all(report[k]['success'] for k in ['policy64','policy32','policy-sanitized','adapter','staging'])
    except Exception as error:
        report['error'] = repr(error)
    finally:
        try:
            run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
            report['original_manifest_verified_before_after'] = True
        except Exception as error:
            report['success'] = False;report['manifest_error'] = repr(error)
        report['sources_stable'] = all(sha(ROOT/name) == digest for name,digest in sources.items())
        report['success'] = report['success'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__ == '__main__':
    raise SystemExit(main())
