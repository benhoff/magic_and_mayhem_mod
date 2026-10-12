#!/usr/bin/env python3
"""Full native default campaign with an owned actual empty audio graph."""
import argparse,json
from pathlib import Path
import subprocess
import tempfile
from private_audio_backend import PrivateAudioBackend
ROOT=Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser(description=__doc__);parser.add_argument('--hardware',action='store_true');args=parser.parse_args()
    parent=ROOT/'working/tests/native-silent-campaign';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    backend=PrivateAudioBackend(out);report=dict(schema=1,success=False)
    try:
        with backend:
            command=['python3',str(ROOT/'tools/test-native-campaign.py'),'--default-audio','--native-loading','--native-factory',
                     '--default-media','--default-music','--normal-quit','--initial-silent-start',
                     '--source-game',str(ROOT/'working/experiments/native-music-input/run-fliws_ct/game'),'--timeout','600']
            if args.hardware:command.append('--hardware')
            with (out/'campaign.log').open('x') as log:
                subprocess.run(command,cwd=ROOT,env=backend.environment,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=720)
            lines=(out/'campaign.log').read_text().splitlines();paths=[Path(line.removeprefix('Report: ')) for line in lines if line.startswith('Report: ')]
            if len(paths)!=1 or not paths[0].resolve().is_relative_to(ROOT/'working/tests/native-campaign'):
                raise ValueError('Private campaign result path missing/redirected')
            result=json.loads(paths[0].read_text());assert result['success']
            report['campaign_record']=str(paths[0]);backend.finish_graph();report['success']=True
    except Exception as error:report['error']=repr(error);raise
    finally:
        backend.close();report['backend_cleanup']=backend.report
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print('Native silent campaign passed',out/'report.json')
if __name__=='__main__':main()
