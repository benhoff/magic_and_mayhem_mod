"""Decode exact completed original cursor journal points; never infer clicks."""
import struct


def decode(data):
    if len(data)!=64:raise ValueError('Input point bytes')
    r=struct.unpack('<16I',data)
    if r[:3]!=(1,64,0x40209ca7) or r[3]&~3 or not r[4] or r[5]>262144 or r[6]>262144 or r[7]>1024:
        raise ValueError('Input point header/prefix')
    if r[11]>r[5] or r[12]>r[6] or bool(r[11])!=bool(r[12]):raise ValueError('Input point cursor identity')
    if r[3]&2 and not r[11] or not r[3]&2 and any(r[14:]):raise ValueError('Input point cursor validity')
    if not r[11] and r[13]:raise ValueError('Input point missing cursor tick')
    return dict(version=1,flags=r[3],owner_thread=r[4],record_prefix=r[5],entry_prefix=r[6],after_frame=r[7],drops=r[8],gaps=r[9],pending=r[10],cursor_record=r[11],cursor_entry=r[12],cursor_tick=r[13],cursor=list(r[14:]) if r[3]&2 else None,current_cursor_admissible=r[3]==3 and not r[8] and not r[9],selection_verified=False,click_dispatch_proved=False)


def validate_cadence_mode(args):
    """Permit only the explicit original cursor metadata diagnostic exception."""
    if not args.engine_cadence or args.cadence_control:return
    if not args.control and args.observer_mode=='physical':return
    if not args.control and args.observer_mode=='metadata' and args.hud_input_point and args.hud_state and args.semantic_state:return
    raise ValueError('Cadence observation requires original-active physical acquisition or explicit cursor metadata observation')
