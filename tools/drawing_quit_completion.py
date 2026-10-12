"""Keep command acknowledgement separate from actual original process exit."""
import time


def wait_for_quit(process,state,heartbeat,request,timeout=15,monotonic=time.monotonic,sleep=time.sleep):
    if request<=0 or not 0<timeout<=15:raise ValueError('Invalid Quit wait bounds')
    deadline=monotonic()+timeout;ack=False
    while process.poll() is None and monotonic()<deadline:
        current=state()
        if current and current[0][3]==request:
            if current[0][4]!=1:raise ValueError('Original Quit command refused')
            ack=True
        heartbeat();sleep(.02)
    code=process.poll()
    if code is None:raise ValueError('Normal original Quit timed out')
    if code:raise ValueError('Normal original Quit failed: '+str(code))
    return dict(request=request,screen=3,action=3,ack_observed=ack,normal_exit=True,exit_code=code)
