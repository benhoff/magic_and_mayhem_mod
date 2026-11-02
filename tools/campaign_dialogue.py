"""Authored bounded ordinary-input hint for actor-state quiet during World play.

A hint permits a Return key attempt; it does not identify an original dialogue
state, recover a pause/clock API, or establish any mission outcome.
"""

class ActorQuietWindow:
    """Bounded conversation-input hint; unchanged state is not a dialogue API."""
    def __init__(self,seconds=6,dismissals=48):
        import math
        if not math.isfinite(seconds) or seconds<=0 or not 1<=dismissals<=96:
            raise ValueError('Invalid quiet actor input policy')
        self.seconds,self.maximum=seconds,dismissals
        self.signature=self.changed=self.previous=None
        self.dismissals=0

    def observe(self,actors,now):
        import math
        if not math.isfinite(now) or self.previous is not None and now<self.previous:
            raise ValueError('Quiet actor clock regressed')
        self.previous=now
        # Sampler sequence numbers deliberately do not represent state changes.
        fields=('type','owner','active','health','x','y','z','behavior','action','target','mana')
        signature=tuple((slot,*(actor[k] for k in fields)) for slot,actor in sorted(actors.items()))
        if signature!=self.signature:
            self.signature,self.changed=signature,now
        elif signature and now-self.changed>=self.seconds:
            if self.dismissals>=self.maximum:raise ValueError('Bounded quiet actor dismissal budget exhausted')
            self.dismissals+=1;self.changed=now;return True
        return False
