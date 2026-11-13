"""Apply the controlled child scheduling policy only to declared hotspot cases."""
import subprocess
from broader_hotspot_visible_launch import WorldBinding as BaseWorldBinding
from broader_copy_diagnostic_scheduler import CopyDiagnosticScheduler
from rendering_pilot import require


class WorldBinding(BaseWorldBinding):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        require(self.plan['family'] == 'hotspot', 'Scheduled COPY observer requires the hotspot family')
        self.original_popen = self.popen
        self.scheduling = CopyDiagnosticScheduler(self.root, self.root / self.plan['player_binary']['path'],
                                                  self.plan['player_binary']['sha256'],
                                                  self.plan['hotspot_copy_scheduling'])

    def install(self):
        # This is below the existing provider/history/capture adapters. They keep
        # seeing the exact Native argv and select their unchanged environments.
        self.popen = self.scheduling.wrap(self.original_popen)
        super().install()

    def restore(self):
        try:
            super().restore()
        finally:
            subprocess.Popen = self.original_popen
            self.popen = self.original_popen

    def receipt(self):
        return super().receipt() | dict(copy_diagnostic_scheduling=self.scheduling.receipt())
