#!/usr/bin/env python3
"""Freeze the enabled-intro player build, source closure and bounded game budget."""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
from coverage_evidence import write_report
from rendering_build_binding import native_runtime_source_paths
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/rendering-enabled-intros-results-20261107/'
sha=lambda path:hashlib.sha256(Path(path).read_bytes()).hexdigest()


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build-dir',type=Path,required=True);p.add_argument('--report',required=True);args=p.parse_args()
    require(args.report.startswith(PREFIX) and '..' not in Path(args.report).parts and not (ROOT/args.report).exists(),'Fresh owned plan required')
    spec=importlib.util.spec_from_file_location('intro_build',ROOT/'tools/test-native-campaign.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
    build=args.build_dir.resolve();compiled={n:sha(ROOT/n) for n in sorted(m.linked_compiler_sources(build,ROOT))}
    path=ROOT/'working/build'/Path(args.report).stem/'mnm-qt-shell';path.parent.mkdir(parents=True,exist_ok=True);raw=(build/'mnm-qt-shell').read_bytes()
    require(not path.exists() or path.read_bytes()==raw,'Preserve differing prior player')
    path.write_bytes(raw);path.chmod(0o755)
    name=args.report.removesuffix('.json')+'-player.bin';binary=dict(path=str(path.relative_to(ROOT)),record=name,sha256=write_report(ROOT,name,raw))
    q=dict(schema=1,success=True,kind='bounded_enabled_intros_plan',compiler_sources=compiled,player_binary=binary,
        runtime_sources={n:sha(ROOT/n) for n in sorted(native_runtime_source_paths(ROOT))},
        sources={n:sha(ROOT/n) for n in ['tools/prepare-rendering-enabled-intros.py','tools/run-rendering-enabled-intros.py','tools/rendering_build_binding.py','tools/test-native-campaign.py']},
        bounds=dict(primary_cases=4,maximum_original_games=5,maximum_attempts=8,prior_zero_game_refusals=3,prior_capture_refusals=1,stop_on_first_failure=True,pilot_comparison_before_expansion=True),
        budget_reason='Retain all three prelaunch refusals: AT-SPI, unsupported executable and missing prepared music. Retain the first actual game with its refused1GiB output capture. Four complete new movie-policy cases cover the required nominal domains: five total actual original games, eight requests. No omitted failure or drive-floor change; new captures stage in RAM.',
        scope='Both enabled native intros in default/software High/Low contexts, complete OpenGL viewport output and normal Main/Quit. No World entry, physical-driver or original DirectShow equivalence. Earlier accepted rows remain historical.')
    digest=write_report(ROOT,args.report,(json.dumps(q,indent=2)+'\n').encode());print(json.dumps(dict(success=True,record=args.report,sha256=digest,compiler_sources=len(compiled))),flush=True)


if __name__=='__main__':main()
