"""Strict live-thread/process-handle observation; launcher status is independent."""
import os,struct,subprocess,time
def decode(data,complete=True):
 if len(data)<128 or len(data)>192 or (len(data)-64)%64:raise ValueError('Process exit envelope')
 h=struct.unpack('<16I',data[:64])
 if data[:8]!=b'MNMDPX01' or h[2:4]!=(1,64) or not h[4] or not 1<=h[5]<=600000 or any(h[6:]):raise ValueError('Process exit header')
 rows=list(struct.iter_unpack('<16I',data[64:]))
 for i,r in enumerate(rows,1):
  if r[0]!=i or r[2]!=h[4] or any(r[9:]):raise ValueError('Process exit identity/reserved')
 first=rows[0]
 if first[1]!=1 or not first[3] or first[4:7]!=(258,1,259) or first[8]:raise ValueError('Process not attached alive')
 result=dict(thread=first[2],process=first[3],attached_alive=True,complete=False,exit_code=None)
 if len(rows)==2:
  final=rows[1]
  if final[1]!=2 or final[3]!=first[3] or final[4:6]!=(0,1) or final[8]:raise ValueError('Process exit wait/query failed')
  result.update(complete=True,exit_code=final[6])
 if complete and not result['complete']:raise ValueError('Missing process exit result')
 return result

class Monitor:
 def __init__(self,binary,env,thread,run,refresh):
  self.path=run/'process-exit.bin';self.log=(run/'process-exit.log').open('x')
  clean={k:v for k,v in env.items() if not k.startswith('MNM_') and k!='LD_PRELOAD'}
  clean.update(MNM_PROCESS_EXIT_THREAD=str(thread),MNM_PROCESS_EXIT_TIMEOUT_MS='600000',MNM_PROCESS_EXIT_FILE='Z:'+str(self.path).replace('/','\\'))
  self.process=subprocess.Popen(['wine',str(binary)],env=clean,stdout=self.log,stderr=subprocess.STDOUT)
  try:
   deadline=time.monotonic()+15
   while time.monotonic()<deadline:
    if self.path.exists() and self.path.stat().st_size>=128:
     self.attached=decode(self.path.read_bytes(),complete=False)
     if self.attached['thread']!=thread or self.attached['complete']:raise ValueError('Process exited during monitor attachment')
     return
    if self.process.poll() is not None:raise ValueError('Process monitor failed before attachment')
    refresh();time.sleep(.02)
   raise ValueError('Process monitor attachment timed out')
  except BaseException:
   self.close();raise
 def finish(self):
  code=self.process.wait(timeout=10)
  if code:raise ValueError('Process monitor refused: '+str(code))
  return decode(self.path.read_bytes())
 def poll_exit(self):
  if self.path.exists() and self.path.stat().st_size>=192:
   result=decode(self.path.read_bytes())
   if (result['thread'],result['process'])!=(self.attached['thread'],self.attached['process']):raise ValueError('Process monitor identity changed')
   return result
  if self.process.poll() is not None:raise ValueError('Process monitor ended without an exit record')
  return None
 def close(self):
  if self.process.poll() is None:self.process.terminate();self.process.wait(timeout=5)
  self.log.close()
