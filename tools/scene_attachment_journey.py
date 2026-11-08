"""Ordinary Quick Battle Excalibur selection and one source-directed self cast.

No process/channel writes, injected input, hit-feedback search or simulation
configuration edits. Original offered recipes and public controls own the case.
"""
import json
import struct
import time

from campaign_route import hud_point
from campaign_input_observation import decode as input_observations
from rendering_interaction_live import LiveSources
from rendering_pilot import require
from scene_actor_observation import decode
from scene_effects import module

menus = module('run-rendering-menu-acceptance')


def spell_inventory(raw):
    require(len(raw) >= 18000+11384, 'Incomplete original Spellbox inventory')
    at = 18000
    def text(offset):
        value = raw[at+offset:at+offset+128]
        require(b'\0' in value, 'Unterminated original inventory name')
        return value.split(b'\0', 1)[0].decode('cp1252')
    counts = struct.unpack_from('<3I', raw, at+16)
    require(all(n <= 7 for n in counts), 'Original talisman bounds')
    return dict(owner=struct.unpack_from('<I', raw, at)[0],
                seconds=struct.unpack_from('<I', raw, at+4)[0],
                offered=struct.unpack_from('<I', raw, at+8)[0],
                counts=list(counts), assignments=list(struct.unpack_from('<63i', raw, at+28)),
                recipes=list(struct.unpack_from('<63I', raw, at+380)),
                ingredients=[text(632+128*i) for i in range(21)],
                spells=[text(3320+128*i) for i in range(63)])


class AttachmentJourney(menus.Journey):
    def __init__(self, child, out, env, size):
        super().__init__(child, out, env)
        self.size = tuple(size)
        self.proof = dict(policy='public-excalibur-self-cast-v1', success=False,
                          simulation_writes=False, cast_requests=0)

    def node(self, object_name):
        found = [(n, v) for n, v in self.access.nodes()
                 if v.get('accessible_id', '').endswith('.'+object_name)]
        require(len(found) == 1, 'Public control missing/ambiguous: '+object_name)
        return found[0]

    def press(self, object_name):
        _, value = self.node(object_name)
        require(value['enabled'], 'Disabled public control: '+object_name)
        self.actions.append(dict(action='click-control', object_name=object_name,
                                 host_ns=time.monotonic_ns(),
                                 physical=self.input.click(value['rect'])))
        time.sleep(.12)

    def frame_click(self, point, button=1):
        _, viewport = self.node('nativeGameViewport')
        x, y, w, h = viewport['rect']; width, height = self.size
        scale = min(w/width, h/height)
        require(scale >= 1, 'Integer attachment viewport smaller than source')
        scale = int(scale); iw, ih = width*scale, height*scale
        ox, oy = x+(w-iw)//2, y+h-ih-(h-ih)//2
        require(0 <= point[0] < width and 0 <= point[1] < height,
                'Attachment target outside actual World')
        rect=[ox+point[0]*scale, oy+point[1]*scale, scale, scale]
        require(button in(1,3), 'Unsupported attachment physical button')
        require(self.input.xt.XTestFakeMotionEvent(self.input.display,-1,rect[0]+scale//2,rect[1]+scale//2,0),
                'Attachment pointer motion refused')
        self.input.x.XFlush(self.input.display);time.sleep(.3)
        try:
            require(self.input.xt.XTestFakeButtonEvent(self.input.display,button,1,0), 'Attachment button press refused')
            self.input.x.XFlush(self.input.display);time.sleep(.2)
        finally:
            self.input.xt.XTestFakeButtonEvent(self.input.display,button,0,0)
            self.input.x.XFlush(self.input.display)
        physical=dict(rect=rect,button=button,held_ms=200,down_up=True,host_focus=self.input.focus())
        self.actions.append(dict(action='world-click', source_point=list(point),button=button,
                                 source_extent=list(self.size), viewport=viewport,
                                 host_ns=time.monotonic_ns(), physical=physical))
        time.sleep(.2)

    def latest(self):
        require(self.session is not None, 'No owned original session')
        paths = list(self.session.rglob('scene-actors-0001.bin'))
        require(len(paths) <= 1, 'Ambiguous live actor stream')
        if not paths: return None
        poses = decode(paths[0].read_bytes(), complete=False)
        require(poses['version'] == 2, 'Attachment requires corrected actor producer')
        return poses['frames'][-1] if poses['frames'] else None

    def until_frame(self, predicate, seconds):
        deadline = time.monotonic()+seconds
        while time.monotonic() < deadline:
            require(self.child.poll() is None, 'Native host exited during attachment case')
            frame = self.latest()
            if frame:
                require(tuple(frame['extent']) == self.size, 'Actual attachment resolution differs')
                if predicate(frame): return frame
            time.sleep(.1)
        raise ValueError('Bounded attachment observation deadline')

    def game_key(self,label,symbol,hold=.2):
        key=self.input.x.XKeysymToKeycode(self.input.display,symbol)
        require(key,'Physical game key missing')
        try:
            require(self.input.xt.XTestFakeKeyEvent(self.input.display,key,1,0),'Physical game key press refused')
            self.input.x.XFlush(self.input.display);time.sleep(hold)
        finally:
            self.input.xt.XTestFakeKeyEvent(self.input.display,key,0,0)
            self.input.x.XFlush(self.input.display)
        self.actions.append(dict(action=label,symbol=symbol,held_ms=round(hold*1000),down_up=True,
                                 host_ns=time.monotonic_ns(),host_focus=self.input.focus()))

    def ready_spellbox_inventory(self):
        require(self.last and self.last['screen'] == 7 and self.last['ready'],
                'A saved ready Spellbox visit is required')
        captured = menus.state(menus.ROOT/self.last['wire_path'])
        require(captured and all(captured[0][key] == self.last[key] for key in
                ('sequence', 'generation', 'screen', 'ready', 'ack', 'status', 'thread', 'handoff')),
                'Saved Spellbox wire differs from the ready visit')
        return spell_inventory(captured[1])

    def select_spell(self):
        initial = self.ready_spellbox_inventory(); self.proof['inventory'] = initial
        offered = [i for i in range(21) if initial['offered'] & (1 << i)]
        candidates = [(i, a) for i in offered for a in range(3)
                      if initial['counts'][a] and initial['recipes'][i*3+a] == 83]
        require(len(candidates) == 1, 'Original offered inventory lacks unique effect83 recipe')
        ingredient, alignment = candidates[0]
        self.proof['recipe'] = dict(ingredient=ingredient, alignment=alignment, effect=83,
                                    name=initial['spells'][ingredient*3+alignment])
        slots = [(a, n) for a in range(3) for n in range(initial['counts'][a])]
        # Every change is a physical interaction with the native draft. Lifting
        # an occupied talisman and dropping on the shelf clears that assignment.
        for visible, (a, n) in enumerate(slots):
            if initial['assignments'][a*21+n] >= 0:
                self.press('spellboxTalisman'+str(visible))
                self.press('spellboxItem0')
        for visible in range(len(slots)):
            require(self.node('spellboxTalisman'+str(visible))[1]['name'].endswith(' — empty'),
                    'Original default draft did not clear through public controls')
        target = next(i for i, (a, n) in enumerate(slots) if a == alignment)
        self.press('spellboxItem'+str(offered.index(ingredient)))
        self.press('spellboxTalisman'+str(target))
        labels = [self.node('spellboxTalisman'+str(i))[1]['name'] for i in range(len(slots))]
        require(initial['spells'][ingredient*3+alignment] in labels[target]
                and all(i == target or label.endswith(' — empty') for i, label in enumerate(labels)),
                'Public single-spell draft differs')
        self.proof['draft_labels'] = labels
        self.capture('single-excalibur-draft')
        self.press('spellboxOK')

    def cast_and_observe(self):
        frame = self.until_frame(lambda f: f['generation'] >= 20 and any(
            a[0] == 0 and a[1:3] == (0, 0) and a[4] and a[12] & 1
            for a in f['actors']), 45)
        self.proof['before_generation'] = frame['generation']
        self.key('existing-fullscreen-control',0xffc6)  # F9, existing public action.
        time.sleep(.6)
        self.capture('world-before-attachment')
        # Source-directed targeting uses only one known actor body and one
        # deterministic opaque source pixel, never a feedback-guided search.
        for unused in range(2): self.frame_click(hud_point((744, 522), self.size))
        time.sleep(.5)
        def bar():
            raw=(self.session/'input-admission.bin').read_bytes()
            require(raw[:16]==b'MNMCAI01'+struct.pack('<II',3,144), 'Unexpected passive spell-bar observation')
            rows=input_observations(raw[:16+(len(raw)-16)//144*144])['rows']
            require(rows and rows[-1]['flags']&1, 'No original spell-bar state')
            return rows[-1]
        before=bar();self.proof['spell_bar_before']=before
        require(before['spell_count']==1 and before['receiver_slot_plus_one']==1,
                'Original single-spell player receiver differs')
        # The bar toggles an already selected spell. Read its original state
        # instead of blindly clicking the sole icon and toggling it off.
        if before['selected_slot']!=0 or before['mode']!=2:
            # The retained single-spell Quick HUD places its sole offered icon
            # at575,575. The campaign's477,575 belongs to a different bar layout.
            self.frame_click(hud_point((575, 575), self.size))
        deadline=time.monotonic()+3
        while time.monotonic()<deadline:
            selected=bar()
            if selected['spell_count']==1 and selected['selected_slot']==0 and selected['mode']==2:break
            time.sleep(.1)
        require(selected['spell_count']==1 and selected['selected_slot']==0 and selected['mode']==2,
                'Original sole spell did not become selected')
        self.proof['spell_bar_selected']=selected
        frame=self.until_frame(lambda f:len([d for d in f['draws']if d[0:2]==(0,0)and d[7]in(0,31)])==1,15)
        bodies = [d for d in frame['draws'] if d[0:2] == (0, 0) and d[7] in (0, 31)]
        require(len(bodies) == 1, 'Player body source is ambiguous before single cast')
        with_source = LiveSources(self.out/'worlds.bin')
        try: target = with_source.target(frame['generation'], list(bodies[0][5:7]))
        finally: with_source.close()
        require(0 <= target['point'][0] < self.size[0]-200 and
                0 <= target['point'][1] < self.size[1]-50, 'Player body is covered by native HUD')
        self.proof['target'] = target
        self.proof['cast_requests'] += 1
        self.frame_click(target['point'],3)
        def active(f):
            return any(a[0] == 0 and a[14] == 1 and a[184+1] and a[12] & 2 for a in f['actors']) and any(
                d[0:2] == (0, 1) for d in f['draws'])
        first = self.until_frame(active, 10)
        views = []
        for ordinal in range(4):
            current = self.until_frame(active, 3)
            views.append(dict(generation=current['generation'], view=current['view']))
            self.game_key('attachment-view-'+str(ordinal+1), ord('.'))
            time.sleep(1)
        require({v['view'] for v in views} == set(range(4)), 'Actual attachment four-view state incomplete')
        self.proof.update(first_attachment_generation=first['generation'], views=views,
                          actual_extent=list(first['extent']), success=True)
        self.capture('world-active-attachment')

    def prepare_ui(self):
        self.input.activate()
        _,control=self.node('viewportScaling')
        require(control['enabled'], 'Public scaling control disabled')
        self.input.click(control['rect']);self.key('existing-integer-scaling',0xff57)
        self.key('confirm-existing-scaling',0xff0d)
        deadline=time.monotonic()+3
        while time.monotonic()<deadline:
            selected=self.node('viewportScaling')[1]
            if selected['name']=='Integer (sharp)':break
            time.sleep(.1)
        require(selected['name']=='Integer (sharp)', 'Public Integer scaling selection failed before original launch')
        self.proof['public_scaling_before_launch']=selected

    def run(self):
        self.click('Launch game')
        self.wait(3, 0, timeout=420)
        self.press('mainMenuAction2'); self.wait(22, 1)
        self.press('quickBattleAction2'); initial = self.wait(14, 2)
        changed = []
        for index in range(7):
            n, value = self.node('singlePlayerSlider'+str(index+1))
            self.input.click(value['rect']); self.key('max-original-setup-'+str(index), 0xff57)
            changed.append(dict(index=index, before=value['value']['current'],
                                after=n.get_value_iface().get_current_value()))
        self.proof['setup_changes'] = changed
        self.press('singlePlayerMap'); maps = self.wait(25, 3)
        self.proof['original_setup'] = maps['setup']
        lists = [(n, v) for n, v in self.access.nodes() if v['role'] == 'list' and n.get_child_count() > 1]
        require(len(lists) == 1, 'Original map list is ambiguous')
        self.input.click(lists[0][1]['rect']); self.key('first-original-map', 0xff50)
        self.press('mapSelectionOk'); selected = self.wait(14, 4)
        self.proof['selected_map'] = selected['setup']
        self.press('singlePlayerStart'); self.wait(7, 5)
        self.select_spell()
        try: self.cast_and_observe()
        except Exception as error: self.proof['observation_error'] = str(error)
        # Always attempt the ordinary Results -> Quick -> Main -> Quit route;
        # failed attachment admission never earns a positive placement claim.
        self.game_key('ordinary-world-escape', 0xff1b,.6); self.wait(26, 6, timeout=30)
        self.press('quickResultAction2'); self.wait(22, 7)
        self.press('quickBattleAction3'); self.wait(3, 8)
        self.press('mainMenuAction4')
        return self.proof
