'use strict';
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const {test} = require('node:test');
const vm = require('node:vm');
const root = path.resolve(__dirname, '..');
const source = fs.readFileSync(path.join(root,'apps/coverage-ui/app.js'),'utf8').split("document.addEventListener('click'")[0];

test('required unknown scope displays its case and exact closure without calling it resolved',()=>{
  const context = vm.createContext({});vm.runInContext(source,context);
  vm.runInContext("behaviorById=new Map([['TEST.supplier',{id:'TEST.supplier',title:'Owned storage'}]])",context);
  context.work={scope_resolution_record:'research/runtime/review.json',scope_domains:[{
    name:'Physical loss',disposition:'unresolved',finding:'Recovery remains open',validation:'No fresh loss run',
    required_case:'Lose the context with a pending World',closure_condition:'Original/native results and ownership agree',
    suppliers:['TEST.supplier'],paths:['runtime/native-draw/surfaces.h']}]};
  const html=vm.runInContext('trancheScopeDomains(work)',context);
  assert.match(html,/Required scope unresolved/);assert.match(html,/Required case:/);assert.match(html,/Closes when:/);
  assert.match(html,/Lose the context with a pending World/);assert.doesNotMatch(html,/Resolved scope:/);
});

test('profile passes remain distinct from complete renderer acceptance and legacy diagnostics',()=>{
  const context = vm.createContext({});vm.runInContext(source,context);
  context.work={acceptance_profile_record:'research/runtime/accepted.json',acceptance_profiles:[{
    backend:'hardware',name:'corrected_storage',outcome:'PASS',frames:222,mismatches:0}]};
  const html=vm.runInContext('trancheAcceptanceProfiles(work)',context);
  assert.match(html,/hardware.*corrected_storage.*PASS/);assert.match(html,/222 complete frames/);
  assert.match(html,/separate diagnostic regression/);assert.match(html,/do not establish full renderer/);
});
