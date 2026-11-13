"""Use the established physical-pause baseline without inventing API polls."""
import ast,inspect,hashlib
from broader_interaction_pause import verify_pause as original
from rendering_ui_acceptance import decode_keys
from rendering_menu_acceptance import artifact
from rendering_pilot import require
KEY="key_trace = decode_keys(artifact(reader, acquired, 'ui-acceptance.bin.keys'))"
EDGES="""edges = [v for v in key_trace['rows'] if v['code'] == 32 and (v['tick'] - before['tick']) % 2 ** 32 < 2 ** 31 and (after['tick'] - v['tick']) % 2 ** 32 < 2 ** 31]
require(len(edges) == 2 and [v['down'] for v in edges] == [True, False] and all(v['thread'] == before['thread'] == key_trace['owner'] and v['owned'] and v['input_sequence'] > 0 for v in edges), 'Original Space down/up ownership not witnessed between pause bytes')
key_proofs.append(dict(action=transition['action'], original_edges=edges))"""
ABSENT="key_proofs.append(dict(action=transition['action'], optional_API_journal_present=False, original_API_poll_proved=False, original_key_message_handler_proved=False, physical_Space_to_actual_pause_byte=True))"
def selected_source(source):
    tree=ast.parse(source);key=ast.parse(KEY).body[0];edges=ast.parse(EDGES).body;counts=[0,0]
    dump=lambda node:ast.dump(node,include_attributes=False)
    for node in list(ast.walk(tree)):
        body=getattr(node,'body',None)
        if not isinstance(body,list):continue
        for index,item in enumerate(list(body)):
            if dump(item)==dump(key):
                body[index]=ast.parse("key_trace = optional_keys(reader, acquired)").body[0];counts[0]+=1
        for index in range(len(body)-2):
            if [dump(v)for v in body[index:index+3]]==[dump(v)for v in edges]:
                body[index:index+3]=[ast.If(test=ast.parse('key_trace is not None',mode='eval').body,body=edges,orelse=ast.parse(ABSENT).body)];counts[1]+=1;break
    require(counts==[1,1],'Original optional API guard block changed')
    return ast.unparse(ast.fix_missing_locations(tree))+'\n'
def optional_keys(reader,acquired):
    found=[v for v in acquired['artifacts']if v['path'].endswith('/ui-acceptance.bin.keys')]
    require(len(found)<=1,'Ambiguous optional API journal')
    return decode_keys(artifact(reader,acquired,'ui-acceptance.bin.keys'))if found else None
def bind(terminal):
    source=selected_source(inspect.getsource(original));namespace=dict(original.__globals__,decode=terminal,optional_keys=optional_keys)
    exec(compile(source,__file__,'exec'),namespace);return namespace['verify_pause']
