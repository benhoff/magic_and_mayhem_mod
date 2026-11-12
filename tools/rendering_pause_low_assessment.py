"""Reuse unchanged wire/pixel/cleanup checks with an explicit640title guard."""
import campaign_storage
from rendering_pause_assessment import assess as common,read_samples
from rendering_menu_acceptance import artifact
from rendering_pilot import require
from pause_low_ingress import validate_inputs

def assess(reader,a,c,observed):
    require(a.get('control_variant')=='source640-calibration-v1' and a['configuration']['actual_size']==[640,480],'Wrong calibrated control domain')
    rows,_=read_samples(artifact(reader,a,'pause-samples.sqlite3'),artifact(reader,a,'frame-input.bin'))
    require(all(r['input']['size']==[640,480] for r in rows),'Actual input source dimensions differ')
    legacy=campaign_storage.validate_mini_inputs
    # One stopped-case analysis process, no engine/UI code or concurrent users.
    # Keep the accepted800 callable and all shared bytes unchanged.
    campaign_storage.validate_mini_inputs=lambda directory:validate_inputs(directory,legacy)
    try:return common(reader,a,c,observed)
    finally:campaign_storage.validate_mini_inputs=legacy
