"""Apply derived-source admission only to the declared Native player's children."""
import json,os,subprocess
from pathlib import Path
from coverage_evidence import Reader
from broader_derived_admission import checked_configuration,STARTUP,FLAG,P,POLICY,BASE
from rendering_pilot import require


def derived_binding(base):
    class Binding(base):
        def __init__(self,*args,**kwargs):
            super().__init__(*args,**kwargs)
            self._actual_popen=self.popen
            binary=(self.root/self.plan['player_binary']['path']).resolve()
            def popen(command,*args,**kwargs):
                if command and Path(str(command[0])).is_absolute()and Path(str(command[0])).resolve()==binary:
                    env=dict(kwargs['env']);require(FLAG in env,'Missing explicit preparer selection')
                    config=json.loads(env[FLAG])
                    with Reader(self.root,memoize=True)as reader:
                        checked_configuration(reader,self.root,config['phase'],config['acquisition'])
                        require(config['phase']['record']==self.phase_name and config['acquisition']==self.report,
                                'Foreign derived Native player launch')
                    path=str(self.root/Path(STARTUP).parent)
                    env['PYTHONPATH']=os.pathsep.join([path]+([env['PYTHONPATH']]if env.get('PYTHONPATH')else[]))
                    kwargs['env']=env
                return self._actual_popen(command,*args,**kwargs)
            self.popen=popen
        def restore(self):
            super().restore();subprocess.Popen=self._actual_popen
        def receipt(self):
            try:
                result=super().receipt()
                with Reader(self.root,memoize=True)as reader:
                    case=next(c for c in self.plan['broader_cases']if c['report']==self.report)
                    name=P+case['id']+'-actual-derived-admission-01.json';value=reader.json(name)
                    require(value['success']and value['phase']==dict(record=self.phase_name,sha256=reader.digest(self.phase_name))
                            and value['acquisition_path']==self.report
                            and value['native_only']['source_identity_policy']==POLICY,
                            'Missing exact actual pre-Wine derived admission')
                    result['native_only_derived_admission']=dict(record=name,sha256=reader.digest(name))
                return result
            except FileNotFoundError as error:
                raise ValueError('Explicit startup receipt missing before complete acquisition: '+str(error.filename))from error
    return Binding
