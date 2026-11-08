"""Source640 caption hints and focus policy; ordinary input remains authoritative."""
from pathlib import Path
import subprocess
import struct
import time
from campaign_input_admission import DialogueReadiness
from rendering_pilot import require

CONTROL=['tools/run-rendering-ui-low.py','tools/rendering_ui_low.py',
         'tools/prepare-rendering-ui-low.py','tests/test-rendering-ui-low.py']


def input_hint(raw,paint,point=None):
    """Read-only readiness hint; original callbacks still decide the answer."""
    if len(raw)!=1088 or raw[:8]!=b'MNMINK01':return False
    version,size,sequence,active,x,y,width,height=struct.unpack_from('<8I',raw,8)
    return (version==1 and size==1088 and not sequence%2 and active==1
            and [width,height]==[640,480] and paint is not None
            and paint.get('presentation_owner')=='gui' and paint.get('mode')==0
            and [paint.get('image_width'),paint.get('image_height')]==[640,480]
            and paint.get('scaling')==2 and (point is None or [x,y]==list(point)))


def caption_image(image,rect,paint,size):
    from PIL import ImageOps
    require(size==[640,480] and paint['scaling']==2
            and (paint['frame_width'],paint['frame_height'],paint['image_width'],paint['image_height'])==(640,480,640,480),
            'Source640 caption requires the actual integer-sized World')
    x,y=rect[0]+paint['left'],rect[1]+paint['top']
    require(0<=x and 0<=y and x+640<=image.width and y+480<=image.height,'Source640 image leaves screenshot')
    # Include the complete title. The old scaled crop clipped its upper strokes
    # and the retained original Hermes card was recognized as "Fermes".
    crop=image.crop((x+160,y+25,x+540,y+240)).convert('L')
    return ImageOps.expand(crop.resize((crop.width*4,crop.height*4)),20,255)


def make_journey(high):
    class LowJourney(high):
        confirmation_pending=False

        def click_id(self,name):
            result=super().click_id(name)
            if name=='miniMenuButton4':self.confirmation_pending=True
            return result

        def pointer(self,point):
            if not self.confirmation_pending:return super().pointer(point)
            require(self.size==[640,480] and point==[212,316],'Unexpected source640 confirmation point')
            self.confirmation_pending=False
            deadline=time.monotonic()+8
            def ready(target=None):
                self.viewport.poll();path=self.session/'render-input.bin'
                raw=path.read_bytes()
                return raw==path.read_bytes() and input_hint(raw,self.viewport.paint,target)
            # The original event may precede Qt's Mini->viewport handoff. Wait
            # for a GUI paint and active focused input, then generate fresh
            # motion and observe its published position before the base click.
            while time.monotonic()<deadline:
                self.viewport.poll()
                if self.viewport.paint and self.viewport.paint.get('presentation_owner')=='gui':
                    self.focus_viewport()
                    if ready():break
                time.sleep(.05)
            else:raise ValueError('Source640 confirmation input handoff deadline')
            super().pointer([40,100]);time.sleep(.15)
            super().pointer(point)
            while time.monotonic()<deadline:
                if ready(point):
                    self.actions.append(dict(action='source640-confirmation-input-ready',logical=point,host_ns=time.monotonic_ns(),scope='Read-only GUI/input hint before physical click; original answer and retirement remain authoritative.'))
                    return
                time.sleep(.05)
            raise ValueError('Source640 confirmation motion admission deadline')

        def set_scaling(self,mode):
            require(self.size==[640,480],'Source640 adapter cannot alter the800 controller')
            paint=self.viewport.poll()
            if mode=='integer' and paint and paint['scaling']==2:
                # No toolbar action occurred. Keep native focus; the ordinary W
                # controls then centre the wizard before seeking a sprite point.
                self.focus_viewport()
                self.actions.append(dict(action='source640-retain-integer-focus',host_ns=time.monotonic_ns()))
                return
            return super().set_scaling(mode)

        def clear_dialogue(self,label):
            from PIL import Image
            gate=DialogueReadiness(True,12,48)
            while True:
                shot=self.capture(label+'-'+str(gate.observations));paint=self.viewport.poll()
                require(self.viewport_geometry is not None,'Source640 viewport geometry missing')
                rect=self.screen_geometry(self.viewport_geometry)
                source=Path(__file__).resolve().parents[1]/shot['path']
                crop=self.out/(label+'-'+str(gate.observations)+'-source640-caption.png')
                caption_image(Image.open(source),rect,paint,self.size).save(crop)
                text=subprocess.run(['tesseract',str(crop),'stdout','--psm','6'],capture_output=True,text=True,check=True,timeout=10).stdout
                visible='hermes' in text.lower();decision=gate.observe({'native':visible,'original':visible},time.monotonic())
                self.actions.append(dict(action='dialogue-readiness',label=label,source_frame=[640,480],visible=visible,text=text,decision=decision,host_ns=time.monotonic_ns(),scope='Complete-title grayscale/enlarged native screenshot hint only; original state/callback/output proofs remain independent.'))
                if decision=='ready':return
                if decision=='dismiss':self.left(label+'-dismiss-'+str(gate.dismissals),[320,240])
                time.sleep(3)
    return LowJourney
