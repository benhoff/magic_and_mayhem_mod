"""Installed font sources and committed commands; DC custody is a separate chain."""
import hashlib,struct
from pathlib import Path
from collections import Counter
from font_lifetime_observation import decode,source,word
from resident_capture import records
from scene_actor_observation import crc32c
from scene_effects import module
from rendering_pilot import require

def normalized(raw):
 image=bytearray(raw);count,rows,profiles=word(image,12),word(image,16),word(image,36)
 table=808+profiles*rows*8;base=table+count*4
 for i in range(count):
  at=base+word(image,table+4*i);require(at+32<=len(image),'Installed font glyph extent');image[at+28:at+32]=bytes(4)
 source(image);return bytes(image)

def route(root,reader,acquired,session,bound):
 driver=acquired['font_lifetime_route'];size=list(map(int,acquired['configuration']['requested_size'].split('x')))
 require(driver['success']and driver['policy']=='public-sft-load-use-reload-teardown-v1'and driver['simulation_writes']is False and driver['cast_requests']==0,'Incomplete public font route')
 require(len(driver['worlds'])==2 and len(driver['reloads'])==2 and all(w['actual_extent']==size for w in driver['worlds']),'Missing repeated actual World or preference reload')
 states=acquired['states'];require([r['screen']for r in states]==[3,22,14,25,14,7,26,22,3,10,3,10,3,22,14,25,14,7,26,22,3],'Original font route states differ')
 require(len({s['thread']for s in states})==1 and all(b['generation']>a['generation']and b['ack']>a['ack']for a,b in zip(states,states[1:])),'Original font route owner/generation/ack')
 menus=module('run-rendering-menu-acceptance')
 for saved in states:
  path=root/saved['wire_path'];bound(path);actual=menus.state(path)[0]
  require(all(saved[k]==actual[k]for k in ('sequence','generation','screen','ready','ack','status','thread','handoff')),'Saved original font route differs')
 def one(suffix):
  matches=[p for p in session.rglob('*')if p.is_file()and p.name==suffix];require(len(matches)==1,'Expected one retained '+suffix);bound(matches[0]);return matches[0].read_bytes()
 font=decode(one('font-lifetime-0001.bin'));require(font['version']==2,'Current World/source-bound font protocol required');require(font['owner']==states[0]['thread'],'Font observation owner differs')
 fonts=font['fonts'];require(fonts and all(f['loaded']for f in fonts.values()),'Failed installed font load')
 installed={};roles=Counter();paths=set()
 for f in fonts.values():
  name=f['path'].replace('\\','/');require(not name.startswith('/')and '..'not in name.split('/'),'Unbounded installed font path')
  matches=[p for p in(session/'game').rglob('*.sft')if p.relative_to(session/'game').as_posix().lower()==name.lower()];require(len(matches)==1,'Installed font path does not resolve uniquely: '+name)
  p=matches[0];relative=p.relative_to(session/'game').as_posix();raw=p.read_bytes();h=hashlib.sha256(raw).hexdigest()
  require(acquired['installation_inputs'][relative]['sha256']==h,'Installed font input hash differs')
  expected=normalized(raw);require(f['image']==expected,'Observed loaded font differs from independently read installed bytes')
  role=f['role'];lower=relative.lower();require(('heading text'in lower if role==1 else'body text'in lower if role==2 else'tooltip text'in lower if role==3 else'yellowtext'in lower),'Installed font object/path role differs')
  roles[role]+=1;paths.add(lower);installed[relative]=dict(sha256=h,bytes=len(raw))
 require(set(roles)=={1,2,3,4}and roles[1]>=3 and roles[2]>=3 and min(roles.values())>=1 and len(paths)==6,'All four roles, both size fonts and two real reloads required')
 require(all(f['retired']or f['superseded']for f in fonts.values())and font['original_reload_retained_generations']>0,'Missing final ownership or original reload-retention accounting')
 original=root/acquired['temporary_root']/'worlds.bin';bound(original)
 cps=module('inspect-canvas-producers');committed={};next_prefix=1;max_world=0;last_context=(0,0)
 for g,chunk,stream,raster in records(original):
  context=(g,chunk or next_prefix);last_context=context
  if chunk:next_prefix=chunk+1
  else:max_world=g
  header,rows=cps.decode(stream,complete=False)
  for r,body in rows:
   if r[2]==8:
    key=(*context,r[1]);require(key not in committed,'Duplicate committed glyph source identity');committed[key]=(g,chunk,r,body)
 require(committed,'No consumed font source commands')
 uses=Counter();used_ids=set();world_uses=Counter();matched=set();seen=set();unpublished=[];last_matched_ordinal=0
 for glyph in font['glyphs']:
  key=(glyph['world_generation'],glyph['source_ordinal'],glyph['sequence']);require(key not in seen,'Duplicate observed glyph source identity');seen.add(key)
  if key not in committed:unpublished.append(glyph);continue
  g,chunk,r,body=committed[key];matched.add(key);h=glyph['header'];f=fonts[glyph['font']];frame=f['frames'][glyph['ordinal']]
  require((h[10],h[11],h[12],h[13])==(r[1],r[3],r[19],crc32c(body[:r[19]]))and body[:r[19]]==frame,'Font generation does not join exact consumed source')
  require(h[15:20]==[r[8],r[9],r[15],r[16],r[17]]and h[20]==r[22],'Font glyph position/colour/caller differs')
  uses[f['role']]+=1;used_ids.add(f['id']);world_uses[g]+=1;last_matched_ordinal=max(last_matched_ordinal,h[1])
 require(matched==set(committed),'Consumed glyph source lacks generation custody')
 require(set(uses)>={1,2,3},'Heading/body/tooltip consumed draws missing')
 # Native retirement may leave a final WRITING prefix of the original hidden
 # Main menu unused. It must be strictly after every consumed glyph, in only
 # the exact next World/source context, with no intervening or middle gap.
 require(len(unpublished)<16384 and all(g['header'][1]>last_matched_ordinal and g['world_generation']==max_world+1 and g['source_ordinal']==next_prefix for g in unpublished),'Unconsumed font records are not a bounded final hidden-menu tail')
 # These original draws can belong to hidden menus. Visible native menu
 # delivery is checked separately against the Qt font and screenshot trace.
 require(any(fonts[i]['previous']for i in used_ids),'No font use after real reload')
 artifacts=acquired['artifacts']
 def artifact(suffix):
  rows=[v for v in artifacts if v['path'].endswith(suffix)];require(len(rows)==1,'Missing unique DC input '+suffix);v=rows[0];raw=reader.read_bytes(v['record']);require(hashlib.sha256(raw).hexdigest()==v['sha256'],'DC artifact changed');return raw
 from cpu_reader_journal import decode as dc_decode,chains,wire_reads
 commands=[v for v in artifacts if '/capture/commands-'in v['path']and v['path'].endswith('.bin')];require(len(commands)==1,'DC commands missing')
 command=reader.read_bytes(commands[0]['record'])
 header,rows=dc_decode(artifact('/cpu-reader-journal-v1.bin'));require(header[25]==29,'Nominal font/DC journal mode required')
 if header[20]:
  requests,counts,stream_hash=wire_reads(command,acquired['session_identity'],artifact('/native-cpu-read-counts.bin'),artifact('/render-commands.bin.cpu-reply'))
 else:
  from rendering_capture import capture
  from rendering_pilot import commands as decode_commands
  wire=capture(command);require(wire[0][0]['role']=='commands'and wire[0][0]['session']==acquired['session_identity']and wire[-2][0]['kind']=='ended','Closed nominal command stream required')
  require(not any(row['kind']=='cpu-read'for row,body in wire),'Zero-request journal has a CPU reply')
  stream=b''.join(body for row,body in wire if row['kind']=='fragment');require(not any(op==21 for op,seq,body in decode_commands(stream)),'Zero-request journal has a wire request')
  requests={};counts=dict(completed=0);stream_hash=hashlib.sha256(stream).hexdigest()
 dc=chains(header,rows,requests)
 ring=artifact('/render-commands.bin');require(len(ring)==1048640 and ring[:8]==b'MNMRDC03'and struct.unpack_from('<4I',ring,8)==(3,1048640,header[11],struct.unpack_from('<I',ring,36)[0])and struct.unpack_from('<2I',ring,24)==(2,0),'DC journal command session differs')
 require(dc['complete_observed_chain']and dc['dc_borrows']>0,'Incomplete actual DC acquire/release chain')
 require(acquired['retirement']['state']=='completed','Normal native ownership retirement missing')
 return dict(verified=True,installed_inputs=installed,load_counts=dict(roles),font_generations=len(fonts),glyph_commands=len(committed),observed_glyph_commands=len(font['glyphs']),unconsumed_final_hidden_menu_glyphs=len(unpublished),loaded_but_undrawn_roles=sorted(set(roles)-set(uses)),used_roles=dict(uses),used_generations=sorted(used_ids),original_reload_retained_generations=font['original_reload_retained_generations'],original_reload_retention_policy='Replaced original raw/palette sources remain retained until process retirement; not claimed freed at reload.',dc=dc,dc_wire_requests=len(requests),dc_command_stream_sha256=stream_hash,normal_native_retirement=True,original_gdi_text_claimed=False)
