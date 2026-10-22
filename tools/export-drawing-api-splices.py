#!/usr/bin/env python3
"""Plan exact original CALL splices alongside existing census hooks; no patch."""
from coverage_evidence import read_bytes as evidence_bytes, read_text as evidence_text
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
from drawing_census import ROOT, BUILD_SHA256, instructions
from drawing_api_journal import plan, breakpoint_plan
from drawing_api_sites import call_mode


def load(name, relative):
    spec=importlib.util.spec_from_file_location(name,ROOT/relative)
    module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module);return module


def export(executable, boundaries, api_only=False):
    paths=['tools/export-drawing-api-splices.py','tools/drawing_api_journal.py','tools/drawing_api_pairs.py','tools/drawing_api_sites.py','tools/drawing_census.py','tools/build-drawing-census.py','tools/export-menu-support.py','runtime/scene/canvas_producers.c','research/runtime/drawing-inventory.json','research/runtime/coverage/binary-nocd.json',str(boundaries.relative_to(ROOT))]
    sources={p:hashlib.sha256(evidence_bytes(ROOT/p)).hexdigest() for p in paths}
    raw=evidence_bytes(executable)
    if hashlib.sha256(raw).hexdigest()!=BUILD_SHA256:raise ValueError('Unsupported original executable')
    selected=json.loads(evidence_text(boundaries))
    if selected['source_sha256']!=BUILD_SHA256 or selected['image_base']!=0x400000 or selected['input_unchanged'] is not True:
        raise ValueError('API boundary build/base/input mismatch')
    code=instructions(subprocess.check_output(['objdump','-w','-d','-Mintel',str(executable)],text=True))
    census,core_refused=load('api_core_splices','tools/build-drawing-census.py').hook_rows(executable)
    if api_only:census=[]
    occupied=[(r['entry'],r['entry']+r['length']) for r in census]
    offset=load('api_image_offsets','tools/export-menu-support.py').image_offset
    hooks=[];refused=[];seen=set()
    for boundary in selected['calls']:
        site=boundary['site']
        if type(site) is not int or site in seen:raise ValueError('Duplicate/invalid API boundary site')
        seen.add(site)
        if code[site]['bytes']!=boundary['bytes'] or code[site]['operand']!=boundary['operand'] or call_mode(code[site])!=boundary['target_mode']:
            raise ValueError('API boundary no longer matches exact original CALL')
        try:
            try:
                item=plan(site,code,occupied)
            except ValueError as error:
                if str(error) != 'Incoming original direct flow enters overwritten interior': raise
                item=breakpoint_plan(site,code,occupied)
                item['five_byte_refusal']=str(error)
            signature=bytes.fromhex(item['signature'])
            if any(not any(start<=site and site+item['length']<=end for start,end in selection['ranges']) for selection in boundary['selections']):
                raise ValueError('API splice crosses selected original range')
            start=offset(raw,site,len(signature))
            if raw[start:start+len(signature)]!=signature:raise ValueError('Original file-backed splice bytes mismatch')
            item['tag']=len(hooks);item['selections']=boundary['selections'];item['discovered_owners']=boundary['discovered_owners'];item['exact_import']=boundary['exact_import']
            hooks.append(item);occupied.append((site,site+item['length']))
        except (KeyError,ValueError) as error:refused.append({'site':site,'reason':str(error)})
    if not hooks or len(hooks)>512:raise ValueError('API hook count outside bounded capacity')
    if hashlib.sha256(evidence_bytes(executable)).hexdigest()!=BUILD_SHA256:raise ValueError('Original executable changed')
    if sources!={p:hashlib.sha256(evidence_bytes(ROOT/p)).hexdigest() for p in paths}:raise ValueError('Planner sources changed during export')
    return {'schema':1,'scope':'Static exact original CALL splice plan in explicit '+('API-only, core-absent' if api_only else 'core/API')+' cohort. No actual hook execution, API result/ABI/output/ownership/borrowing or DRAW-01 closure.','cohort':'api_only' if api_only else 'core_api','original_sha256':BUILD_SHA256,'input_unchanged':True,'sources_stable':True,'hooks':hooks,'refused':refused,'core_hook_count':len(census),'core_refused':core_refused,'counts':{'selected':len(selected['calls']),'planned':len(hooks),'refused':len(refused)},'sources':sources}


if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--executable',type=Path,required=True);parser.add_argument('--boundaries',type=Path,required=True);parser.add_argument('--output',type=Path,required=True);parser.add_argument('--api-only',action='store_true');args=parser.parse_args()
    out=args.output.resolve();boundaries=args.boundaries.resolve()
    if not out.is_relative_to(ROOT) or out.relative_to(ROOT).parts[0] in ('original','.git') or out.exists():raise ValueError('Output must be a new repository-owned artifact')
    subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        record=export(args.executable,boundaries,args.api_only);out.parent.mkdir(parents=True,exist_ok=True)
        with out.open('x') as f:json.dump(record,f,indent=2);f.write('\n')
        print(json.dumps(record['counts']))
    finally:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True)
