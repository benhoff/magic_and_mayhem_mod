#!/usr/bin/env python3
"""Compare captured pre/post-load native World draws to unchanged original code.

The private original process receives a diagnostic native entry canvas; native
rendering never reads an original oracle. Two authored-baseline original runs
also count pixels equal under those two histories; this does not prove general
baseline independence. This is scoped
World raster proof, not independent presented-session or save-state equivalence.
"""
import argparse
import array
import hashlib
import json
import os
import shlex
from pathlib import Path
import subprocess
import tempfile
from campaign_storage import validate_storage_flow

ROOT = Path(__file__).resolve().parents[1]
ORIGINAL = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'


def sha(path):
    with path.open('rb') as f: return hashlib.file_digest(f, 'sha256').hexdigest()


def compare_words(actual, expected, zero, poison):
    if not len(actual) == len(expected) == len(zero) == len(poison) == 960000:
        raise ValueError('World checkpoint pixel extent')
    words = [array.array('H', b) for b in (actual, expected, zero, poison)]
    mismatch = sum(a != b for a, b in zip(words[0], words[1]))
    independent = [i for i, (a,b) in enumerate(zip(words[2],words[3])) if a == b]
    independent_mismatch = sum(words[0][i] != words[2][i] for i in independent)
    return dict(words=480000, mismatched_words=mismatch,
                two_baseline_equal_words=len(independent),
                two_baseline_equal_mismatches=independent_mismatch,
                two_baseline_different_words=480000-len(independent))


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--campaign', type=Path, required=True, help='Completed native campaign report.json')
    a = p.parse_args()
    parent=ROOT/'working/tests/native-storage-comparison';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print('Comparison evidence:',out,flush=True)
    report=dict(success=False,scope=__doc__,original_pixels_used_as_native_inputs=False,
                independent_presented_session_equivalence=False)
    inputs={};sources={};verified=False
    env={**os.environ,'QT_QPA_PLATFORM':'xcb','LIBGL_ALWAYS_SOFTWARE':'1','GALLIUM_DRIVER':'llvmpipe','LP_NUM_THREADS':'4'}
    def run(label, command, timeout=300):
        with (out/(label+'.log')).open('x') as log:
            subprocess.run([str(x) for x in command],cwd=ROOT,env=env,stdout=log,stderr=log,check=True,timeout=timeout)
    def pin(path): inputs[str(path.resolve())]=sha(path)
    try:
        campaign=a.campaign.resolve()
        if not campaign.is_relative_to(ROOT/'working/tests'):raise ValueError('Require an owned campaign report')
        game=json.loads(campaign.read_text());pin(campaign)
        if not all(game.get(k) is True for k in ('success','save_load_required','other_pixel_producers_required','sources_unchanged','normal_quit_verified','private_wine_stopped','original_manifest_verified_after')):
            raise ValueError('Campaign lacks completed stable native Save/Load evidence')
        checkpoints=validate_storage_flow(game['flow'])
        capture=Path(game['resident_capture']['path']);pin(capture)
        if inputs[str(capture.resolve())]!=game['resident_capture']['sha256']:raise ValueError('Capture changed after gameplay')
        stage=game['resident_world']['stage_profile']
        if not stage['capture']['complete'] or stage['capture_truncated'] or stage['capture']['error']:
            raise ValueError('Incomplete World source capture')
        sources=game['sources'].copy()
        if any(sha(ROOT/n)!=h for n,h in sources.items()):raise ValueError('Gameplay sources changed before replay')
        run('claims',['python3',ROOT/'tools/draft-coverage-claims.py','--behavior','NR.native-storage-world-comparison','--scenario','NR.native-storage-world-comparison.captured','--behavior','NP.native-producer-cpu-reader','--scenario','NP.native-producer-cpu-reader.controls','--output',out/'claims.json'])
        declaration=json.loads((out/'claims.json').read_text());sources.update(declaration['sources'])
        report.update(sources=sources,claims=declaration['claims'],campaign_report=str(campaign),campaign_report_sha256=sha(campaign))
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        pe=ROOT/'original/Arcane_Nocd/Chaos.exe';pin(pe)
        if sha(pe)!=ORIGINAL:raise ValueError('Original build mismatch')
        assets=Path(game['experiment'])/'game'
        for f in assets.rglob('*'):
            if f.is_file() and f.suffix.lower() in ('.spr','.pcx','.jpg','.jpeg','.bmp','.sft','.fp'):pin(f)
        (out/'checkpoints.json').write_text(json.dumps(checkpoints,indent=2)+'\n')
        build=ROOT/'working/build/native-storage-comparison'
        run('configure',['cmake','-S',ROOT/'tests/native-world-rolling','-B',build,'-DCMAKE_BUILD_TYPE=RelWithDebInfo','-DBUILD_TESTING=ON'])
        run('build',['cmake','--build',build,'--target','resident-storage-replay','canvas-native-reader-test','canvas-sequence-test','--parallel','4'],600)
        run('cpu-reader-controls',['xvfb-run','-a',build/'world-rolling/canvas-native-reader-test'])
        report['cpu_reader_controls']=json.loads((out/'cpu-reader-controls.log').read_text().splitlines()[-1])
        run('sequence-controls',[build/'world-rolling/canvas-producers/canvas-sequence-test'])
        binary=build/'world-rolling/resident-storage-replay'
        run('replay',['xvfb-run','-a',binary,capture,assets,out/'checkpoints.json',out/'native'],600)
        reference=out/'original-reference'
        run('reference-build',['g++','-m32','-std=c++17','-O2','-Wall','-Wextra','-Werror','-no-pie','-MD','-MF',out/'reference.d',ROOT/'tests/resident-storage-reference.cpp','-o',reference])
        compiled=set()
        for dep in [*build.rglob('*.o.d'),out/'reference.d']:
            for token in shlex.split(dep.read_text().replace('\\\n',' ')):
                path=Path(token).resolve()
                if path.is_file() and path.is_relative_to(ROOT) and not path.is_relative_to(ROOT/'working'):
                    compiled.add(path.relative_to(ROOT).as_posix())
        if not compiled or compiled-set(sources):raise ValueError('Undeclared comparison compiler dependencies: '+repr(sorted(compiled-set(sources))))
        report['compiled_sources']={n:sha(ROOT/n) for n in sorted(compiled)}
        if any(sources[n]!=h for n,h in report['compiled_sources'].items()):raise ValueError('Changed comparison compiler inputs')
        replay=json.loads((out/'native/replay.json').read_text())
        (out/'zero.565').write_bytes(bytes(960000));(out/'poison.565').write_bytes(b'\xa5\x5a'*480000)
        results=[]
        for row,checkpoint in zip(replay['checkpoints'],checkpoints,strict=True):
            if row['generation']!=checkpoint['source_generation']:raise ValueError('Checkpoint identity mismatch')
            base=out/'native'/row['base'];draws=Path(str(base)+'-draws.bin');actual=Path(str(base)+'-native.565')
            variants=[]
            for name,baseline in [('original',Path(str(base)+'-before.565')),('zero',out/'zero.565'),('poison',out/'poison.565')]:
                dest=out/(row['base']+'-'+name+'.565')
                run(row['base']+'-'+name,[reference,pe,draws,baseline,dest]);variants.append(dest.read_bytes())
            result=compare_words(actual.read_bytes(),*variants)
            results.append(dict(phase=checkpoint['phase'],generation=row['generation'],rasters=row['rasters'],draws=row['draws'],**result,
                                native_sha256=sha(actual),original_sha256=sha(out/(row['base']+'-original.565'))))
        report.update(replay=replay,checkpoints=results,original_binary_sha256=ORIGINAL,
                      native_binary_sha256=sha(binary),reference_binary_sha256=sha(reference),
                      success=all(not r['mismatched_words'] and not r['two_baseline_equal_mismatches'] and r['two_baseline_equal_words']>0 for r in results))
        if not report['success']:report['error']='Original/native checkpoint mismatch; retained all results'
    except (OSError,ValueError,KeyError,RuntimeError,subprocess.SubprocessError) as e:report['error']=str(e)
    finally:
        if verified:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except (OSError,subprocess.SubprocessError) as e:report.update(success=False,cleanup_error=str(e))
        report.update(inputs=inputs,sources=sources,inputs_stable=all(sha(Path(n))==h for n,h in inputs.items()),sources_stable=all(sha(ROOT/n)==h for n,h in sources.items()))
        report['success'] &= report['inputs_stable'] and report['sources_stable']
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Report:',out/'report.json',flush=True)
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
