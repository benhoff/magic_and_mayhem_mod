import copy
import sys
import struct
import unittest
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from rendering_caller_state_join import obligations,source_match,join_native,profile_operations,validate_model_results,retained_census,join_retained_sources
from drawing_caller_state import fx_state


def analysis(required=None,closed=True):
    return {r:dict(closed=closed,escapes=[]if closed else[dict(reason='unknown')],required={r:v}if v else{})
            for r,v in zip(('eax','ecx','edx','eflags'),required or(0xffff,0,0,0))}


def fixture(tag=22):
    row=dict(id=1,kind=1,tag=tag,caller=0x500845,context_kind=0,enclosing_caller=0,
             before_arguments=(0x100000,2,3,0,0),before_registers=(0,0,0,0,0,2,0x100000,0,0x246),
             after_registers=(0,0,0,0,0,0,0,0,0x246),queue=1,sequence_before=0,sequence_after=1,
             environment=dict(stride=800,format=0),epoch=dict(nonce=123,generation_before=1,generation_after=1,
                  source_before=1,source_after=1,world_before=True,world_after=True))
    fields=[0]*24;fields[0]=136;fields[1]=1;fields[2]=9;fields[3]=99;fields[5:8]=800,600,800
    fields[8:10]=2,3;fields[12:14]=800,600;fields[19]=40;fields[22]=row['caller']
    payload=bytearray(40);payload[0]=40
    operation=dict(fields=tuple(fields),payload=bytes(payload),generation=1,source=1,chunk=0,queue=1)
    return row,operation


class Joins(unittest.TestCase):
    def result_fixture(self):
        row,op=fixture();fx=bytes(224)
        row.update(after_arguments=None,after_workspace=(0,)*16,after_fx=fx_state(fx),last_error_before=7,last_error_after=7)
        proof=obligations(row,dict(sites=[]),{})
        raw=b'MNMCSO01'+struct.pack('<II',1,1)+struct.pack('<8I',380,1,1,1,1,1,1,1)
        raw+=struct.pack('<9I',*row['after_registers'])+bytes(24+64)+fx
        return raw,[dict(row=row,operation=op,obligations=proof)]

    def test_expected_bytes_independently_pass(self):
        raw,joined=self.result_fixture();self.assertTrue(validate_model_results(raw,joined)['verified'])

    def test_expected_bytes_reject_false_success(self):
        for offset in(16+32,16+32+7*4,16+32+8*4,16+92,16+156+2):
            with self.subTest(offset=offset):
                raw,joined=self.result_fixture();altered=bytearray(raw);altered[offset]^=1
                with self.assertRaises(ValueError):validate_model_results(altered,joined)

    def test_expected_bytes_reject_false_failure(self):
        raw,joined=self.result_fixture();altered=bytearray(raw);altered[16+12]=0
        with self.assertRaises(ValueError):validate_model_results(altered,joined)

    def test_expected_bytes_reject_foreign_id(self):
        raw,joined=self.result_fixture();altered=bytearray(raw);altered[20]=2
        with self.assertRaises(ValueError):validate_model_results(altered,joined)

    def test_expected_bytes_reject_last_error(self):
        raw,joined=self.result_fixture();joined[0]['row']['last_error_after']=8
        with self.assertRaises(ValueError):validate_model_results(raw,joined)

    def test_fast_preserves_direction_flag(self):
        row,_=fixture(17);p=obligations(row,dict(sites=[dict(address=row['caller'],signature='83c414',analyses=analysis())]),{})
        self.assertEqual(p['eflags'],0x400)

    def test_retained_nested_census(self):
        parent,_=fixture();child=copy.deepcopy(parent)
        parent.update(id=1,kind=2,thread=1,entry_sp=1000,return_sp=1016,last_error_before=0,last_error_after=0)
        child.update(id=2,thread=1,entry_sp=900,return_sp=916)
        census=retained_census([child,parent]);self.assertEqual(census['parents'][0]['native_children'],[2])

    def test_retained_overlap_refuses(self):
        parent,_=fixture();child=copy.deepcopy(parent)
        parent.update(id=1,kind=2,thread=1,entry_sp=1000,return_sp=1016,last_error_before=0,last_error_after=0)
        child.update(id=2,thread=1,entry_sp=1100,return_sp=1116)
        with self.assertRaises(ValueError):retained_census([child,parent])

    def test_retained_root_source(self):
        row,op=fixture();row['kind']=2
        result=join_retained_sources([row],{(1,1,1):op},set(),dict(parents=[]),123)
        self.assertEqual(result['roots'][0]['id'],1)

    def test_retained_root_not_reused(self):
        row,op=fixture();row['kind']=2
        with self.assertRaises(ValueError):join_retained_sources([row],{(1,1,1):op},{(1,1,1)},dict(parents=[]),123)

    def test_unobserved_source_refuses(self):
        row,op=fixture();row['kind']=2
        with self.assertRaises(ValueError):join_retained_sources([row],{(1,1,1):op,(1,1,2):op},set(),dict(parents=[]),123)

    def test_nested_retained_requires_actual_parent(self):
        row,_=fixture();row['kind']=2
        with self.assertRaises(ValueError):join_retained_sources([row],{},set(),dict(parents=[]),123)

    def test_fast_cdecl_cleanup_required(self):
        row,_=fixture(17)
        with self.assertRaises(ValueError):obligations(row,dict(sites=[dict(address=row['caller'],signature='83c410',analyses=analysis())]),{})

    def test_fast_cdecl_arity_required(self):
        row,_=fixture(17);row['before_arguments']=(1,2,3)
        with self.assertRaises(ValueError):obligations(row,dict(sites=[dict(address=row['caller'],signature='83c414',analyses=analysis())]),{})

    def test_full_state_has_full_obligations(self):
        row,_=fixture();p=obligations(row,dict(sites=[]),{});self.assertEqual((p['model'],p['eax'],p['ecx'],p['edx']),('word',0xffffffff,0xffffffff,0xffffffff))

    def test_native_source_identity(self):
        row,op=fixture();p=join_native([row],{(1,1,1):op},dict(sites=[]),{},123)
        self.assertEqual(len(p['joined']),1);self.assertNotIn('whole_operation_acceptance',p)

    def test_foreign_nonce(self):
        row,op=fixture()
        with self.assertRaises(ValueError):join_native([row],{(1,1,1):op},dict(sites=[]),{},124)

    def test_foreign_generation(self):
        row,op=fixture()
        with self.assertRaises(ValueError):join_native([row],{(2,1,1):op},dict(sites=[]),{},123)

    def test_foreign_source_prefix(self):
        row,op=fixture()
        with self.assertRaises(ValueError):join_native([row],{(1,2,1):op},dict(sites=[]),{},123)

    def test_foreign_queue(self):
        row,op=fixture();op['queue']=2
        with self.assertRaises(ValueError):join_native([row],{(1,1,1):op},dict(sites=[]),{},123)

    def test_world_scope_mismatch(self):
        row,op=fixture();row['epoch']['world_after']=False
        with self.assertRaises(ValueError):join_native([row],{(1,1,1):op},dict(sites=[]),{},123)

    def test_duplicate_source_consumption(self):
        row,op=fixture();second=copy.deepcopy(row);second['id']=2
        with self.assertRaises(ValueError):join_native([row,second],{(1,1,1):op},dict(sites=[]),{},123)

    def test_caller_mismatch(self):
        row,op=fixture();f=list(op['fields']);f[22]+=1;self.assertFalse(source_match(row,f,op['payload']))

    def test_argument_mismatch(self):
        row,op=fixture();f=list(op['fields']);f[9]+=1;self.assertFalse(source_match(row,f,op['payload']))

    def test_stride_mismatch(self):
        row,op=fixture();row['environment']['stride']=801;self.assertFalse(source_match(row,op['fields'],op['payload']))

    def test_noncanonical_frame(self):
        row,op=fixture();p=bytearray(op['payload']);p[28]=1;self.assertFalse(source_match(row,op['fields'],p))

    def test_unknown_family(self):
        row,_=fixture(34)
        with self.assertRaises(ValueError):obligations(row,dict(sites=[]),{})

    def test_unknown_fast_caller(self):
        row,_=fixture(17)
        with self.assertRaises(ValueError):obligations(row,dict(sites=[]),{})

    def test_fast_consumed_register_refuses(self):
        row,_=fixture(17);r=dict(sites=[dict(address=row['caller'],analyses=analysis((0xffff,1,0,0)))])
        with self.assertRaises(ValueError):obligations(row,r,{})

    def test_fast_upper_return_refuses(self):
        row,_=fixture(17);r=dict(sites=[dict(address=row['caller'],analyses=analysis((0xffffffff,0,0,0)))])
        with self.assertRaises(ValueError):obligations(row,r,{})

    def test_unknown_flow_refuses(self):
        row,_=fixture(17);r=dict(sites=[dict(address=row['caller'],analyses=analysis(closed=False))])
        with self.assertRaises(ValueError):obligations(row,r,{})

    def test_conditional_selectors_and_parent(self):
        row,_=fixture(17);row.update(context_kind=2,enclosing_caller=0x500566)
        context=dict(context_kind=2,returns=[0x500566],analyses=analysis())
        r=dict(sites=[dict(address=row['caller'],signature='83c414',analyses=analysis(closed=False),callee_summary_contextual=[context])],
               conditional_callee_requirements=dict(word_targets=[0x597086],generic_targets=[0x5947b2]))
        p=obligations(row,r,{1:dict(word_target=0x597086,generic_target=0x5947b2)});self.assertTrue(p['conditional_callees'])
        with self.assertRaises(ValueError):obligations(row,r,{1:dict(word_target=0x12345678,generic_target=0x5947b2)})
        row['enclosing_caller']+=1
        with self.assertRaises(ValueError):obligations(row,r,{1:dict(word_target=0x597086,generic_target=0x5947b2)})

    def test_colour_no_source_is_not_automatically_noop(self):
        row,_=fixture(52);row['sequence_after']=2
        with self.assertRaises(ValueError):join_native([row],{},dict(sites=[]),{},123)

    def test_profile_keeps_sequence_namespaces(self):
        _,op=fixture();operations=[(op['fields'],op['payload'])]
        def decoder(raw,complete):return None,operations
        rows=profile_operations([(1,1,b'prefix',b''),(1,0,b'world',b'request'),(2,0,b'world',b'request')],decoder)
        self.assertEqual(set(rows),{(1,1,1),(1,2,1),(2,2,1)})

    def test_profile_rejects_foreign_world(self):
        with self.assertRaises(ValueError):profile_operations([(2,0,b'',b'')],lambda raw,complete:(None,[]))

    def test_profile_rejects_source_gap(self):
        with self.assertRaises(ValueError):profile_operations([(1,2,b'',b'')],lambda raw,complete:(None,[]))


if __name__=='__main__':unittest.main()
