"""Decode every prefix dependency but materialize only the requested World target."""
import struct
from rendering_interaction_live import LiveSources,body_record
from rendering_pilot import require
POLICY='verified-prefix-requested-World-body-v1'

class RecentSources(LiveSources):
    def poll(self,through):
        require(type(through)is int and self.worlds<=through<=8192,'Recent source request must be monotonic and bounded')
        end=self.path.stat().st_size
        with self.path.open('rb')as f:
            require(f.read(8)==b'MNMPROF4','Live source archive version')
            while self.worlds<through and end>=self.at+80:
                f.seek(self.at);h=f.read(80);g,c,s,q,n,_=struct.unpack('<QQIIII',h[:32])
                require(g==self.worlds+1 and g<=8192 and 16<=s<=16777216 and(q==0 if c else 64<=q<=197184),'Live source identity/bounds')
                require(4<n<=s+q+(s+q)//128+1024,'Live source compressed extent')
                if end<self.at+80+n:break
                body=self.decoder.decode(h,f.read(n),s+q);self.at+=80+n
                if c:
                    require(c==self.chunks+1,'Live source chunk sequence');self.chunks+=1;continue
                self.worlds=g
                if g==through:
                    _,rows=self.module.decode(body[:s],complete=False)
                    self.frames[g]=[(a,b)for a,b in rows if body_record(a)]
                for old in list(self.frames):
                    if old+64<g:del self.frames[old]


def fresh_target(journey):
    import time
    from rendering_interaction_live import SpriteNotReady
    deadline=time.monotonic()+15
    while time.monotonic()<deadline:
        require(journey.child.poll()is None,'Player exited before visible wizard body')
        state=journey.latest();t=state['tracked']
        require(t and t['slot']==0 and t['type']==0 and t['owner']==0 and t['active']and t['health']>0,'Original living wizard role changed')
        try:target=journey.source_frames.target(state['frame']['generation'],state['history'])
        except SpriteNotReady:
            time.sleep(.1);continue
        after=journey.latest();age=after['frame']['generation']-state['frame']['generation']
        require(age>=0 and after['frame']['nonce']==state['frame']['nonce']and after['thread']==state['thread'],'Source target session changed')
        if age<=2 and all(after[k]==state[k]for k in('history','position','camera','mode')):
            target['recent_source_check']=dict(policy=POLICY,source_row=state['ordinal'],checked_row=after['ordinal'],
                checked_generation=after['frame']['generation'],age_frames=age,unchanged_fields=['history','position','camera','mode'],hit_feedback_used=False)
            return state,target
    raise ValueError('Original visible wizard body readiness deadline')
