#!/usr/bin/env python3
"""Compare the actual native fade splice against pinned PE32 caller/physical state."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
ORIGINAL='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--claims',type=Path,required=True)
    parser.add_argument('--corpus',type=Path,required=True)
    args=parser.parse_args();declaration=json.loads(args.claims.read_text())
    sources={p:sha(ROOT/p) for p in declaration['sources']}
    if sources!=declaration['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/fade-loop';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    executable=ROOT/'working/game-nocd/Chaos.exe'
    inputs={str(args.corpus.resolve().relative_to(ROOT)):sha(args.corpus)}
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,inputs=inputs,
        original_sha256=ORIGINAL,experiment=str(out.relative_to(ROOT)),live_replacement=False,
        original_work_bypassed=False,scope='Actual 58edb0 interior native splice under unchanged original wrapper and supplied external Lock/Unlock adapters. Full guarded physical words, eight GPRs/stack, defined EFLAGS/DF, full FX save region, callback arguments/count/state and original-once fallback. Positive bounded dimensions, both masks, odd/padded layouts, two finite active x87 values and XMM scratch clobber. No installed game hook, failed Lock recovery or arbitrary process lease/lifetime proof.')
    commands=[]
    def run(label,argv):
        result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=180)
        log=out/(label+'.log');log.write_text(result.stdout+result.stderr)
        commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=result.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        result.check_returncode();return result.stdout
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);assert sha(executable)==ORIGINAL
        run('unit-build',['gcc','-O1','-g','-std=c11','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=address,undefined','-fno-omit-frame-pointer',ROOT/'reconstruction/rendering/fade_kernel.c',ROOT/'tests/fade-kernel-test.c','-o',out/'unit'])
        report['atomic_result']=run('unit-sanitized',[out/'unit']).strip()
        objects=[];pe_objects=[]
        names=['reconstruction/rendering/fade_kernel.c','runtime/canvas-writers/fade_route.c','runtime/canvas-writers/fade_entry.S','runtime/canvas-writers/fade_finish.S','tests/fade-loop-state-probe.S']
        for name in names:
            obj=out/(Path(name).stem+'.o');run(obj.stem,['gcc','-m32','-O2','-fno-pie','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj]);objects.append(obj)
            if not name.startswith('tests/'):
                pe=out/(Path(name).stem+'.obj');run(pe.stem+'-pe32',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MMD','-MF',pe.with_suffix('.pe.d'),'-c',ROOT/name,'-o',pe]);pe_objects.append(pe)
        run('pe32-combine',['ld','-r','-m','i386pe',*pe_objects,'-o',out/'fade-combined.obj'])
        report['pe32_undefined']=run('pe32-imports',['nm','-u',out/'fade-combined.obj']).strip()
        if set(report['pe32_undefined'].split())!={'U','_mnm_fade_finish','_mnm_fade_tail'}:raise ValueError('Unexpected native runtime helper: '+report['pe32_undefined'])
        reference=out/'fade-reference'
        run('reference-build',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',ROOT/'tests/fade-loop-reference.cpp',*objects,'-o',reference])
        closure={'tools/test-fade-loop.py','tools/original-manifest.sh','tests/fade-kernel-test.c'}
        for dep in out.glob('*.d'):
            for p in dep.read_text().replace('\\\n',' ').split(':',1)[1].split():
                if p.startswith(str(ROOT)+'/'):closure.add(str(Path(p).resolve().relative_to(ROOT)))
        if closure-sources.keys():raise ValueError('Unbound compiler dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources']=sorted(closure)
        result=out/'results.bin';run('original-native-wrapper',[reference,executable,args.corpus,result])
        data=result.read_bytes();cases=struct.unpack_from('<I',args.corpus.read_bytes(),8)[0]
        assert data[:8]==b'MNMFDLP1' and len(data)==8+cases*20*32
        rows=[struct.unpack_from('<8I',data,8+i*32) for i in range(cases*20)]
        failures=[list(r) for r in rows if r[3]!=255]
        report.update(case_count=cases,execution_count=len(rows),compared_words=sum(r[4] for r in rows),native_handled=sum(r[5] for r in rows),original_once_fallbacks=sum(r[7] for r in rows),failures=failures,
            original_entries={'fade':dict(address=0x58ed80,bytes='5155568b35881f6e00'),'splice':dict(address=0x58edb0,bytes='538b471033db'),'lock':dict(address=0x58b660,bytes='83ec0833c0')})
        if failures:raise ValueError(str(len(failures))+' mismatches; first '+repr(failures[0]))
        report['success']=True
    except Exception as error:report['error']=repr(error)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify'])
        except Exception as error:report.update(success=False,manifest_error=repr(error))
        finally:
            report['commands']=commands;report['sources_stable']=all(sha(ROOT/p)==h for p,h in sources.items());report['inputs_stable']=all(sha(ROOT/p)==h for p,h in inputs.items()) and sha(executable)==ORIGINAL
            report['success']=report['success'] and report['sources_stable'] and report['inputs_stable'] and all(c['returncode']==0 for c in commands)
            report['artifacts']={str(p.relative_to(ROOT)):dict(sha256=sha(p),bytes=p.stat().st_size) for p in sorted(out.iterdir()) if p.is_file()}
            (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ['success','experiment','sources_stable','inputs_stable']}))
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
