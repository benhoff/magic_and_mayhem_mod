"""Same physical public restart journey, with actual native target destruction."""
import json,os,secrets,time
from pathlib import Path
from rendering_transition_low_live import TransitionJourney as Base
from rendering_transition_live import TransitionJourney as Wide
from rendering_pilot import require

class SurfaceLossJourney(Base):
    def normal_quit(self):
        if self.size==[640,480]:return super().normal_quit()
        require(self.size==[800,600],"Unsupported surface-loss source extent")
        return Wide.normal_quit(self)
    def inject_fault(self,channel,header):
        require(header[2]==3 and header[6]==1 and header[5]==header[9] and not header[7] and not header[8],
                'Surface-loss request requires healthy drained producer')
        request=dict(schema=1,producer=header[4],transport_version=3,request_id=secrets.token_hex(16),action='destroy-render-target')
        path=self.session/'surface-loss-request-v1.json';pending=path.with_suffix('.json.pending')
        raw=(json.dumps(request,separators=(',',':'))+'\n').encode()
        with pending.open('xb') as out:out.write(raw);out.flush();os.fsync(out.fileno())
        at=time.monotonic_ns();os.link(pending,path)
        self.actions.append(dict(action='declared-native-surface-loss',host_ns=at,producer=header[4],record='fault-injection.json'))
        observed=self.session/'surface-loss-observation-v1.json';deadline=time.monotonic()+8
        while time.monotonic()<deadline:
            require(self.child.poll() is None,'Host exited during native provider loss')
            if observed.exists():
                try:observation=json.loads(observed.read_bytes())
                except json.JSONDecodeError:time.sleep(.01);continue
                require(observation['graphics_failure'] is True and observation['error']=='OpenGL offscreen target is unavailable',
                        'Wrong native provider loss outcome')
                return dict(kind='native-offscreen-target-loss',request_host_ns=at,request=request,observation=observation)
            time.sleep(.01)
        raise ValueError('Native provider target-loss observation deadline')
    def terminal(self):
        super().terminal()
        path=self.session/'surface-loss-retirement-v1.json'
        require(path.is_file(),'Native target-loss retirement receipt absent')
        fault=self.epochs[0]['fault'];fault['retirement']=json.loads(path.read_bytes())
        (self.out/'fault-injection.json').write_text(json.dumps(fault,indent=2)+'\n')
