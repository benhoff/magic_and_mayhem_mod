'use strict';

const $ = selector => document.querySelector(selector);
const escapeHTML = value => String(value ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;'}[c]));
const number = value => Number(value || 0).toLocaleString();
const percent = (count, total) => total ? `${(100 * count / total).toFixed(1)}%` : 'N/A';
const hex = value => `0x${Number(value).toString(16).padStart(8, '0')}`;
const titleCase = value => String(value).replaceAll('_', ' ').replace(/\b\w/g, c => c.toUpperCase());
const labels = {engine:'Engine overview', simulation:'Simulation roadmap', rendering:'Native rendering', tracks:'Major tracks', overview:'Detailed coverage', work:'Work remaining', behaviors:'Behavior register', binary:'Binary explorer'};
const subsystemNames = {terrain_lighting:'Terrain & effects', audio_reconstruction:'Original audio', native_audio:'Native audio', rendering_reconstruction:'Original rendering', native_rendering:'Native rendering', qt_menus:'Menus', qt_services:'Menu services', menu_adapters:'Menu adapters', runtime_and_hosting:'Runtime & hosting', gameplay_gaps:'Core gameplay', intentional_gameplay_changes:'Gameplay changes', spell_dispatch:'Spell dispatch', remaining_engine_boundaries:'Engine boundaries', native_world:'Native world'};
const subsystemName = id => subsystemNames[id] || titleCase(id);
const gapLabels = {unimplemented:'Not implemented', partial:'Registered partial', understanding:'Understanding incomplete', comparison:'No original comparison', stale:'Evidence needs rerun', tests:'Tests missing', integration:'No integration', replacement:'No live replacement', validation:'Current validation pending'};
const gapActions = {unimplemented:'Implement the documented behavior and add bounded tests.', partial:'Review the declared scope and identify concrete missing code. Separate implemented mechanisms, unresolved aggregates, validation work and acceptance failures before planning a fix.', understanding:'Recover the remaining behavior within scope and record evidence for unresolved paths.', comparison:'Run an independent original comparison for the recovered scope; retain controlled-input limits.', stale:'Rerun the affected comparison or integration against current sources under a new evidence ID.', tests:'Add meaningful tests for the implemented scope.', integration:'Exercise the behavior in a bounded integrated scenario and record its evidence.', replacement:'Establish scoped live equivalence and verify that original work is bypassed before claiming replacement.', validation:'Review the exact scope and dependencies, then register a new execution result bound to the behavior and exercised scenarios.'};
let data;
let state = {view:'engine', kind:'recovered', track:'all', subsystem:'all', gap:'all', query:'', page:0, mapped:'all'};
let behaviorById = new Map(), functionById = new Map(), evidenceById = new Map();
let importanceStatus='partial', importanceType='all', importanceLevel='all';

function restoreState() {
  const params = new URLSearchParams(location.hash.slice(1));
  for (const key of Object.keys(state)) if (params.has(key)) state[key] = params.get(key);
  if (!labels[state.view]) state.view = 'engine';
  if (!['all', 'recovered', 'native_policy'].includes(state.kind)) state.kind = 'recovered';
  if (!(state.gap in gapLabels) && state.gap !== 'all') state.gap = 'all';
  if (!['all','mapped','unmapped','classified'].includes(state.mapped)) state.mapped = 'all';
  if (data && state.subsystem !== 'all' && !data.behaviors.some(b => b.subsystem === state.subsystem)) state.subsystem = 'all';
  if (data && state.track !== 'all' && !data.tracks.some(t => t.id === state.track)) state.track = 'all';
  state.page = Math.max(0, Number.parseInt(state.page, 10) || 0);
}
function persist() {
  const params = new URLSearchParams();
  for (const [key, value] of Object.entries(state)) if (value !== '' && value !== 0 && !(value === 'all' && key !== 'kind')) params.set(key, value);
  history.replaceState(null, '', `#${params}`);
}
function go(changes) {state = {...state, page:0, ...changes}; persist(); render();}
function scopedBehaviors() {const track=data.tracks.find(t=>t.id===state.track); return data.behaviors.filter(b => (!track || track.behaviors.includes(b.id)) && (state.kind === 'all' || b.kind === state.kind) && (state.subsystem === 'all' || b.subsystem === state.subsystem));}
function matches(b) {
  const search = state.query.trim().toLowerCase();
  return !search || [b.id, b.title, b.scope, b.subsystem, ...b.original.map(r => hex(r.address)), ...b.implementation].join(' ').toLowerCase().includes(search);
}
function badge(value, label=titleCase(value)) {return `<span class="badge ${escapeHTML(value)}">${escapeHTML(label)}</span>`;}
const stageNames = {understanding:'Understanding', implementation:'Implementation', comparison:'Original comparison', integration:'Integration', replacement:'Live replacement'};
const statusLabels = {
  implementation:{scoped:'Implemented', partial:'Registered partial', none:'Not implemented', unknown:'Implementation unknown'},
  understanding:{scoped:'Understood within scope', partial:'Partially understood', unknown:'Understanding unknown'},
  comparison:{recorded:'Original comparison recorded', none:'No original comparison'},
  integration:{none:'No integration', headless:'Headless integration', preview:'Preview integration', live_observation:'Live observation', live_equivalence:'Live equivalence'},
  replacement:{none:'No live replacement', scoped_live:'Live replacement within scope'}
};
const statusLabel = (stage, value) => statusLabels[stage]?.[value] || titleCase(value);
const statusBadge = (stage, value) => `<span class="badge ${escapeHTML(value)}" data-stage="${escapeHTML(stage)}">${escapeHTML(statusLabel(stage,value))}</span>`;
function milestoneImplementation(m) {
  const values=m.behaviors.map(id=>behaviorById.get(id)?.status.implementation);
  if(m.missing.length || !values.length || values.some(v=>!['scoped','partial','none'].includes(v)))return 'unknown';
  return values.every(v=>v==='scoped')?'scoped':values.some(v=>v!=='none')?'partial':'none';
}
function implementationSummary(ids) {return summarize(ids.map(id=>behaviorById.get(id)).filter(Boolean));}
function implementationCounts(s) {
  return `<div class="track-summary"><div><strong>${s.scoped} / ${s.total}</strong><span>Behaviors implemented</span></div><div><strong>${s.partial}</strong><span>Registered partial</span></div><div><strong>${s.missing}</strong><span>Not implemented</span></div><div><strong>${percent(s.scoped,s.total)}</strong><span>Behaviors implemented</span></div></div>`;
}
function milestoneStatus(m) {
  return `<div class="badge-row milestone-status">${statusBadge('implementation',milestoneImplementation(m))} ${badge(m.state,milestoneLabels[m.state])}</div>`;
}
function fileLink(path) {return `<a target="_blank" rel="noopener" href="/files?path=${encodeURIComponent(path)}">${escapeHTML(path)}</a>`;}
function featureLink(b) {return `<button class="feature-button" data-behavior="${escapeHTML(b.id)}">${escapeHTML(b.title)}</button>`;}
function stack(rows, large=false) {
  const total = rows.length || 1;
  return `<div class="stack ${large ? 'implementation-stack' : ''}" aria-label="${rows.filter(b => b.status.implementation==='scoped').length} implemented, ${rows.filter(b => b.status.implementation==='partial').length} registered partial, ${rows.filter(b => b.status.implementation==='none').length} not implemented">${['scoped','partial','none'].map(s => `<span class="${s}" style="width:${100 * rows.filter(b=>b.status.implementation===s).length/total}%"></span>`).join('')}</div>`;
}
function legend() {return '<div class="legend"><span><i></i>Implemented</span><span><i class="partial"></i>Registered partial</span><span><i class="none"></i>Not implemented</span></div>';}
function panel(title, subtitle, contents, action='') {return `<section class="panel"><div class="panel-head"><div><h2>${title}</h2>${subtitle ? `<p>${subtitle}</p>` : ''}</div>${action}</div>${contents}</section>`;}
function metric(label, count, total, note, action, extra='') {
  return `<button class="metric" ${action}><div class="metric-label">${label}</div><div class="metric-value">${total ? (100*count/total).toFixed(1) : 'N/A'}${total ? '<small>%</small>' : ''}</div><div class="metric-count">${number(count)} / ${number(total)} ${extra}</div><div class="metric-note">${note}</div></button>`;
}
function summarize(rows) {
  const count = (key, value) => rows.filter(b=>b.status[key]===value).length;
  const recovered = rows.filter(b=>b.kind==='recovered');
  return {total:rows.length, scoped:count('implementation','scoped'), partial:count('implementation','partial'), missing:count('implementation','none'), understood:count('understanding','scoped'), integrated:rows.filter(b=>b.status.integration!=='none').length, recovered:recovered.length, compared:recovered.filter(b=>b.status.comparison==='recorded').length, current:recovered.filter(b=>b.comparison_freshness==='current').length, replaced:recovered.filter(b=>b.status.replacement==='scoped_live').length};
}
function render() {
  if (!data) return;
  $('#kind').value = state.kind;
  $('#subsystem').value = state.subsystem;
  $('#track-filter').value = state.track;
  $('#detail-filters').hidden = ['engine','simulation','tracks','rendering'].includes(state.view);
  $('.scope-control').hidden = ['engine','simulation','tracks','rendering'].includes(state.view);
  $('nav').querySelectorAll('button').forEach(b => {b.classList.toggle('active', b.dataset.view===state.view); if(b.dataset.view===state.view)b.setAttribute('aria-current','page');else b.removeAttribute('aria-current');});
  $('#crumb').textContent = labels[state.view];
  const headings = {engine:['Engine overview.', 'Track native capabilities, original comparisons, integration and the remaining engine boundaries.'], simulation:['Simulation, from rules to a playable world.', 'Follow owned state, creature decisions, combat, spells and campaign continuation through bounded milestones.'], rendering:['Native rendering, in focus.', 'Follow native World rendering from independent comparison and shadow presentation to verified live replacement.'], tracks:['The rebuild, track by track.', 'See what is implemented, registered partial and not implemented across engine tracks.'], overview:['Detailed coverage.', 'Inspect binary links, registered implementation stages, and independent evidence.'], work:['What remains to be done.', 'Explore missing features, partial implementations, and validation boundaries.'], behaviors:['Every behavior has a boundary.', 'Trace documented scope through native code, tests, and original evidence.'], binary:['Start with the original engine.', 'Browse discovered functions and follow their explicit behavior links.']};
  $('#page-title').textContent = headings[state.view][0];
  $('#page-description').textContent = headings[state.view][1];
  $('#view-content').innerHTML = ({engine:engineView, simulation:()=>renderingView('simulation'), rendering:renderingView, tracks:tracksView, overview:overview, work:workView, behaviors:behaviorView, binary:binaryView}[state.view])();
  const errors = data.audit.errors || [];
  $('#notice').innerHTML = errors.length ? `<div class="notice error"><strong>Audit errors: displayed claims need review.</strong> ${errors.map(escapeHTML).join(' · ')}</div>` : '';
  if (!['binary','engine','simulation','tracks','rendering'].includes(state.view)) $('#notice').innerHTML += `<div class="notice"><strong>${escapeHTML(state.kind==='recovered'?'Recovered baseline':state.kind==='native_policy'?'Intentional native policies & tooling':'All registered behaviors')}${state.track !== 'all' ? ` · ${escapeHTML(data.tracks.find(t=>t.id===state.track).title)}` : ''}${state.subsystem !== 'all' ? ` · ${escapeHTML(subsystemName(state.subsystem))}` : ''}.</strong> Each behavior counts once. These percentages do not measure whole-game completion${state.kind === 'native_policy' ? '; original comparison and replacement are not applicable to native policies' : ''}.</div>`;
}
const milestoneLabels = {demonstrated:'Milestone requirements met',in_progress:'Milestone requirements pending',not_started:'Milestone requirements pending',unknown:'Mapping needs review'};
const validationLabels = {current:'Current validation',historical:'Historical validation',pending:'Validation pending',unknown:'Validation unknown'};
function milestoneValidation(m) {
  if(m.missing.length || !m.checks.length)return 'unknown';
  if(m.checks.every(c=>c.validation_current===true))return 'current';
  return m.checks.every(c=>c.validation_met===true)?'historical':'pending';
}
function importanceBadge(b) {
  const i=b?.importance;
  return i?`<span class="badge importance-${i.weight}" data-importance="${i.weight}">${i.weight}/5 · ${escapeHTML(i.label)}</span>${i.review_state!=='current'?'<span class="badge stale">Importance needs review</span>':''}`:'<span class="small muted">Importance unrated</span>';
}
function importanceOrder(a,b) {return (b.importance?.weight||0)-(a.importance?.weight||0) || a.id.localeCompare(b.id);}
const importanceTypes={capability:'Capability',original_recovery:'Original behavior recovery',aggregate:'Broad aggregate',acceptance:'Acceptance / assurance',diagnostic:'Diagnostic / research'};
function trancheTaskResult(task) {
  const result=task.execution_result;if(!result)return '';
  return `<div class="notice" data-task-result="${escapeHTML(result.outcome)}"><strong>${task.work_review?'Historical bounded task result':'Bounded task result'}: ${escapeHTML(result.outcome)}</strong><p class="small">${escapeHTML(result.note)}</p><p class="small">${fileLink(result.report)} · Behavior implementation and validation remain independent.</p></div>${task.execution_history?.length?`<details data-task-history><summary>Previous bounded results (${task.execution_history.length})</summary>${task.execution_history.map(previous=>`<p data-history-result="${escapeHTML(previous.outcome)}"><strong>${escapeHTML(previous.outcome)}:</strong> ${escapeHTML(previous.note)} ${fileLink(previous.report)}</p>`).join('')}</details>`:''}`;
}
function trancheWorkBadges(task) {
  const work=task?.work_review;
  if(!work)return statusBadge('implementation',task.implementation);
  const labels=data.importance.tranche_plan.work_labels;
  return `<span data-work-implementation="${escapeHTML(work.implementation)}">${badge(work.implementation==='implemented_within_scope'?'scoped':work.implementation==='not_implemented'?'none':'partial',work.label)}</span> ${work.acceptance==='acceptance_failure'?`<span data-work-acceptance="acceptance_failure">${badge('none',labels.acceptance_failure.label)}</span>`:''} ${work.freshness!=='current'?badge('stale','Source assessment needs review'):''}`;
}
function implementationBadge(b) {
  if(b.tranche_task?.work_review)return trancheWorkBadges(b.tranche_task);
  if(b.status.implementation==='partial')return `${badge('partial','Registered partial')} ${badge('planned','Scope review pending')}`;
  return statusBadge('implementation',b.status.implementation);
}
function implementationGapAdvice(b) {
  const work=b.tranche_task?.work_review;
  if(work?.implementation==='implemented_within_scope')return 'Implemented within scope. No concrete missing code is identified; follow the separate validation and acceptance next steps. The registered partial status retains historical accounting.';
  if(work?.implementation==='not_implemented')return 'No implementation is linked for the reviewed contract. Recovery or implementation work: '+work.implementation_gaps.join(' · ');
  if(work?.implementation==='unresolved_aggregate')return 'Unresolved aggregate. Reconcile delivered services before identifying or sizing missing code; validation gaps do not establish missing implementation.';
  if(work?.implementation==='partially_implemented')return 'Partially implemented. Missing code: '+work.implementation_gaps.join(' · ');
  return 'Review the declared scope before naming missing code. Partially implemented requires a concrete required branch, mechanism or supported domain that is absent. Pending validation and excluded domains do not establish a code gap.';
}
function trancheScopeDomains(work) {
  if(!work.scope_domains?.length)return '';
  const labels={supplied:'Implemented',guarded_refusal:'Implemented refusal',outside_contract:'Outside this contract'};
  return `<details data-scope-resolution><summary>Resolved scope: supplied services and remaining validation</summary><p class="small">${fileLink(work.scope_resolution_record)}</p><ul class="detail-list">${work.scope_domains.map(domain=>`<li><strong>${escapeHTML(domain.name)} · ${labels[domain.disposition]}</strong><p>${escapeHTML(domain.finding)}</p><p class="small"><strong>Validation boundary:</strong> ${escapeHTML(domain.validation)}</p><p class="small">Suppliers: ${domain.suppliers.map(id=>featureLink(behaviorById.get(id))).join(' · ')}</p><p class="small">${domain.paths.map(fileLink).join(' · ')}</p></li>`).join('')}</ul></details>`;
}
function trancheWorkDetail(task) {
  const work=task.work_review;if(!work)return '';
  const kinds={implementation:'Implementation',validation:'Validation',scope_review:'Scope review',diagnosis:'Diagnosis'};
  return `<div data-work-review><p><strong>Current state:</strong> ${escapeHTML(work.current_state)}</p><p class="small"><strong>Implementation gaps:</strong> ${work.implementation_gaps.length?work.implementation_gaps.map(escapeHTML).join(' · '):work.implementation==='unresolved_aggregate'?'Unresolved; reconcile delivered services before naming missing code.':'No concrete missing code identified within the declared scope.'}</p><p class="small"><strong>Validation state:</strong> ${escapeHTML(work.validation_state)} · ${escapeHTML(work.validation_summary)}</p><p class="small"><strong>Acceptance:</strong> ${escapeHTML(work.acceptance_summary)}</p>${trancheScopeDomains(work)}<h4>Next steps by kind of work</h4><ol>${work.next_steps.map(step=>`<li><strong>${kinds[step.kind]}:</strong> ${escapeHTML(step.action)}</li>`).join('')}</ol><p class="small muted">Source review: ${escapeHTML(work.freshness)}. Registered execution-accounting status: ${escapeHTML(statusLabel('implementation',task.implementation))}. The source assessment does not renew execution evidence or promote a registered milestone.</p></div>`;
}
function trancheTaskDetail(task) {
  const brief=task.execution_brief;
  const list=items=>`<ul class="detail-list">${items.map(item=>`<li>${escapeHTML(item)}</li>`).join('')}</ul>`;
  const supplied=brief?.supplied_controls?`<h4>Supplied controls and retained result</h4>${list(brief.supplied_controls)}<p class="small">These controls already exist. Review source drift before renewing execution; the retained result is historical.</p>`:'';
  const execution=brief?`<div class="execution-brief" data-execution-brief><h4>Before starting</h4><p class="small">Prerequisites: ${brief.requires.length?brief.requires.map(id=>featureLink(behaviorById.get(id))).join(' · '):'No earlier task prerequisite.'}</p>${list(brief.inputs)}<h4>Entry points & current coverage</h4>${brief.runners.map(r=>`<div class="runner-template">${fileLink(r.path)}<p class="small">${escapeHTML(r.coverage)}</p><code>${escapeHTML(r.command)}</code>${r.claims_binding==='needs_prospective_wrapper'?'<p class="small"><strong>Before using as current evidence:</strong> prepare a prospectively bound wrapper.</p>':''}</div>`).join('')}${brief.fixture_gaps.length?`<h4>Fixture work required first</h4>${list(brief.fixture_gaps)}`:'<p class="small">The named baseline cases have existing entry points; execution and evidence collection are still pending.</p>'}<h4>Required pass checks</h4>${list(brief.acceptance)}<p class="small"><strong>Handoff:</strong> ${escapeHTML(brief.handoff)}</p></div>`:'';
  return `${trancheTaskResult(task)}${trancheWorkDetail(task)}${supplied}<p class="small"><strong>Work mode:</strong> ${escapeHTML(task.mode)}</p><p class="small"><strong>Implementation boundary:</strong> ${escapeHTML(task.implementation_boundary)}</p><p class="small"><strong>Validation:</strong> ${escapeHTML(task.validation)}</p><p class="small"><strong>Completion condition:</strong> ${escapeHTML(task.exit_condition)}</p>${execution}<p class="small"><strong>Retained wider boundary:</strong> ${escapeHTML(task.remaining_at_review)}</p><div class="detail-files">${[...task.checks,...task.documents].map(fileLink).join('')}</div>`;
}
function trancheWorkSummary(tranche) {
  const reviewed=tranche.tasks.filter(task=>task.work_review);if(!reviewed.length)return '';
  const labels=data.importance.tranche_plan.work_labels;
  return `<div data-tranche-work-summary><p class="small"><strong>Implementation assessment:</strong> ${['implemented_within_scope','partially_implemented','unresolved_aggregate',...(reviewed.some(task=>task.work_review.implementation==='not_implemented')?['not_implemented']:[])].map(key=>`${reviewed.filter(task=>task.work_review.implementation===key).length} ${escapeHTML(labels[key].label)}`).join(' · ')}.</p><p class="small"><strong>Acceptance:</strong> ${reviewed.filter(task=>task.work_review.acceptance==='acceptance_failure').length} retained acceptance failures; these overlap the implementation assessment. See each task for validation and acceptance outcomes.</p><details data-work-definitions><summary>Implementation and acceptance labels</summary>${Object.values(labels).map(item=>`<p class="small"><strong>${escapeHTML(item.label)}:</strong> ${escapeHTML(item.definition)}</p>`).join('')}<p class="small">These task assessments identify work separately from historical registered stage counts. Source review is not engine execution validation.</p></details></div>`;
}
function trancheView(t,key='tranche_plan') {
  const high=key==='high_tranche_plan',level=high?'High':'Critical',prefix=high?'high':'critical';
  const plan=data.importance?.[key];if(!plan || data.importance.track!==t.id)return '';
  const batches=plan.tranches.map(tranche=>`<details class="critical-tranche" data-${prefix}-tranche="${escapeHTML(tranche.id)}" ${tranche.order===1?'open':''}><summary>Tranche ${tranche.order} · ${escapeHTML(tranche.title)} (${tranche.tasks.length} behaviors)</summary><p class="small">${escapeHTML(tranche.goal)}</p><p class="small"><strong>Review before moving on:</strong> ${escapeHTML(tranche.exit_gate)}</p>${trancheWorkSummary(tranche)}${tranche.execution_review?`<details class="tranche-run-rules"><summary>Tranche execution order, pilot & pass rules</summary><p class="small">${escapeHTML(tranche.execution_review.scope)}</p><ul class="detail-list">${[...tranche.execution_review.run_rules,...tranche.execution_review.result_contract].map(rule=>`<li>${escapeHTML(rule)}</li>`).join('')}</ul></details>`:''}${tranche.tasks.map((task,index)=>`<article class="tranche-task" data-${high?'high-tranche':'tranche'}-behavior="${escapeHTML(task.behavior)}"><div class="badge-row">${importanceBadge(behaviorById.get(task.behavior))} ${trancheWorkBadges(task)} ${task.execution_result?badge(task.execution_result.outcome==='PASS'?'scoped':'partial',(task.work_review?'Historical task ':'Task ')+task.execution_result.outcome):badge('planned','Planned task')}${task.review_state!=='current'?badge('stale','Task needs review'):''}</div>${featureLink(behaviorById.get(task.behavior))}<div class="code">${escapeHTML(task.behavior)} · ${escapeHTML(task.mode)}</div>${task.work_review?`<p class="small"><strong>Current state:</strong> ${escapeHTML(task.work_review.current_state)}</p>`:''}<p class="small"><strong>Next action:</strong> <span class="muted">${index+1}/${tranche.tasks.length}</span> ${escapeHTML(task.next_action)}</p><details><summary>Implementation, validation & completion condition</summary>${trancheTaskDetail(task)}</details></article>`).join('')}</details>`).join('');
  return panel(level+' work: 3 tranches',high?'51 unfinished High contracts · 17 per tranche. Implementation, validation and original recovery are assessed separately.':'33 Critical contracts · 11 per tranche. All three tranches separate implementation, validation and acceptance.',`<div data-${prefix}-plan><p class="small">${escapeHTML(plan.selection)}</p><p class="small">${escapeHTML(plan.completion_rule)}</p>${(high?plan.unplanned_high:plan.unplanned_critical).length?`<p class="notice">${(high?plan.unplanned_high:plan.unplanned_critical).length} new ${level} unfinished behaviors need a tranche assignment.</p>`:''}${plan.review_document?`<p class="small">${fileLink(plan.review_document)}</p>`:''}${batches}<details class="tranche-rules"><summary>Execution and evidence rules</summary><ul class="detail-list">${plan.execution_rules.map(rule=>`<li>${escapeHTML(rule)}</li>`).join('')}</ul><p class="small">This page plans the work; it does not start experiments. Plan reviewed ${escapeHTML(plan.reviewed_at)}. ${fileLink('apps/coverage-ui/importance.json')}</p></details></div>`);
}
function importanceView(t) {
  const catalog=data.importance;if(!catalog || catalog.track!==t.id)return '';
  const all=t.behaviors.map(id=>behaviorById.get(id)), partial=all.filter(b=>b.status.implementation==='partial');
  const statusMatches=b=>{
    if(importanceStatus==='all')return true;
    if(importanceStatus==='open')return b.status.implementation!=='scoped';
    if(importanceStatus==='acceptance_failure')return b.tranche_task?.work_review?.acceptance==='acceptance_failure';
    if(['implemented_within_scope','partially_implemented','unresolved_aggregate','not_implemented'].includes(importanceStatus))return b.tranche_task?.work_review?.implementation===importanceStatus;
    return b.status.implementation===importanceStatus;
  };
  const rows=all.filter(b=>statusMatches(b) && (importanceType==='all'||b.importance?.work_type===importanceType) && (importanceLevel==='all'||String(b.importance?.weight)===importanceLevel)).sort(importanceOrder);
  const options=(values,value)=>Object.entries(values).map(([key,label])=>`<option value="${key}" ${key===value?'selected':''}>${escapeHTML(label)}</option>`).join('');
  const counts=`<div class="importance-counts">${[5,4,3,2,1].map(w=>`<div><strong>${partial.filter(b=>b.importance?.weight===w).length}</strong><span>${w}/5 · ${escapeHTML(catalog.labels[w])}</span></div>`).join('')}</div>`;
  const controls=`<div class="toolbar importance-controls"><label>Status / source assessment<select id="importance-status" class="control">${options({partial:'Registered partial',partially_implemented:'Partially implemented (reviewed missing code)',implemented_within_scope:'Implemented within scope',unresolved_aggregate:'Unresolved aggregate',acceptance_failure:'Acceptance failure',not_implemented:'Not implemented (reviewed)',none:'Registered not implemented',open:'All registered unfinished',all:'All rendering contracts'},importanceStatus)}</select></label><label>Kind of work<select id="importance-type" class="control">${options({all:'All kinds of work',...importanceTypes},importanceType)}</select></label><label>Importance<select id="importance-level" class="control">${options({all:'All levels',...Object.fromEntries([5,4,3,2,1].map(w=>[w,`${w}/5 · ${catalog.labels[w]}`]))},importanceLevel)}</select></label></div>`;
  const list=rows.map(b=>`<article class="importance-entry" data-importance-behavior="${escapeHTML(b.id)}" data-weight="${b.importance?.weight||0}"><div class="badge-row">${importanceBadge(b)} ${implementationBadge(b)}</div>${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(importanceTypes[b.importance?.work_type]||'Unrated')}</div><p class="small">${escapeHTML(b.importance?.reason||'New contract: importance needs review.')}</p>${b.tranche_task?`<p class="small"><strong>${b.tranche_task.plan_priority||'Critical'} tranche ${b.tranche_task.tranche_order} next action:</strong> ${escapeHTML(b.tranche_task.next_action)}</p>`:''}${b.reconciliation?`<p class="small"><strong>Remaining:</strong> ${escapeHTML(b.reconciliation.remaining)}</p>`:''}</article>`).join('');
  return panel('Importance & remaining priorities','Highest importance first; registered partial contracts selected by default.',`<div data-rendering-importance><p class="small">Initial engineering judgments for reliable native rendering: player impact, breadth and safety or release consequences. The weight applies to the documented contract, not a measured severity or size of its remaining gap. Equal weights are ties, ordered by behavior ID. Implementation and validation keep their own statuses.</p><h3>Registered partial contracts by importance (${partial.length})</h3><p class="small">This historical filter includes implemented mechanisms awaiting validation and unresolved aggregates. A partial implementation needs a named missing branch, mechanism or supported domain; use the source assessment and next steps on each reviewed task.</p>${counts}${catalog.unrated.length?`<p class="notice">${catalog.unrated.length} contracts are unrated.</p>`:''}<details class="importance-rubric"><summary>How the weights work</summary>${[5,4,3,2,1].map(w=>`<p class="small"><strong>${w}/5 · ${escapeHTML(catalog.labels[w])}.</strong> ${escapeHTML(catalog.rubric[w])}</p>`).join('')}<p class="small">${escapeHTML(catalog.limits)} Reviewed ${escapeHTML(catalog.reviewed_at)}. ${fileLink('apps/coverage-ui/importance.json')}</p></details>${controls}<p class="small importance-result-count">${rows.length} contracts shown · All ${all.length} rendering contracts remain available.</p><div class="importance-list" tabindex="0" role="region" aria-label="Rendering contracts by importance">${list||'<p>No contracts match these filters.</p>'}</div></div>`);
}
function reconciliationView(t) {
  const review=data.implementation_reviews?.find(r=>r.track===t.id);
  if(!review)return '';
  const groups=Object.entries(review.counts).map(([key,count])=>`<details data-reconciliation-group="${escapeHTML(key)}"><summary>${escapeHTML(review.labels[key])} (${count})</summary>${review.entries.filter(e=>e.disposition===key).sort((a,b)=>importanceOrder(behaviorById.get(a.behavior),behaviorById.get(b.behavior))).map(e=>`<article class="reconciliation-entry" data-reconciled-behavior="${escapeHTML(e.behavior)}">${importanceBadge(behaviorById.get(e.behavior))} ${featureLink(behaviorById.get(e.behavior))}<div class="code">${escapeHTML(e.behavior)} · ${escapeHTML(titleCase(e.role))} · ${e.freshness==='current'?'Current source review':'Historical source review'}</div><p>${escapeHTML(e.finding)}</p><p><strong>Remaining:</strong> ${escapeHTML(e.remaining)}</p>${e.related_behaviors.length?`<p class="small">Narrower / related contracts: ${e.related_behaviors.map(id=>featureLink(behaviorById.get(id))).join(' · ')}. Each retains its own scope.</p>`:''}</article>`).join('')}</details>`).join('');
  return panel('Open contracts, reconciled',`${review.reviewed_count} contracts reviewed individually; these are different kinds of work.`,
    `<div data-rendering-reconciliation><p class="small">This source/scope review separates located bounded implementations, diagnostic tools, unresolved original behavior, broader contracts and acceptance targets. It does not add them to the implemented count or establish original equivalence. The original and broad scopes can still require substantial implementation.</p>${review.freshness==='historical'?'<p class="notice">Some review inputs changed. Historical findings need renewed review before applying to current code.</p>':''}${groups}<p class="small">${fileLink(review.document)} · ${fileLink(review.record)}</p></div>`);
}
function separatedRenderingView(t, contractList) {
  const focus=t.focus, s=implementationSummary(t.behaviors);
  const unfinished=['partial','none'].map(status=>{
    const rows=t.behaviors.map(id=>behaviorById.get(id)).filter(b=>b.status.implementation===status).sort(importanceOrder);
    return `<details class="implementation-gap-list"><summary>${statusLabel('implementation',status)} (${rows.length})</summary>${rows.map(b=>`<div class="render-contract">${importanceBadge(b)} ${featureLink(b)}<div class="code">${escapeHTML(b.id)}</div>${b.reconciliation?`<p class="small">${escapeHTML(b.reconciliation.label)} · ${b.reconciliation.freshness==='current'?'Current source review':'Historical source review'}</p>`:''}</div>`).join('') || '<p>No registered behaviors in this category.</p>'}</details>`;
  }).join('');
  const implementation=panel('Implementation','What is implemented across the registered rendering and animation scope.',
    `<div data-progress-section="implementation">${implementationCounts(s)}<p class="small roadmap-note">${s.total} registered behaviors: ${s.scoped} implemented, ${s.partial} registered partial and ${s.missing} not implemented. Registered partial counts can include implemented mechanisms or unresolved scope until reviewed. These are registered statuses across native services, original contracts, animation and supporting diagnostics. Open entries do not each mean a missing renderer feature; the reconciliation below explains their code, scope and remaining work. Counts remain independent of validation.</p><h3>Next implementation task</h3><p><strong>${escapeHTML(focus.implementation_title)}</strong></p><p>${escapeHTML(focus.implementation_task)}</p>${contractList(focus.implementation_behaviors,['implementation'])}${unfinished}<button class="text-button" data-track-work="rendering" data-gap="partial">Review registered partial entries →</button></div>`);
  const states=t.milestones.map(m=>milestoneValidation(m));
  const validationCounts=`<div class="track-summary">${Object.entries(validationLabels).map(([state,label])=>`<div><strong>${states.filter(s=>s===state).length}</strong><span>${label}</span></div>`).join('')}</div>`;
  const validation=panel('Validation','What the execution evidence establishes, independently of implementation status.',
    `<div data-progress-section="validation">${validationCounts}<p class="small roadmap-note">These counts cover ${t.milestones.length} curated milestones, a different denominator from the ${s.total} registered behaviors above. Current validation requires evidence bound to the current scope and sources. Historical validation retains an earlier recorded result. Pending means required evidence or comparison/integration/replacement stages are incomplete; it does not by itself mean code is missing. All milestone requirements met: ${t.progress.count} / ${t.progress.total}, including code and understanding requirements.</p><h3>Next validation task</h3><p><strong>${escapeHTML(focus.next_title)}</strong></p><p>${escapeHTML(focus.next_task)}</p>${contractList(focus.next_behaviors,['comparison','integration','replacement'])}<button class="text-button" data-track-work="rendering" data-gap="validation">Browse current validation gaps →</button></div>`);
  const roadmap=`<div class="render-roadmap">${t.milestones.map(m=>{
    const validation=milestoneValidation(m), code=implementationSummary(m.behaviors);
    return `<article class="panel render-milestone" data-render-milestone="${escapeHTML(m.id)}"><h3>${escapeHTML(m.title)}</h3><p class="small">${escapeHTML(m.scope)}</p><div class="milestone-dimensions"><section class="milestone-implementation"><h4>Implementation</h4>${statusBadge('implementation',milestoneImplementation(m))}<p>${code.scoped} / ${code.total} required behaviors implemented; ${code.partial} registered partial; ${code.missing} not implemented.</p></section><section class="milestone-validation" data-validation-state="${validation}"><h4>Validation</h4>${badge(validation,validationLabels[validation])}<p>${validation==='current'?'Current scope-bound evidence meets the execution requirements.':validation==='historical'?'Execution requirements have recorded evidence; current scope-bound validation remains pending.':validation==='unknown'?'Required behavior mappings are missing.':'Execution evidence or required comparison, integration or replacement stages remain pending.'}</p></section></div><p class="small muted">${m.state==='demonstrated'?'All milestone requirements met within the recorded scope.':'Remaining milestone scope: '+escapeHTML(m.remaining)}</p>${m.missing.length?`<p class="notice error">Missing mappings: ${m.missing.map(escapeHTML).join(', ')}</p>`:''}<details><summary>Contracts, stages & evidence (${m.behaviors.length})</summary>${contractList(m.behaviors)}<p class="small muted">Open a contract for its exact scope, independent stages and evidence.</p></details></article>`;
  }).join('')}</div>`;
  return `<div class="notice render-boundary"><strong>Rendering scope.</strong> ${escapeHTML(focus.boundary)}</div>`+implementation+trancheView(t)+trancheView(t,'high_tranche_plan')+importanceView(t)+reconciliationView(t)+validation+panel('Rendering milestones','Implementation and validation are shown separately for every milestone.',roadmap,`<button class="button" data-track-detail="rendering">Inspect milestone requirements</button>`)+panel('Research & remaining work','Authoritative release gates and current blockers.',`<div class="detail-files">${focus.documents.map(fileLink).join('')}</div><button class="text-button" data-track-work="rendering">Browse all rendering behavior gaps →</button>`);
}
function renderingView(trackId='rendering') {
  const t=data.tracks.find(t=>t.id===trackId);
  if(!t?.focus)return '<div class="empty">Roadmap is not registered.</div>';
  const focus=t.focus, demonstrated=t.milestones.filter(m=>m.state==='demonstrated');
  const contractList=(ids,stages=['implementation','comparison','integration','replacement'])=>ids.map(id=>{
    const b=behaviorById.get(id);
    return b ? `<div class="render-contract">${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${b.kind==='recovered'?'Recovered baseline':'Native policy'}</div><div class="badge-row">${stages.map(stage=>b.kind==='native_policy'&&['comparison','replacement'].includes(stage)?badge('not_applicable',stageNames[stage]+': N/A'):statusBadge(stage,b.status[stage])).join(' ')}</div></div>` : `<p class="notice error">Missing registered mapping: ${escapeHTML(id)}</p>`;
  }).join('');
  if(trackId==='rendering')return separatedRenderingView(t,contractList);
  const summary=implementationCounts(implementationSummary(t.behaviors))+`<p class="small roadmap-note">${t.progress.count} / ${t.progress.total} milestones have all requirements met; ${t.current} / ${demonstrated.length} of those have current validation. Milestones can also require original comparison, integration or live replacement. Historical evidence remains separate from current validation.</p>`;
  const nextMilestone=t.milestones.find(m=>m.id===t.next);
  const next=panel('Next engineering task',focus.next_title || nextMilestone?.title || 'Review the next scope',`<p>${escapeHTML(focus.next_task || nextMilestone?.remaining || 'Review scope before expanding this roadmap.')}</p>${contractList(focus.next_behaviors || nextMilestone?.behaviors || [])}`);
  const roadmap=`<div class="render-roadmap">${t.milestones.map(m=>`<article class="panel render-milestone" data-${trackId==='rendering'?'render':'simulation'}-milestone="${escapeHTML(m.id)}"><div class="milestone-heading"><h3>${escapeHTML(m.title)}</h3>${milestoneStatus(m)}</div><p class="small">${escapeHTML(m.scope)}</p><p class="small muted">${m.state==='demonstrated'?(m.current?'Scope-bound current validation for every required contract.':'Milestone requirements met with historical evidence; current validation pending.'):'Remaining: '+escapeHTML(m.remaining)}</p>${m.missing.length?`<p class="notice error">Missing mappings: ${m.missing.map(escapeHTML).join(', ')}</p>`:''}<details><summary>Contracts, stages & evidence (${m.behaviors.length})</summary>${contractList(m.behaviors)}<p class="small muted">Open a contract for its exact scope, independent stages, source hashes and evidence records.</p></details></article>`).join('')}</div>`;
  return `<div class="notice render-boundary"><strong>${trackId==='rendering'?'Current replacement boundary.':'Simulation scope.'}</strong> ${escapeHTML(focus.boundary)}</div>`+summary+next+panel(escapeHTML(t.title)+' roadmap','Implementation, original comparison, integration and live replacement remain separate.',roadmap,`<button class="button" data-track-detail="${trackId}">Inspect milestone requirements</button>`)+panel('Research & remaining work','The authoritative release gates and current blockers.',`<div class="detail-files">${focus.documents.map(fileLink).join('')}</div><button class="text-button" data-track-work="${trackId}">Browse ${escapeHTML(t.title.toLowerCase())} behavior gaps →</button>`);
}
function engineView() {
  const tracks=data.tracks.filter(t=>!t.planned), ids=new Set(tracks.flatMap(t=>t.behaviors));
  const rows=data.behaviors.filter(b=>ids.has(b.id)), s=summarize(rows);
  const milestones=tracks.flatMap(t=>t.milestones);
  const summary=implementationCounts(s)+`<p class="small roadmap-note">${tracks.length} engine tracks; ${milestones.filter(m=>m.state==='demonstrated').length} / ${milestones.length} milestones have all requirements met. Shared behaviors count once in these totals. Implementation, original comparison, integration and live replacement are independent. These counts do not estimate effort or whole-engine completion.</p>`;
  const table=`<div class="scroll-table"><table class="engine-table"><thead><tr><th>Engine track</th><th>Implementation</th><th>Original comparison</th><th>Integration</th><th>Live replacement</th><th>Next milestone</th></tr></thead><tbody>${tracks.map(t=>{
    const bs=t.behaviors.map(id=>behaviorById.get(id)), stats=summarize(bs), next=t.milestones.find(m=>m.id===t.next);
    const integrated=bs.filter(b=>b.status.integration!=='none');
    const integrations=[...new Set(integrated.map(b=>b.status.integration))].map(v=>`${integrated.filter(b=>b.status.integration===v).length} ${statusLabel('integration',v)}`).join(' · ');
    return `<tr data-engine-track="${escapeHTML(t.id)}"><td><button class="feature-button" data-track-detail="${escapeHTML(t.id)}">${escapeHTML(t.title)}</button><div class="small muted">${t.progress.count} / ${t.progress.total} milestones with requirements met</div></td><td>${stats.scoped} / ${stats.total} implemented<div class="small muted">${stats.partial} registered partial · ${stats.missing} not implemented</div></td><td>${stats.recovered?`${stats.current} / ${stats.recovered} current<div class="small muted">${stats.compared} recorded historically</div>`:'N/A · native policies'}</td><td>${integrations?escapeHTML(integrations):'None registered'}</td><td>${stats.recovered?`${stats.replaced} / ${stats.recovered} replaced within scope`:'N/A · native policies'}</td><td><button class="text-button" data-track-detail="${escapeHTML(t.id)}">${escapeHTML(next?.title || 'Review next scope')}</button><div><button class="text-button" data-track-work="${escapeHTML(t.id)}">Browse gaps →</button></div></td></tr>`;
  }).join('')}</tbody></table></div>`;
  return summary+panel('Simulation work', 'Owned state, command execution, AI decisions, combat, spells and campaign continuation.', '<p>Inspect the simulation checklist for implementation status, current evidence and open original-engine boundaries.</p><button class="button" data-view="simulation">Open simulation roadmap →</button>')+panel('Engine work by track', 'Implementation, independent original comparison, integration mode and live replacement are independent stages.',table)+panel('Planned gameplay changes', 'Commander orders, veterancy and mana economy remain separate from baseline engine validation.', '<button class="text-button" data-track-detail="future-gameplay">Inspect planned gameplay milestones →</button>');
}
function tracksView() {
  const active=data.tracks.filter(t=>!t.planned), ids=[...new Set(active.flatMap(t=>t.behaviors))];
  const intro=implementationCounts(implementationSummary(ids))+'<p class="small roadmap-note">Percentages count registered behaviors implemented within scope. Shared behaviors count once in these totals. Milestone requirements and current validation are shown separately. These counts do not measure effort or whole-game completion.</p>';
  const cards=tracks=>`<div class="track-grid">${tracks.map(t=>{
    const s=implementationSummary(t.behaviors), ready=t.milestones.filter(m=>milestoneImplementation(m)==='scoped'), partial=t.milestones.filter(m=>milestoneImplementation(m)==='partial'), next=t.milestones.find(m=>m.id===t.next);
    return `<article class="track-card" data-track-card="${escapeHTML(t.id)}"><div class="track-card-head"><h2><button class="feature-button" data-track-detail="${escapeHTML(t.id)}">${escapeHTML(t.title)} ↗</button></h2><span class="track-percent" aria-label="${percent(s.scoped,s.total)} of registered behaviors implemented">${s.total?Math.round(100*s.scoped/s.total):'N/A'}${s.total?'<small>%</small>':''}</span></div><p class="track-goal">${escapeHTML(t.goal)}</p>${stack(t.behaviors.map(id=>behaviorById.get(id)))}<div class="track-caption"><strong>${s.scoped} / ${s.total} behaviors implemented</strong><span>${s.partial} registered partial · ${s.missing} not implemented</span></div><div class="track-now"><h3>${ready.length?'Required behaviors implemented':partial.length?'Partially implemented':'Not implemented'}</h3>${ready.length?`<ul>${ready.map(m=>`<li>${escapeHTML(m.title)}</li>`).join('')}</ul>`:partial.length?`<ul>${partial.map(m=>`<li>${escapeHTML(m.title)}</li>`).join('')}</ul>`:'<p>Not implemented.</p>'}</div><div class="track-next"><h3>Next milestone requirement</h3><p><strong>${escapeHTML(next?.title || 'All milestone requirements met')}</strong></p><p>${escapeHTML(next?.remaining || 'Review scope before expanding the roadmap.')}</p></div><p class="small freshness-note">${t.progress.count} / ${t.progress.total} milestones have all requirements met; ${t.current} / ${t.progress.count} of those have current validation. Historical evidence retains its original scope.</p><button class="text-button track-open" data-track-detail="${escapeHTML(t.id)}">Milestones, gaps & evidence →</button></article>`;
  }).join('')}</div>`;
  const unassigned=data.behaviors.filter(b=>!data.tracks.some(t=>t.behaviors.includes(b.id)));
  return intro+cards(active)+panel('Planned gameplay changes.', 'These remain separate from recovering and validating the original engine.',cards(data.tracks.filter(t=>t.planned)))+panel('Supporting work & roadmap scope.', 'Newly registered behavior still appears in the detailed coverage views.',`<p class="small">${unassigned.length} registered behaviors sit outside these product tracks. Behavior percentages count each contract equally; the spell dispatch checklist can therefore outweigh smaller tracks. The milestone checklist groups spell dispatch as one outcome.</p><button class="text-button" data-all-behaviors>Browse all registered work →</button>`);
}
function openTrack(id) {
  const t=data.tracks.find(t=>t.id===id);if(!t)return;
  const stats=implementationSummary(t.behaviors);
  const goals=t.milestones.map(m=>`<section class="milestone-detail"><div class="milestone-heading"><h3>${escapeHTML(m.title)}</h3>${milestoneStatus(m)}</div><p>${escapeHTML(m.scope)}</p>${m.state==='demonstrated'?`<p class="small">${m.current?'Scope-bound current validation is available for every required contract.':'Milestone requirements met; one or more required execution records are historical. Rerun affected evidence before claiming current-source validation.'}</p>`:`<p class="milestone-remaining"><strong>Remaining:</strong> ${escapeHTML(m.remaining)}</p>`}${m.missing.length?`<p class="notice error">Missing registered mappings: ${m.missing.map(escapeHTML).join(', ')}</p>`:''}<details><summary>Required contracts & evidence (${m.behaviors.length})</summary><div class="milestone-contracts">${m.checks.map(c=>{const b=behaviorById.get(c.behavior);return `<div>${featureLink(b)} ${badge(c.met?'Requirement met':'Requirement pending')}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(Object.entries(c.expected).map(([s,v])=>`${stageNames[s] || titleCase(s)}: ${(Array.isArray(v)?v:[v]).map(value=>statusLabel(s,value)).join(' / ')}`).join(' · '))}</div></div>`;}).join('')}</div></details></section>`).join('');
  openDialog('MAJOR TRACK / '+t.id,`<h2 id="detail-title" class="detail-title">${escapeHTML(t.title)}</h2><p>${escapeHTML(t.goal)}</p><div class="scope-text"><strong>${stats.scoped} / ${stats.total} behaviors implemented; ${stats.partial} registered partial; ${stats.missing} not implemented.</strong> ${t.progress.count} / ${t.progress.total} milestones have all requirements met. Every required contract must meet its explicit stages and have execution evidence. Partial work earns no invented fraction. Previews, headless comparisons and original observation retain their own scope; observation does not mean native replacement.</div>${goals}<button class="button" data-track-work="${escapeHTML(t.id)}">Browse this track’s behavior gaps →</button>`);
}
function overview() {
  const rows = scopedBehaviors(), s = summarize(rows);
  const mapped = data.binary.builds.reduce((n,b)=>n+b.functions.count,0), functions = data.binary.functions.length;
  const metrics = `<div class="metric-grid">${metric('FUNCTIONS WITH BEHAVIOR LINKS',mapped,functions,'Global binary inventory. A link may cover only part of a function.','data-go="binary"','functions')}${metric('IMPLEMENTED',s.scoped,s.total,`${s.partial} registered partial · ${s.missing} not implemented.`,'data-go="behaviors"','behaviors')}${metric('ORIGINAL COMPARISON',s.compared,s.recovered,`${s.current} have current original comparison validation. Historical results retain their scope.`,'data-go="work" data-gap="comparison"','baseline behaviors')}${metric('LIVE REPLACEMENT',s.replaced,s.recovered,'Requires live equivalence and evidence that original work was bypassed.','data-go="work" data-gap="replacement"','baseline behaviors')}</div>`;
  const stage = (label, count, total) => `<div class="stage-row"><span>${label}</span><div class="stack"><span style="width:${total?count/total*100:0}%"></span></div><strong>${percent(count,total)}</strong></div>`;
  const progress = panel('Implementation status.', `${s.total} registered behaviors in the selected scope.`, `${stack(rows,true)}${legend()}<div class="stat-list"><div><strong>${s.scoped}</strong><span>Implemented · ${percent(s.scoped,s.total)}</span></div><div><strong>${s.partial}</strong><span>Registered partial · ${percent(s.partial,s.total)}</span></div><div><strong>${s.missing}</strong><span>Not implemented · ${percent(s.missing,s.total)}</span></div></div><p class="stage-description">Registered partial entries are counted separately; source review identifies any concrete missing code. Implemented behaviors may still have comparison, integration, or replacement work outstanding.</p>`);
  const stages = panel('Independent milestones.', 'Implementation and validation advance separately.', `${stage('Understood within scope',s.understood,s.total)}${stage('Any integration',s.integrated,s.total)}${stage('Current original comparison',s.current,s.recovered)}${stage('Live replacement',s.replaced,s.recovered)}<p class="stage-description">Any integration includes headless tests, previews, and live observation. Current comparison requires an exact scope binding and complete current source provenance. Historical results remain separate. Comparison and replacement use recovered behaviors as the denominator.</p>`);
  return metrics + `<div class="two-col">${progress}${stages}</div>` + subsystemTable(rows) + workSummary(rows) + binaryBoundaries();
}
function subsystemTable(rows) {
  const groups = [...new Set(rows.map(b=>b.subsystem))].sort((a,b)=>subsystemName(a).localeCompare(subsystemName(b)));
  const table = `<div class="scroll-table"><table><thead><tr><th>Subsystem</th><th>Behaviors</th><th>Implementation</th><th>Compared / baseline</th><th>Remaining implementation</th></tr></thead><tbody>${groups.map(id=>{
    const bs=rows.filter(b=>b.subsystem===id), s=summarize(bs);
    return `<tr><td><button class="subsystem-button" data-subsystem="${escapeHTML(id)}">${escapeHTML(subsystemName(id))} ↗</button></td><td>${s.total}</td><td class="bar-cell">${stack(bs)}<div class="bar-caption"><span>${s.scoped} implemented</span><span>${percent(s.scoped,s.total)}</span></div></td><td>${s.recovered ? `${s.compared} / ${s.recovered}<div class="code">${s.current} currently validated</div>` : 'N/A · native policy'}</td><td>${badge(`${s.partial} registered partial`)} ${badge(`${s.missing} not implemented`)}</td></tr>`;
  }).join('')}</tbody></table></div>`;
  return panel('Subsystem coverage.', 'Select a subsystem to inspect its partial features and remaining milestones.', groups.length?table:'<div class="empty">No behaviors in this scope.</div>',legend());
}
function workSummary(rows) {
  const partial=rows.filter(b=>b.status.implementation==='partial'), missing=rows.filter(b=>b.status.implementation==='none');
  const list=items=>items.slice(0,5).map(b=>`<tr><td>${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(subsystemName(b.subsystem))}</div></td><td>${implementationBadge(b)}</td></tr>`).join('');
  return `<div class="two-col">${panel(`${partial.length} registered partial.`, 'Registered partial entries need scope review; pending validation alone is not missing code.', `<table><tbody>${list(partial)}</tbody></table>`, '<button class="text-button" data-go="work" data-gap="partial">See all →</button>')}${panel(`${missing.length} not implemented.`, 'Recorded behaviors without an implementation claim.', `<table><tbody>${list(missing)}</tbody></table>`, '<button class="text-button" data-go="work" data-gap="unimplemented">See all →</button>')}</div>`;
}
function workView() {
  const rows = scopedBehaviors();
  const chips = ['partial','unimplemented','comparison','stale'].map(g=>{
    const n=rows.filter(b=>b.gaps.includes(g)).length;
    const denominator=g==='comparison'?rows.filter(b=>b.kind==='recovered').length:rows.length;
    return `<button class="work-chip ${state.gap===g?'active':''}" data-gap-select="${g}"><strong>${n}</strong><span>${gapLabels[g]}</span><span class="muted">${percent(n,denominator)} of ${g==='comparison'?'baseline':'selected'} behaviors</span></button>`;
  }).join('');
  const filtered = rows.filter(b=>matches(b) && (state.gap==='all' ? b.gaps.length : b.gaps.includes(state.gap)));
  const helper = state.gap !== 'all' ? `<p class="small muted">Next milestone: ${escapeHTML(gapActions[state.gap])} The recorded scope is shown verbatim. Reviewed implementation gaps, validation and acceptance remain separate.</p>` : '<p class="small muted">A behavior can have several gaps. Counts overlap. Missing original comparison does not apply to intentional native policies.</p>';
  return `<div class="work-grid">${chips}</div>${panel('Feature gaps.', 'Choose a gap, then open a feature to see its recorded scope and evidence.', helper+behaviorToolbar(true)+behaviorTable(filtered,true))}${dispatchPanel()}`;
}
function behaviorToolbar(gaps=false) {
  return `<div class="toolbar"><input id="search" class="search" type="search" placeholder="Search behavior, scope, source file, or 0x address…" aria-label="Search behaviors" value="${escapeHTML(state.query)}">${gaps?`<select id="gap-filter" class="control" aria-label="Filter remaining milestone"><option value="all">All remaining milestones</option>${Object.entries(gapLabels).map(([id,label])=>`<option value="${id}" ${state.gap===id?'selected':''}>${label}</option>`).join('')}</select>`:''}<button class="button" data-clear>Clear filters</button></div>`;
}
function behaviorView() {
  const filtered = scopedBehaviors().filter(matches);
  return panel('Behavior register.', 'Open a behavior for the exact scope, confidence, evidence and outstanding milestones.', behaviorToolbar()+behaviorTable(filtered));
}
function paginate(rows, size) {
  const pages = Math.max(1, Math.ceil(rows.length/size));
  state.page=Math.min(state.page,pages-1);
  return {rows:rows.slice(state.page*size,(state.page+1)*size), footer:`<div class="pagination"><span>${number(rows.length)} results · Page ${state.page+1} of ${pages}</span><button class="button" data-page="${state.page-1}" ${state.page===0?'disabled':''}>← Previous</button><button class="button" data-page="${state.page+1}" ${state.page+1===pages?'disabled':''}>Next →</button></div>`};
}
function behaviorTable(rows, gaps=false) {
  const page=paginate(rows,25);
  return `<div class="scroll-table"><table><thead><tr><th>Feature / scope</th><th>Importance</th><th>Implementation</th>${gaps?'<th>Remaining milestones</th>':'<th>Original comparison</th><th>Integration</th>'}</tr></thead><tbody>${page.rows.map(b=>`<tr><td>${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(subsystemName(b.subsystem))} · ${b.kind==='recovered'?'baseline':'native policy'}</div>${gaps?`<p class="feature-scope">${escapeHTML(b.scope)}</p>`:''}</td><td>${importanceBadge(b)}</td><td>${implementationBadge(b)}</td>${gaps?`<td><div class="badge-row">${b.gaps.map(g=>badge(g==='partial'&&b.tranche_task?.work_review?b.tranche_task.work_review.label:gapLabels[g])).join('')}</div></td>`:`<td>${b.kind==='native_policy'?'N/A':statusBadge('comparison',b.status.comparison)}${b.status.comparison==='recorded'?`<div class="badge-row">${badge(b.comparison_freshness)}</div>`:''}</td><td>${statusBadge('integration',b.status.integration)}${Object.keys(b.stale_evidence).length?'<div class="badge-row"><span class="badge stale">Stale evidence</span></div>':''}</td>`}</tr>`).join('')}</tbody></table>${!rows.length?'<div class="empty">No behaviors match these filters.</div>':''}</div>${page.footer}`;
}
function dispatchPanel() {
  const ids = new Set(scopedBehaviors().map(b=>b.id));
  const tables = data.dispatch_tables.filter(t=>state.subsystem==='all' || t.entries.some(e=>e.behaviors.some(b=>ids.has(b))));
  return panel('Dispatch coverage is still bounded.', 'A mapped case may remain unimplemented. Unlinked cases and unknown default paths stay visible.', tables.map(t=>`<div class="dispatch-card"><h3>${escapeHTML(t.id)} ${badge(t.complete?'complete':'incomplete')}</h3><div class="code">${hex(t.original.address)} · ${escapeHTML(t.original.build)}</div><p class="feature-scope">${escapeHTML(t.scope)}</p><a class="inline-link" href="/files?path=${encodeURIComponent(t.document)}" target="_blank" rel="noopener">Read dispatch research ↗</a><div class="dispatch-entries">${t.entries.map(e=>`<button class="${e.behaviors.length?'linked':''}" ${e.behaviors.length?`data-dispatch="${escapeHTML(e.behaviors.join(','))}"`:'disabled'} title="${escapeHTML(e.behaviors.length?e.behaviors.map(id=>`${id}: ${statusLabel('implementation',behaviorById.get(id)?.status.implementation || 'unknown')}`).join('; '):'No behavior mapping registered')}">${escapeHTML(e.value)} · ${e.behaviors.length?'mapped':'unlinked'}</button>`).join('')}</div></div>`).join('') || '<p class="muted small">No dispatch tables linked to this subsystem.</p>');
}
function binaryBoundaries() {
  const counts=data.audit.counts, builds=data.binary.builds;
  return panel('The unmapped engine.', 'Global discovery boundaries, independent of the selected behavior checklist.', `<div class="boundary-grid"><div class="boundary"><span class="small">Functions without behavior links</span><strong>${number(data.binary.functions.filter(f=>!f.behaviors.length).length)}</strong><p>Function discovery does not establish semantic understanding. Classified support functions remain separate.</p></div><div class="boundary"><span class="small">Unresolved indirect flows</span><strong>${number(counts.unresolved_indirect_flows)}</strong><p>Targets require investigation. Inferred targets also need dispatch validation.</p></div><div class="boundary"><span class="small">Incomplete dispatch tables</span><strong>${number(counts.incomplete_dispatch_tables)}</strong><p>${number(counts.unmapped_dispatch_entries)} recorded entries have no behavior mapping.</p></div></div><p class="stage-description">${number(counts.unassigned_executable_ranges)} unassigned executable ranges · ${number(counts.unmapped_imports)} unmapped imports · ${number(counts.unmapped_callback_candidates)} callback candidates. Unassigned bytes may be padding or data. External DLL bodies are outside this executable inventory.</p>${builds.map(b=>`<p class="small muted"><strong>${percent(b.bytes.count,b.bytes.total)} associated function footprint</strong> · ${number(b.bytes.count)} / ${number(b.bytes.total)} file-backed executable bytes fall in discovered functions with a behavior link. Whole function bodies are counted for this footprint; linked semantics may cover only a small part. ${b.recovered_ranges.length} separately documented recovered ranges are retained outside this percentage.</p>`).join('')}`);
}
function binaryView() {
  const selected = new Set(scopedBehaviors().map(b=>b.id));
  const q = state.query.trim().toLowerCase();
  const fs=data.binary.functions.filter(f=>{
    if(state.mapped==='mapped'&&!f.behaviors.length || state.mapped==='unmapped'&&f.behaviors.length || state.mapped==='classified'&&!f.classification)return false;
    if((state.kind!=='all'||state.subsystem!=='all') && f.behaviors.length && !f.behaviors.some(id=>selected.has(id)))return false;
    if((state.subsystem!=='all' || state.track!=='all') && !f.behaviors.some(id=>selected.has(id)))return false;
    return !q || `${hex(f.entry)} ${f.name} ${f.behaviors.join(' ')}`.toLowerCase().includes(q) || (q.startsWith('0x') && Number.isFinite(Number(q)) && f.ranges.some(([a,b])=>a<=Number(q)&&Number(q)<b));
  });
  const page=paginate(fs,50);
  const controls=`<div class="toolbar"><input id="search" class="search" type="search" aria-label="Search functions" value="${escapeHTML(state.query)}" placeholder="Search function, behavior ID, or interior 0x address…"><select id="mapped-filter" class="control" aria-label="Filter function mapping"><option value="all">All functions</option><option value="mapped" ${state.mapped==='mapped'?'selected':''}>With behavior links</option><option value="unmapped" ${state.mapped==='unmapped'?'selected':''}>Without behavior links</option><option value="classified" ${state.mapped==='classified'?'selected':''}>Classified support</option></select><button class="button" data-clear>Clear filters</button></div>`;
  const table=`<div class="scroll-table"><table><thead><tr><th>Address / function</th><th>Body bytes</th><th>Behavior links</th><th>Direct entry calls</th></tr></thead><tbody>${page.rows.map(f=>`<tr><td><button class="feature-button code" data-function="${escapeHTML(f.build)}:${f.entry}">${hex(f.entry)}</button><div class="code">${escapeHTML(f.name)}</div></td><td>${number(f.bytes)}</td><td>${f.behaviors.length?f.behaviors.map(id=>`<button class="text-button code" data-behavior="${escapeHTML(id)}">${escapeHTML(id)}</button>`).join(' · '):f.classification?badge(f.classification.category):'<span class="muted">Unmapped</span>'}</td><td>${f.callers.length} callers · ${f.callees.length} callees</td></tr>`).join('')}</tbody></table>${!fs.length?'<div class="empty">No functions match these filters. Unmapped functions have no known subsystem.</div>':''}</div>${page.footer}`;
  return binaryBoundaries()+panel('Function inventory.', 'Binary totals are global. Subsystem filters show only linked functions; unmapped functions have no assigned subsystem. Calls list only discovered entry-to-entry edges.', controls+table)+panel('Recovered ranges outside discovery.', 'These documented exceptions retain original evidence without altering Ghidra discovery boundaries.', data.binary.builds.flatMap(b=>b.recovered_ranges).map(r=>`<p class="small">${fileLink(r.document)}<br><span class="code">${escapeHTML(r.id)} · ${hex(r.start)}–${hex(r.end)} (exclusive end)</span></p>`).join(''));
}
function openDialog(label, body) {
  $('#detail-label').textContent=label;
  $('#detail-body').innerHTML=body;
  if(!$('#detail').open)$('#detail').showModal();
  $('#detail').scrollTop=0;
}
function detailSection(title, body) {return `<section class="detail-section"><h3>${title}</h3>${body}</section>`;}
function openBehavior(id) {
  const b=behaviorById.get(id);if(!b)return;
  const stages=Object.entries(b.status).map(([stage,value])=>`<div><small>${stageNames[stage] || titleCase(stage)}</small>${b.kind==='native_policy'&&['comparison','replacement'].includes(stage)?'<span class="badge">N/A</span>':statusBadge(stage,value)}</div>`).join('');
  const review=b.reconciliation;
  const importance=detailSection('Planning importance',`${importanceBadge(b)}${b.importance?`<p>${escapeHTML(b.importance.reason)}</p><p class="small">${escapeHTML(importanceTypes[b.importance.work_type])}. Planning judgment for native rendering; no effort or completion fraction. ${b.importance.review_state==='current'?'Reviewed contract scope matches.':'Contract scope changed; renew the importance review.'}</p>`:'<p class="small">Importance has been reviewed for the rendering track. Other contracts remain unrated.</p>'}`);
  const task=b.tranche_task;
  const planned=task?detailSection('Planned '+(task.plan_priority||'Critical')+' tranche '+task.tranche_order+' · '+escapeHTML(task.tranche_title),`<p>${escapeHTML(task.next_action)}</p><p class="small">Planned work · ${task.review_state==='current'?'Reviewed contract scope matches.':'Task needs renewed review.'}</p>${trancheTaskDetail(task)}`):'';
  const reconciliation=importance+planned+(review?detailSection('Contract reconciliation',`<p><strong>${escapeHTML(review.label)}</strong> · ${review.freshness==='current'?'Current source review':'Historical source review'}</p><p>${escapeHTML(review.finding)}</p><p><strong>Remaining:</strong> ${escapeHTML(review.remaining)}</p>${review.code_witness?`<p class="small">Source witness: ${fileLink(review.code_witness.path)}:${review.code_witness.line}</p>`:''}<p class="small">Static source/scope review; registered completion and execution validation remain separate. ${fileLink(review.record)}</p>`):'');
  const gaps=b.gaps.map(g=>`<li><strong>${escapeHTML(g==='partial'&&b.tranche_task?.work_review?b.tranche_task.work_review.label:gapLabels[g])}.</strong> ${escapeHTML(g==='partial'?implementationGapAdvice(b):gapActions[g])}</li>`).join('');
  const originals=b.original.map(r=>{
    const owner=data.binary.functions.find(f=>f.build===r.build&&f.ranges.some(([a,z])=>a<=r.address&&r.address<z));
    return `<li><span class="code">${hex(r.address)} · ${escapeHTML(r.build)} · ${escapeHTML(r.relation || 'code')}</span>${owner?` <button class="text-button" data-function="${escapeHTML(owner.build)}:${owner.entry}">Open function →</button>`:''}<br>${escapeHTML(r.scope)}${r.recovered_range?`<br>Recovered-range exception: ${escapeHTML(r.recovered_range)}`:''}</li>`;
  }).join('');
  const evidence=b.evidence.map(id=>{
    const e=evidenceById.get(id);if(!e)return '';
    return `<div class="evidence-item"><strong>${escapeHTML(id)}</strong> ${badge(e.kind)} ${badge(e.freshness)}<p>${escapeHTML(e.scope)}</p>${fileLink(e.record)}${e.changed_sources.length?`<details><summary>${e.changed_sources.length} changed source fingerprints</summary><ul class="detail-list">${e.changed_sources.map(p=>`<li>${escapeHTML(p)}</li>`).join('')}</ul></details>`:''}</div>`;
  }).join('');
  const scenario=data.scenarios.filter(s=>b.scenarios.includes(s.id)).map(s=>`<div class="evidence-item"><strong>${escapeHTML(s.id)}</strong> ${badge(s.level)}<p>${escapeHTML(s.scope)}</p></div>`).join('');
  const files=role=>b[role].length?`<div class="detail-files">${b[role].map(fileLink).join('')}</div>`:'<p class="small muted">No links registered.</p>';
  openDialog('BEHAVIOR / '+b.id, `<h2 id="detail-title" class="detail-title">${escapeHTML(b.title)}</h2><div class="code">${escapeHTML(subsystemName(b.subsystem))} · ${b.kind==='recovered'?'Recovered baseline':'Intentional native policy'} · Confidence: ${escapeHTML(b.confidence)}</div><div class="scope-text"><strong>Recorded scope & remaining boundary</strong><br>${escapeHTML(b.scope)}</div>${b.tranche_task?.work_review?`<div class="badge-row" data-behavior-implementation>${implementationBadge(b)}</div>`:''}<p class="small muted">Registered stages below retain historical accounting. Source assessments distinguish missing code from validation and acceptance work.</p><div class="detail-stages">${stages}</div>${reconciliation}${detailSection('What remains',gaps?`<ul class="status-list">${gaps}</ul>`:'<p class="small">No registered gaps remain within this scope. Remaining behavior beyond this scope is not implied to be complete.')}${detailSection('Original address links',originals?`<ul class="detail-list">${originals}</ul>`:'<p class="small muted">No original address association registered.</p>')}${detailSection('Research / detailed branches',files('documents'))}${detailSection('Implementation',files('implementation'))}${detailSection('Tests',files('tests'))}${detailSection('Evidence',`<p class="small muted">Evidence fingerprints describe source identity. Current comparison also requires the exact behavior scope, all declared dependencies and matching scenarios. Comparison validation: ${escapeHTML(titleCase(b.comparison_freshness))}.</p>`+(evidence||'<p class="small muted">No evidence registered.</p>'))}${detailSection('Integration scenarios',scenario||'<p class="small muted">No scenarios registered.</p>')}`);
}
function openFunction(key) {
  const f=functionById.get(key);if(!f)return;
  const relations=(entries)=>`<div class="call-links">${entries.map(entry=>`<button data-function="${escapeHTML(f.build)}:${entry}">${hex(entry)}</button>`).join('')}</div>`;
  openDialog('ORIGINAL FUNCTION / '+f.build,`<h2 id="detail-title" class="detail-title">${hex(f.entry)}</h2><div class="code">${escapeHTML(f.name)} · ${number(f.bytes)} discovered body bytes</div><div class="scope-text">${f.behaviors.length?'This function has scoped behavior associations. A link does not establish complete function recovery.':'No behavior association is registered for this function. Its purpose and implementation coverage remain unassigned.'}</div>${detailSection('Body ranges',`<div class="code">${f.ranges.map(([a,b])=>`${hex(a)}–${hex(b)} (exclusive end)`).join('<br>')}</div>`)}${detailSection('Linked behaviors',f.behaviors.map(id=>{const b=behaviorById.get(id);return `<p>${featureLink(b)} ${statusBadge('implementation',b.status.implementation)}<br><span class="code">${escapeHTML(id)}</span></p>`;}).join('')||'<p class="small muted">None registered.</p>')}${f.classification?detailSection('Classification',`<p>${escapeHTML(f.classification.category)}: ${escapeHTML(f.classification.reason)}</p>`):''}${detailSection(`Direct callers (${f.callers.length})`,relations(f.callers)||'')}${detailSection(`Direct callees (${f.callees.length})`,relations(f.callees)||'')}<p class="small muted">Only discovered entry-to-entry direct call edges are listed. Interior calls, imports, unresolved indirect flows, and inferred dispatch remain in the inventory and audit. Preferred addresses are build-specific addresses, not stable runtime pointers.</p>`);
}
async function load(refresh=false) {
  $('#refresh').disabled=true;
  $('#refresh').textContent=refresh?'Auditing…':'Loading…';
  try {
    const response=await fetch(refresh?'/api/refresh':'/api/coverage', {method:refresh?'POST':'GET'});
    const next=await response.json();if(!response.ok)throw new Error(next.error || 'Coverage request failed');
    data=next;
    behaviorById=new Map(data.behaviors.map(b=>[b.id,b]));
    functionById=new Map(data.binary.functions.map(f=>[`${f.build}:${f.entry}`,f]));
    evidenceById=new Map(data.evidence.map(e=>[e.id,e]));
    $('#subsystem').innerHTML='<option value="all">All subsystems</option>'+[...new Set(data.behaviors.map(b=>b.subsystem))].sort((a,b)=>subsystemName(a).localeCompare(subsystemName(b))).map(id=>`<option value="${escapeHTML(id)}">${escapeHTML(subsystemName(id))}</option>`).join('');
    $('#track-filter').innerHTML='<option value="all">All tracks</option>'+data.tracks.map(t=>`<option value="${escapeHTML(t.id)}">${escapeHTML(t.title)}</option>`).join('');
    $('#snapshot-time').textContent='Snapshot '+new Date(data.generated_at).toLocaleTimeString([], {hour:'2-digit',minute:'2-digit'});
    $('#snapshot-time').title=data.generated_at;
    $('#provenance').textContent=`${data.binary.builds.map(b=>b.id).join(', ')} · Register ${data.register_sha256.slice(0,12)}`;
    restoreState();render();
    if($('#detail').open)$('#detail').close();
  } catch(error) {
    $('#notice').innerHTML=`<div class="notice error">${escapeHTML(error.message)}. ${data?'The previous snapshot is still displayed.':'Run python3 tools/serve-coverage-ui.py from the repository to start the dashboard.'}</div>`;
    if(!data)$('#view-content').innerHTML='<div class="empty">Coverage data could not be loaded. Refresh to retry.</div>';
  } finally {$('#refresh').disabled=false;$('#refresh').textContent='↻ Refresh audit';}
}
document.addEventListener('click',event=>{
  const target=event.target.closest('button');if(!target||target.disabled)return;
  if(target.dataset.view)go({view:target.dataset.view,query:''});
  if(target.dataset.trackDetail)openTrack(target.dataset.trackDetail);
  if(target.dataset.trackWork){$('#detail').close();go({view:'work',track:target.dataset.trackWork,kind:'all',subsystem:'all',gap:target.dataset.gap || 'all',query:''});}
  if(target.hasAttribute('data-all-behaviors'))go({view:'behaviors',track:'all',subsystem:'all',kind:'all',query:''});
  if(target.dataset.go)go({view:target.dataset.go,gap:target.dataset.gap || 'all',query:''});
  if(target.dataset.subsystem)go({subsystem:target.dataset.subsystem,view:'work',gap:'all',query:''});
  if(target.dataset.gapSelect)go({view:'work',gap:target.dataset.gapSelect,query:''});
  if(target.dataset.behavior)openBehavior(target.dataset.behavior);
  if(target.dataset.function)openFunction(target.dataset.function);
  if(target.dataset.dispatch){const ids=target.dataset.dispatch.split(',');openDialog('DISPATCH ENTRY',`<h2 id="detail-title" class="detail-title">Mapped behaviors</h2>${ids.map(id=>`<p>${featureLink(behaviorById.get(id))} ${statusBadge('implementation',behaviorById.get(id).status.implementation)}</p>`).join('')}`);}
  if(target.dataset.page!==undefined)go({page:Number(target.dataset.page)});
  if(target.hasAttribute('data-clear'))go({query:'',track:'all',subsystem:'all',gap:'all',mapped:'all'});
});
document.addEventListener('change',event=>{
  if(event.target.id.startsWith('importance-')) {
    if(event.target.id==='importance-status')importanceStatus=event.target.value;
    if(event.target.id==='importance-type')importanceType=event.target.value;
    if(event.target.id==='importance-level')importanceLevel=event.target.value;
    const position=window.scrollY;render();window.scrollTo(0,position);return;
  }
  const key={'kind':'kind','track-filter':'track','subsystem':'subsystem','gap-filter':'gap','mapped-filter':'mapped'}[event.target.id];
  if(key)go({[key]:event.target.value});
});
let searchTimer;
document.addEventListener('input',event=>{
  if(event.target.id!=='search')return;
  clearTimeout(searchTimer);
  const value=event.target.value,position=event.target.selectionStart;
  searchTimer=setTimeout(()=>{go({query:value});const input=$('#search');if(input){input.focus();if(input.type!=='search')input.setSelectionRange(position,position);}},180);
});
$('#close-detail').addEventListener('click',()=>$('#detail').close());
$('#detail').addEventListener('click',event=>{if(event.target===$('#detail')){const r=$('#detail').getBoundingClientRect();if(event.clientX<r.left||event.clientX>r.right||event.clientY<r.top||event.clientY>r.bottom)$('#detail').close();}});
$('#refresh').addEventListener('click',()=>load(true));
window.addEventListener('hashchange',()=>{restoreState();render();});
restoreState();load();
