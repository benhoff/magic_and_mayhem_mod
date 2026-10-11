#!/usr/bin/env python3
"""Compare bounded glyph caller state/table effects with unchanged pinned PE32 code."""
import argparse
from collections import Counter
import hashlib
import importlib.util
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
    args=parser.parse_args();declaration=json.loads(args.claims.read_text())
    sources={p:sha(ROOT/p) for p in declaration['sources']}
    if sources!=declaration['sources']:raise ValueError('Prospective source changed')
    parent=ROOT/'working/tests/glyph-backend-state';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    report=dict(schema=1,success=False,claims=declaration['claims'],sources=sources,
        original_sha256=ORIGINAL,experiment=str(out.relative_to(ROOT)),live_replacement=False,
        original_work_bypassed=False,scope='Independent bounded glyph EAX/ECX/EDX, four rewritten stack arguments, callee GPRs/RET16/defined return flags/DF and exact64-float cold/initialized table effects. Selected original x87 state observations are not native FP emulation; safe two-active-slot diagnostics outside equivalence. Source/padding/guards, all installed font glyphs and native state atomic refusals. No pixel comparison, live bypass, whole text layout/lifecycle or arbitrary FP/Win32 claim.')
    commands=[];inputs={}
    def run(label,argv):
        result=subprocess.run([str(a) for a in argv],capture_output=True,text=True,timeout=180)
        log=out/(label+'.log');log.write_text(result.stdout+result.stderr)
        commands.append(dict(label=label,argv=[str(a) for a in argv],returncode=result.returncode,log=str(log.relative_to(ROOT)),sha256=sha(log)))
        result.check_returncode();return result.stdout
    executable=ROOT/'working/game-nocd/Chaos.exe'
    try:
        run('manifest-before',[ROOT/'tools/original-manifest.sh','verify']);assert sha(executable)==ORIGINAL
        report['disassembly']=run('original-assembly',['objdump','-d','-Mintel','--start-address=0x581ec0','--stop-address=0x582207',executable])
        objects=[]
        for name in ['reconstruction/rendering/glyph_backend_state.c','tests/glyph-backend-state-test.c']:
            obj=out/(Path(name).stem+'.o');run(obj.stem,['gcc','-std=c11','-O1','-g','-Wall','-Wextra','-Werror','-pedantic','-fsanitize=address,undefined','-fno-omit-frame-pointer','-MMD','-MF',obj.with_suffix('.d'),'-c',ROOT/name,'-o',obj]);objects.append(obj)
        unit=out/'state-admission';run('unit-link',['gcc','-fsanitize=address,undefined',*objects,'-o',unit]);report['atomic_result']=run('atomic-sanitized',[unit]).strip()
        run('pe32-model',['clang','--target=i686-pc-windows-msvc','-O2','-ffreestanding','-fno-builtin','-fno-stack-protector','-mno-sse','-mno-mmx','-Wall','-Wextra','-Werror','-MMD','-MF',out/'pe32.d','-c',ROOT/'reconstruction/rendering/glyph_backend_state.c','-o',out/'state-pe32.obj'])
        report['pe32_undefined']=run('pe32-imports',['nm','-u',out/'state-pe32.obj']).strip()
        if report['pe32_undefined']:raise ValueError('Model requires runtime helpers: '+report['pe32_undefined'])
        run('model32',['gcc','-m32','-O2','-std=c11','-Wall','-Wextra','-Werror','-pedantic','-MMD','-MF',out/'model32.d','-c',ROOT/'reconstruction/rendering/glyph_backend_state.c','-o',out/'model32.o'])
        run('probe',['gcc','-m32','-c',ROOT/'tests/glyph-backend-state-probe.S','-o',out/'probe.o'])
        reference=out/'glyph-state-reference'
        run('reference',['g++','-m32','-O2','-std=c++17','-fno-pie','-no-pie','-Wall','-Wextra','-Werror','-MMD','-MF',out/'reference.d',ROOT/'tests/glyph-backend-state-reference.cpp',out/'model32.o',out/'probe.o','-o',reference])
        closure={'tools/test-glyph-backend-state.py','tools/test-font-glyph-raster.py','tools/original-manifest.sh','tests/glyph-backend-state-probe.S'}
        for dependency in out.glob('*.d'):
            for p in dependency.read_text().replace('\\\n',' ').split(':',1)[1].split():
                if p.startswith(str(ROOT)+'/'):closure.add(str(Path(p).resolve().relative_to(ROOT)))
        if closure-sources.keys():raise ValueError('Unbound compiler/source dependencies: '+repr(sorted(closure-sources.keys())))
        report['compiled_project_sources']=sorted(closure)
        spec=importlib.util.spec_from_file_location('glyphs',ROOT/'tools/test-font-glyph-raster.py');glyphs=importlib.util.module_from_spec(spec);spec.loader.exec_module(glyphs)
        records=[];cases=[]
        positions={'interior':lambda w,h,fw,fh:(4,4,(0,0,w,h)),
            'exact-left':lambda w,h,fw,fh:(0,4,(0,0,w,h)),
            'exact-right':lambda w,h,fw,fh:(w-fw,4,(0,0,w,h)),
            'partial-left':lambda w,h,fw,fh:(-1,4,(0,0,w,h)),
            'partial-right':lambda w,h,fw,fh:(w-fw+1,4,(0,0,w,h)),
            'top-cropped':lambda w,h,fw,fh:(4,-3,(0,0,w,h)),
            'bottom-cropped':lambda w,h,fw,fh:(4,h-3,(0,0,w,h)),
            'hidden-top':lambda w,h,fw,fh:(4,-fh,(0,0,w,h)),
            'hidden-bottom':lambda w,h,fw,fh:(4,h,(0,0,w,h)),
            'hidden-left':lambda w,h,fw,fh:(-fw,4,(0,0,w,h)),
            'hidden-right':lambda w,h,fw,fh:(w,4,(0,0,w,h)),
            'clip-origin':lambda w,h,fw,fh:(4,4,(5,5,w,h))}
        def case(frame,label,branch,tint,cold,variant,active,pad=0):
            fw,fh,ox,oy=struct.unpack_from('<IIii',frame,4);w,h=max(32,fw+8),max(24,fh+8)
            x,y,clip=positions[branch](w,h,fw,fh);fields=[328+len(frame),len(frame),w,h,w+pad,x+ox,y+oy,*clip,*tint,113,cold,variant,active]
            table=[i/64 for i in range(64)]
            records.append(struct.pack('<18I',*(v&0xffffffff for v in fields))+struct.pack('<64f',*table)+frame)
            cases.append(dict(source=label,placement=branch,tint=tint,cold=cold,fp_control_seed=variant,active_x87=active,padding=pad))
        def padded():
            rows=[];stream=bytearray();base=40+8*8
            for y in range(8):
                control=base+len(stream);stream+=bytes([0,16])+bytes([0xd7])*3
                colour=base+len(stream);stream+=bytes((x+y*16)%64 for x in range(16))+bytes([0xb9])*5;rows.append((control,colour))
            return struct.pack('<10I',base+len(stream),16,8,2,3,0,0,0,0,0)+b''.join(struct.pack('<2I',*r) for r in rows)+stream
        def transparent():
            base=104;return struct.pack('<10I',112,16,8,2,3,0,0,0,0,0)+b''.join(struct.pack('<2I',base+y,112) for y in range(8))+bytes([16])*8
        def empty(w,h):
            size=40+8*h;return struct.pack('<10I',size,w,h,0,0,0,0,0,0,0)+b''.join(struct.pack('<2I',size,size) for _ in range(h))
        synthetic=[('mixed-'+str(i),glyphs.synthetic(i)) for i in range(4)]+[('padded',padded()),('transparent',transparent()),('empty00',empty(0,0)),('empty02',empty(0,2)),('empty20',empty(2,0))]
        tints=[(0,0,0),(255,255,255),(173,91,237),(7,3,7)]
        for label,frame in synthetic:
            for branch in positions:
                for tint in tints:
                    for cold in (0,1):
                        for variant in range(12):
                            for active in (0,1):case(frame,label,branch,tint,cold,variant,active,variant%3)
        synthetic_count=len(cases)
        installed=glyphs.fonts(ROOT/'working/game-clean/Sprites')
        for index,(path,ordinal,frame) in enumerate(installed):
            inputs[str(path.relative_to(ROOT))]=sha(path)
            for branch in ('interior','top-cropped','partial-left'):
                for cold in (0,1):case(frame,path.name+':'+str(ordinal),branch,tints[2],cold,index%12,index%2,2)
        # Masked arithmetic remains memory-bounded with two incoming finite values;
        # report observed x87 damage only, never promote it to native equivalence.
        for label,frame in synthetic[:4]:
            for cold in (0,1):case(frame,label,'interior',tints[2],cold,4,2,2)
        corpus=out/'cases.bin';corpus.write_bytes(b'MNMGLS01'+struct.pack('<I',len(records))+b''.join(records));(out/'cases.json').write_text(json.dumps(cases,indent=2)+'\n')
        result=out/'results.bin';run('original-state',[reference,executable,corpus,result])
        data=result.read_bytes();assert data[:8]==b'MNMGLSR1' and struct.unpack_from('<I',data,8)[0]==112 and len(data)==12+112*len(cases)
        counts=Counter();branches=Counter();failures=[];diagnostics=[];status_pairs=Counter()
        for index,c in enumerate(cases):
            raw=data[12+112*index:12+112*(index+1)];r=struct.unpack('<28I',raw);counts['integer_table_guard_matches']+=r[0]==255;branches[r[4]]+=1
            counts['selected_fp_value_control_tag_preserved']+=c['active_x87']<2 and bool(r[1])
            counts['original_x87_status_changed']+=r[2]!=r[3];counts['cold_table_index63_above_one']+=c['cold'] and r[7]>0x3f800000
            status_pairs[(r[2],r[3])]+=1
            if c['active_x87']==2:diagnostics.append(dict(case=index,state_mask=r[0],active_values_preserved=bool(r[1]),status_before=r[2],status_after=r[3],stack_fault=bool(r[3]&64),source=c['source']))
            if r[0]!=255 or (c['active_x87']<2 and not r[1]):failures.append(dict(case=index,**c,record=list(r)))
        report.update(case_count=len(cases),synthetic_cases=synthetic_count,installed_cases=6*len(installed),installed_glyphs=len(installed),installed_fonts=len(inputs),counts=dict(counts),branches=dict(branches),failures=failures,two_active_diagnostics=diagnostics,status_pairs=[dict(before=a,after=b,count=n) for (a,b),n in sorted(status_pairs.items())],corpus_sha256=sha(corpus),case_index_sha256=sha(out/'cases.json'),results_sha256=sha(result),reference_sha256=sha(reference),pe32_object_sha256=sha(out/'state-pe32.obj'),atomic_refusals=16,inputs=inputs)
        report['success']=not failures and counts['integer_table_guard_matches']==len(cases) and len(installed)==1205 and len(inputs)==6 and counts['selected_fp_value_control_tag_preserved']==len(cases)-8
    except Exception as error:report['error']=str(error)
    finally:
        try:run('manifest-after',[ROOT/'tools/original-manifest.sh','verify']);assert sha(executable)==ORIGINAL
        except Exception as error:report.update(success=False,manifest_error=str(error))
        report['commands']=commands;report['changed_sources']=[p for p,h in sources.items() if sha(ROOT/p)!=h];report['changed_inputs']=[p for p,h in inputs.items() if sha(ROOT/p)!=h];report['sources_stable']=not report['changed_sources'];report['inputs_stable']=not report['changed_inputs'];report['success']=report['success'] and report['sources_stable'] and report['inputs_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report.get(k) for k in ['success','case_count','counts','error']}));print(out/'report.json')
    return 0 if report['success'] else 1
if __name__=='__main__':raise SystemExit(main())
