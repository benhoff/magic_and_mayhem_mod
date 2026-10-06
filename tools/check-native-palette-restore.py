#!/usr/bin/env python3
"""Offline native indexed validity/reload policy from real API capture inputs."""
import argparse,importlib.util,json,os,shlex,subprocess,tempfile
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
s=importlib.util.spec_from_file_location('palette',ROOT/'tools/check-surface-palette-restore.py');r=importlib.util.module_from_spec(s);s.loader.exec_module(r)
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--build',type=Path,default=ROOT/'working/build/palette-restore');p.add_argument('--report',type=Path,required=True);a=p.parse_args()
 if a.report.exists():p.error('Refusing overwrite')
 capture,rows=r.load();states=r.validate(capture,rows);cases=[x for x in states if x['phase'] in (6,8,9)];require=r.require;require(len(cases)==9,'Expected nine explicitly reloaded states')
 names=['tests/render-palette-restore-test.cpp','tools/check-native-palette-restore.py','tools/check-surface-palette-restore.py','renderer/blit.cpp','renderer/blit.hpp','renderer/known_pixels.hpp','renderer/CMakeLists.txt','renderer/surface_copy.cpp','renderer/surface_copy.hpp',str(r.DEFAULT.relative_to(ROOT)),capture['corpus']['path']];sources={n:r.sha(ROOT/n) for n in names}
 parent=ROOT/'working/tests/native-palette-restore';parent.mkdir(parents=True,exist_ok=True);run=Path(tempfile.mkdtemp(prefix='run-',dir=parent));file=run/'inputs.txt'
 def find(phase,id):return next(x for x in states if x['phase']==phase and x['surface']==id)
 file.write_text(''.join(' '.join(str(v) for v in [x['surface'],x['phase'],*find(0,x['surface'])['colors'],*find(6,x['surface'])['colors'],*x['colors'],*find(0,x['surface'])['indices'],*find(6,x['surface'])['indices'],*x['indices']])+'\n' for x in cases))
 qt=shlex.split(subprocess.check_output(['pkg-config','--cflags','--libs','Qt6Gui','Qt6OpenGL'],text=True));binary=run/'probe';subprocess.run(['g++','-std=c++17','-O2','-fPIC','-Wall','-Wextra','-Werror','-pedantic','-I'+str(ROOT/'renderer'),str(ROOT/names[0]),str(a.build.resolve()/'libmnm-renderer.a'),*qt,'-o',str(binary)],check=True)
 output=subprocess.run([str(binary),str(file)],env=dict(os.environ,QT_QPA_PLATFORM='xcb',LIBGL_ALWAYS_SOFTWARE='1'),capture_output=True,text=True,timeout=60);(run/'native.log').write_text(output.stdout+output.stderr);require(output.returncode==0,output.stdout+output.stderr);native=json.loads(output.stdout);require(native==dict(success=True,cases=9,unknown_refusals=45,index_checks=18,rgb_pixel_checks=864),'Unexpected native check counts');require(all(r.sha(ROOT/n)==h for n,h in sources.items()),'Sources changed')
 report=dict(schema=1,success=True,sources=sources,native=native,binary_sha256=r.sha(binary),library_sha256=r.sha(a.build.resolve()/'libmnm-renderer.a'),inputs_sha256=r.sha(file),artifacts=str(run.relative_to(ROOT)),original_wrapper_executed=False,scope='Native GL indexed8 validity and explicit reload/palette policy using captured API entries/index inputs from nine defined states.45 unknown-content refusals,18 index checks and864 input-derived RGB pixel comparisons. Captured palette/indices are inputs; RGB presentation is not independently captured original/driver display output. No native shared COM identity, asset reload or live recovery equivalence.')
 with a.report.open('x') as f:json.dump(report,f,indent=2);f.write('\n')
 print(json.dumps(native))
if __name__=='__main__':main()
