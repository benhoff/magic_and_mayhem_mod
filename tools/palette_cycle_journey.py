"""Ordinary World refresh plus bounded public camera movement; no spell cast."""
import struct,time
from campaign_route import hud_point
from campaign_input_observation import decode as input_rows
from rendering_interaction_live import LiveSources
from scene_actor_observation import decode as actors
from palette_observation import decode as palettes
from scene_attachment_journey import AttachmentJourney,menus
from rendering_pilot import require

class PaletteJourney(AttachmentJourney):
 def __init__(self,child,out,env,size):
  super().__init__(child,out,env,size)
  self.proof=dict(policy='public-world-palette-refresh-v1',success=False,simulation_writes=False,cast_requests=0)

 def select_spell(self):
  initial=self.ready_spellbox_inventory();self.proof['inventory']=initial
  offered=[i for i in range(21)if initial['offered']&(1<<i)]
  candidates=[(i,a)for i in offered for a in range(3)if initial['counts'][a]and initial['recipes'][i*3+a]==74]
  require(len(candidates)==1,'Original inventory lacks unique Iron Skin recipe');ingredient,alignment=candidates[0]
  require(initial['spells'][ingredient*3+alignment]=='Iron Skin','Original recipe/name binding differs')
  self.proof['recipe']=dict(ingredient=ingredient,alignment=alignment,effect=74,name='Iron Skin')
  slots=[(a,n)for a in range(3)for n in range(initial['counts'][a])]
  for visible,(a,n)in enumerate(slots):
   if initial['assignments'][a*21+n]>=0:self.press('spellboxTalisman'+str(visible));self.press('spellboxItem0')
  for i in range(len(slots)):require(self.node('spellboxTalisman'+str(i))[1]['name'].endswith(' — empty'),'Original draft did not clear through public controls')
  target=next(i for i,(a,n)in enumerate(slots)if a==alignment)
  self.press('spellboxItem'+str(offered.index(ingredient)));self.press('spellboxTalisman'+str(target))
  labels=[self.node('spellboxTalisman'+str(i))[1]['name']for i in range(len(slots))]
  require('Iron Skin'in labels[target]and all(i==target or label.endswith(' — empty')for i,label in enumerate(labels)),'Original single-spell draft differs')
  self.proof['draft_labels']=labels;self.capture('single-iron-skin-draft');self.press('spellboxOK')

 def cast_and_observe(self):
  # The original World entry owns the refresh. No spell or simulation writes
  # are needed to manufacture a cycling producer or its palette values.
  frame=self.until_frame(lambda f:f['generation']>=32,45)
  paths=list(self.session.rglob('palette-observation-0001.bin'));require(len(paths)==1,'Missing actual World palette source')
  palette=palettes(paths[0].read_bytes(),complete=False);require(palette['version']==3,'Direct in-place refresh observation required')
  refreshed=[dict(node=n['id'],mode=n['mode'],revision=r['revision'],entry=r['entry_record'],returned=r['return_record'],queue=r['queue'])for n in palette['nodes'].values()for r in n['refreshes']if r['result']and r['before']!=r['after']]
  require({v['mode']for v in refreshed}>={1,9},'Original World did not refresh both WORD/DWORD blue nodes')
  # Retained source puts changed blue terrain below/left of the initial viewport.
  # Move through ordinary forwarded arrow keys, never game-memory writes.
  self.game_key('palette-camera-left',0xff51,1.0)
  self.game_key('palette-camera-down',0xff54,1.0)
  moved=self.until_frame(lambda f:f['generation']>=frame['generation']+8,10)
  later=self.until_frame(lambda f:f['generation']>=moved['generation']+32,10)
  self.proof.update(refreshed_nodes=refreshed,first_generation=frame['generation'],camera_after_generation=moved['generation'],after_generation=later['generation'],actual_extent=list(later['extent']),success=True)
  self.capture('world-after-original-palette-refresh')
