"""Keep command acknowledgement separate from actual original process exit."""
import time


def wait_for_quit(process,state,heartbeat,request,timeout=15,monotonic=time.monotonic,sleep=time.sleep,alive_before_dispatch=False,target_exit=None):
    if request<=0 or not 0<timeout<=15:raise ValueError('Invalid Quit wait bounds')
    if process.poll() is not None and not alive_before_dispatch:raise ValueError('Launcher already exited before Quit wait')
    deadline=monotonic()+timeout;ack=False;target=None
    while monotonic()<deadline:
        current=state()
        if current and current[0][3]==request:
            if current[0][4]!=1:raise ValueError('Original Quit command refused')
            ack=True
        if target_exit:
            target=target_exit()
            if target is not None:
                if not isinstance(target,dict) or target.get('complete') is not True or not target.get('attached_alive') or not target.get('thread') or not target.get('process'):raise ValueError('Original process exit identity incomplete')
                if target['exit_code']:raise ValueError('Original process exit failed: '+str(target['exit_code']))
                break
        elif process.poll() is not None:break
        heartbeat();sleep(.02)
    code=process.poll()
    if target_exit and target is None:raise ValueError('Observed original process Quit timed out')
    if not target_exit and code is None:raise ValueError('Normal original Quit timed out')
    if not target_exit and code:raise ValueError('Normal original Quit failed: '+str(code))
    return dict(request=request,screen=3,action=3,ack_observed=ack,normal_exit=True,exit_code=target['exit_code'] if target else code,
                launcher_exit_code=code,launcher_exit_observed=code is not None,exit_boundary='observed_original_process' if target else 'launcher',original_child_exit_code_verified=target is not None)
