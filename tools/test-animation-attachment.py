#!/usr/bin/env python3
"""Bounded No-CD mode-one attachment selection/reset and original controller proof."""
import hashlib, importlib.util, json, struct, subprocess, tempfile
from pathlib import Path
REPO=Path(__file__).resolve().parent.parent
HASH='40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    parent=REPO/'working/tests/animation-attachment';parent.mkdir(parents=True,exist_ok=True)
    out=Path(tempfile.mkdtemp(prefix='run-',dir=parent));print(out,flush=True)
    def verify(phase):
        p=subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],capture_output=True,text=True,timeout=120)
        (out/f'original-{phase}.log').write_text(p.stdout+p.stderr);p.check_returncode()
    verify('before')
    try:
        exe=REPO/'working/game-nocd/Chaos.exe';cfg=REPO/'working/game-clean/CFG/Encrypted/effectani.cfg'
        if sha(exe)!=HASH:raise ValueError('Unsupported executable')
        spec=importlib.util.spec_from_file_location('cfg',REPO/'tools/decode-cfg.py');module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        _,clear=module.decode(cfg.read_bytes());sections={};section=None
        for line in clear.decode('ascii').splitlines():
            line=line.split(';')[0].strip()
            if line.startswith('['):section=line[1:-1].upper();sections[section]={}
            elif '=' in line:
                key,value=line.split('=',1);sections[section][key.strip().upper()]=value.strip()
        recipe=sections['ANI_36'];base=int(recipe['ANIMATIONNO']);ref=recipe['ANIMATIONFILEREF']
        if recipe['SPRITEPRINTER']!='NORMAL' or not ref.startswith('EFFECTS') or not ref[7:].isdigit():raise ValueError('Unsupported recipe printer/reference')
        index=int(ref[7:]);ani=next(p for p in (REPO/'working/game-clean/Sprites').iterdir() if p.name.lower()==ref.lower()+'.ani')
        sources=['tests/animation-attachment-reference.cpp','reconstruction/animation/attachment.cpp','reconstruction/animation/attachment.hpp','reconstruction/animation/no_cd.cpp','reconstruction/animation/no_cd.hpp','tools/test-animation-attachment.py','tools/decode-cfg.py']
        inputs={p:sha(p) for p in [exe,cfg,ani]+[REPO/s for s in sources]}
        helper=out/'reference';command=['g++','-m32','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(REPO/'assets'),'-I'+str(REPO/'reconstruction/animation'),str(REPO/sources[0]),str(REPO/sources[1]),str(REPO/sources[3]),'-o',str(helper)]
        p=subprocess.run(command,capture_output=True,text=True,timeout=60);(out/'compile.log').write_text(p.stdout+p.stderr);p.check_returncode()
        ranges={'mode_change':(0x51ff00,0x5200f1),'mode_getter':(0x51fef0,0x51fef7),'draw':(0x4fa9c2,0x4fab41),'tick_gate':(0x50e670,0x50e697),'recipe_reset':(0x513f50,0x513f9d),'effect_loader':(0x49c540,0x49cc3e),'controller_reset':(0x464c80,0x464ca0)}
        for name,(lo,hi) in ranges.items():
            p=subprocess.run(['objdump','-d','-Mintel',f'--start-address={lo}',f'--stop-address={hi}',str(exe)],check=True,capture_output=True,text=True)
            (out/(name+'.asm')).write_text(p.stdout)
        cases=[]
        for facing in range(8):
            p=subprocess.run([str(helper),str(exe),str(ani),str(base),str(index),str(facing),'64'],capture_output=True,text=True,timeout=10)
            (out/f'facing-{facing}.json').write_text(p.stdout);(out/f'facing-{facing}.stderr').write_text(p.stderr);p.check_returncode()
            cases.append({'facing':facing,**json.loads(p.stdout)})
        if any(sha(p)!=v for p,v in inputs.items()):raise ValueError('Source/input changed')
        report={'scope':'Mode 0 to 1 and 1 to 0 complete original transitions with fixture configuration/asset tables; direct original child ticks. Drawing gates/composition and scheduling are static evidence, not executed whole-scene/gameplay.',
                'recipe':{'entry':36,'asset_index':index,'animation_base':base,'ani':str(ani.relative_to(REPO)),'printer':recipe['SPRITEPRINTER']},
                'input_unchanged':True,'live_validated':False,'source_and_input_sha256':{str(p.relative_to(REPO)):v for p,v in inputs.items()},
                'helper_sha256':sha(helper),'compile_command':command,'static_artifacts':{p.name:sha(p) for p in out.glob('*.asm')},
                'summary':{'facings':8,'states':sum(len(c['rows']) for c in cases),'same_mode_noops':sum(len(c['rows']) for c in cases),'remove_reenter_pairs':8,'all_match':True},'cases':cases}
        (out/'report.json').write_text(json.dumps(report,indent=2)+'\n');print(report['summary'],flush=True)
    finally:verify('after')
if __name__=='__main__':main()
