'use strict';

const $ = selector => document.querySelector(selector);
const escapeHTML = value => String(value ?? '').replace(/[&<>"']/g, c => ({'&':'&amp;', '<':'&lt;', '>':'&gt;', '"':'&quot;', "'":'&#39;'}[c]));
const number = value => Number(value || 0).toLocaleString();
const percent = (count, total) => total ? `${(100 * count / total).toFixed(1)}%` : 'N/A';
const hex = value => `0x${Number(value).toString(16).padStart(8, '0')}`;
const titleCase = value => String(value).replaceAll('_', ' ').replace(/\b\w/g, c => c.toUpperCase());
const labels = {rendering:'Native rendering', tracks:'Major tracks', overview:'Detailed coverage', work:'Work remaining', behaviors:'Behavior register', binary:'Binary explorer'};
const subsystemNames = {terrain_lighting:'Terrain & effects', audio_reconstruction:'Original audio', native_audio:'Native audio', rendering_reconstruction:'Original rendering', native_rendering:'Native rendering', qt_menus:'Menus', qt_services:'Menu services', menu_adapters:'Menu adapters', runtime_and_hosting:'Runtime & hosting', gameplay_gaps:'Core gameplay', intentional_gameplay_changes:'Gameplay changes', spell_dispatch:'Spell dispatch', remaining_engine_boundaries:'Engine boundaries', native_world:'Native world'};
const subsystemName = id => subsystemNames[id] || titleCase(id);
const gapLabels = {unimplemented:'No implementation', partial:'Partial implementation', understanding:'Understanding incomplete', comparison:'No original comparison', stale:'Evidence needs rerun', tests:'Tests missing', integration:'No integration', replacement:'No live replacement', validation:'Current validation pending'};
const gapActions = {unimplemented:'Implement the documented behavior and add bounded tests.', partial:'Finish the remaining branches and failure paths within the documented scope; review the research links for the exact boundary.', understanding:'Recover the remaining behavior within scope and record evidence for unresolved paths.', comparison:'Run an independent original comparison for the recovered scope; retain controlled-input limits.', stale:'Rerun the affected comparison or integration against current sources under a new evidence ID.', tests:'Add meaningful tests for the implemented scope.', integration:'Exercise the behavior in a bounded integrated scenario and record its evidence.', replacement:'Establish scoped live equivalence and verify that original work is bypassed before claiming replacement.', validation:'Review the exact scope and dependencies, then register a new execution result bound to the behavior and exercised scenarios.'};
let data;
let state = {view:'rendering', kind:'recovered', track:'all', subsystem:'all', gap:'all', query:'', page:0, mapped:'all'};
let behaviorById = new Map(), functionById = new Map(), evidenceById = new Map();

function restoreState() {
  const params = new URLSearchParams(location.hash.slice(1));
  for (const key of Object.keys(state)) if (params.has(key)) state[key] = params.get(key);
  if (!labels[state.view]) state.view = 'rendering';
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
function badge(value) {return `<span class="badge ${escapeHTML(value)}">${escapeHTML(titleCase(value))}</span>`;}
function fileLink(path) {return `<a target="_blank" rel="noopener" href="/files?path=${encodeURIComponent(path)}">${escapeHTML(path)}</a>`;}
function featureLink(b) {return `<button class="feature-button" data-behavior="${escapeHTML(b.id)}">${escapeHTML(b.title)}</button>`;}
function stack(rows, large=false) {
  const total = rows.length || 1;
  return `<div class="stack ${large ? 'implementation-stack' : ''}" aria-label="${rows.filter(b => b.status.implementation==='scoped').length} scoped, ${rows.filter(b => b.status.implementation==='partial').length} partial, ${rows.filter(b => b.status.implementation==='none').length} missing">${['scoped','partial','none'].map(s => `<span class="${s}" style="width:${100 * rows.filter(b=>b.status.implementation===s).length/total}%"></span>`).join('')}</div>`;
}
function legend() {return '<div class="legend"><span><i></i>Scoped implementation</span><span><i class="partial"></i>Partial</span><span><i class="none"></i>Missing</span></div>';}
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
  $('#detail-filters').hidden = ['tracks','rendering'].includes(state.view);
  $('.scope-control').hidden = ['tracks','rendering'].includes(state.view);
  $('nav').querySelectorAll('button').forEach(b => {b.classList.toggle('active', b.dataset.view===state.view); if(b.dataset.view===state.view)b.setAttribute('aria-current','page');else b.removeAttribute('aria-current');});
  $('#crumb').textContent = labels[state.view];
  const headings = {rendering:['Native rendering, in focus.', 'Follow native World rendering from independent comparison and shadow presentation to verified live takeover.'], tracks:['The rebuild, track by track.', 'What works today, what is partly built, and what needs to happen next.'], overview:['Detailed coverage.', 'Inspect binary links, registered implementation stages, and independent evidence.'], work:['What remains to be done.', 'Explore missing features, partial implementations, and validation boundaries.'], behaviors:['Every behavior has a boundary.', 'Trace documented scope through native code, tests, and original evidence.'], binary:['Start with the original engine.', 'Browse discovered functions and follow their explicit behavior links.']};
  $('#page-title').textContent = headings[state.view][0];
  $('#page-description').textContent = headings[state.view][1];
  $('#view-content').innerHTML = ({rendering:renderingView, tracks:tracksView, overview:overview, work:workView, behaviors:behaviorView, binary:binaryView}[state.view])();
  const errors = data.audit.errors || [];
  $('#notice').innerHTML = errors.length ? `<div class="notice error"><strong>Audit errors: displayed claims need review.</strong> ${errors.map(escapeHTML).join(' · ')}</div>` : '';
  if (!['binary','tracks','rendering'].includes(state.view)) $('#notice').innerHTML += `<div class="notice"><strong>${escapeHTML(state.kind==='recovered'?'Recovered baseline':state.kind==='native_policy'?'Intentional native policies & tooling':'All registered behaviors')}${state.track !== 'all' ? ` · ${escapeHTML(data.tracks.find(t=>t.id===state.track).title)}` : ''}${state.subsystem !== 'all' ? ` · ${escapeHTML(subsystemName(state.subsystem))}` : ''}.</strong> Each behavior counts once. “Scoped” means implemented within its documented boundary. These percentages do not measure whole-game completion${state.kind === 'native_policy' ? '; original comparison and replacement are not applicable to native policies' : ''}.</div>`;
}
const milestoneLabels = {demonstrated:'Demonstrated in scope',in_progress:'In progress',not_started:'Not started',unknown:'Mapping needs review'};
function renderingView() {
  const t=data.tracks.find(t=>t.id==='rendering');
  if(!t?.focus)return '<div class="empty">Native rendering roadmap is not registered.</div>';
  const focus=t.focus, demonstrated=t.milestones.filter(m=>m.state==='demonstrated');
  const contractList=ids=>ids.map(id=>{
    const b=behaviorById.get(id);
    return b ? `<div class="render-contract">${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${b.kind==='recovered'?'Recovered baseline':'Native policy'}</div><div class="badge-row">${badge(b.status.implementation)} ${badge(b.status.integration)} ${badge(b.status.replacement==='scoped_live'?'Scoped live bypass':'No live bypass claim')}</div></div>` : `<p class="notice error">Missing registered mapping: ${escapeHTML(id)}</p>`;
  }).join('');
  const summary=`<div class="track-summary"><div><strong>${t.progress.count} / ${t.progress.total}</strong><span>Named milestones demonstrated</span></div><div><strong>${t.counts.in_progress||0}</strong><span>Milestones in progress</span></div><div><strong>${t.current} / ${demonstrated.length}</strong><span>Demonstrations with current validation</span></div><div><strong>${t.counts.not_started||0}</strong><span>Milestones not started</span></div></div><p class="small roadmap-note">These are equally counted, bounded milestones. They do not measure effort or complete rendering parity. Historical demonstrations remain separate from scope-bound current validation.</p>`;
  const next=panel('Next engineering task',focus.next_title,`<p>${escapeHTML(focus.next_task)}</p>${contractList(focus.next_behaviors)}`);
  const roadmap=`<div class="render-roadmap">${t.milestones.map(m=>`<article class="panel render-milestone" data-render-milestone="${escapeHTML(m.id)}"><div class="milestone-heading"><h3>${escapeHTML(m.title)}</h3>${badge(milestoneLabels[m.state])}</div><p class="small">${escapeHTML(m.scope)}</p><p class="small muted">${m.state==='demonstrated'?(m.current?'Scope-bound current validation for every required contract.':'Historical demonstration; scope-bound current validation remains pending.'):'Remaining: '+escapeHTML(m.remaining)}</p>${m.missing.length?`<p class="notice error">Missing mappings: ${m.missing.map(escapeHTML).join(', ')}</p>`:''}<details><summary>Contracts, stages & evidence (${m.behaviors.length})</summary>${contractList(m.behaviors)}<p class="small muted">Open a contract for its exact scope, independent stages, source hashes and evidence records.</p></details></article>`).join('')}</div>`;
  return `<div class="notice render-boundary"><strong>Current replacement boundary.</strong> ${escapeHTML(focus.boundary)}</div>`+summary+next+panel('Native rendering roadmap','Implementation, original comparison, integration and live bypass remain separate.',roadmap,`<button class="button" data-track-detail="rendering">Inspect milestone requirements</button>`)+panel('Research & remaining work','The authoritative release gates and current blockers.',`<div class="detail-files">${focus.documents.map(fileLink).join('')}</div><button class="text-button" data-track-work="rendering">Browse rendering behavior gaps →</button>`);
}
function tracksView() {
  const active=data.tracks.filter(t=>!t.planned), milestones=active.flatMap(t=>t.milestones);
  const count=s=>milestones.filter(m=>m.state===s).length;
  const intro=`<div class="track-summary"><div><strong>${active.length}</strong><span>Engine tracks</span></div><div><strong>${count('demonstrated')}</strong><span>Milestones demonstrated</span></div><div><strong>${count('in_progress')}</strong><span>Milestones partly built</span></div><div><strong>${count('not_started')+count('unknown')}</strong><span>Milestones still open</span></div></div><p class="small roadmap-note">Each track uses a named milestone checklist. Percentages count demonstrated milestones equally, within their stated scope. Partial work is visible separately. They measure this roadmap, not effort or whole-game completion.</p>`;
  const cards=tracks=>`<div class="track-grid">${tracks.map(t=>{
    const ready=t.milestones.filter(m=>m.state==='demonstrated'),partial=t.milestones.filter(m=>m.state==='in_progress'),next=t.milestones.find(m=>m.id===t.next);
    const stage=t.planned?'Planned':ready.length?'Bounded milestones demonstrated':partial.length?'Foundations in progress':'Awaiting implementation';
    return `<article class="track-card" data-track-card="${escapeHTML(t.id)}"><div class="track-card-head"><h2><button class="feature-button" data-track-detail="${escapeHTML(t.id)}">${escapeHTML(t.title)} ↗</button></h2><span class="track-percent">${Math.round(t.progress.percent)}<small>%</small></span></div><p class="track-goal">${escapeHTML(t.goal)}</p><div class="roadmap-stack" aria-label="${t.progress.count} of ${t.progress.total} milestones demonstrated"><span class="demonstrated" style="width:${t.progress.percent}%"></span><span class="in_progress" style="width:${100*(t.counts.in_progress||0)/t.progress.total}%"></span></div><div class="track-caption"><strong>${t.progress.count} / ${t.progress.total} demonstrated</strong><span>${partial.length} in progress</span></div><p class="track-stage">${escapeHTML(stage)}</p><div class="track-now"><h3>${ready.length?'Demonstrated today':'Partly built today'}</h3>${ready.length?`<ul>${ready.map(m=>`<li>${escapeHTML(m.title)}</li>`).join('')}</ul>`:partial.length?`<ul>${partial.map(m=>`<li>${escapeHTML(m.title)}</li>`).join('')}</ul>`:'<p>These milestones have no registered implementation yet.</p>'}</div><div class="track-next"><h3>Next milestone</h3><p><strong>${escapeHTML(next?.title || 'All named milestones demonstrated')}</strong></p><p>${escapeHTML(next?.remaining || 'Review scope before expanding the roadmap.')}</p></div>${ready.length?`<p class="small freshness-note">${t.current} / ${ready.length} demonstrated milestones have scope-bound current validation.${t.current<ready.length?' Earlier demonstrations remain historical; affected evidence needs rerunning.':''}</p>`:''}<button class="text-button track-open" data-track-detail="${escapeHTML(t.id)}">Milestones, gaps & evidence →</button></article>`;
  }).join('')}</div>`;
  const unassigned=data.behaviors.filter(b=>!data.tracks.some(t=>t.behaviors.includes(b.id)));
  return intro+cards(active)+panel('Planned gameplay changes.', 'These remain separate from recovering and validating the original engine.',cards(data.tracks.filter(t=>t.planned)))+panel('Supporting work & roadmap scope.', 'The roadmap is curated; newly registered behavior still appears in the detailed coverage views.',`<p class="small">${unassigned.length} registered behaviors sit outside these product tracks, including tooling and supporting boundaries. The ${data.behaviors.filter(b=>b.subsystem==='spell_dispatch').length} spell dispatch entries contribute one spell milestone, rather than outweighing every other track.</p><button class="text-button" data-all-behaviors>Browse all registered work →</button>`);
}
function openTrack(id) {
  const t=data.tracks.find(t=>t.id===id);if(!t)return;
  const goals=t.milestones.map(m=>`<section class="milestone-detail"><div class="milestone-heading"><h3>${escapeHTML(m.title)}</h3>${badge(milestoneLabels[m.state])}</div><p>${escapeHTML(m.scope)}</p>${m.state==='demonstrated'?`<p class="small">${m.current?'Scope-bound current validation is available for every required contract.':'Recorded demonstration; one or more required execution records are historical. Rerun affected evidence before claiming current-source validation.'}</p>`:`<p class="milestone-remaining"><strong>Remaining:</strong> ${escapeHTML(m.remaining)}</p>`}${m.missing.length?`<p class="notice error">Missing registered mappings: ${m.missing.map(escapeHTML).join(', ')}</p>`:''}<details><summary>Required contracts & evidence (${m.behaviors.length})</summary><div class="milestone-contracts">${m.checks.map(c=>{const b=behaviorById.get(c.behavior);return `<div>${featureLink(b)} ${badge(c.met?'Requirement met':'Open')}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(Object.entries(c.expected).map(([s,v])=>`${titleCase(s)}: ${Array.isArray(v)?v.join(' / '):v}`).join(' · '))}</div></div>`;}).join('')}</div></details></section>`).join('');
  openDialog('MAJOR TRACK / '+t.id,`<h2 id="detail-title" class="detail-title">${escapeHTML(t.title)}</h2><p>${escapeHTML(t.goal)}</p><div class="scope-text"><strong>${t.progress.count} of ${t.progress.total} named milestones demonstrated (${Math.round(t.progress.percent)}%).</strong> Every required contract must meet its explicit stages and have execution evidence. Partial work earns no invented fraction. Previews, headless comparisons and original observation retain their own scope; observation does not mean native replacement.</div>${goals}<button class="button" data-track-work="${escapeHTML(t.id)}">Browse this track’s behavior gaps →</button>`);
}
function overview() {
  const rows = scopedBehaviors(), s = summarize(rows);
  const mapped = data.binary.builds.reduce((n,b)=>n+b.functions.count,0), functions = data.binary.functions.length;
  const metrics = `<div class="metric-grid">${metric('FUNCTIONS WITH BEHAVIOR LINKS',mapped,functions,'Global binary inventory. A link may cover only part of a function.','data-go="binary"','functions')}${metric('SCOPED IMPLEMENTATION',s.scoped,s.total,`${s.partial} partial · ${s.missing} missing in this checklist.`,'data-go="behaviors"','behaviors')}${metric('ORIGINAL COMPARISON',s.compared,s.recovered,`${s.current} have only current comparison fingerprints. Historical results retain their scope.`,'data-go="work" data-gap="comparison"','baseline behaviors')}${metric('SCOPED LIVE REPLACEMENT',s.replaced,s.recovered,'Requires live equivalence and evidence that original work was bypassed.','data-go="work" data-gap="replacement"','baseline behaviors')}</div>`;
  const stage = (label, count, total) => `<div class="stage-row"><span>${label}</span><div class="stack"><span style="width:${total?count/total*100:0}%"></span></div><strong>${percent(count,total)}</strong></div>`;
  const progress = panel('Implementation has layers.', `${s.total} registered behaviors in the selected scope.`, `${stack(rows,true)}${legend()}<div class="stat-list"><div><strong>${s.scoped}</strong><span>Scoped · ${percent(s.scoped,s.total)}</span></div><div><strong>${s.partial}</strong><span>Partial · ${percent(s.partial,s.total)}</span></div><div><strong>${s.missing}</strong><span>Missing · ${percent(s.missing,s.total)}</span></div></div><p class="stage-description">Partial behavior is shown separately. Its unfinished scope receives no invented completion weight. Scoped implementations may still have comparison, integration, or replacement work outstanding.</p>`);
  const stages = panel('Independent milestones.', 'Implementation and validation advance separately.', `${stage('Scoped understanding',s.understood,s.total)}${stage('Any integration',s.integrated,s.total)}${stage('Current comparison',s.current,s.recovered)}${stage('Live replacement',s.replaced,s.recovered)}<p class="stage-description">Any integration includes headless tests, previews, and live observation. Current comparison requires an exact scope binding and complete current source provenance. Historical results remain separate. Comparison and replacement use recovered behaviors as the denominator.</p>`);
  return metrics + `<div class="two-col">${progress}${stages}</div>` + subsystemTable(rows) + workSummary(rows) + binaryBoundaries();
}
function subsystemTable(rows) {
  const groups = [...new Set(rows.map(b=>b.subsystem))].sort((a,b)=>subsystemName(a).localeCompare(subsystemName(b)));
  const table = `<div class="scroll-table"><table><thead><tr><th>Subsystem</th><th>Behaviors</th><th>Implementation within scope</th><th>Compared / baseline</th><th>Still partial / missing</th></tr></thead><tbody>${groups.map(id=>{
    const bs=rows.filter(b=>b.subsystem===id), s=summarize(bs);
    return `<tr><td><button class="subsystem-button" data-subsystem="${escapeHTML(id)}">${escapeHTML(subsystemName(id))} ↗</button></td><td>${s.total}</td><td class="bar-cell">${stack(bs)}<div class="bar-caption"><span>${s.scoped} scoped</span><span>${percent(s.scoped,s.total)}</span></div></td><td>${s.recovered ? `${s.compared} / ${s.recovered}<div class="code">${s.current} current fingerprints</div>` : 'N/A · native policy'}</td><td>${badge(`${s.partial} partial`)} ${badge(`${s.missing} missing`)}</td></tr>`;
  }).join('')}</tbody></table></div>`;
  return panel('Subsystem coverage.', 'Select a subsystem to inspect its partial features and remaining milestones.', groups.length?table:'<div class="empty">No behaviors in this scope.</div>',legend());
}
function workSummary(rows) {
  const partial=rows.filter(b=>b.status.implementation==='partial'), missing=rows.filter(b=>b.status.implementation==='none');
  const list=items=>items.slice(0,5).map(b=>`<tr><td>${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(subsystemName(b.subsystem))}</div></td><td>${badge(b.status.implementation)}</td></tr>`).join('');
  return `<div class="two-col">${panel(`${partial.length} partially implemented.`, 'Existing code has remaining branches or boundaries.', `<table><tbody>${list(partial)}</tbody></table>`, '<button class="text-button" data-go="work" data-gap="partial">See all →</button>')}${panel(`${missing.length} waiting for implementation.`, 'Recorded behaviors without an implementation claim.', `<table><tbody>${list(missing)}</tbody></table>`, '<button class="text-button" data-go="work" data-gap="unimplemented">See all →</button>')}</div>`;
}
function workView() {
  const rows = scopedBehaviors();
  const chips = ['partial','unimplemented','comparison','stale'].map(g=>{
    const n=rows.filter(b=>b.gaps.includes(g)).length;
    const denominator=g==='comparison'?rows.filter(b=>b.kind==='recovered').length:rows.length;
    return `<button class="work-chip ${state.gap===g?'active':''}" data-gap-select="${g}"><strong>${n}</strong><span>${gapLabels[g]}</span><span class="muted">${percent(n,denominator)} of ${g==='comparison'?'baseline':'selected'} behaviors</span></button>`;
  }).join('');
  const filtered = rows.filter(b=>matches(b) && (state.gap==='all' ? b.gaps.length : b.gaps.includes(state.gap)));
  const helper = state.gap !== 'all' ? `<p class="small muted">Next milestone: ${escapeHTML(gapActions[state.gap])} The recorded boundary below is verbatim; specific omitted branches are documented in the research links.</p>` : '<p class="small muted">A behavior can have several gaps. Counts overlap. Missing original comparison does not apply to intentional native policies.</p>';
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
  return `<div class="scroll-table"><table><thead><tr><th>Feature / scope</th><th>Implementation</th>${gaps?'<th>Remaining milestones</th>':'<th>Comparison</th><th>Integration</th>'}</tr></thead><tbody>${page.rows.map(b=>`<tr><td>${featureLink(b)}<div class="code">${escapeHTML(b.id)} · ${escapeHTML(subsystemName(b.subsystem))} · ${b.kind==='recovered'?'baseline':'native policy'}</div>${gaps?`<p class="feature-scope">${escapeHTML(b.scope)}</p>`:''}</td><td>${badge(b.status.implementation)}</td>${gaps?`<td><div class="badge-row">${b.gaps.map(g=>badge(gapLabels[g])).join('')}</div></td>`:`<td>${b.kind==='native_policy'?'N/A':badge(b.status.comparison)}${b.status.comparison==='recorded'?`<div class="badge-row">${badge(b.comparison_freshness)}</div>`:''}</td><td>${badge(b.status.integration)}${Object.keys(b.stale_evidence).length?'<div class="badge-row"><span class="badge stale">Stale evidence</span></div>':''}</td>`}</tr>`).join('')}</tbody></table>${!rows.length?'<div class="empty">No behaviors match these filters.</div>':''}</div>${page.footer}`;
}
function dispatchPanel() {
  const ids = new Set(scopedBehaviors().map(b=>b.id));
  const tables = data.dispatch_tables.filter(t=>state.subsystem==='all' || t.entries.some(e=>e.behaviors.some(b=>ids.has(b))));
  return panel('Dispatch coverage is still bounded.', 'A mapped case may remain unimplemented. Unlinked cases and unknown default paths stay visible.', tables.map(t=>`<div class="dispatch-card"><h3>${escapeHTML(t.id)} ${badge(t.complete?'complete':'incomplete')}</h3><div class="code">${hex(t.original.address)} · ${escapeHTML(t.original.build)}</div><p class="feature-scope">${escapeHTML(t.scope)}</p><a class="inline-link" href="/files?path=${encodeURIComponent(t.document)}" target="_blank" rel="noopener">Read dispatch research ↗</a><div class="dispatch-entries">${t.entries.map(e=>`<button class="${e.behaviors.length?'linked':''}" ${e.behaviors.length?`data-dispatch="${escapeHTML(e.behaviors.join(','))}"`:'disabled'} title="${escapeHTML(e.behaviors.length?e.behaviors.map(id=>`${id}: ${behaviorById.get(id)?.status.implementation || 'unknown'}`).join('; '):'No behavior mapping registered')}">${escapeHTML(e.value)} · ${e.behaviors.length?'mapped':'unlinked'}</button>`).join('')}</div></div>`).join('') || '<p class="muted small">No dispatch tables linked to this subsystem.</p>');
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
  const stages=Object.entries(b.status).map(([stage,value])=>`<div><small>${titleCase(stage)}</small>${b.kind==='native_policy'&&['comparison','replacement'].includes(stage)?'<span class="badge">N/A</span>':badge(value)}</div>`).join('');
  const gaps=b.gaps.map(g=>`<li><strong>${escapeHTML(gapLabels[g])}.</strong> ${escapeHTML(gapActions[g])}</li>`).join('');
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
  openDialog('BEHAVIOR / '+b.id, `<h2 id="detail-title" class="detail-title">${escapeHTML(b.title)}</h2><div class="code">${escapeHTML(subsystemName(b.subsystem))} · ${b.kind==='recovered'?'Recovered baseline':'Intentional native policy'} · Confidence: ${escapeHTML(b.confidence)}</div><div class="scope-text"><strong>Recorded scope & remaining boundary</strong><br>${escapeHTML(b.scope)}</div><div class="detail-stages">${stages}</div>${detailSection('What remains',gaps?`<ul class="status-list">${gaps}</ul>`:'<p class="small">All registered milestones are scoped. Remaining behavior beyond this scope is not implied to be complete.')}${detailSection('Original address links',originals?`<ul class="detail-list">${originals}</ul>`:'<p class="small muted">No original address association registered.</p>')}${detailSection('Research / detailed branches',files('documents'))}${detailSection('Implementation',files('implementation'))}${detailSection('Tests',files('tests'))}${detailSection('Evidence',`<p class="small muted">Evidence fingerprints describe source identity. Current comparison also requires the exact behavior scope, all declared dependencies and matching scenarios. Comparison validation: ${escapeHTML(titleCase(b.comparison_freshness))}.</p>`+(evidence||'<p class="small muted">No evidence registered.</p>'))}${detailSection('Integration scenarios',scenario||'<p class="small muted">No scenarios registered.</p>')}`);
}
function openFunction(key) {
  const f=functionById.get(key);if(!f)return;
  const relations=(entries)=>`<div class="call-links">${entries.map(entry=>`<button data-function="${escapeHTML(f.build)}:${entry}">${hex(entry)}</button>`).join('')}</div>`;
  openDialog('ORIGINAL FUNCTION / '+f.build,`<h2 id="detail-title" class="detail-title">${hex(f.entry)}</h2><div class="code">${escapeHTML(f.name)} · ${number(f.bytes)} discovered body bytes</div><div class="scope-text">${f.behaviors.length?'This function has scoped behavior associations. A link does not establish complete function recovery.':'No behavior association is registered for this function. Its purpose and implementation coverage remain unassigned.'}</div>${detailSection('Body ranges',`<div class="code">${f.ranges.map(([a,b])=>`${hex(a)}–${hex(b)} (exclusive end)`).join('<br>')}</div>`)}${detailSection('Linked behaviors',f.behaviors.map(id=>{const b=behaviorById.get(id);return `<p>${featureLink(b)} ${badge(b.status.implementation)}<br><span class="code">${escapeHTML(id)}</span></p>`;}).join('')||'<p class="small muted">None registered.</p>')}${f.classification?detailSection('Classification',`<p>${escapeHTML(f.classification.category)}: ${escapeHTML(f.classification.reason)}</p>`):''}${detailSection(`Direct callers (${f.callers.length})`,relations(f.callers)||'')}${detailSection(`Direct callees (${f.callees.length})`,relations(f.callees)||'')}<p class="small muted">Only discovered entry-to-entry direct call edges are listed. Interior calls, imports, unresolved indirect flows, and inferred dispatch remain in the inventory and audit. Preferred addresses are build-specific addresses, not stable runtime pointers.</p>`);
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
  if(target.dataset.trackWork){$('#detail').close();go({view:'work',track:target.dataset.trackWork,kind:'all',subsystem:'all',gap:'all',query:''});}
  if(target.hasAttribute('data-all-behaviors'))go({view:'behaviors',track:'all',subsystem:'all',kind:'all',query:''});
  if(target.dataset.go)go({view:target.dataset.go,gap:target.dataset.gap || 'all',query:''});
  if(target.dataset.subsystem)go({subsystem:target.dataset.subsystem,view:'work',gap:'all',query:''});
  if(target.dataset.gapSelect)go({view:'work',gap:target.dataset.gapSelect,query:''});
  if(target.dataset.behavior)openBehavior(target.dataset.behavior);
  if(target.dataset.function)openFunction(target.dataset.function);
  if(target.dataset.dispatch){const ids=target.dataset.dispatch.split(',');openDialog('DISPATCH ENTRY',`<h2 id="detail-title" class="detail-title">Mapped behaviors</h2>${ids.map(id=>`<p>${featureLink(behaviorById.get(id))} ${badge(behaviorById.get(id).status.implementation)}</p>`).join('')}`);}
  if(target.dataset.page!==undefined)go({page:Number(target.dataset.page)});
  if(target.hasAttribute('data-clear'))go({query:'',track:'all',subsystem:'all',gap:'all',mapped:'all'});
});
document.addEventListener('change',event=>{
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
