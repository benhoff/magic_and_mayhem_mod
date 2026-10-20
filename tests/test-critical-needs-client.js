'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {test} = require('node:test');
const vm = require('node:vm');
const source = fs.readFileSync(path.join(__dirname,'../apps/coverage-ui/app.js'),'utf8').split("document.addEventListener('click'")[0];

test('Critical needs show actionable work and closure independently of registered partial and evidence',()=>{
  const context=vm.createContext({});vm.runInContext(source,context);
  vm.runInContext(`data={critical_needs:{total:1,labels:{validation:'Validate supplied code',review:'Assessment needs review'},counts:{validation:1,review:0},record:'apps/coverage-ui/critical-needs.json',entries:[{behavior:'TEST.one',primary_need:'validation',assessment_state:'current',reason:'Supplied <bounded> service',requirements:[{kind:'validation',action:'Compare all declared pixels'}],closure:'Complete matrix passes',changed_sources:[]}]}};behaviorById=new Map([['TEST.one',{id:'TEST.one',title:'Supplied mechanism',status:{implementation:'partial'},current_validation:{implementation:{state:'pending'}}}]]);`,context);
  const html=vm.runInContext("criticalNeedsView({id:'rendering'})",context);
  assert.match(html,/data-critical-need-behavior="TEST.one"/);
  assert.match(html,/Supplied &lt;bounded&gt; service/);
  assert.match(html,/Compare all declared pixels/);
  assert.match(html,/Closes when:.*Complete matrix passes/);
  assert.match(html,/Registered implementation: Registered partial/);
  assert.match(html,/implementation: pending/);
  assert.match(html,/Each Critical partial contract counts once/);
  vm.runInContext("data.critical_needs.entries[0].primary_need='review';data.critical_needs.entries[0].assessment_state='needs_review';data.critical_needs.entries[0].changed_sources=['impl.cpp']",context);
  const changed=vm.runInContext("criticalNeedsView({id:'rendering'})",context);
  assert.match(changed,/Changed inputs: impl.cpp/);
  assert.equal(vm.runInContext("criticalNeedsView({id:'simulation'})",context),'');
});

test('older server missing breakdown keeps the page usable and asks for refreshed producer',()=>{
  const context=vm.createContext({});vm.runInContext(source,context);
  vm.runInContext('data={}',context);
  assert.match(vm.runInContext("criticalNeedsView({id:'rendering'})",context),/Restart the dashboard server and select Refresh audit/);
});

test('completed scope decision exposes supplied boundary and wider owner without acceptance promotion',()=>{
  const context=vm.createContext({});vm.runInContext(source,context);
  vm.runInContext("behaviorById=new Map([['TEST.owner',{id:'TEST.owner',title:'Ownership recovery'}]])",context);
  context.entry={assessment_state:'current',scope_resolution:{id:'TEST.scope',state:'reconciled',
    decision:'Keep the selected supplier',bounded_scope:'Owned <finite> source replay',
    supplied_paths:['renderer/canvas_sequence.cpp'],record:'research/runtime/review.md',
    execution_evidence:['TEST.bounded'],remaining:[{kind:'recovery',required_case:'Observe real aliases',
      closes_when:'Lease and writer identities agree',behaviors:['TEST.owner']}]}};
  const html=vm.runInContext("criticalScopeResolution(entry,{recovery:'Original recovery'})",context);
  assert.match(html,/Scope reconciled/);assert.match(html,/Owned &lt;finite&gt; source replay/);
  assert.match(html,/Observe real aliases/);assert.match(html,/Lease and writer identities agree/);
  assert.match(html,/Owned by:.*Ownership recovery/);assert.match(html,/TEST.bounded/);
  assert.match(html,/Wider cases retain their independent stages/);
  context.entry.assessment_state='needs_review';
  assert.match(vm.runInContext("criticalScopeResolution(entry,{recovery:'Recovery'})",context),/Historical scope reconciliation/);
});

test('completed native scope is visible without inflating partial-contract counts',()=>{
  const context=vm.createContext({});vm.runInContext(source,context);
  vm.runInContext(`behaviorById=new Map([['TEST.native',{id:'TEST.native',title:'Finite native supplier',status:{implementation:'scoped'}}]]);data={critical_needs:{total:0,labels:{acceptance:'Session acceptance'},counts:{acceptance:0},entries:[],record:'apps/coverage-ui/critical-needs.json',scope_decisions:[{behavior:'TEST.native',implementation:'scoped',assessment_state:'current',scope_resolution:{id:'TEST.resolved',state:'reconciled',decision:'Retain finite policy',bounded_scope:'Owned retained source history',record:'research/runtime/review.md',supplied_paths:['renderer/canvas_sequence.cpp'],execution_evidence:['TEST.native-proof'],remaining:[{kind:'acceptance',required_case:'Run the supported session',closes_when:'Independent complete session passes',behaviors:['TEST.native']}]}}]}}`,context);
  const html=vm.runInContext("criticalNeedsView({id:'rendering'})",context);
  assert.match(html,/0 contracts, grouped/);assert.match(html,/1 scope decisions reconciled/);
  assert.match(html,/Implemented bounded contracts \(1\)/);
  assert.match(html,/Finite native supplier/);
  assert.doesNotMatch(html,/data-critical-need-behavior=/);
});
