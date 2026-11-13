"""Retained-only terminal UI identity extension, joined to actual frame-input rows."""
import ast,hashlib,inspect,struct
from types import FunctionType
from rendering_pilot import require
from rendering_interaction import decode as frame_decode
from rendering_ui_acceptance import decode as original_decode
from rendering_menu_acceptance import artifact
POLICY='stopped-terminal-UI-exact-frame-input-identity-v1'
MESSAGE='UI World return has neither its sealed generation nor a terminal menu transfer'

def selected_decoder_source(source):
    tree=ast.parse(source);count=0
    expected=ast.parse("frame and (w[5]==frame['generation'] or w[16]!=2 and w[5]==frame['generation']+1)",mode='eval').body
    class Select(ast.NodeTransformer):
        def visit_Call(self,node):
            nonlocal count
            if (isinstance(node.func,ast.Name)and node.func.id=='require'and len(node.args)==2
            and isinstance(node.args[1],ast.Constant)and node.args[1].value==MESSAGE):
                require(ast.dump(node.args[0],include_attributes=False)==ast.dump(expected,include_attributes=False),'Old UI identity predicate changed')
                count+=1;node.args[0]=ast.parse('_terminal_UI_identity(w,owner,frame)',mode='eval').body
            return self.generic_visit(node)
    tree=Select().visit(tree);require(count==1,'Missing unique old terminal UI identity guard')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'

class TerminalUi:
    def __init__(self,ui_raw,frame_raw,completed,nonce,normal_quit):
        require(normal_quit is True and type(completed)is int and 1<=completed<=8192,'Stopped complete Native source retirement required')
        require(len(ui_raw)>=16 and ui_raw[:8]==b'MNMUIA01'and struct.unpack_from('<2I',ui_raw,8)==(3,512)
            and (len(ui_raw)-16)%512==0 and len(ui_raw)<16+8192*512,'Complete version3 UI bytes required')
        self.raw=ui_raw;self.completed=completed;self.nonce=str(nonce);self.frames=frame_decode(frame_raw)['rows'];self.diagnostics=[];self.keys={}
        world=[r for r in self.frames if r['kind']==1 and r['frame']];require(world and max(r['frame']['generation']for r in world)==completed,'Frame-input/retired Native source extent differs')
        for index,row in enumerate(self.frames):
            if row['kind']==1 and row['frame']:
                key=(row['world_return'],row['frame']['generation'],row['frame']['nonce'],row['thread'])
                self.keys.setdefault(key,[]).append((index,row))
        self.ui_words=list(struct.iter_unpack('<128I',ui_raw[16:]));source=selected_decoder_source(inspect.getsource(original_decode));ns=dict(original_decode.__globals__,_terminal_UI_identity=self.identity)
        exec(compile(source,__file__,'exec'),ns);self.selected=ns['decode'];self.selected_source_sha256=hashlib.sha256(source.encode()).hexdigest()
    def identity(self,w,owner,frame):
        if frame and(w[5]==frame['generation']or w[16]!=2 and w[5]==frame['generation']+1):return True
        if (not frame or w[16]==2 or frame['generation']!=self.completed or frame['nonce']!=self.nonce
        or frame['owner']!=owner or w[4]!=owner or w[5]<=self.completed):return False
        found=self.keys.get((w[5],frame['generation'],frame['nonce'],owner),[])
        if len(found)!=1:return False
        index,row=found[0]
        if (w[3]-row['tick'])%2**32>=2**31:return False
        following=[r for r in self.frames[index+1:]if r['kind']==1]
        if any(not r['frame']or r['frame']['generation']!=self.completed for r in following):return False
        if following and(following[0]['tick']-w[3])%2**32>=2**31:return False
        if any(v[1]==1 and v[16]==2 for v in self.ui_words[w[0]:]):return False
        fact=dict(UI_row=w[0],original_World_return=w[5],sealed_source_generation=frame['generation'],nonce=frame['nonce'],
            thread=owner,non_World_top=w[16],frame_input_row=row['ordinal'],frame_input_tick=row['tick'],UI_tick=w[3],
            source_World_credit=0,action_witness_credit=0)
        if fact not in self.diagnostics:self.diagnostics.append(fact)
        return True
    def __call__(self,raw,allow_pending=False):
        require(raw==self.raw and allow_pending is False,'Terminal identity extension consumes only exact stopped UI bytes')
        return self.selected(raw,allow_pending=False)

def from_acquisition(reader,a):
    require(a['success']and a['sources_stable']and a['private_wine_stopped']and not a.get('forced_private_prefix_stop',False)
        and a['resident']['normal_quit_retired'],'Complete normally stopped acquisition required')
    return TerminalUi(artifact(reader,a,'ui-acceptance.bin'),artifact(reader,a,'frame-input.bin'),
        int(a['resident']['completed']),a['resident']['nonce'],True)


def bind_pause(original,terminal):
    """Retain every pause verifier instruction and replace only its UI reader."""
    selected=FunctionType(original.__code__,dict(original.__globals__,decode=terminal),original.__name__,original.__defaults__,original.__closure__)
    selected.__kwdefaults__=original.__kwdefaults__
    require(selected.__code__ is original.__code__,'Pause verifier code changed')
    return selected
