"""Wait for real Qt/input ownership after the original640Quit acknowledgment."""
import json,struct,time
from rendering_transition_live import TransitionJourney as Base
from rendering_pause_live import PauseJourney
from rendering_pause import stable,input_state
from rendering_pilot import require

def confirmation_ready(nodes,original,inp):
 require(original['ack']==6 and original['confirmation']==1,'Focus wait left acknowledged original Quit confirmation')
 view=[v for v in nodes if v.get('accessible_id','').endswith('.nativeGameViewport')]
 require(len(view)<=1,'Ambiguous public confirmation viewport')
 return bool(view and view[0]['showing'] and view[0]['enabled'] and inp['active']==1 and not inp['held'] and inp['size']==[640,480])

class TransitionJourney(Base):
 def read(self):
  # Launch can synchronously stage the original runner on the Qt thread. Do
  # not make an AT-SPI RPC to that thread while awaiting its admission. The
  # inherited readiness loop still checks child exit and its finite deadline;
  # only the owned session journal and original seqlock admit Main readiness.
  value=PauseJourney.read(self)
  if self.session is not None:
   path=self.session/'native-only-session.json'
   if path.exists():
    native=json.loads(path.read_bytes())
    require(native.get('session')==str(self.session),'Native startup outcome belongs to another session')
    require(native.get('state')!='failed','Native epoch failed before expected progress: '+native.get('error',''))
  return value
 def normal_quit(self):
  require(self.size==[640,480],'Low confirmation controller requires actual640source')
  self.confirmation_focus_wait=True
  try:return super().normal_quit()
  finally:self.confirmation_focus_wait=False
 def focus_viewport(self):
  nodes=[v for _,v in self.access.nodes()]
  if not getattr(self,'confirmation_focus_wait',False):return super().focus_viewport()
  state,raw=self.original()
  if state['ack']!=6 or struct.unpack_from('<I',raw,42004)[0]!=1:return super().focus_viewport()
  started=time.monotonic_ns();end=time.monotonic()+3
  while time.monotonic()<end:
   require(self.child.poll() is None,'Player exited before actual confirmation viewport')
   state,raw=self.original();lease=dict(ack=state['ack'],confirmation=struct.unpack_from('<I',raw,42004)[0]);inp=input_state(stable(self.session/'render-input.bin',1088,(16,)))
   if confirmation_ready(nodes,lease,inp):
    self.actions.append(dict(action='original-confirmation-focus-ready',started_host_ns=started,host_ns=time.monotonic_ns(),original=lease,input=inp,viewport=next(v for v in nodes if v.get('accessible_id','').endswith('.nativeGameViewport'))));return super().focus_viewport()
   time.sleep(.02);nodes=[v for _,v in self.access.nodes()]
  raise ValueError('Actual Qt/input Quit confirmation ownership did not settle within3seconds')
