#!/usr/bin/env python3
"""Sustained owned mutation inputs with independent complete engine/GPU pixels."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import subprocess
import tempfile
import importlib.util
ROOT=Path(__file__).resolve().parents[1]
spec=importlib.util.spec_from_file_location('mutation_resource',ROOT/'tools/test-render-resource-lifecycle.py')
resource=importlib.util.module_from_spec(spec);spec.loader.exec_module(resource)
live,ring=resource.live,resource.ring
METADATA_CASES=[mode for name in ['desc','attached','add','delete','restore','batch'] for mode in ['cross-'+name,name+'-timeout']]
CASES=METADATA_CASES+['source-key','cross-key', 'key-timeout', 'cross-clipper', 'clipper-timeout', 'cross-palette', 'palette-timeout','cross-dc','dc-timeout','cross-flip','flip-timeout','cross-lock','lock-timeout','cross-fast','cross-copy','copy-timeout','source-write','source-key','meta16','meta-change','mx16','mx24','mx32','indexed','dc16','dc24','dc32','reshape','bounded','partial','unsupported','palette-flags']


def frames(data):
    at=0;result=[]
    while at<len(data):
        width,height,bits,size,red,green,blue=struct.unpack_from('<7I',data,at);at+=28
        assert size==width*height*(bits//8)
        native=data[at:at+size];palette=data[at+size:at+size+1024];at+=size+1024
        assert len(palette)==1024
        rgba=bytearray()
        for i in range(width*height):
            v=int.from_bytes(native[i*(bits//8):(i+1)*(bits//8)],'little')
            if bits==8:r,g,b=palette[v*4:v*4+3]
            else:
                components=[]
                for mask in [red,green,blue]:
                    shift=(mask & -mask).bit_length()-1
                    channel=(v & mask)>>shift
                    # Canonical RGB565 presentation follows retained Surface2 DC
                    # bit expansion; other masks retain normalized scaling.
                    if bits==16 and (red,green,blue)==(0xf800,0x7e0,0x1f):
                        precision=(mask>>shift).bit_length()
                        components.append((channel<<(8-precision))|(channel>>(2*precision-8)))
                    else:components.append(channel*255//(mask>>shift))
                r,g,b=components
            rgba.extend([b,g,r,255])
        result.append(hashlib.sha256(rgba).hexdigest())
    assert at==len(data)
    return result


def archive(data):
    if data[:8]==b'MNMCMD02':
        assert struct.unpack_from('<II',data,8)==(2,16)
        at=16;result=[];seq=0
        sizes={1:28,2:20,3:40,4:12,5:4,6:4,7:4,8:0,9:4,10:4,11:8,12:8,13:16,14:12,15:8}
        while at<len(data):
            op,ordinal,size=struct.unpack_from('<III',data,at);seq+=1
            assert ordinal==seq and op in sizes and size>=sizes[op] and at+12+size<=len(data)
            result.append(dict(op=op,fields=list(struct.unpack_from('<'+'I'*(sizes[op]//4),data,at+12))))
            at+=12+size
        assert at==len(data)
        return result
    records=resource.continuous.archive_records(data);at=16;result=[]
    for op,size,_ in records:
        result.append(dict(op=op,fields=list(struct.unpack_from('<'+'I'*(min(size,28 if op==1 else 12 if op==4 else 20 if op==2 else 40 if op==3 else 8 if op==11 else 4)//4),data,at+12))))
        at+=12+size
    return result


def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('build',type=Path);parser.add_argument('--case',action='append',choices=CASES);parser.add_argument('--palette-resources',action='store_true');parser.add_argument('--claims',type=Path,help='Prospective coverage/source declaration');args=parser.parse_args()
    if not os.environ.get('DISPLAY'):parser.error('Run under xvfb-run -a')
    paths=sorted((ROOT/'runtime/render').glob('*.[ch]'))+[ROOT/p for p in [
        'tools/test-render-mutations.py','tools/test-render-resource-lifecycle.py','tools/test-render-continuous-producer.py',
        'tools/test-live-render-channel.py','tools/build-render-bridge.py','tools/prepare-shadow-experiment.py',
        'runtime/shadow/win32_min.h','tests/live-render-channel-test.cpp',
        'renderer/blit.cpp','renderer/blit.hpp','renderer/commands.cpp','renderer/commands.hpp',
        'renderer/command_state.hpp','renderer/command_consumer.cpp',
        'apps/qt-shell/command_channel.cpp','apps/qt-shell/command_channel.hpp',
        'apps/qt-shell/live_command_renderer.cpp','apps/qt-shell/live_command_renderer.hpp',
        'apps/qt-shell/gl_viewport.cpp','apps/qt-shell/gl_viewport.hpp',
        'protocols/include/mnm/render_commands_v1.h','protocols/include/mnm/render_commands_v2.h',
        'protocols/include/mnm/render_command_ring.h','protocols/include/mnm/render_stream_v2.h','protocols/schemas/render_stream-v2.json',
        'protocols/python/mnm_protocols/render_commands_v1.py','protocols/python/mnm_protocols/render_commands_v2.py']]
    paths += [p for p in [ROOT/'renderer/dib.cpp',ROOT/'renderer/dib.hpp'] if p.exists()]
    fingerprints={str(p.relative_to(ROOT)):live.sha(p) for p in paths}
    declaration=json.loads(args.claims.read_text()) if args.claims else None
    if declaration:
        for path,expected in declaration['sources'].items():
            target=(ROOT/path).resolve()
            if Path(path).is_absolute() or not target.is_relative_to(ROOT) or live.sha(target)!=expected:
                raise ValueError('Invalid or changed prospective source: '+path)
        fingerprints.update(declaration['sources'])
    parent=ROOT/'working/tests/render-mutations' ;parent.mkdir(parents=True,exist_ok=True)
    run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(run,flush=True)
    dll=live.load('mutation_build','tools/build-render-bridge.py').build(True)
    stage=live.load('mutation_stage','tools/prepare-shadow-experiment.py')
    env={k:v for k,v in os.environ.items() if not k.startswith('MNM_')}
    env.update(QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1',WINEDEBUG='-all',WINEPREFIX=str(run/'wineprefix'))
    report=dict(schema=1,sources=fingerprints,cases=[],scope='Actual synthetic PE32 full/partial negative-pitch unlock, fill, keyed/unkeyed copy, two-buffer flip, palette RGB/flags and real Wine DIB ReleaseDC through continuous v2 GPU. Successful full layout replacement via fresh IDs; failed calls/retries, bounded/partial/unsupported refusal. Independent engine native pixels/palettes generate complete frame hashes; no original game/driver equivalence or recovery.')
    if declaration:report['claims']=declaration['claims']
    report.update(success=False,status='running')
    (run/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    try:
        for mode in args.case or CASES:
            case=run/mode;case.mkdir();capture=case/'capture';capture.mkdir();channel=case/'commands.bin';frame=case/'frame.bin'
            header=bytearray(ring.initial_header());struct.pack_into('<I',header,16,123)
            live.create(channel,header,ring.SIZE);live.create(frame,live.frame_v1.initial_header(),live.frame_v1.SIZE)
            shutil.copyfile(dll,case/dll.name)
            (case/'selftest.exe').write_bytes(stage.add_import((dll.parent/'selftest.exe').read_bytes(),dll='MnmRender.dll',symbol_name='RenderAnchor',section_name=b'.mnmgl'))
            output,active=case/'qt.json',case/'producer.active'
            child=dict(env,MNM_RENDER_ORDERED_COPIES='1' if mode in METADATA_CASES or mode in ['cross-key', 'key-timeout', 'cross-clipper', 'clipper-timeout', 'cross-palette', 'palette-timeout', 'indexed','cross-dc','dc-timeout','cross-flip','flip-timeout','cross-lock','lock-timeout','cross-fast','cross-copy','copy-timeout','source-write','source-key','dc32','mx16'] else '0',MNM_RENDER_PALETTE_RESOURCES='1' if args.palette_resources else '0',MNM_MUTATION_SELFTEST=mode,MNM_RENDER_CONTINUOUS='0' if mode=='bounded' else '1',MNM_RENDER_OWNED_SESSION='1',MNM_RENDER_SESSION_ARCHIVE='1',MNM_RENDER_COMMAND_CHANNEL='Z:'+str(channel).replace('/', '\\'),MNM_RENDER_STREAM='Z:'+str(frame).replace('/', '\\'),MNM_RENDER_LOCK_CAPTURE_DIR='Z:'+str(capture).replace('/', '\\'))
            valid=(mode not in METADATA_CASES or mode in ['cross-desc','cross-attached']) and mode not in ['bounded','partial','unsupported','palette-flags','meta-change','source-write','source-key','copy-timeout','lock-timeout','dc-timeout','flip-timeout','key-timeout','clipper-timeout','palette-timeout']
            with (case/'qt.log').open('w') as qlog,(case/'wine.log').open('w') as wlog:
                qt=subprocess.Popen([str(args.build.resolve()/'live-render-channel-test'),str(channel),str(active),str(output)],env=env,stdout=qlog,stderr=qlog);wine=None
                try:
                    live.wait_ready(Path(str(output)+'.ready'),qt)
                    wine=subprocess.Popen(['wine',str(case/'selftest.exe')],cwd=case,env=child,stdout=wlog,stderr=wlog)
                    active.write_text(str(wine.pid));status=wine.wait(timeout=35)
                    assert status==0,(mode,status,(case/'wine.log').read_text());active.unlink()
                    assert qt.wait(timeout=35)==(0 if valid else 8),(mode,output.read_text())
                finally:
                    for process in [wine,qt]:
                        if process is not None and process.poll() is None:process.terminate();process.wait(timeout=5)
            observed=json.loads(output.read_text());expected=frames((case/'mutation-frames.bin').read_bytes());counts=struct.unpack('<10I',(case/'mutation-counts.bin').read_bytes())
            expected_count=201 if (mode.startswith('mx') or mode=='meta16') else 281 if mode=='indexed' else 13 if mode.startswith('dc') and mode!='dc-timeout' else 6 if mode=='reshape' else 2
            if mode in ['cross-flip','flip-timeout','cross-palette','palette-timeout']:expected_count=3
            if mode=='indexed' and args.palette_resources:expected_count+=40
            assert len(expected)==expected_count,(mode,len(expected),expected_count)
            if mode in ['cross-palette','palette-timeout']:assert len(set(expected))==3,'Independent palette phases must be distinguishable'
            displayed=expected if valid or mode in METADATA_CASES and mode.startswith('cross-') else expected[:1]
            assert observed['frames']==displayed,(mode,observed,displayed)
            assert observed['success']==valid and observed['presentations']==len(displayed) and observed['before_producer_exit']
            assert observed.get('native_readbacks',0)==observed.get('rgba_readbacks',0)==observed['viewport_uploads']==observed.get('live_surfaces',0)==0
            if mode in METADATA_CASES:
                want=(2,2,3 if 'desc' in mode else 1,2,1,3 if 'attached' in mode else 1,1,0,0,0)
                assert struct.unpack('<I',(case/'metadata-counts.bin').read_bytes())==(2,)
            elif (mode.startswith('mx') or mode=='meta16') or mode=='indexed':want=(123,123,1,1,1,1,121,41,81 if mode=='indexed' else 0,0)
            elif mode=='source-key':want=(2,2,1,2,2,1,1,0,0,0)
            elif mode in ['cross-key','key-timeout']:want=(2,2,1,2,3,1,1,0,0,0)
            elif mode in ['cross-clipper','clipper-timeout']:want=(2,2,1,4,1,1,1,0,0,0)
            elif mode in ['cross-palette','palette-timeout']:want=(2,2,1,2,1,1,1,0,2,0)
            elif mode in ['cross-dc','dc-timeout']:want=(2,2,1,2,1,1,1,0,0,2)
            elif mode in ['cross-flip','flip-timeout']:want=(2,2,1,2,1,1,1,1,0,0)
            elif mode.startswith('dc'):want=(1,1,0,0,0,0,0,0,0,25)
            elif mode=='reshape':want=(6,6,0,0,0,0,0,0,1,0)
            elif mode in ['bounded','partial']:want=(3,3,0,0,0,0,0,0,0,0)
            elif mode in ['cross-lock','lock-timeout']:want=(3,3,1,2,1,1,1,0,0,0)
            elif mode in ['source-write','source-key','cross-fast','cross-copy','copy-timeout']:want=(2,2,1,2,1,1,2,0,0,0)
            elif mode in ['unsupported','meta-change']:want=(2,2,2 if mode=='meta-change' else 1,1,1,1,1,0,0,0)
            else:want=(1,1,0,0,0,0,0,0,1,0)
            if mode=='indexed' and args.palette_resources:want=(203,203,1,1,1,1,121,41,81,0)
            if mode=='meta16':want=(123,123,201,1,1,1,121,41,0,0)
            assert counts==want,(mode,counts,want)
            records=archive((capture/'session-00000001.bin').read_bytes());ops=[r['op'] for r in records]
            assert ops[-1]==(8 if valid else 9)
            if mode!='bounded':assert not any(op in [5,10] for op in ops)
            if (mode.startswith('mx') or mode=='meta16') or mode=='indexed':
                assert all(op in ops for op in [1,2,3,6,7,8,11]) and ops.count(11)==40 and ops.count(3)==80
                assert ops.count(1)==(82 if mode=='indexed' and args.palette_resources else 2) and ops.count(7)==ops.count(1) and ops.count(2)==159
            if mode=='indexed' and args.palette_resources:
                assert ops.count(12)==ops.count(15)==41 and ops.count(14)==42
                assert [r['fields'][2:] for r in records if r['op']==13]==[[5,2]]*40
                created=[tuple(r['fields']) for r in records if r['op']==12];deleted=[tuple(r['fields']) for r in records if r['op']==15]
                assert created==deleted and len(set(created))==41
                assert [x[0] for x in created]==list(range(1,42)) and all(b[1]>a[1] for a,b in zip(created,created[1:]))
            if mode=='indexed' and not args.palette_resources:
                colors=[r['fields'][1:] for r in records if r['op']==4]
                assert colors==[[0,256]]+[[5,2]]*40,colors
            if mode=='reshape':
                created=[r['fields'] for r in records if r['op']==1];assert [r[0] for r in created]==[1,2,3,4,5]
                assert [r[3] for r in created]==[32,16,16,24,8] and ops.count(2)==0 and ops.count(7)==5
                assert created[2][4:7]==[0x7c00,0x3e0,0x1f]
            control=channel.read_bytes()[:64];published,state,reason=struct.unpack_from('<III',control,20)
            assert state==(2 if valid else 3) and reason==(0 if valid else 2)
            if valid:assert struct.unpack_from('<I',control,36)[0]==published
            if mode in METADATA_CASES or mode in ['cross-key', 'key-timeout', 'cross-clipper', 'clipper-timeout', 'cross-palette', 'palette-timeout','cross-dc','dc-timeout','cross-flip','flip-timeout','cross-lock','lock-timeout','cross-fast','cross-copy','copy-timeout','source-write','source-key']:
                diagnostics=(capture/'lifecycle.log').read_text()
                assert ('copy_order_wait_acquired ' if mode in METADATA_CASES and mode.startswith('cross-') or mode in ['cross-copy','cross-fast','cross-lock','cross-dc','cross-flip','cross-key','cross-clipper','cross-palette'] else 'copy_order_timeout ' if mode in METADATA_CASES and mode.endswith('-timeout') or mode in ['copy-timeout','lock-timeout','dc-timeout','flip-timeout','key-timeout','clipper-timeout','palette-timeout'] else 'copy_order_reentrant ') in diagnostics
            if mode in ['source-write','source-key']:
                conflicts=[list(map(lambda x:int(x,16),line.split()[1:])) for line in (capture/'lifecycle.log').read_text().splitlines() if line.startswith('blit_commit_refused ')]
                assert any(c[3] and c[4]==c[5] and c[6]!=c[7] and c[8]==c[9] and c[14]==(6 if mode=='source-key' else 8) for c in conflicts),'Nested source mutation did not invalidate pending copy'
            report['cases'].append(dict(mode=mode,valid_session=valid,original_counts=list(counts),original_frames=len(expected),published_bytes=published,state=state,reason=reason,opcodes={str(op):ops.count(op) for op in set(ops)},**observed))
            print(mode+': passed',flush=True)
        assert all(live.sha(ROOT/p)==h for p,h in fingerprints.items()),'Source changed during execution'
        report.update(sources_unchanged=True,status='passed',palette_resources=args.palette_resources,success=True,full_frame_comparisons=sum(c['presentations'] for c in report['cases']),valid_sessions=sum(c['valid_session'] for c in report['cases']),refused_sessions=sum(not c['valid_session'] for c in report['cases']))
        (run/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(run/'report.json',flush=True)
    finally:
        # This exact private prefix belongs only to this fixture invocation.
        subprocess.run(['wineserver','-k'],env=env,stdout=subprocess.DEVNULL,
                       stderr=subprocess.DEVNULL,timeout=15,check=False)



if __name__=='__main__':main()
