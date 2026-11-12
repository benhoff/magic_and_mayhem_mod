#!/usr/bin/env python3
"""Pin the built v4 collector player and its complete linked source inputs."""
import argparse,hashlib,json,importlib.util
from pathlib import Path
from coverage_evidence import Reader,write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1];PREFIX='research/runtime/rendering-pause-results-20261107/'
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,required=True);p.add_argument('--report',required=True);a=p.parse_args()
 require(a.report.startswith(PREFIX) and not (ROOT/a.report).exists(),'Fresh owned player plan required')
 build=a.build.resolve();source=build/'mnm-qt-shell';require(source.is_file(),'Built player absent')
 spec=importlib.util.spec_from_file_location('pause_compile_inputs',ROOT/'tools/test-native-campaign.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
 with Reader(ROOT) as r:
  template=r.json('research/runtime/rendering-enabled-intros-results-20261107/input-plan-03.json');names=m.linked_compiler_sources(build,ROOT)
  require(names==set(template['compiler_sources']),'Linked player source universe changed unexpectedly')
  compiled={n:r.digest(n) for n in sorted(names)};require(all(source.stat().st_mtime_ns>=(ROOT/n).stat().st_mtime_ns for n in names),'Player predates an input')
  cache=(build/'CMakeCache.txt').read_text();require('CMAKE_BUILD_TYPE:STRING=Release' in cache and 'MNM_EXPERIMENTAL_MINI_MENUS:BOOL=OFF' in cache and str(ROOT/'apps/qt-shell') in cache,'Wrong player build configuration')
  raw=source.read_bytes();binary=ROOT/'working/build/pause-command-history/mnm-qt-shell';binary.parent.mkdir(parents=True,exist_ok=True);binary.write_bytes(raw);binary.chmod(0o755)
  rec=a.report.removesuffix('.json')+'-player.bin';sha=write_report(ROOT,rec,raw)
  require(hashlib.sha256(binary.read_bytes()).hexdigest()==sha,'Copied player differs')
  logs=[]
  for n in ('working/pause-command-cmake.log','working/pause-command-build.log'):
   record=a.report.removesuffix('.json')+'-'+Path(n).name+'.bin';logs.append(dict(path=n,record=record,sha256=write_report(ROOT,record,(ROOT/n).read_bytes())))
  q=dict(schema=1,kind='bounded_pause_command_history_player_plan',success=True,compiler_sources=compiled,runtime_sources={n:r.digest(n) for n in sorted(native_runtime_source_paths(ROOT))},
   player_binary=dict(path=str(binary.relative_to(ROOT)),record=rec,sha256=sha),sources={n:r.digest(n) for n in ('tools/prepare-pause-command-player.py','tools/test-native-campaign.py','tools/rendering_build_binding.py')},
   compiler_context=dict(build_directory=str(build),type='Release',experimental_mini_menus=False,build_testing=False,parallel_jobs=4,logs=logs),original_game_runs=0,new_native_capture_code=True,new_native_gameplay_or_renderer_code=False,
   scope='Only the explicit version4 diagnostic command-history mode is added to the reviewed master player inputs. Full linked/compiler and native/runtime universes are pinned; no source-applicability waiver or validation credit follows from this build alone.')
  print(write_report(ROOT,a.report,(json.dumps(q,indent=2)+'\n').encode()),flush=True)
if __name__=='__main__':main()
