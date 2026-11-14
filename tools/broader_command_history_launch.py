"""Select the already-built bounded command-history recorder at player launch."""
import hashlib
from pathlib import Path
import subprocess
from broader_worker_world_launch import WorldBinding as BaseWorldBinding
from rendering_pilot import require


def history_environment(root, binary, command, environment):
    root, binary = Path(root), Path(binary)
    shell = bool(command) and str(command[0]) == str(root/'run-native.sh')
    direct = bool(command) and Path(str(command[0])).is_absolute() and Path(str(command[0])).resolve() == binary.resolve()
    if not (shell or direct):
        return dict(environment), False
    require(environment.get('MNM_ACCEPTANCE_NORMAL') == '1'
            and environment.get('MNM_ACCEPTANCE_CAMPAIGN') == '1'
            and environment.get('MNM_ACCEPTANCE_CAPTURE_DIRECTORY')
            and environment.get('MNM_ACCEPTANCE_CAPTURE_ID'), 'Missing prospectively declared lossless World capture')
    require(environment.get('MNM_ACCEPTANCE_COMMAND_HISTORY') in (None, '1'), 'Conflicting command-history policy')
    result = dict(environment)
    result['MNM_ACCEPTANCE_COMMAND_HISTORY'] = '1'
    return result, True


class WorldBinding(BaseWorldBinding):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.history_launches = []

    def install(self):
        super().install()
        parent = subprocess.Popen
        binary = self.root/self.plan['player_binary']['path']
        expected = self.plan['player_binary']['sha256']
        def popen(command, *args, **kwargs):
            environment, selected = history_environment(self.root, binary, command, kwargs.get('env', {}))
            if selected:
                require(hashlib.sha256(binary.read_bytes()).hexdigest() == expected, 'Current player changed at history selection')
                kwargs['env'] = environment
                self.history_launches.append(dict(command=list(map(str, command)), policy='command-history-lossless-v1', version=4,
                    row_limit=65536, raw_limit_bytes=8*1024**3, queued_byte_limit=256*1024**2,
                    queued_job_limit=4096, flag=environment['MNM_ACCEPTANCE_COMMAND_HISTORY']))
            return parent(command, *args, **kwargs)
        subprocess.Popen = popen

    def receipt(self):
        result = super().receipt()
        require(len(self.history_launches) == 1, 'Expected exactly one history-enabled player launch')
        result['command_history_selection'] = dict(launches=self.history_launches, native_code_or_limits_changed=False,
            game_actions_or_deadlines_changed=False, old_capture_completed=False,
            scope='Select the existing v4lossless command policy for this exact player only. All native caps,encoding,ownership,actions,independent checks and retirement remain mandatory.')
        return result
