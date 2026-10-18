#!/usr/bin/env python3
"""Pinned, read-only wizard AI disassembly and optional pseudocode."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
HASH = '40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
RANGES = {'wizard-40b420': [4240416, 4251595], 'wizard-40eed0': [4255440, 4255690], 'wizard-40eff0': [4255728, 4256130], 'wizard-411e50': [4267600, 4267630], 'wizard-416590': [4285840, 4287358], 'wizard-421f70': [4333424, 4335906], 'wizard-423590': [4339088, 4349090], 'wizard-43c610': [4441616, 4442112], 'wizard-43c800': [4442112, 4445200], 'wizard-43d410': [4445200, 4445235], 'wizard-43d440': [4445248, 4447728], 'wizard-4420d0': [4464848, 4465312], 'wizard-4422a0': [4465312, 4466268], 'wizard-442680': [4466304, 4466384], 'wizard-4426d0': [4466384, 4468368], 'wizard-442e90': [4468368, 4468656], 'wizard-442fb0': [4468656, 4469784], 'wizard-443440': [4469824, 4470256], 'wizard-4435f0': [4470256, 4470784], 'wizard-443800': [4470784, 4471040], 'wizard-443900': [4471040, 4474272], 'wizard-4445c0': [4474304, 4474544], 'wizard-4446b0': [4474544, 4475280], 'wizard-444990': [4475280, 4475760], 'wizard-444b70': [4475760, 4477773], 'wizard-445370': [4477808, 4477840], 'wizard-445390': [4477840, 4478448], 'wizard-4455f0': [4478448, 4478720], 'wizard-445700': [4478720, 4480176], 'wizard-445cd0': [4480208, 4480800], 'wizard-445f20': [4480800, 4481360], 'wizard-446150': [4481360, 4481542], 'wizard-446210': [4481552, 4482544], 'wizard-446600': [4482560, 4482800], 'wizard-4466f0': [4482800, 4483104], 'wizard-446820': [4483104, 4483232], 'wizard-4468a0': [4483232, 4484960], 'wizard-446f80': [4484992, 4485232], 'wizard-447070': [4485232, 4485968], 'wizard-447350': [4485968, 4486352], 'wizard-4474d0': [4486352, 4489008], 'wizard-447f50': [4489040, 4489072], 'wizard-447f70': [4489072, 4489392], 'wizard-4480b0': [4489392, 4489776], 'wizard-448230': [4489776, 4492512], 'wizard-448d00': [4492544, 4492576], 'wizard-448d20': [4492576, 4492896], 'wizard-449020': [4493344, 4494092], 'wizard-449310': [4494096, 4496400], 'wizard-449c30': [4496432, 4496672], 'wizard-449d20': [4496672, 4497568], 'wizard-44a0a0': [4497568, 4497752], 'wizard-44a160': [4497760, 4498352], 'wizard-44a3c0': [4498368, 4498384], 'wizard-44a3d0': [4498384, 4498400], 'wizard-44a3e0': [4498400, 4498688], 'wizard-44a500': [4498688, 4500640], 'wizard-44acc0': [4500672, 4501104], 'wizard-44ae70': [4501104, 4501632], 'wizard-44b080': [4501632, 4502100], 'wizard-44b260': [4502112, 4503740], 'wizard-44b8e0': [4503776, 4504208], 'wizard-44ba90': [4504208, 4504736], 'wizard-44bca0': [4504736, 4504893], 'wizard-44bd40': [4504896, 4506720], 'wizard-44c470': [4506736, 4506976], 'wizard-44c560': [4506976, 4507296], 'wizard-44c6a0': [4507296, 4507552], 'wizard-44c7a0': [4507552, 4508899], 'wizard-44cd30': [4508976, 4509264], 'wizard-44ce50': [4509264, 4509696], 'wizard-44d000': [4509696, 4509968], 'wizard-44d120': [4509984, 4511120], 'wizard-44d5b0': [4511152, 4511584], 'wizard-44d760': [4511584, 4512112], 'wizard-44d9f0': [4512240, 4512518], 'wizard-44db10': [4512528, 4514176], 'wizard-44e1a0': [4514208, 4514448], 'wizard-44e290': [4514448, 4515024], 'wizard-44e4d0': [4515024, 4515296], 'wizard-44e5e0': [4515296, 4517936], 'wizard-44f050': [4517968, 4518208], 'wizard-44f140': [4518208, 4518816], 'wizard-44f3a0': [4518816, 4518998], 'wizard-44f460': [4519008, 4520133], 'wizard-44f8e0': [4520160, 4520400], 'wizard-44f9d0': [4520400, 4520688], 'wizard-44faf0': [4520688, 4520870], 'wizard-44fbb0': [4520880, 4522034], 'wizard-450050': [4522064, 4522176], 'wizard-4500c0': [4522176, 4522480], 'wizard-4501f0': [4522480, 4522662], 'wizard-4502b0': [4522672, 4523808], 'wizard-450730': [4523824, 4523904], 'wizard-450780': [4523904, 4524208], 'wizard-4508b0': [4524208, 4524390], 'wizard-450970': [4524400, 4525682], 'wizard-450e90': [4525712, 4525792], 'wizard-450ee0': [4525792, 4526096], 'wizard-451010': [4526096, 4526496], 'wizard-4511a0': [4526496, 4527906], 'wizard-451730': [4527920, 4527952], 'wizard-451750': [4527952, 4528368], 'wizard-4518f0': [4528368, 4528656], 'wizard-451a10': [4528656, 4530576], 'wizard-4521b0': [4530608, 4530864], 'wizard-4522b0': [4530864, 4531408], 'wizard-4524d0': [4531408, 4531920], 'wizard-4526d0': [4531920, 4532656], 'wizard-4529c0': [4532672, 4532752], 'wizard-452a10': [4532752, 4532896], 'wizard-452aa0': [4532896, 4533152], 'wizard-452ba0': [4533152, 4534592], 'wizard-453160': [4534624, 4534928], 'wizard-453290': [4534928, 4535456], 'wizard-4534a0': [4535456, 4536131], 'wizard-453750': [4536144, 4538112], 'wizard-453f20': [4538144, 4538384], 'wizard-454010': [4538384, 4538960], 'wizard-454250': [4538960, 4539216], 'wizard-454350': [4539216, 4540784], 'wizard-454990': [4540816, 4541056], 'wizard-454a80': [4541056, 4541632], 'wizard-454cc0': [4541632, 4541968], 'wizard-454e10': [4541968, 4543552], 'wizard-455460': [4543584, 4543824], 'wizard-455550': [4543824, 4544400], 'wizard-455790': [4544400, 4544688], 'wizard-4558b0': [4544688, 4547584], 'wizard-456420': [4547616, 4547856], 'wizard-456510': [4547856, 4548432], 'wizard-456750': [4548432, 4548720], 'wizard-456870': [4548720, 4550128], 'wizard-456e10': [4550160, 4550400], 'wizard-456f00': [4550400, 4550816], 'wizard-4570a0': [4550816, 4551104], 'wizard-4571c0': [4551104, 4554822], 'wizard-458070': [4554864, 4555104], 'wizard-458160': [4555104, 4555696], 'wizard-4583b0': [4555696, 4556304], 'wizard-458610': [4556304, 4559520], 'wizard-4592c0': [4559552, 4559792], 'wizard-4593b0': [4559792, 4560368], 'wizard-4595f0': [4560368, 4560854], 'wizard-4597e0': [4560864, 4564592], 'wizard-45a690': [4564624, 4564864], 'wizard-45a780': [4564864, 4565440], 'wizard-45a9c0': [4565440, 4565808], 'wizard-45ab30': [4565808, 4567680], 'wizard-45b2a0': [4567712, 4567952], 'wizard-45b390': [4567952, 4568528], 'wizard-45b5d0': [4568528, 4568710], 'wizard-45b690': [4568720, 4569856], 'wizard-45bb10': [4569872, 4569952], 'wizard-45bb60': [4569952, 4570256], 'wizard-45bc90': [4570256, 4570624], 'wizard-45be00': [4570624, 4572976], 'wizard-45c750': [4573008, 4573248], 'wizard-45c840': [4573248, 4573824], 'wizard-45ca80': [4573824, 4574006], 'wizard-45cb40': [4574016, 4575656], 'wizard-45d1c0': [4575680, 4575920], 'wizard-45d2b0': [4575920, 4576224], 'wizard-45d3e0': [4576224, 4576464], 'wizard-45d4d0': [4576464, 4577160], 'wizard-45d790': [4577168, 4577408], 'wizard-45d880': [4577408, 4577664], 'wizard-45d980': [4577664, 4578144], 'wizard-45db60': [4578144, 4579093], 'wizard-45e1e0': [4579808, 4580032], 'wizard-45e2c0': [4580032, 4580832], 'wizard-45e5e0': [4580832, 4581120], 'wizard-45e700': [4581120, 4582928], 'wizard-45ee30': [4582960, 4583200], 'wizard-45ef20': [4583200, 4583776], 'wizard-45f160': [4583776, 4583984], 'wizard-45f230': [4583984, 4586176], 'wizard-45fae0': [4586208, 4586448], 'wizard-45fbd0': [4586448, 4587024], 'wizard-45fe10': [4587024, 4587344], 'wizard-45ff50': [4587344, 4589008], 'wizard-4605f0': [4589040, 4589280], 'wizard-4606e0': [4589280, 4589856], 'wizard-460920': [4589856, 4590288], 'wizard-460ad0': [4590288, 4592976], 'wizard-4615c0': [4593088, 4593328], 'wizard-4616b0': [4593328, 4593904], 'wizard-4618f0': [4593904, 4594696], 'wizard-461c10': [4594704, 4595856], 'wizard-4620b0': [4595888, 4596128], 'wizard-4621a0': [4596128, 4596576], 'wizard-466870': [4614256, 4614320], 'wizard-466980': [4614528, 4614590], 'wizard-52a3a0': [5415840, 5415949], 'spell-admission': [4237648, 4237888], 'cast-command': [5752544, 5753008], 'spell-gate': [5367088, 5367696], 'mana-change': [5275392, 5275680], 'spell-target': [4238592, 4239104], 'task-score': [4254208, 4255440], 'wizard-damage-targets': [4258064, 4258608], 'wizard-summon-next': [4465312, 4466272], 'wizard-summon-prepare': [4464848, 4465312], 'wizard-summon-execute': [4466384, 4468368]}


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--project', type=Path)
    p.add_argument('--headless', type=Path)
    a = p.parse_args()
    a.output.mkdir(parents=True, exist_ok=False)
    manifest = [str(ROOT / 'tools/original-manifest.sh'), 'verify']
    subprocess.run(manifest, check=True)
    try:
        exe = ROOT / 'original/Arcane_Nocd/Chaos.exe'
        if sha(exe) != HASH:
            raise ValueError('Unsupported original executable')
        for name, (start, end) in RANGES.items():
            result = subprocess.run(['objdump', '-d', '-M', 'intel',
                                     f'--start-address={start}',
                                     f'--stop-address={end}', str(exe)],
                                    check=True, capture_output=True)
            (a.output / (name + '.asm')).write_bytes(result.stdout)
        if a.project:
            if not a.headless:
                p.error('--project requires --headless')
            env = dict(os.environ, XDG_CONFIG_HOME=str(a.output / 'config'),
                       XDG_CACHE_HOME=str(a.output / 'cache'),
                       GHIDRA_HEADLESS_JAVA_OPTIONS=
                       f'-Dapplication.cachedir={a.output}/cache -Dapplication.tempdir={a.output}/tmp')
            command = [str(a.headless), str(a.project), 'MagicMayhem',
                       '-process', 'Chaos.exe', '-readOnly', '-noanalysis',
                       '-scriptPath', str(ROOT / 'tools/ghidra'), '-postScript',
                       'ExportPersistenceProgression.java', str(a.output / 'c'),
                       *[f'{v[0]:x}' for v in RANGES.values()], '-max-cpu', '2']
            with (a.output / 'ghidra.log').open('w') as log:
                subprocess.run(command, env=env, stdout=log,
                               stderr=subprocess.STDOUT, check=True, timeout=600)
        if sha(exe) != HASH:
            raise ValueError('Original executable changed')
        report = {'success': True, 'original_sha256': HASH,
                  'scope': 'Static discovery only; no execution equivalence.',
                  'ranges': RANGES,
                  'sources': {'tools/export-wizard-ai.py': sha(Path(__file__)), 'tools/ghidra/ExportPersistenceProgression.java': sha(ROOT/'tools/ghidra/ExportPersistenceProgression.java')},
                  'artifacts': {str(q.relative_to(a.output)): sha(q)
                                for q in a.output.rglob('*')
                                if q.is_file() and (q.suffix in ('.asm', '.c', '.refs'))}}
        (a.output / 'report.json').write_text(json.dumps(report, indent=2) + '\n')
    finally:
        subprocess.run(manifest, check=True)


if __name__ == '__main__':
    main()
