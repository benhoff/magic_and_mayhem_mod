#!/usr/bin/env python3
"""Attribute a finite presentation interval without accepting a failed session."""
import argparse
import hashlib
import json
from pathlib import Path
import struct
import subprocess

from coverage_evidence import evidence_digest, read_json, write_report
from drawing_corpus import artifact
from drawing_egl_physical import analyze_egl_physical
from drawing_egl_swap_completion import analyze as completion
from storage_upload_causality import admit_namespace, analyze_original

ROOT=Path(__file__).resolve().parents[1]
BUILD='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def verify_original_sections(stock, staged):
    if hashlib.sha256(stock).hexdigest()!=BUILD:raise ValueError('Original executable build')
    pe=struct.unpack_from('<I',stock,60)[0]
    count=struct.unpack_from('<H',stock,pe+6)[0]
    table=pe+24+struct.unpack_from('<H',stock,pe+20)[0]
    for i in range(count):
        size,at=struct.unpack_from('<II',stock,table+40*i+16)
        if stock[at:at+size]!=staged[at:at+size]:raise ValueError('Original section bytes differ')
    return count


def legacy_packaging(root):
    catalog_path=root/'tests/fixtures/drawing-storage-session/session.json'
    catalog=json.loads(catalog_path.read_bytes())
    register=json.loads((root/'research/runtime/coverage/register.json').read_bytes())
    evidence={e['id']:e for e in register['evidence']}
    retained=evidence['DRAW02.storage-session.retention-20261016']
    if read_json(root/retained['record'],root=root)!=catalog:raise ValueError('Legacy catalog/retention differs')
    e=evidence[catalog['evidence']]
    if e['sha256']!=catalog['original_report_sha256'] or evidence_digest(root/e['record'],root=root)!=e['sha256']:
        raise ValueError('Legacy original report identity')
    original=read_json(root/e['record'],root=root)
    rows=original['egl_physical_observation']['frames']
    if len(rows)!=238 or len(catalog['frames'])!=238 or original['source_sha256']!=BUILD:
        raise ValueError('Legacy complete reference denominator/build')
    for saved,row in zip(catalog['frames'],rows,strict=True):
        raw=artifact(root,saved['oracle'])
        if (saved['ordinal']!=row['ordinal'] or len(raw)!=row['width']*row['height']*4 or
                hashlib.sha256(raw).hexdigest()!=row['sha256']):raise ValueError('Legacy physical plane/ordinal changed')
    return dict(frames=238,every_original_physical_plane_byte_verified=True,
        catalog_sha256=hashlib.sha256(catalog_path.read_bytes()).hexdigest(),original_record=e['record'],
        original_report_sha256=e['sha256'],reference_packaging_changed=False,
        exact_historical_upload_source_available=False,
        scope='Lossless retained planes match original acquisition hashes; no new original capture or unique historical120-row attribution.')


def main():
    p=argparse.ArgumentParser(description=__doc__)
    for name in ('capture-report','claims','isolation-record','report'):p.add_argument('--'+name,type=Path,required=True)
    a=p.parse_args()
    if a.report.exists():p.error('Require a new immutable diagnosis path')
    declaration=json.loads(a.claims.read_bytes());sources=declaration['sources']
    stable=lambda:all(evidence_digest(ROOT/n,root=ROOT)==h for n,h in sources.items())
    if not stable():raise ValueError('Prospective analysis source changed')
    result=dict(schema=1,success=False,sources=sources,claims=declaration['claims'],
        original_execution=False,native_replay_performed=False,original_session_accepted=False,
        normal_quit_accepted=False,source_session_accepted=False,full_renderer_accepted=False,
        scope='Read-only attribution of a separately closed EGL physical interval and complete individual upload points. '
              'The input source-recorder session may remain failed and terminated; it receives no source/session/normal-Quit/native acceptance. '
              'No original oracle is a native input, no reference is rewritten, and private driver allocation internals remain unproved.')
    verified=False
    try:
        result['isolation']=admit_namespace(ROOT,a.isolation_record)
        subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True);verified=True
        parent=read_json(a.capture_report,root=ROOT)
        if (parent.get('sources_stable') is not True or parent.get('original_manifest_verified_before_after') is not True or
                parent.get('isolation',{}).get('private_mount_and_network') is not True):
            raise ValueError('Input acquisition lacks source/manifest/private namespace provenance')
        child=a.capture_report.with_name(a.capture_report.stem+'-original.json')
        original=read_json(child,root=ROOT)
        if original.get('success') is not True and original.get('error')!='Session source interval did not close quiescently':
            raise ValueError('Only an independently closed presentation interval may survive source-closure refusal')
        run=Path(original['experiment']).resolve()
        if not run.is_relative_to(ROOT/'working'):raise ValueError('Capture outside owned working storage')
        capture=run/'capture';manifest=json.loads((run/'manifest.json').read_bytes())
        staged=(run/'game/Chaos.exe').read_bytes()
        if hashlib.sha256(staged).hexdigest()!=manifest['binaries']['Chaos.exe'] or manifest.get('original_sections_unchanged') is not True or manifest.get('api_observer_enabled') is not True:
            raise ValueError('Original staged executable/observation identity')
        sections=verify_original_sections((ROOT/'original/Arcane_Nocd/Chaos.exe').read_bytes(),staged)
        library=manifest['x11_delivery_library']
        if hashlib.sha256(Path(library['path']).read_bytes()).hexdigest()!=library['sha256']:
            raise ValueError('Original observer library changed')
        helper=parent.get('observer_build',{})
        if parent['profile']!='plain' and hashlib.sha256(Path(helper['helper']).read_bytes()).hexdigest()!=helper['helper_sha256']:
            raise ValueError('Original SHM diagnostic dependency changed')
        flow=json.loads((run/'storage-flow.json').read_bytes())
        if flow.get('success') is not True or flow.get('display_rebuilds')!=[1,0]:raise ValueError('Storage/rebuild stimulus incomplete')
        physical=analyze_egl_physical(capture,run/'egl-physical-report.json')
        observed=completion(capture,physical)
        profile=parent['profile']
        diagnosis=analyze_original(dict(egl_physical_observation=physical,egl_swap_completion_observation=observed),
                                   capture,require_samples=profile!='plain',point_only=profile!='plain')
        if profile=='plain':
            if not diagnosis['physical_mismatch_ordinals'] or set(diagnosis['physical_mismatch_ordinals'])-set(diagnosis['shrink_ordinals']):
                raise ValueError('Plain observer does not reproduce a shrink-only presentation difference')
            classification='original_presentation_control_without_shm_sampling'
        elif profile=='baseline':
            failed=[r for r in diagnosis['upload_samples'] if r['after_nominal_destination']]
            if (not diagnosis['every_physical_difference_explained'] or not failed or
                    any(r.get('geometry_delta_destination_mismatches')!=0 for r in failed)):
                raise ValueError('Actual source bytes do not establish the source-window attribution')
            classification='original_software_upload_uses_wrong_source_window'
        elif profile=='corrected':
            if diagnosis['physical_mismatch_ordinals'] or any(r['submitted_physical_mismatches'] for r in diagnosis['upload_samples']):
                raise ValueError('Corrected submitted source differs from independent output')
            applied=[p for r in observed['frames'] for p in r['xcb_images'] if p['resize_crop_applied']]
            if len(applied)!=1:raise ValueError('Corrected profile requires one explicit independently witnessed crop')
            classification='explicit_corrected_upload_matches_source_destination_and_output'
        else:raise ValueError('Unknown acquisition profile')
        result.update(classification=classification,diagnosis=diagnosis,legacy_reference=legacy_packaging(ROOT),
            input_original_execution_observed=True,input_original_collector_success=original.get('success') is True,
            input_original_collector_error=original.get('error'),input_normal_quit_verified=original.get('normal_original_quit_verified') is True,
            original_sections_compared=sections,source_sha256=BUILD,
            inputs={a.capture_report.relative_to(ROOT).as_posix() if a.capture_report.is_absolute() else a.capture_report.as_posix():evidence_digest(a.capture_report,root=ROOT),
                    child.relative_to(ROOT).as_posix() if child.is_absolute() else child.as_posix():evidence_digest(child,root=ROOT)},
            capture=run.relative_to(ROOT).as_posix(),success=True)
    except Exception as error:result['error']=repr(error)
    finally:
        if verified:
            try:subprocess.run([str(ROOT/'tools/original-manifest.sh'),'verify'],check=True);result['original_manifest_verified_before_after']=True
            except Exception as error:result.update(success=False,manifest_error=repr(error))
        result['sources_stable']=stable();result['success'] &= result['sources_stable']
        write_report(ROOT,a.report,(json.dumps(result,indent=2)+'\n').encode())
    print(json.dumps({k:result.get(k) for k in ('success','classification','error','original_session_accepted')}))
    return 0 if result['success'] else 1


if __name__=='__main__':raise SystemExit(main())
