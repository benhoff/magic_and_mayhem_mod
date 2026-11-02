"""Four finite storage leaves per explicit configuration, never whole-player acceptance."""
from coverage_claims import fingerprint
from rendering_acceptance import requirements, validate
from rendering_normal_acceptance import admit, receipt
from rendering_pilot import require

CONFIGURATIONS=('player-default','player-software')
LEAVES={
 'journey.save-load.save':'Original Save acknowledgement and exact owned persisted bytes.',
 'journey.save-load.load':'Mini and Main Load dispatch, fresh generations and both original display rebuilds.',
 'journey.save-load.restored-world':'Every captured restored viewport independently matches complete owned original history.',
 'journey.pause-preferences.cancel':'Changed draft Cancel restores original values and profile bytes.'}


def progress(plan,reader,cohorts):
    spec=validate(plan);runs=[];sources={};identities=set();domains=set()
    for configuration,pair in cohorts:
        require(configuration in CONFIGURATIONS,'Unknown storage target')
        run=admit(reader,pair,configuration)
        require(run['scenario']=='normal-storage','Storage receipt cannot certify another journey')
        acquired=receipt(reader,pair['acquisition']);identity=acquired['session_identity']
        require(type(identity) is str and len(identity)==32 and all(c in '0123456789abcdef' for c in identity),'Invalid actual session identity')
        require(identity not in identities,'Actual execution reused');identities.add(identity)
        domain=(configuration,tuple(tuple(p) for p in run['world_extents']))
        require(domain not in domains,'Choose one reviewed execution per target/domain');domains.add(domain)
        for n,h in run['sources'].items():
            require(n not in sources or sources[n]==h,'Different current storage source phases');sources[n]=h
        runs.append(dict(run,configuration=configuration))
    require(runs and all(reader.digest(n)==h for n,h in sources.items()),'No current source-bound storage executions')
    sizes={c:{tuple(p) for r in runs if r['configuration']==c for p in r['world_extents']} for c in CONFIGURATIONS}
    rows=[]
    for row in requirements(plan):
        proof=[]
        if row['id'] in LEAVES and sizes[row['configuration']]=={(640,480),(800,600)}:
            proof=[dict(reason=LEAVES[row['id']],execution=r['pair'],provider=r['provider'],actual_world_extents=r['world_extents']) for r in runs if r['configuration']==row['configuration']]
        rows.append(dict(row,state='passed' if proof else 'pending',admitted_execution_evidence=proof))
    require(len(rows)==156 and sum(r['state']=='passed' for r in rows)<=8,'Storage scope/denominator changed')
    return dict(schema=1,kind='native_rendering_storage_acceptance_progress',success=True,
                dossier_identity=fingerprint(dict(spec=spec,requirements=requirements(plan))),sources=sources,
                executions=runs,requirements=rows,passed_requirements=sum(r['state']=='passed' for r in rows),
                pending_requirements=sum(r['state']=='pending' for r in rows),
                decisions={'supported-player':'pending','original-recovery':'pending'},
                historical_failures_retired=False,
                scope='Only four finite storage/Cancel leaves per named target, each with actual World proof at both sizes. Other journeys, operations, gates, original recovery and historical failure causes remain pending. Earlier positive source phases remain historical; no final decision follows.')
