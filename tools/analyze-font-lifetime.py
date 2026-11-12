#!/usr/bin/env python3
"""Bind the actual installed font journal to acquisition, assets and retirement."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import struct
from coverage_evidence import Reader,write_report
from font_lifetime import decode,require
from rendering_menu_acceptance import artifact

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-lifetime-results-20261108/'

def analyze(reader,acquired):
    require(acquired['success'] and acquired['sources_stable'] and acquired['original_game_runs']==1
            and acquired['original_manifest_verified_before_after'] and acquired['private_wine_stopped']
            and acquired['prefix_pool']['returned'] and acquired['retirement']['state']=='completed',
            'Complete normally retired font acquisition required')
    refs=[v for v in acquired['artifacts'] if v['path'].endswith(('/font-lifetime-v1.bin','/font-lifetime-v2.bin'))]
    require(len(refs)==1,'Missing or ambiguous installed font observation')
    ref=refs[0];raw=reader.read_bytes(ref['record'])
    require(hashlib.sha256(raw).hexdigest()==ref['sha256'],'Changed font journal bytes')
    p=decode(raw);entries={};at=64
    for row in p['rows']:
        if row[2]==1 and row[5]==8:
            path=raw[at+128:at+row[0]].split(b'\0',1)[0].decode('cp1252').replace('\\','/')
            require(path and not path.startswith('/') and '..' not in Path(path).parts,'Unsafe original font path')
            entries[row[1]]=path
        at+=row[0]
    manifest=acquired['installation_inputs'];by_name={n.casefold():n for n in manifest}
    require(len(by_name)==len(manifest),'Ambiguous installed path case')
    assets={};token_assets={}
    for load in p['loads']:
        if not load['success']:continue
        token=load['token'];entry=p['tokens'][token]['entry'];path=entries[entry]
        require(path.casefold() in by_name,'Observed font path absent from frozen installation')
        name=by_name[path.casefold()]
        if name not in assets:
            data=(ROOT/'working/game-nocd'/name).read_bytes()
            require(hashlib.sha256(data).hexdigest()==manifest[name]['sha256'] and len(data)==manifest[name]['bytes'],'Observed original font asset differs')
            require(len(data)>=40 and struct.unpack_from('<3I',data)==(0x00544653,len(data),3),'Observed font must be complete SFTv3')
            assets[name]=dict(path=name,sha256=manifest[name]['sha256'],bytes=len(data),raw=data)
        token_assets[token]=assets[name]
    matched=Counter()
    for use in p['uses']:
        require(use['token'] in token_assets,'Glyph resource has no successful installed load')
        data=token_assets[use['token']]['raw'];off=use['offset'];size=use['bytes']
        require(off<=len(data)-size and size>=40 and struct.unpack_from('<I',data,off)[0]==size,'Glyph extent differs from independent installed asset')
        words=struct.unpack_from('<10I',data)
        count,rows,metrics=words[3],words[4],words[9]
        table=40+(768 if words[8] else 0)+rows*metrics*8
        base=table+count*4
        require(count<=4096 and base<=len(data),'Independent font frame table extent')
        offsets={base+struct.unpack_from('<I',data,table+i*4)[0] for i in range(count)}
        require(off in offsets,'Observed source is not an independently indexed SFT glyph')
        frame=bytearray(data[off:off+size]);frame[28:32]=bytes(4);h=2166136261
        for byte in frame:h=((h^byte)*16777619)&0xffffffff
        require(h==use['normalized_frame_fnv'],'Observed glyph diagnostic differs from independent asset')
        snapshot=p['frame_snapshots'].get((use['token'],off))
        if snapshot is not None:require(snapshot==frame,'Complete observed glyph source differs from independent asset')
        matched[(use['role'],use['token'])]+=1
    require(p['loads'] and p['uses'],'Actual installed load and glyph-use evidence is missing')
    require(not p['live_at_detach'],'Current font generations still live at DLL detach')
    successful=[v for v in p['loads'] if v['success']]
    require({v['role'] for v in successful}=={1,2,3,4} and any(v['prior'] for v in successful),
            'Required four roles and actual reload were not observed')
    return dict(journal=ref,records=len(p['rows']),loads=p['loads'],retired=p['retired'],
                bound_glyph_uses=len(p['uses']),font_generations=len(p['tokens']),
                roles_with_glyph_use=sorted({v['role'] for v in p['uses']}),
                uses_per_generation=[dict(role=role,token=token,uses=n) for (role,token),n in sorted(matched.items())],
                assets=assets,live_at_detach=p['live_at_detach'],
                diagnostic_hashes_checked=True,whole_glyph_bytes_observed=bool(p['frame_snapshots']),
                distinct_complete_source_snapshots=len(p['frame_snapshots']),
                presented_glyph_pixels_verified=False,dc_image_lifetime_verified=False,
                overwritten_allocations_retired=False,acceptance_ready=False,
                interesting_branches=len(successful)+len(p['uses']))

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--acquisition',required=True);p.add_argument('--report',required=True);a=p.parse_args()
    require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned result required')
    q=dict(schema=1,kind='installed_font_lifecycle_analysis',success=False,original_game_runs=0,
           newly_passed_requirements=0,pixel_output_verified=False,dc_image_lifetime_verified=False)
    with Reader(ROOT) as r:
        names=['tools/analyze-font-lifetime.py','tools/font_lifetime.py','tools/rendering_menu_acceptance.py','tools/coverage_evidence.py']
        sources={n:r.digest(n) for n in names};q['sources']=sources
        q['acquisition']=dict(record=a.acquisition,sha256=r.digest(a.acquisition))
        try:
            q.update(analyze(r,r.json(a.acquisition)))
            for asset in q['assets'].values():
                raw=asset.pop('raw');name=a.report.removesuffix('.json')+'-asset-'+hashlib.sha256(asset['path'].encode()).hexdigest()[:16]+'.bin'
                asset['retained']=dict(record=name,sha256=write_report(ROOT,name,raw))
            q['success']=True
        except Exception as e:
            q['error']=str(e)
        q['sources_stable']=sources=={n:r.digest(n) for n in names}
        q['success'] &= q['sources_stable']
    digest=write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode())
    print(json.dumps(dict(success=q['success'],record=a.report,sha256=digest,error=q.get('error'),uses=q.get('bound_glyph_uses'))),flush=True)
    return int(not q['success'])

if __name__=='__main__':raise SystemExit(main())
