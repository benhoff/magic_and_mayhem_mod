"""Public Quick World, Main preference reload and repeated text use; no casts."""
import json,time
from palette_cycle_journey import PaletteJourney,menus
from scene_effects import module
from rendering_pilot import require
safe=module('run-rendering-copy-branches')
class FontJourney(PaletteJourney):
 def __init__(self,*args):
  super().__init__(*args)
  safe.uncache_accessibility(self.access);original=self.access.nodes
  self.access.nodes=lambda:safe.optional_accessible_ids(safe.fresh_nodes(original,lambda:self.child.poll()is None,reset=lambda:safe.uncache_accessibility(self.access)))
  self.proof=dict(policy='public-sft-load-use-reload-teardown-v1',success=False,simulation_writes=False,cast_requests=0,worlds=[],reloads=[])
 def select_spell(self):
  # Font custody does not require a particular offered recipe or a new draft.
  inventory=self.ready_spellbox_inventory();self.proof.setdefault('inventories',[]).append(inventory)
  self.capture('font-original-spellbox-'+str(len(self.proof['inventories'])))
  self.press('spellboxOK')
 await_main=safe.Journey.await_main
 def read(self):
  if self.session:
   path=self.session/'native-terminal-state.json'
   if path.exists():
    terminal=json.loads(path.read_bytes());require(not terminal['state']['terminal'],'Native terminal before requested font route: '+terminal['error'])
  return menus.Journey.read(self)
 def action(self,name,screen):
  ack=self.last['ack']+1;self.press(name);return self.wait(screen,ack)
 def world(self,ordinal):
  baseline=self.latest();generation=baseline['generation']if baseline else 0
  self.action('mainMenuAction2',22);self.capture('font-quick-'+str(ordinal));self.action('quickBattleAction2',14)
  self.action('singlePlayerMap',25)
  lists=[(n,v)for n,v in self.access.nodes()if v['role']=='list'and n.get_child_count()>1];require(len(lists)==1,'Font route map list ambiguity')
  self.input.click(lists[0][1]['rect']);self.key('first-original-font-map-'+str(ordinal),0xff50)
  self.action('mapSelectionOk',14);self.action('singlePlayerStart',7)
  self.select_spell()
  first=self.until_frame(lambda f:f['generation']>=generation+32,45)
  node,_=self.node('nativeGameViewport');require(node.get_component_iface().grab_focus(),'World font viewport focus refused');self.input.activate()
  self.game_key('font-route-public-up-'+str(ordinal),0xff52,.15)
  last=self.until_frame(lambda f:f['generation']>=first['generation']+8,15)
  self.capture('font-world-'+str(ordinal))
  self.proof['worlds'].append(dict(ordinal=ordinal,first_generation=first['generation'],last_generation=last['generation'],actual_extent=list(last['extent']),entry_after_state=len(self.states)-1))
  self.game_key('font-world-normal-escape-'+str(ordinal),0xff1b,.5);self.wait(26,self.last['ack']+1,timeout=30)
  self.action('quickResultAction2',22);self.action('quickBattleAction3',3)
 def reload(self,low,label):
  opened=self.action('mainMenuAction3',10)
  self.press('preferencesRadio'+('2'if low else'1'))
  applied=self.action('preferencesOk',3)
  self.proof['reloads'].append(dict(label=label,low=low,opened=opened,returned=applied))
 def run(self):
  self.input.activate();self.click('Launch game');self.await_main();self.capture('font-main-initial')
  self.world(1)
  target_low=self.size==(640,480)
  self.reload(not target_low,'opposite-mode');self.reload(target_low,'restore-target-mode');self.capture('font-main-restored')
  self.world(2)
  self.proof['success']=True;self.press('mainMenuAction4');return self.proof
