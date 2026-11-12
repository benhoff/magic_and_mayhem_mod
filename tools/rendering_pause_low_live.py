"""Source640title controls; preserve the accepted800 control path byte for byte."""
import json,subprocess,time
from pathlib import Path
from rendering_pause_live import PauseJourney as Base
from rendering_pilot import require
ROOT=Path(__file__).resolve().parents[1]
class PauseJourney(Base):
    def admit_mini(self,label,ack):
        # Reuse the existing title-qualified Hermes ingress, without tutorial
        # gameplay commands. An input receipt never substitutes for original ACK.
        started=time.monotonic();escapes=0
        for attempt in range(12):
            require(time.monotonic()-started<120,'Qualified Mini ingress exceeded120seconds')
            self.focus_viewport();paint=self.viewport.poll()
            nodes=[v for _,v in self.access.nodes() if v.get('accessible_id','').endswith('.nativeGameViewport')]
            require(len(nodes)==1,'Missing viewport for physical Mini ingress')
            rect=nodes[0]['rect'];image=[rect[0]+paint['left'],rect[1]+paint['top'],paint['image_width'],paint['image_height']]
            output=self.out/('mini-ingress-'+str(label)+'-'+str(attempt)+'.json')
            subprocess.run(['python3',str(ROOT/'tools/pause-low-dialogue-input.py'),'--source-frame-size','640','480','--mode','mini','--skip-click','--dismiss-dialogue','--rect',*[str(x) for x in image],
                '--input-channel',str(self.session/'render-input.bin'),'--output',str(output)],cwd=ROOT,env=self.env,check=True,timeout=60)
            receipt=json.loads(output.read_text());require(receipt['success'] and receipt['skip_click'],'Physical Mini ingress failed')
            self.actions.append(dict(action='qualified-mini-ingress',label=str(label),attempt=attempt,host_ns=time.monotonic_ns(),record=str(output.relative_to(ROOT)),escape_sent=receipt['escape_sent'],tutorial_actions=receipt['tutorial_actions']))
            if receipt['tutorial_actions']:
                require(not receipt['escape_sent'] and all(v['action']=='dismiss' for v in receipt['tutorial_actions']),'Unclassified tutorial control')
                self.wait_progress(int(paint['world']),minimum=2,timeout=15)
                continue
            require(receipt['escape_sent'],'Neither qualified dismissal nor physical Escape')
            escapes+=1;require(escapes<=4,'Mini ingress exceeded4physical Escape controls')
            deadline=time.monotonic()+3
            while time.monotonic()<deadline:
                value=self.original()
                if value[0]['ready'] and value[0]['screen']==17 and value[0]['ack']==ack:return self.wait(17,ack)
                time.sleep(.05)
        raise ValueError('Bounded Hermes/Mini admission exhausted12qualified controls')
