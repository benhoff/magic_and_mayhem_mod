import struct
import importlib.util
import sys
from pathlib import Path
import unittest
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'tools'))
from drawing_caller_state import decode,fx_state,decode_bindings,decode_tail
spec=importlib.util.spec_from_file_location('caller_tail_decoder',Path(__file__).resolve().parents[1]/'tools/inspect-canvas-producers.py')
producer=importlib.util.module_from_spec(spec);spec.loader.exec_module(producer)


def tail_fixture(empty=False,kind=4):
    body=b'MNMPRO03'+struct.pack('<14I',3,64,0x40209ca7,262144,32,*([0]*9))
    if not empty:
        row=[0]*24;row[:4]=96,1,kind,99;row[5:8]=3,2,3;row[12:15]=3,2,0x1234
        if kind==11:row[14]=1
        body+=struct.pack('<24I',*row)
    header=b'MNMCST01'+struct.pack('<14I',1,64,len(body),7,0,3,0,123,456,0,0,int(not empty),1,1)
    return bytearray(header+body)


def fixture(kind=1,tag=22,count=3,pop=0):
    header=b'MNMCSA01'+struct.pack('<14I',1,64,768,262144,0x40209ca7,*([0]*9))
    row=[0]*192;row[:16]=[1,kind,tag,7,0x500845,0x123000,0x123004+pop,3,12,3,13,count,1234,1234,int(not pop),0x700000]
    row[19]=row[5];row[28]=row[6];row[24]=0x247;row[33]=0x246
    row[34:34+count]=range(1,count+1)
    if not pop:row[40:40+count]=range(1,count+1)
    data=bytearray(struct.pack('<192I',*row))
    fx=bytearray(224);struct.pack_into('<HH',fx,0,0x37f,5<<11);fx[4]=0xe0
    struct.pack_into('<I',fx,8,0x1f80)
    for i in range(3):fx[12+i*10:22+i*10]=bytes(range(i+1,i+11))
    fx[92:220]=bytes(range(128));data[312:536]=fx;data[536:760]=fx
    footer=struct.pack('<192I',2,3,1,1,0,0,*([0]*186))
    return bytearray(header+data+footer)


def fixture_v2(context=1,parent=0x5006b2):
    raw=fixture();raw[:8]=b'MNMCSA02';struct.pack_into('<I',raw,8,2)
    struct.pack_into('<II',raw,64+190*4,context,parent)
    return raw


def bindings_fixture(word=0x597086,generic=0x5947b2):
    state_raw=fixture_v2(2,0x500a8c);struct.pack_into('<I',state_raw,64+16,0x57e8fb)
    state=decode(state_raw);r=state['rows'][0]
    header=b'MNMCSB01'+struct.pack('<14I',1,64,32,262144,0x40209ca7,*([0]*9))
    row=struct.pack('<8I',r['id'],r['thread'],r['caller'],r['tag'],r['queue'],r['sequence_before'],word,generic)
    footer=struct.pack('<8I',0,1,1,0,0,0,0,0)
    return bytearray(header+row+footer),state


def fixture_v3():
    old=fixture_v2();header=old[:64];header[:8]=b'MNMCSA03';struct.pack_into('<I',header,8,3);struct.pack_into('<I',header,16,832)
    epoch=struct.pack('<16I',1,0,2,0,1,0,2,0,123,0,1,1,1,0,0,0)
    return bytearray(header+old[64:832]+epoch+old[832:]+bytes(64))


def fixture_v4():
    old=fixture_v3();header=old[:64];header[:8]=b'MNMCSA04';struct.pack_into('<I',header,8,4);struct.pack_into('<I',header,16,848)
    extension=list(struct.unpack_from('<16I',old,832))[:13]+[0x7d00,0x7d00,0,7,13,0,800]
    return bytearray(header+old[64:832]+struct.pack('<20I',*extension)+old[896:]+bytes(16))


class CallerStateTests(unittest.TestCase):
    def test_tail_source_prefix(self):
        r=decode_tail(tail_fixture(),producer.decode);self.assertEqual((r['generation'],r['source'],r['records']),(7,3,1))

    def test_tail_empty_prefix(self):self.assertEqual(decode_tail(tail_fixture(True),producer.decode)['records'],0)

    def test_tail_missing_nonce(self):
        raw=tail_fixture();struct.pack_into('<2I',raw,36,0,0)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_wrong_length(self):
        raw=tail_fixture();struct.pack_into('<I',raw,16,64)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_truncated(self):
        with self.assertRaises(ValueError):decode_tail(tail_fixture()[:-1],producer.decode)

    def test_tail_wrong_version(self):
        raw=tail_fixture();struct.pack_into('<I',raw,8,2)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_budget(self):
        raw=tail_fixture();struct.pack_into('<I',raw,16,16777217)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_active_queue(self):
        raw=tail_fixture();struct.pack_into('<I',raw,44,1)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_wrong_slot_state(self):
        raw=tail_fixture();struct.pack_into('<I',raw,56,2)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_sequence(self):
        raw=tail_fixture();struct.pack_into('<I',raw,52,2)
        with self.assertRaises(ValueError):decode_tail(raw,producer.decode)

    def test_tail_cannot_be_world(self):
        with self.assertRaises(ValueError):decode_tail(tail_fixture(kind=11),producer.decode)

    def test_tail_cannot_contain_failure(self):
        with self.assertRaises(ValueError):decode_tail(tail_fixture(kind=13),producer.decode)

    def test_v5_publication_identity_layout(self):
        raw=fixture_v4();raw[:8]=b'MNMCSA05';struct.pack_into('<I',raw,8,5)
        r=decode(raw);self.assertEqual(r['version'],5);self.assertEqual(r['rows'][0]['environment']['amplitude'],13)

    def test_v5_magic_version_mismatch(self):
        raw=fixture_v4();raw[:8]=b'MNMCSA05'
        with self.assertRaises(ValueError):decode(raw)

    def test_v4_raw_initializer_inputs(self):
        r=decode(fixture_v4());self.assertEqual(r['version'],4)
        self.assertEqual(r['rows'][0]['environment'],dict(wave_guard=0x7d00,source_rows_guard=0x7d00,glyph_sentinel=0,phase=7,amplitude=13,format=0,stride=800))

    def test_v4_inactive_channel_keeps_raw_inputs(self):
        raw=fixture_v4();raw[832:884]=bytes(52);r=decode(raw)['rows'][0]
        self.assertIsNone(r['epoch']);self.assertEqual(r['environment']['amplitude'],13)

    def test_v4_unknown_input_is_not_admission(self):
        raw=fixture_v4();struct.pack_into('<I',raw,832+18*4,0x12345678)
        self.assertEqual(decode(raw)['rows'][0]['environment']['format'],0x12345678)

    def test_v4_truncated_inputs(self):
        with self.assertRaises(ValueError):decode(fixture_v4()[:-16])

    def test_v4_wrong_row_size(self):
        raw=fixture_v4();struct.pack_into('<I',raw,16,832)
        with self.assertRaises(ValueError):decode(raw)

    def test_v4_footer_inputs_are_reserved(self):
        raw=fixture_v4();struct.pack_into('<I',raw,len(raw)-4,800)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_cannot_receive_v4_fields(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+13*4,0x7d00)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_world_identity(self):
        r=decode(fixture_v3());self.assertEqual(r['version'],3);self.assertEqual(r['rows'][0]['epoch']['generation_before'],1)

    def test_v3_no_continuous_channel(self):
        raw=fixture_v3();raw[832:896]=bytes(64);self.assertIsNone(decode(raw)['rows'][0]['epoch'])

    def test_v3_reserved_epoch(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+60,1)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_world_cannot_change_generation(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+16,2)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_world_cannot_change_source_epoch(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+24,3)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_source_reset_requires_new_identity(self):
        raw=fixture_v3();struct.pack_into('<I',raw,64+40,1)
        with self.assertRaises(ValueError):decode(raw)
        struct.pack_into('<2I',raw,832+44,0,0);struct.pack_into('<I',raw,832+24,3)
        self.assertEqual(decode(raw)['rows'][0]['epoch']['source_after'],3)

    def test_v3_backwards_epoch(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+24,1)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_inactive_epoch_payload(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+40,0)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_epoch_footer_is_zero(self):
        raw=fixture_v3();struct.pack_into('<I',raw,len(raw)-64,1)
        with self.assertRaises(ValueError):decode(raw)

    def test_v3_truncated_extension(self):
        with self.assertRaises(ValueError):decode(fixture_v3()[:-4])

    def test_v3_nonce_required(self):
        raw=fixture_v3();struct.pack_into('<I',raw,832+32,0)
        with self.assertRaises(ValueError):decode(raw)

    def reject_word(self,offset,value):
        raw=fixture();struct.pack_into('<I',raw,offset,value)
        with self.assertRaises(ValueError):decode(raw)

    def test_native_saved_output(self):
        r=decode(fixture());self.assertTrue(r['normal_retirement']);self.assertEqual(r['completed'],1)
        self.assertEqual(r['rows'][0]['after_registers'][8],0x246)
        self.assertEqual(r['rows'][0]['before_fx']['registers'][3],bytes(10))

    def test_retained_return_and_popped_arguments(self):
        r=decode(fixture(2,21,2,8));self.assertEqual(r['rows'][0]['kind'],2)
        self.assertIsNone(r['rows'][0]['after_arguments'])

    def test_missing_footer(self):
        with self.assertRaises(ValueError):decode(fixture()[:-768])

    def test_truncated_record(self):
        with self.assertRaises(ValueError):decode(fixture()[:-1])

    def test_active_return_is_incomplete(self):self.reject_word(64+768+16,1)
    def test_incomplete_completed_count(self):self.reject_word(64+768+12,0)
    def test_failed_output_is_incomplete(self):self.reject_word(64+768+20,1)
    def test_unknown_version(self):self.reject_word(8,2)
    def test_unbounded_capacity(self):self.reject_word(20,0xffffffff)
    def test_unknown_kind(self):self.reject_word(64+4,4)
    def test_unknown_entry(self):self.reject_word(64+8,35)
    def test_invalid_id(self):self.reject_word(64,2)
    def test_bad_arity(self):self.reject_word(64+44,6)
    def test_bad_callee_pop(self):self.reject_word(64+24,0x123008)
    def test_bad_output_stack(self):self.reject_word(64+28*4,0x123008)
    def test_bad_input_stack(self):self.reject_word(64+19*4,0x123004)
    def test_unused_argument_bytes(self):self.reject_word(64+39*4,1)
    def test_reserved_row_bytes(self):self.reject_word(64+190*4,1)
    def test_queue_changed_inside_entry(self):self.reject_word(64+36,4)
    def test_backwards_sequence(self):self.reject_word(64+40,11)
    def test_non_cpu_flags(self):self.reject_word(64+33*4,0x244)

    def test_empty_fx_register_has_no_payload(self):
        raw=fixture();raw[64+312+42]=1
        with self.assertRaises(ValueError):decode(raw)

    def test_reserved_fx_bytes(self):
        raw=fixture();raw[64+536+220]=1
        with self.assertRaises(ValueError):decode(raw)

    def test_observation_does_not_assert_equivalence(self):
        raw=fixture();struct.pack_into('<I',raw,64+25*4+7*4,0x55)
        r=decode(raw);self.assertNotEqual(r['rows'][0]['before_registers'][7],r['rows'][0]['after_registers'][7])

    def test_v2_enclosing_contexts(self):
        for context in range(1,5):
            r=decode(fixture_v2(context));self.assertEqual(r['version'],2)
            self.assertEqual((r['rows'][0]['context_kind'],r['rows'][0]['enclosing_caller']),(context,0x5006b2))

    def test_v2_missing_parent(self):
        with self.assertRaises(ValueError):decode(fixture_v2(1,0))

    def test_v2_parent_without_kind(self):
        with self.assertRaises(ValueError):decode(fixture_v2(0,0x5006b2))

    def test_v2_unknown_context_kind(self):
        with self.assertRaises(ValueError):decode(fixture_v2(5))

    def test_v2_magic_version_mismatch(self):
        raw=fixture_v2();struct.pack_into('<I',raw,8,1)
        with self.assertRaises(ValueError):decode(raw)

    def test_v2_footer_cannot_contain_context(self):
        raw=fixture_v2();struct.pack_into('<II',raw,64+768+190*4,1,0x5006b2)
        with self.assertRaises(ValueError):decode(raw)

    def test_v2_unknown_parent_remains_an_observation(self):
        r=decode(fixture_v2(2,0x4feedd));self.assertEqual(r['rows'][0]['enclosing_caller'],0x4feedd)

    def test_binding_pairs_exact_state_identity(self):
        raw,state=bindings_fixture();r=decode_bindings(raw,state)
        self.assertEqual(r['rows'],[dict(id=1,word_target=0x597086,generic_target=0x5947b2)])

    def test_binding_missing_footer(self):
        raw,state=bindings_fixture()
        with self.assertRaises(ValueError):decode_bindings(raw[:-32],state)

    def test_binding_duplicate_identity(self):
        raw,state=bindings_fixture();raw=raw[:-32]+raw[64:96]+struct.pack('<8I',0,2,1,0,0,0,0,0)
        with self.assertRaises(ValueError):decode_bindings(raw,state)

    def test_binding_missing_opaque_entry(self):
        raw,state=bindings_fixture();raw=raw[:64]+struct.pack('<8I',0,0,1,0,0,0,0,0)
        with self.assertRaises(ValueError):decode_bindings(raw,state)

    def test_binding_changed_thread(self):
        raw,state=bindings_fixture();struct.pack_into('<I',raw,68,8)
        with self.assertRaises(ValueError):decode_bindings(raw,state)

    def test_binding_unknown_targets_remain_raw(self):
        raw,state=bindings_fixture(0x12345678,0);r=decode_bindings(raw,state)
        self.assertEqual(r['rows'][0]['word_target'],0x12345678);self.assertEqual(r['rows'][0]['generic_target'],0)

    def test_binding_active_footer(self):
        raw,state=bindings_fixture();struct.pack_into('<I',raw,len(raw)-20,1)
        with self.assertRaises(ValueError):decode_bindings(raw,state)

    def test_binding_unknown_version(self):
        raw,state=bindings_fixture();struct.pack_into('<I',raw,8,2)
        with self.assertRaises(ValueError):decode_bindings(raw,state)

    def test_binding_unknown_state_id(self):
        raw,state=bindings_fixture();struct.pack_into('<I',raw,64,2)
        with self.assertRaises(ValueError):decode_bindings(raw,state)


if __name__=='__main__':unittest.main()
