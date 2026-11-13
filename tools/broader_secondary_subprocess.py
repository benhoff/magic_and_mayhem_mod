"""Carry the explicit scene choice across Qt's preparer process boundary."""
import hashlib,json,os,sys
from pathlib import Path
from coverage_evidence import Reader,write_report
from broader_secondary_scene_variant import verify_variant,BUILDER
from rendering_pilot import require
P='research/runtime/broader-player-acceptance-results-20261110/'
STARTUP='patches/secondary-scene-python-startup/sitecustomize.py'
HELPER='tools/broader_secondary_subprocess.py'
FLAG='MNM_SECONDARY_SCENE_PREPARER'


def configuration(root,phase,report,digest):
    require(report.startswith('research/runtime/')and '..'not in Path(report).parts,'Unsafe acquisition path')
    cases=[c for c in phase['broader_cases']if c['report']==report]
    require(len(cases)==1 and phase['family']=='interaction','Unknown subprocess interaction context')
    selected=phase['subprocess_scene_binding']
    require(selected['startup']==STARTUP and selected['helper']==HELPER
            and selected['source_waivers']==[] and all(digest(n)==h for n,h in selected['sources'].items()),
            'Subprocess binding source changed')
    return dict(root=str(root),acquisition=report,selection=P+cases[0]['id']+'-subprocess-scene-selection-01.json')


def startup(root,config):
    root=Path(root);require(Path(sys.argv[0]).resolve()==root/'tools/prepare-menu-observer.py',
                            'Only the actual Qt preparer selects a scene variant')
    with Reader(root,memoize=True)as reader:
        require(reader.digest(config['phase']['record'])==config['phase']['sha256'],'Preparer phase changed')
        phase=reader.json(config['phase']['record']);expected=configuration(root,phase,config['acquisition'],reader.digest)
        require(config==expected|{'phase':config['phase']},'Foreign subprocess configuration')
        proof=verify_variant(reader,root,phase['scene_runtime_variant'],materialized=True)
        sources={n:reader.digest(n)for n in [STARTUP,HELPER]}
    import launch_build_cache
    original=launch_build_cache.build_adapter
    selected=False
    def adapter(builder,build,*,options=None,root=launch_build_cache.ROOT):
        nonlocal selected
        if builder!=BUILDER:return original(builder,build,options=options,root=root)
        require(options is None and Path(root).resolve()==Path(config['root']).resolve(),
                'Unexpected scene builder options/root')
        with Reader(Path(config['root']),memoize=True)as reader:
            actual=verify_variant(reader,Path(config['root']),phase['scene_runtime_variant'],materialized=True)
            require(actual==proof,'Scene changed during preparer execution')
        if not selected:
            name=config['selection'];require(not(Path(config['root'])/name).exists(),'Never repeat scene staging automatically')
            q=dict(schema=1,kind='actual_Qt_preparer_explicit_scene_selection',success=True,
                phase=config['phase'],acquisition_path=config['acquisition'],build=phase['scene_runtime_variant'],
                proof=proof,sources=sources,sources_stable=True,preparer=str(Path(sys.argv[0]).resolve()),
                pid=os.getpid(),builder=BUILDER,only_scene_builder_replaced=True,
                physical_base_sources_changed=False,source_waivers=[],Original_started_by_selection=False)
            write_report(Path(config['root']),name,(json.dumps(q,indent=2)+'\n').encode());selected=True
        return Path(proof['physical_scene'])
    launch_build_cache.build_adapter=adapter


def verify_selection(reader,root,phase_ref,report):
    require(reader.digest(phase_ref['record'])==phase_ref['sha256'],'Subprocess phase changed')
    phase=reader.json(phase_ref['record']);config=configuration(root,phase,report,reader.digest)
    name=config['selection'];q=reader.json(name)
    require(q['success'] and q['kind']=='actual_Qt_preparer_explicit_scene_selection'
            and q['phase']==phase_ref and q['acquisition_path']==report
            and q['build']==phase['scene_runtime_variant'] and q['builder']==BUILDER
            and q['preparer']==str(Path(root)/'tools/prepare-menu-observer.py')
            and q['only_scene_builder_replaced'] is True and q['physical_base_sources_changed'] is False
            and q['source_waivers']==[] and q['Original_started_by_selection'] is False
            and all(reader.digest(n)==h for n,h in q['sources'].items()),'Actual preparer selection differs')
    proof=verify_variant(reader,root,phase['scene_runtime_variant'])
    require({k:v for k,v in proof.items()if k!='materialized_checked'}==
            {k:v for k,v in q['proof'].items()if k!='materialized_checked'}and q['proof']['materialized_checked'],
            'Actual preparer selected another binary or recipe')
    return dict(record=name,sha256=reader.digest(name)),q['proof']


def subprocess_binding(base):
    class Binding(base):
        def install(self):
            super().install()
            import subprocess
            parent=subprocess.Popen
            binary=(self.root/self.plan['player_binary']['path']).resolve()
            phase_ref=dict(record=self.phase_name,sha256=hashlib.sha256(b'').hexdigest())
            with Reader(self.root,memoize=True)as reader:
                phase_ref['sha256']=reader.digest(self.phase_name)
                config=configuration(self.root,self.plan,self.report,reader.digest)|{'phase':phase_ref}
            def popen(command,*args,**kwargs):
                if command and Path(str(command[0])).is_absolute()and Path(str(command[0])).resolve()==binary:
                    env=dict(kwargs['env']);require(FLAG not in env,'Conflicting subprocess scene selection')
                    startup_path=str(self.root/Path(STARTUP).parent)
                    env[FLAG]=json.dumps(config,separators=(',',':'))
                    env['PYTHONPATH']=os.pathsep.join([startup_path,str(self.root/'tools')]+([env['PYTHONPATH']]if env.get('PYTHONPATH')else[]))
                    kwargs['env']=env
                return parent(command,*args,**kwargs)
            subprocess.Popen=popen
        def receipt(self):
            with Reader(self.root,memoize=True)as reader:
                phase_ref=dict(record=self.phase_name,sha256=reader.digest(self.phase_name))
                reference,proof=verify_selection(reader,self.root,phase_ref,self.report)
            # The inherited variant receipt requires actual, verified selection.
            # Populate it from the child process receipt, never a parent mock call.
            require(not self._variant_selections,'Unexpected parent-side scene selection')
            self._variant_selections.append(dict(module='tools/prepare-menu-observer.py',proof=proof,
                                                 actual_subprocess_selection=reference))
            result=super().receipt();result['scene_runtime_variant']['actual_subprocess_selection']=reference
            return result
    return Binding
