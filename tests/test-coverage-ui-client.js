'use strict';

// Run with: node --test tests/test-coverage-ui-client.js
// Exercise the actual client with the raw plan an older server passes through.
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {test} = require('node:test');
const vm = require('node:vm');
const root = path.resolve(__dirname, '..');
const read = name => fs.readFileSync(path.join(root, name), 'utf8');
const source = read('apps/coverage-ui/app.js').split("document.addEventListener('click'")[0];
const catalog = JSON.parse(read('apps/coverage-ui/importance.json'));

function client(key, unplanned) {
  const importance = structuredClone(catalog);
  // Empty ordinary lists keep this a client compatibility fixture, independent
  // of changing engine claims and the full audit's execution requirements.
  importance.entries = [];
  importance.labels = {};
  importance.unrated = [];
  importance.tranche_plan.tranches = [];
  importance.tranche_plan.unplanned_critical = [];
  if(key !== 'low_tranche_plan' && importance.low_tranche_plan){
    importance.low_tranche_plan.tranches=[];importance.low_tranche_plan.unplanned_low=[];
  }
  if (unplanned !== undefined) {
    importance[key].tranches = [];
    importance[key][key === 'low_tranche_plan' ? 'unplanned_low' : key === 'high_tranche_plan' ? 'unplanned_high' : 'unplanned_critical'] = unplanned;
  } else if (key === 'tranche_plan') {
    delete importance.tranche_plan.unplanned_critical;
  }
  const track = {id:'rendering', behaviors:[], milestones:[], progress:{count:0,total:0},
    focus:{implementation_behaviors:[], next_behaviors:[], documents:[]}};
  const context = vm.createContext({payload:{importance, tracks:[track], behaviors:[]}});
  vm.runInContext(source, context);
  vm.runInContext('data=payload', context);
  return {context, render:() => vm.runInContext('renderingView()', context)};
}

test('older server raw High plan leaves the native rendering page usable', () => {
  const {render} = client('high_tranche_plan');
  const html = render();
  assert.match(html, /data-plan-refresh="high"/);
  assert.match(html, /Restart the dashboard server/);
  assert.match(html, /Refresh audit/);
  assert.match(html, /data-progress-section="implementation"/);
  assert.match(html, /data-progress-section="validation"/);
  assert.match(html, /Rendering milestones/);
  assert.match(html, /Research &amp; remaining work|Research & remaining work/);
  assert.doesNotMatch(html, /data-high-tranche-behavior=/);
});

test('missing Critical review metadata is also isolated to its plan', () => {
  const html = client('tranche_plan').render();
  assert.match(html, /data-plan-refresh="critical"/);
  assert.match(html, /data-progress-section="validation"/);
});

test('refreshed metadata restores the complete plan in the same client', () => {
  const {context, render} = client('high_tranche_plan');
  assert.match(render(), /data-plan-refresh="high"/);
  vm.runInContext('data.importance.high_tranche_plan.tranches=[];data.importance.high_tranche_plan.unplanned_high=[]', context);
  const html = render();
  assert.doesNotMatch(html, /data-plan-refresh=/);
  assert.match(html, /data-high-plan/);
  assert.match(html, /data-critical-plan/);
});

test('valid plans retain the count of newly unassigned behaviors', () => {
  for (const key of ['tranche_plan', 'high_tranche_plan', 'low_tranche_plan']) {
    const html = client(key, ['new-one', 'new-two']).render();
    assert.match(html, new RegExp('2 new '+(key === 'tranche_plan' ? 'Critical' : key === 'low_tranche_plan' ? 'Low' : 'High')+' unfinished behaviors'));
  }
});

test('older server raw Low plan preserves the rendering page and names its refresh', () => {
  const html=client('low_tranche_plan').render();
  assert.match(html,/data-plan-refresh="low"/);
  assert.match(html,/Low work: first 20/);
  assert.match(html,/data-progress-section="validation"/);
});

function spellClient() {
  const definition=JSON.parse(read('apps/coverage-ui/tracks.json'));
  const register=JSON.parse(read('research/runtime/coverage/register.json'));
  const spells=structuredClone(definition.spell_progress);
  spells.catalog_behaviors=register.behaviors.filter(b=>b.subsystem==='spell_dispatch').map(b=>b.id);
  spells.named_behaviors=spells.catalog_behaviors.filter(id=>!spells.unused_behaviors.includes(id));
  const ids=new Set([...spells.catalog_behaviors,...spells.groups.flatMap(g=>g.behaviors)]);
  const behaviors=register.behaviors.filter(b=>ids.has(b.id));
  const track={id:'simulation',title:'Engine simulation',behaviors:[...ids],milestones:[],progress:{count:0,total:0},
    current:0,focus:{boundary:'Complete original world scheduling remains open.',documents:[]}};
  const context=vm.createContext({payload:{spell_progress:spells,behaviors,tracks:[track]}});
  vm.runInContext(source,context);
  vm.runInContext('data=payload;behaviorById=new Map(data.behaviors.map(b=>[b.id,b]))',context);
  return {context,render:()=>vm.runInContext('spellProgressView()',context)};
}

test('spell progress credits component work while preserving whole-spell gaps', () => {
  const {render}=spellClient();
  const html=render();
  assert.match(html,/data-spell-group="foundations"><summary>Casting foundations: 4 \/ 4 implemented/);
  assert.match(html,/data-spell-group="tasks"><summary>Wizard spell task execution: 3 \/ 3 implemented/);
  assert.match(html,/data-spell-group="effects"><summary>Effect &amp; delivery components: 6 \/ 7 implemented · 0 partial · 1 not implemented/);
  assert.match(html,/data-spell-group="integration"><summary>Native spell integration: 5 \/ 5 implemented/);
  assert.match(html,/data-spell="SP\.073"[^]*?data-stage="implementation">Registered partial/);
  assert.match(html,/data-spell-catalog="named"><summary>Named spell catalog IDs \(75\): 0 \/ 75 implemented · 9 partial · 66 not implemented/);
  assert.match(html,/data-spell-catalog="unused"><summary>Unused-named catalog slots \(29\)/);
  assert.match(html,/Reachability is unresolved/);
  assert.match(html,/No original comparison/);
  assert.match(html,/No live replacement/);
  assert.match(html,/Shared components may appear under several spells; they count once/);
});

test('simulation shows the spell panel and older snapshots give a refresh instruction', () => {
  const {context}=spellClient();
  assert.match(vm.runInContext("renderingView('simulation')",context),/data-spell-progress/);
  vm.runInContext('delete data.spell_progress',context);
  assert.match(vm.runInContext('spellProgressView()',context),/Restart the dashboard server/);
  assert.doesNotMatch(vm.runInContext('spellProgressView()',context),/data-spell="/);
});


test('artifact panel asks older servers to refresh without breaking rendering', () => {
  const html=client('high_tranche_plan', []).render();
  assert.match(html,/Native rendering: artifacts 2, 3 & 4/);
  assert.match(html,/load artifact progress/);
  assert.match(html,/data-progress-section="implementation"/);
});

test('artifact details retain history, failures, task exits and escaped notes', () => {
  const {context,render}=client('high_tranche_plan', []);
  context.artifacts={closure:'docs/native-rendering-closure.json',documents:{contract:'contract.md',gaps:'gaps.md',plan:'plan.md',acceptance:'acceptance.md'},
    contract:{id:'contract',revision:1,baseline_commit:'abc123',configurations:[],gates:[],scope_decisions:[],claim_limits:[]},
    gaps:{journeys:[],leaves:47,operations:[],gates:[],briefs:36},
    progress:{state:'historical',record:'proof.json',counts:{passed:8,pending:147,failed:1},total:156,changed_sources:['changed.cpp'],scope:'Finite proof only',decisions:{'supported-player':'pending','original-recovery':'pending'}},
    plan:{order:['EP03','EP04'],optional:['EP12'],optimality:'No measured minimum',tasks:[{id:'EP04',title:'Normal sessions',state:'planned',required_for_completion:true,depends_on:['EP03'],action:'Continue proof',updates:[{date:'2026-11-03',note:'<script>unsafe</script>',document:'notes.md'}],recorded:{total:2,passed:1,pending:0,failed:1},deliverables:['Bound receipt'],exit_condition:'Adequate proof'}]},next_action:'Close remaining scope'};
  vm.runInContext('data.rendering_artifacts=artifacts',context);
  let html=render();
  for(const id of [2,3,4])assert.match(html,new RegExp('data-render-artifact="'+id+'"'));
  assert.match(html,/8 \/ 156 recorded passes; 147 pending; 1 failed/);
  assert.match(html,/Historical source phase/);
  assert.match(html,/not current-code acceptance/);
  assert.match(html,/Frozen plan state: planned/);
  assert.match(html,/Closes when:/);
  assert.match(html,/&lt;script&gt;unsafe&lt;\/script&gt;/);
  assert.doesNotMatch(html,/<script>unsafe/);
  vm.runInContext("data.rendering_artifacts.progress={state:'unavailable',error:'Missing payload'}",context);
  html=render();
  assert.match(html,/Acceptance snapshot unavailable/);
  assert.doesNotMatch(html,/8 \/ 156/);
  assert.match(html,/data-artifact-task="EP04"/);
});
