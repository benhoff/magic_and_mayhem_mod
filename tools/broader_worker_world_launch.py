"""Task-local host bindings and an explicit non-merge provenance adaptation."""
import ast
import importlib.util
from pathlib import Path
import sqlite3
import subprocess

from broader_native_host import gpu_environment
from broader_player_launch import LaunchBinding
from rendering_pilot import require


def current_checkout_metadata(raw, filename):
    """Remove only the historical collector's unconditional MERGE_HEAD lookup.

    The prior collector was authored inside a merge. This new task is a normal
    branch. Its existing source_commit remains actual HEAD, while the explicitly
    absent merge parent is null. No controller action or assertion is changed.
    """
    tree=ast.parse(raw,filename=filename)
    changed=[]
    for node in ast.walk(tree):
        if (isinstance(node,ast.Assign) and len(node.targets)==1
                and isinstance(node.targets[0],ast.Subscript)
                and isinstance(node.targets[0].value,ast.Name) and node.targets[0].value.id=='result'
                and isinstance(node.targets[0].slice,ast.Constant) and node.targets[0].slice.value=='source_merge_parent'):
            expected="subprocess.check_output(['git', 'rev-parse', 'MERGE_HEAD'], cwd=ROOT, text=True).strip()"
            require(ast.dump(node.value)==ast.dump(ast.parse(expected,mode='eval').body),
                    'Unreviewed merge-provenance collector expression')
            changed.append(node.lineno)
            node.value=ast.copy_location(ast.Constant(None),node.value)
    require(len(changed)==1,'Missing/duplicate historical merge-provenance field')
    return compile(ast.fix_missing_locations(tree),filename,'exec'),changed[0]


class WorldBinding(LaunchBinding):
    def __init__(self,root,plan,report,provider,phase_name):
        super().__init__(root,plan,report,provider)
        self.phase_name=phase_name
        self.spec=importlib.util.spec_from_file_location
        self.public_shell=False
        self.metadata_adapted=None

    def patch(self,module):
        module=super().patch(module)
        filename=Path(getattr(module,'__file__','/')).name
        if filename in self.plan['controller_modules']:
            if hasattr(module,'PREFIX'):module.PREFIX=self.plan['collector_prefix']
            if hasattr(module,'PLAN'):module.PLAN=self.phase_name
            if filename=='run-scene-attachment.py':module.PLAYER=self.plan['current_player_build_binding']['record']
        return module

    def install(self):
        super().install()
        original_connect=sqlite3.connect
        # All actual aliases of the one hash-pinned player, and its public shell
        # launcher, receive the same declared provider. Other subprocesses keep
        # their original environments and command arguments.
        def popen(command,*args,**kwargs):
            shell=bool(command) and str(command[0])==str(self.root/'run-native.sh')
            direct=bool(command) and Path(str(command[0])).is_absolute() and Path(str(command[0])).resolve()==(self.root/self.plan['player_binary']['path']).resolve()
            if shell or direct:
                env=dict(kwargs['env'])
                if self.provider=='nvidia':env=gpu_environment(env)
                kwargs['env']=env
                self.capture.prepare(env)
                self.public_shell=shell
                self.environments.append({n:env.get(n)for n in('DISPLAY','XAUTHORITY','MESA_LOADER_DRIVER_OVERRIDE','GALLIUM_DRIVER','LIBGL_ALWAYS_SOFTWARE','__GLX_VENDOR_LIBRARY_NAME','__EGL_VENDOR_LIBRARY_FILENAMES')})
            process=self.popen(command,*args,**kwargs)
            return self.capture.wrap(process)if shell or direct else process
        subprocess.Popen=popen
        binding=self
        def spec(name,path,*args,**kwargs):
            result=binding.spec(name,path,*args,**kwargs)
            file=Path(path).resolve()
            if file.is_relative_to(binding.root/'tools'):
                delegate=result.loader
                class Loader:
                    def create_module(self,value):return delegate.create_module(value)
                    def exec_module(self,value):
                        if file.name=='run-rendering-caller-state.py':
                            require(subprocess.run(['git','rev-parse','--verify','MERGE_HEAD'],cwd=binding.root,capture_output=True).returncode!=0,'Current caller metadata adapter requires a non-merge checkout')
                            code,line=current_checkout_metadata(file.read_bytes(),str(file))
                            exec(code,value.__dict__)
                            binding.metadata_adapted=dict(path=str(file.relative_to(binding.root)),line=line,field='source_merge_parent',value=None,reason='Current owned branch has no active merge; actual source_commit remains HEAD')
                        else:delegate.exec_module(value)
                        binding.patch(value)
                    def __getattr__(self,attr):return getattr(delegate,attr)
                result.loader=Loader()
            return result
        importlib.util.spec_from_file_location=spec

    def restore(self):
        importlib.util.spec_from_file_location=self.spec
        super().restore()

    def receipt(self):
        result=super().receipt()
        result['controller_metadata_adaptation']=self.metadata_adapted
        result['public_shell_launcher']=self.public_shell
        if self.public_shell:
            result['actual_public_launcher_environments']=result.pop('actual_qt_environments')
            result['actual_qt_environment_directly_sampled']=False
            result['actual_player_provider_requires_resident_driver']=True
        return result
