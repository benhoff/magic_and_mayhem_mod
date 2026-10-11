#!/usr/bin/env python3
"""Pin and export the NoCD higher byte text consumers without changing originals."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import tempfile

ROOT=Path(__file__).resolve().parents[1]
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
ENTRIES=('4a5ce0','4a6190','4a6630')


def main():
    out=Path(tempfile.mkdtemp(prefix='font-text-',dir=ROOT/'working/decompiled'))
    sources={str(p.relative_to(ROOT)):hashlib.sha256(p.read_bytes()).hexdigest()
             for p in (Path(__file__).resolve(),ROOT/'tools/ghidra/ExportSftSupport.java')}
    print(out,flush=True)
    subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    try:
        exe=ROOT/'original/Arcane_Nocd/Chaos.exe'
        if hashlib.sha256(exe.read_bytes()).hexdigest()!=HASH:
            raise ValueError('Unsupported original executable')
        (out/'text.asm').write_bytes(subprocess.check_output(['objdump','-d','-Mintel',
            '--start-address=0x4a5ce0','--stop-address=0x4a7000',exe]))
        launcher=next((ROOT/'working/toolchain').glob('ghidra*/support/analyzeHeadless'))
        env={**os.environ,'XDG_CONFIG_HOME':str(out/'config'),'XDG_CACHE_HOME':str(out/'cache'),
             'GHIDRA_HEADLESS_JAVA_OPTIONS':f'-Dapplication.cachedir={out / "cache"} -Dapplication.tempdir={out / "tmp"}'}
        with (out/'ghidra.log').open('x') as log:
            subprocess.run([launcher,ROOT/'working/decompiled/nocd-96ofmeq_/project','MagicMayhem',
                '-process','Chaos.exe','-readOnly','-noanalysis','-scriptPath',ROOT/'tools/ghidra',
                '-postScript','ExportSftSupport.java',out/'pseudo',*ENTRIES,'-max-cpu','2'],
                env=env,stdout=log,stderr=subprocess.STDOUT,check=True,timeout=180)
        artifacts={str(p.relative_to(out)):hashlib.sha256(p.read_bytes()).hexdigest()
                   for p in out.rglob('*') if p.suffix in ('.asm','.c','.refs')}
        if not all('pseudo/00'+e+'.c' in artifacts for e in ENTRIES):
            raise ValueError('Missing text consumer export')
        report={'success':True,'scope':'Static higher byte text consumer disassembly/decompilation only; no behavioral execution or takeover.',
                'source_executable_sha256':HASH,'sources':sources,'artifacts':artifacts,
                'sources_stable':all(hashlib.sha256((ROOT/n).read_bytes()).hexdigest()==h for n,h in sources.items())}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    finally:
        subprocess.run([ROOT/'tools/original-manifest.sh','verify'],check=True)
    print(out/'report.json',flush=True)


if __name__=='__main__':
    main()
