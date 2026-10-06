#!/usr/bin/env python3
"""Offline v3 clip state/HRESULT/native-pixel and fragmented GPU replay checks."""
import argparse
import gzip
import hashlib
import json
import os
from pathlib import Path
import struct
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'protocols/python'))
from mnm_protocols import render_stream_v3 as wire

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def words(*values):return struct.pack('<'+'I'*len(values),*(v&0xffffffff for v in values))
def pixels(values):return struct.pack('<'+'H'*len(values),*values)
def load():
    parent=ROOT/'tests/fixtures/surfaces';m=json.loads((parent/'driver-clipping-surface2-corpus.json').read_text());assert m['path']=='driver-clipping-surface2-original.json.gz'
    p=parent/m['path'];assert sha(p)==m['sha256'];raw=gzip.decompress(p.read_bytes());assert hashlib.sha256(raw).hexdigest()==m['raw_sha256'];r=json.loads(raw);assert len(r)==408;return r

def stream(rows):
    records=[]
    def add(op,payload=b''):
        sequence=len(records)+1;records.append(words(op,sequence,len(payload))+payload);return sequence
    for i,r in enumerate(rows):
        s,d=2*i+1,2*i+2;c=r['input']
        for id,key in [(s,'source_before'),(d,'destination_before')]:add(1,words(id,8,6,16,0xf800,0x7e0,31)+pixels(r[key]))
        regions={0:[],1:[[2,1,6,5]],2:[[0,0,3,2],[5,3,8,6]],3:[],4:[]}[c['clip']]
        mode=0 if not c['clip'] else 2 if c['clip']==3 else 1
        add(16,words(d,mode,len(regions))+b''.join(words(*a) for a in regions))
        q=add(17,words(s,d,c['fast'],c['flags'],c['held'],*c['source'],*c['destination']))
        add(18,words(q,r['hresult']))
        add(5,words(s)+pixels(r['source_after']));add(5,words(d)+pixels(r['destination_after']))
        if i==len(rows)-1:add(6,words(d))
        add(7,words(s));add(7,words(d))
    add(8);return wire.initial_header()+b''.join(records)

def small_stream(record):return stream([record])

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/renderer');p.add_argument('--report',type=Path);a=p.parse_args()
    if a.report and a.report.exists():p.error('Refusing to overwrite evidence')
    parent=ROOT/'working/tests/native-clipping-stream';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent))
    if not a.report:a.report=run/'report.json'
    paths=['renderer/blit.cpp','renderer/blit.hpp','renderer/surface_copy.cpp','renderer/surface_copy.hpp','renderer/commands.cpp','renderer/commands.hpp','renderer/command_state.hpp','renderer/command_consumer.cpp','renderer/commands_main.cpp','renderer/CMakeLists.txt','tests/render-clipping-stream-test.cpp','tools/check-native-clipping-stream.py','protocols/schemas/render_stream-v3.json','protocols/include/mnm/render_stream_v3.h','protocols/python/mnm_protocols/render_stream_v3.py','protocols/generate.py','tests/fixtures/surfaces/driver-clipping-surface2-corpus.json','tests/fixtures/surfaces/driver-clipping-surface2-original.json.gz']
    sources={n:sha(ROOT/n) for n in paths};env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1');build=a.build.resolve()
    # Independently specified identity and numeric contract, not generated expectations.
    assert wire.initial_header()==b'MNMCMD03'+words(3,16)
    assert (wire.OPERATION_CLIPPER_SET,wire.OPERATION_SURFACE_COPY,wire.OPERATION_SURFACE_RESULT_CHECK,wire.CLIP_REGION_CAPACITY,wire.SURFACE_COPY_WORDS)==(16,17,18,32,13)
    rows=load();data=stream(rows);file=run/'driver-cases.cmd';file.write_bytes(data)
    def command(label,path,success):
        output=run/(label+'.pixels');result=subprocess.run([str(build/'mnm-render-commands'),str(path),'--output',str(output)],env=env,capture_output=True,text=True,timeout=60)
        (run/(label+'.log')).write_text(result.stdout+result.stderr);report=json.loads(result.stdout)
        assert report['rendered']==success,(label,report)
        assert (result.returncode==0)==success
        if not success:assert not output.exists(),'Failed replay published output'
        return report
    complete=command('complete',file,True)
    assert complete['surface_copies']==408 and complete['result_checks']==408 and complete['checks']==816 and complete['surface_copy_failures']==312
    assert (run/'complete.pixels').read_bytes()==pixels(rows[-1]['destination_after'])
    result=subprocess.run([str(build/'render-clipping-stream-test'),str(file)],env=env,capture_output=True,text=True,timeout=60)
    (run/'fragmented.log').write_text(result.stdout+result.stderr);assert result.returncode==0,result.stdout+result.stderr;fragmented=json.loads(result.stdout)
    # v3 inherits session-qualified indexed palettes from v2.
    palette_records=[]
    def palette_add(op,payload=b''):
        palette_records.append(words(op,len(palette_records)+1,len(payload))+payload)
    colors=bytearray(768);colors[3:6]=bytes([255,0,0]);colors[6:9]=bytes([0,255,0])
    palette_add(1,words(1,2,1,8,0,0,0)+bytes([1,2]));palette_add(12,words(1,1)+colors)
    palette_add(14,words(1,1,1));palette_add(10,words(1)+bytes([255,0,0,255,0,255,0,255]));palette_add(6,words(1))
    palette_add(14,words(1,0,0));palette_add(7,words(1));palette_add(15,words(1,1));palette_add(8)
    palette_path=run/'palette-v3.cmd';palette_path.write_bytes(wire.initial_header()+b''.join(palette_records))
    palette_report=command('palette_v3',palette_path,True);assert palette_report['color_checks']==1 and (run/'palette_v3.pixels').read_bytes()==bytes([1,2])
    valid=small_stream(next(r for r in rows if r['input']['label']=='full' and r['input']['clip']==2 and not r['input']['fast'] and not r['input']['flags']))
    # Find record extents independently from framing bytes for targeted corruption.
    offsets=[];pos=16
    while pos<len(valid):
        op,seq,size=struct.unpack_from('<3I',valid,pos);offsets.append((op,pos,size));pos+=12+size
    clip=next(at for op,at,n in offsets if op==16);copy_at=next(at for op,at,n in offsets if op==17);check_at=next(at for op,at,n in offsets if op==18);pixel_check=next(at for op,at,n in offsets if op==5)
    bad={}
    def mutate(label,at,value):
        b=bytearray(valid);struct.pack_into('<I',b,at,value);bad[label]=bytes(b)
    mutate('count_overflow',clip+20,33);mutate('mode',clip+16,3);mutate('clip_outside',clip+24,0xffffffff)
    mutate('overlap',clip+24+16,1);b=bytearray(bad['overlap']);struct.pack_into('<I',b,clip+24+20,1);bad['overlap']=bytes(b)
    mutate('unknown_surface',clip+12,999)
    mutate('invalid_api',copy_at+20,2);mutate('busy_bits',copy_at+28,4);mutate('combined_flags',copy_at+24,3)
    mutate('wrong_result_sequence',check_at+12,1);mutate('wrong_hresult',check_at+16,0x88760096)
    mutate('wrong_pixels',pixel_check+16,0);mutate('sequence_gap',copy_at+4,999)
    for version in (1,2):
        b=bytearray(valid);b[:8]=f'MNMCMD0{version}'.encode();struct.pack_into('<I',b,8,version);bad['legacy_reject_'+str(version)]=bytes(b)
    bad['truncated']=valid[:-1];bad['trailing']=valid+b'junk'
    for label,b in bad.items():
        path=run/(label+'.cmd');path.write_bytes(b);command(label,path,False)
    assert all(sha(ROOT/n)==h for n,h in sources.items()),'Sources changed during validation'
    report=dict(schema=1,success=True,sources=sources,driver_cases=408,hresult_checks=408,pixel_checks=816,partial_failure_cases=sum(bool(r['hresult']) and r['destination_before']!=r['destination_after'] for r in rows),malformed_cases=len(bad),complete=complete,fragmented=fragmented,palette_v3=palette_report,binaries={name:sha(build/name) for name in ['mnm-render-commands','render-clipping-stream-test']},artifacts=str(run.relative_to(ROOT)),stream_sha256=sha(file),scope='Native v3 bounded/fragmented offline replay against retained Wine Surface2 outputs, explicit clip/busy observations, diagnostic HRESULT/native checks and owned cleanup. No original executable, native locks/loss/Restore, Windows driver or live v3 producer/replacement.')
    a.report.parent.mkdir(parents=True,exist_ok=True)
    with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
    print(json.dumps({k:report[k] for k in ['success','driver_cases','malformed_cases']}))
if __name__=='__main__':main()
