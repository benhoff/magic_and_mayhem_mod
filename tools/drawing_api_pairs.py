"""Shared strict diagnostic pair accounting for independent API wire versions."""
from collections import Counter
from drawing_census import BUILD_SHA256


class ApiPairs:
    def __init__(self,manifest):
        self.hooks={r['tag']:r for r in manifest['hooks']}
        if any(type(r['tag']) is not int or type(r['site']) is not int or not 0<r['site']<=0xffffffff for r in manifest['hooks']) or set(self.hooks)!=set(range(len(self.hooks))) or len(self.hooks)!=len(manifest['hooks']):
            raise ValueError('Invalid API hook tags')
        if manifest['original_sha256']!=BUILD_SHA256:raise ValueError('API manifest build mismatch')
        self.opened={};self.threads={};self.sequence=self.serial=0
        self.outcomes=Counter();self.targets=Counter();self.cleanup=Counter();self.phases=Counter()
        self.unreadable_stack=self.unreadable_target=self.unassigned=0

    def accept(self,row):
        if len(row)!=32 or any(type(word) is not int or not 0<=word<=0xffffffff for word in row):
            raise ValueError('Invalid API diagnostic word vector')
        self.sequence+=1
        seq,event,cid,tag,phase,thread,site,sp=row[:8]
        state,slot,target,stack_state=row[14:18]
        if seq!=self.sequence or tag not in self.hooks or site!=self.hooks[tag]['site'] or phase>14 or not cid or not thread or not sp or not row[27]:
            raise ValueError('API row identity/sequence/phase invalid')
        mode=self.hooks[tag]['target_mode'];expected={'register':1,'register_memory':2,'absolute_memory':2,'direct_relative':4}.get(mode['kind'])
        if state not in (expected,3) or state==3 and expected!=2:
            raise ValueError('API target state disagrees with exact CALL mode')
        if state in (1,4) and slot or state==2 and slot==0 or state==3 and target:
            raise ValueError('API target slot/value invariant invalid')
        if mode['kind']=='absolute_memory' and slot!=mode['address'] or mode['kind']=='direct_relative' and target!=mode['target']:
            raise ValueError('API exact absolute/direct target mismatch')
        identity=(tag,phase,thread,site,state,slot,target,row[27])
        if event==1:
            if cid!=self.serial+1 or row[27]!=sp or stack_state not in (0,1) or not stack_state and any(row[18:26]):
                raise ValueError('API BEFORE call/stack invariant invalid')
            register=mode.get('register');registers={'eax':10,'ecx':8,'edx':9,'ebx':28,'esp':7,'ebp':29,'esi':30,'edi':31}
            if mode['kind']=='register' and target!=row[registers[register]]:
                raise ValueError('API register target disagrees with preserved register')
            if mode['kind']=='register_memory' and slot!=(row[registers[register]]+mode['displacement'])&0xffffffff:
                raise ValueError('API memory target slot disagrees with preserved register')
            self.serial=cid;self.opened[cid]=identity;self.threads.setdefault(thread,[]).append(cid)
            if row[26]!=len(self.opened):raise ValueError('API BEFORE active frames mismatch')
            self.phases[phase]+=1;self.targets[(phase,site,state,slot,target)]+=1
            self.unreadable_stack+=stack_state==0;self.unreadable_target+=state==3;self.unassigned+=phase==0
        elif event==2:
            if self.opened.pop(cid,None)!=identity or not self.threads.get(thread) or self.threads[thread].pop()!=cid:
                raise ValueError('API AFTER lacks exact thread/stack/target LIFO BEFORE')
            if stack_state or any(row[18:26]) or row[26]!=len(self.opened):
                raise ValueError('API AFTER stack/active invariant invalid')
            delta=(sp-row[27])&0xffffffff
            if delta&0x80000000:delta-=0x100000000
            self.outcomes[(phase,site,row[10],row[12])]+=1;self.cleanup[(site,delta)]+=1
        else:raise ValueError('Unknown API event type')

    def finish(self,end,version):
        if end[2:5]!=(version,self.sequence,self.serial) or any(end[8:]):
            raise ValueError('API terminal identity/count/reserved mismatch')
        if any(end[5:8]) or self.opened or any(self.threads.values()):
            raise ValueError('API journal has loss/open calls/IO failure')

    def result(self):
        def grouped(counter,fields):return [dict(zip(fields,key),count=count) for key,count in sorted(counter.items())]
        return {'events':self.sequence,'paired_calls':self.serial,'lost_events':0,'open_calls':0,'io_failure':0,'unassigned_calls':self.unassigned,'unreadable_stack_calls':self.unreadable_stack,'unreadable_target_calls':self.unreadable_target,'phase_calls':dict(sorted(self.phases.items())),'targets':grouped(self.targets,['phase','site','target_state','target_slot','target_value']),'outcomes':grouped(self.outcomes,['phase','site','eax','last_error']),'observed_cleanup':grouped(self.cleanup,['site','bytes'])}
