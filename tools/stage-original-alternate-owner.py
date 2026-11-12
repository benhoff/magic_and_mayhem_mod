#!/usr/bin/env python3
"""Add the allocation repair import after verifying the exact original code."""
import argparse
import hashlib
import importlib.util
import json
import re
import shutil
import subprocess
from pathlib import Path
from original_x86 import PE32

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'

def stage_bytes(raw,allow_existing_imports=False):
 image=PE32(raw);original=PE32((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes())
 if original.sha256!=BUILD:raise ValueError('Unsupported immutable original')
 if not allow_existing_imports and image.sha256!=BUILD:raise ValueError('Require pinned original executable')
 # Existing explicitly staged observation imports may be retained, but every
 # original stored section and original entry point must still match exactly.
 if image.base!=original.base or image.entry!=original.entry:raise ValueError('Original image layout changed')
 for s in original.sections:
  other=image.section(s['address'])
  if not other or any(other[k]!=s[k] for k in ['address','stored','virtual','offset','executable']) or image.read(s['address'],s['stored'])!=original.read(s['address'],s['stored']):raise ValueError('Original section bytes changed')
 spec=importlib.util.spec_from_file_location('owner_import',ROOT/'tools/prepare-shadow-experiment.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 result=m.add_import(raw,dll='MnmAlt.dll',symbol_name='AlternateOwnerAnchor',section_name=b'.mnalt')
 out=PE32(result)
 assert all(out.read(s['address'],s['stored'])==original.read(s['address'],s['stored']) for s in original.sections)
 return result

def stage_installation(source,output,report):
 from coverage_evidence import write_report,evidence_digest
 from experiment_environment import stage
 source=Path(source).resolve();output=Path(output).resolve()
 if output.exists() or not output.is_relative_to(ROOT/'working') or not (source/'Chaos.exe').is_file():raise ValueError('Require an existing installation and a fresh owned working output')
 output.mkdir(parents=True);(output/'.retain').write_text('Owned opt-in original allocation repair installation and staging evidence.\n')
 result=dict(schema=1,success=False,scope='Authored guarded alternate owner staging, not game execution or historical allocation recovery.',original_allocation_recovered=False,sources={})
 def load(name,path):
  spec=importlib.util.spec_from_file_location(name,ROOT/path);m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
 def sha(path):
  with path.open('rb') as f:return hashlib.file_digest(f,'sha256').hexdigest()
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],cwd=ROOT,check=True)
 try:
  before={p.relative_to(source).as_posix():sha(p) for p in sorted(source.rglob('*')) if p.is_file()}
  game=output/'game';stage(ROOT,game,source,kind='game',persistent=True)
  for n in ['Chaos.exe','MnmAlt.dll','CFG/chaos.cfg','CFG/Encrypted/chaos.cfg']:
   if not (game/n).resolve().is_relative_to(game):raise ValueError('Escaping repair installation write: '+n)
  builder=load('original_owner_builder','tools/build-original-alternate-owner.py');dll=builder.build(output/'build')
  raw=(ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes();patched=stage_bytes(raw)
  (game/'Chaos.exe').write_bytes(patched);shutil.copyfile(dll,game/'MnmAlt.dll')
  encoder=load('owner_cfg_encoder','tools/encode-cfg.py');decoder=encoder.load_decoder(ROOT);configs=[]
  for path in [game/'CFG/chaos.cfg',game/'CFG/Encrypted/chaos.cfg']:
   if not path.is_file():continue
   packed=path.parent.name=='Encrypted';raw=path.read_bytes();text=decoder.decode(raw)[1] if packed else raw
   edited,count=re.subn(rb'(?im)^\s*;?UseMallocedBackBuffer\s*=\s*(TRUE|FALSE)[^\r\n]*',b'UseMallocedBackBuffer=TRUE',text)
   if count!=1:raise ValueError('Ambiguous DEBUG/UseMallocedBackBuffer configuration')
   data=encoder.encode(edited,decoder)[0] if packed else edited
   if packed and decoder.decode(data)[1]!=edited:raise ValueError('Configuration round trip')
   path.write_bytes(data);configs.append(dict(path=path.relative_to(game).as_posix(),sha256=sha(path)))
  if not configs:raise ValueError('Missing original configuration')
  after={p.relative_to(source).as_posix():sha(p) for p in sorted(source.rglob('*')) if p.is_file()}
  if before!=after:raise ValueError('Source installation changed')
  manifest=json.loads((output/'build/manifest.json').read_bytes());names=set(manifest['compiled_sources'])|{'tools/stage-original-alternate-owner.py','tools/build-original-alternate-owner.py','tools/prepare-shadow-experiment.py','tools/original_x86.py','tools/encode-cfg.py','tools/decode-cfg.py','tools/experiment_environment.py','tools/original-manifest.sh'}
  result.update(success=True,original_sha256=BUILD,game=str(game),staged_sha256=sha(game/'Chaos.exe'),dll_sha256=sha(game/'MnmAlt.dll'),configs=configs,source_installation=str(source),source_inputs=before,source_unchanged=True,original_stored_sections_unchanged=True,sources={n:evidence_digest(ROOT/n,root=ROOT) for n in sorted(names)},enable_environment={'MNM_ORIGINAL_ALTERNATE_OWNER':'1'},optional_diagnostic_environment='MNM_ORIGINAL_ALTERNATE_LOG=<fresh Windows path>')
 except (OSError,ValueError,subprocess.SubprocessError) as error:result['error']=str(error)
 finally:
  subprocess.run([ROOT/'tools/original-manifest.sh','verify'],cwd=ROOT,check=True);result['original_manifest_verified_before_after']=True
  write_report(ROOT,report,(json.dumps(result,indent=2)+'\n').encode())
 return result

if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('input',type=Path);p.add_argument('output',type=Path);p.add_argument('--installation',action='store_true',help='Clone assets, build/install the DLL and enable the original DEBUG option');p.add_argument('--report',help='New SQLite staging report path, required for --installation');a=p.parse_args()
 if a.installation:
  if not a.report:p.error('--installation requires --report')
  value=stage_installation(a.input,a.output,a.report);print(json.dumps(value if not value['success'] else dict(success=True,game=value['game'],enable_environment=value['enable_environment']),indent=2));raise SystemExit(0 if value['success'] else 1)
 if a.report:p.error('--report requires --installation')
 if a.output.exists() or not a.output.resolve().is_relative_to(ROOT/'working'):p.error('Require a new owned working output')
 subprocess.run([ROOT/'tools/original-manifest.sh','verify'],cwd=ROOT,check=True)
 try:
  raw=stage_bytes(a.input.read_bytes());a.output.write_bytes(raw);print(hashlib.sha256(raw).hexdigest())
 finally:subprocess.run([ROOT/'tools/original-manifest.sh','verify'],cwd=ROOT,check=True)
