#!/usr/bin/env python3
"""Frozen NVIDIA EGL benchmark of actual ordered resident campaign World inputs."""
import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
from profile_native_resident_helpers import sha, summarize, normalize_capture

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--campaign',type=Path,required=True,help='Campaign evidence directory with complete bounded resident-source.bin')
    parser.add_argument('--repeats',type=int,default=2,choices=range(1,5))
    args=parser.parse_args()
    parent=ROOT/'working/tests/resident-offline-profile';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    subprocess.run(['python3',str(ROOT/'tools/draft-coverage-claims.py'),'--behavior','NR.resident-stage-profiling',
                    '--scenario','NR.resident-stage-profiling.offline','--output',str(out/'prospective-claims.json')],cwd=ROOT,check=True,stdout=subprocess.DEVNULL)
    declaration=json.loads((out/'prospective-claims.json').read_text());sources=declaration['sources']
    env={k:v for k,v in os.environ.items() if k not in ('DISPLAY','WAYLAND_DISPLAY','LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER')}
    env.update(QT_QPA_PLATFORM='mnm-headless-egl',MNM_PROFILE_EGL_VENDOR='NVIDIA',__EGL_VENDOR_LIBRARY_FILENAMES='/usr/share/glvnd/egl_vendor.d/10_nvidia.json')
    report=dict(success=False,sources=sources,claims=declaration['claims'],live_gameplay_executed=False,desktop_used=False,
                scope='Fresh frozen resident replay of actual public campaign inputs on NVIDIA TITAN EGL. Timestamped and timestamp-disabled all-frame completion throughput include cold replay setup and disk input; diagnostic pixel hashes are outside timing. No simulation/pacing/UI/scanout or60FPS gameplay claim.')
    inputs={};verified=False
    def run(name,command,timeout=900):
        with (out/(name+'.log')).open('x') as log:subprocess.run([str(c) for c in command],cwd=ROOT,env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=timeout)
    try:
        run('original-before',[ROOT/'tools/original-manifest.sh','verify']);verified=True
        campaign=args.campaign.resolve();game=json.loads((campaign/'report.json').read_text())
        stage=game['resident_world']['stage_profile'];corpus=campaign/'resident-source.bin'
        # A diagnostic run may miss a cadence gate; it can still supply complete
        # actual gameplay inputs if all independent gameplay/identity checks pass.
        require=[game.get(k,False) for k in ['casting_verified','combat_verified','movement_verified','normal_quit_verified','sources_unchanged','private_wine_stopped','original_manifest_verified_after']]
        if not all(require) or not stage['capture_complete'] or stage['capture_truncated']:raise ValueError('Incomplete gameplay/capture evidence')
        if sha(corpus)!=game['resident_capture']['sha256'] or corpus.stat().st_size!=stage['capture_bytes']:raise ValueError('Resident corpus changed')
        inputs[str(corpus)]=sha(corpus);inputs[str(campaign/'report.json')]=sha(campaign/'report.json')
        assets=Path(game['experiment'])/'game'
        for n,digest in game['inputs'].items():
            path=Path(n)
            if not path.resolve().is_relative_to(assets.resolve()) or sha(path)!=digest:raise ValueError('Changed/unsafe captured game input: '+n)
            inputs[n]=digest
        report['inputs']=inputs;report['corpus']=dict(path=str(corpus),worlds=stage['captured_worlds'],bytes=corpus.stat().st_size,
                campaign_success=game['success'],campaign_error=game.get('error'),casting_verified=game['casting_verified'],combat_verified=game['combat_verified'],movement_verified=game['movement_verified'],normal_quit_verified=game['normal_quit_verified'])
        normalized=out/'resident-source-v1.bin';report['normalization']=normalize_capture(corpus,normalized)
        if report['normalization']['worlds']!=stage['captured_worlds']:raise ValueError('Normalized capture lost Worlds')
        inputs[str(normalized)]=sha(normalized)
        frozen=out/'source'
        tracked=subprocess.check_output(['git','ls-files','-z'],cwd=ROOT).decode().split('\0')
        configured={n for n in tracked if n and n.split('/')[0] not in ('original','working','research') and
                    (Path(n).suffix in ('.c','.cpp','.h','.hpp','.S','.cmake','.json') or Path(n).name=='CMakeLists.txt')}
        for n in sorted(configured|set(sources)):
            destination=frozen/n;destination.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/n,destination)
        report['frozen_sources']={n:sha(frozen/n) for n in configured|set(sources)}
        build=out/'build';run('configure',['cmake','-S',frozen/'tools/native-render-profile','-B',build,'-DCMAKE_BUILD_TYPE=Release','-DCMAKE_CXX_FLAGS_RELEASE=-O3 -DNDEBUG -g'])
        run('build',['cmake','--build',build,'--target','mnm-resident-profile','mnm-gpu-timing-fixture','mnm_headless_egl','-j4'])
        compiled=set()
        for d in build.rglob('*.o.d'):
            for token in d.read_text().replace('\\\n',' ').split():
                path=Path(token).resolve()
                if path.is_relative_to(frozen.resolve()) and path.is_file():compiled.add(path.relative_to(frozen.resolve()).as_posix())
        report['compiled_sources']={n:sha(frozen/n) for n in sorted(compiled)}
        missing=sorted(compiled-set(sources));report['undeclared_compiler_inputs']=missing
        if missing:raise ValueError('Undeclared resident compiler dependencies: '+repr(missing))
        env['QT_QPA_PLATFORM_PLUGIN_PATH']=str(build/'platforms')
        report['binary_sha256']=sha(build/'mnm-resident-profile');report['egl_plugin_sha256']=sha(build/'platforms/libmnm_headless_egl.so')
        run('gpu-timing-fixture',[build/'mnm-gpu-timing-fixture'],timeout=30);report['gpu_fixture']=json.loads((out/'gpu-timing-fixture.log').read_text().splitlines()[-1])
        native={};digests=[]
        for mode,repeats in [('timestamps',1),('throughput',args.repeats)]:
            run(mode,[build/'mnm-resident-profile',normalized,assets,mode,repeats,out/(mode+'.json')],timeout=1800)
            result=json.loads((out/(mode+'.json')).read_text());native[mode]=result
            for trial in result['runs']:
                if trial['worlds']!=stage['captured_worlds']:raise ValueError('Resident replay dropped World inputs')
                trial['summary']=summarize(trial['samples']);digests.append(trial['final_native_words_sha256'])
        if len(set(digests))!=1:raise ValueError('Timestamp modes changed replay output')
        report['native']=native;report['repeated_output_stable']=True
        report['sources_stable']=all(sha(ROOT/n)==h for n,h in sources.items())
        report['frozen_sources_stable']=all(sha(frozen/n)==h for n,h in report['frozen_sources'].items())
        report['inputs_stable']=all(sha(Path(n))==h for n,h in inputs.items())
        report['success']=report['sources_stable'] and report['frozen_sources_stable'] and report['inputs_stable']
    except (OSError,ValueError,KeyError,subprocess.SubprocessError) as error:report['error']=str(error)
    finally:
        if verified:
            try:run('original-after',[ROOT/'tools/original-manifest.sh','verify']);report['original_manifest_verified_before_after']=True
            except (OSError,subprocess.SubprocessError) as error:report.update(success=False,original_manifest_error=str(error))
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Resident offline profile report:',out/'report.json',flush=True);return 0 if report['success'] else 1

if __name__=='__main__':raise SystemExit(main())
