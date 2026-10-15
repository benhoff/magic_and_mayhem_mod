#!/usr/bin/env python3
"""Compare originalAPIentry/returnsourceoracles to independent nativeprefixoutput."""
import argparse
import json
from pathlib import Path
from drawing_api_session import sha
from drawing_source_byte_witness import analyze
ROOT=Path(__file__).resolve().parents[1]


def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--reference',type=Path,required=True);p.add_argument('--prediction',type=Path,required=True);p.add_argument('--report',type=Path,required=True);a=p.parse_args()
    if a.report.exists():p.error('Refusing evidence overwrite')
    refraw=a.reference.read_bytes();predraw=a.prediction.read_bytes();reference=json.loads(refraw);prediction=json.loads(predraw)
    if reference.get('success') is not True or prediction.get('native_exit')!=0 or prediction['reference']['sha256']!=sha(refraw) or prediction['original_pixels_used_as_native_inputs'] is not False:raise ValueError('Independentreference/nativeprovenance required')
    capture=Path(reference['experiment'])/'capture';observed=analyze(capture)
    if observed!=reference['api_source_byte_observation']:raise ValueError('Originalsourcebyteevidence changed')
    by_entry={}
    for f in observed['frames']:by_entry.setdefault(f['api_entry'],[]).append(f)
    comparisons=[];excluded=[]
    for i,frame in enumerate(prediction['comparisons'],1):
        native=(ROOT/prediction['artifacts']/f'native/frame-{i:04d}.565').read_bytes()
        if sha(native)!=prediction['native']['frames'][i-1]['sha256']:raise ValueError('Nativeoutputchanged')
        for source in by_entry.get(frame['entry'],[]):
            if source['status'] or source['before'][3]!=frame['boundary']['sequence'] or source['before'][6]!=frame['boundary']['canvas']:
                excluded.append(dict(entry=frame['entry'],phase=source['phase'],reason='refused-or-different-source-prefix'));continue
            original=(capture/source['path']).read_bytes()
            if sha(original)!=source['sha256'] or len(original)!=len(native):raise ValueError('Originalsourceplaneidentity/extent')
            differences=sum(original[j:j+2]!=native[j:j+2] for j in range(0,len(native),2))
            comparisons.append(dict(entry=frame['entry'],phase=source['phase'],sequence=frame['boundary']['sequence'],oracle=source['oracle'],pixels=len(native)//2,different_pixels=differences,original_sha256=sha(original),native_sha256=sha(native),source_storage_owned=False))
    names=['tools/analyze-drawing-api-source-bytes.py','tools/drawing_source_byte_witness.py'];report=dict(schema=1,success=True,sources={n:sha((ROOT/n).read_bytes()) for n in names},reference=dict(path=str(a.reference),sha256=sha(refraw)),prediction=dict(path=str(a.prediction),sha256=sha(predraw)),comparisons=comparisons,excluded=excluded,pixels_compared=sum(c['pixels'] for c in comparisons),different_pixels=sum(c['different_pixels'] for c in comparisons),original_game_executed=False,native_reexecuted=False,original_pixels_used_as_native_inputs=False,source_storage_owned=False,complete_presented_journey=False,scope='Offlineexactsourceprefix/canvas originalAPIentry/returnCPUoracle comparison toindependentnative output;originalplanes nevernativeinput. Refusals/differentprefixes retained;zero differences not immutableownership/allwriters/EGLscanout orfulljourney.')
    a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:report[k] for k in ('pixels_compared','different_pixels')}))


if __name__=='__main__':main()
