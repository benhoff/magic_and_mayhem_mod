#!/usr/bin/env python3
"""Compare native WZD decoding against an independent Python text parser."""
import argparse
import configparser
import hashlib
import json
from pathlib import Path
import re
import subprocess
import tempfile

REPO=Path(__file__).resolve().parents[1]
GENERAL_NUMBERS='maxhealth maxmana intelligence magicresistance controllimit combatmodifier difficultymodifier aggressiveattack cautiousattack occupypowerpoint collectmana evadedetection scout retreat collectfood protectwizard starttalismans_law starttalismans_neutral starttalismans_chaos'.split()
ACTION_NUMBERS='begintime endtime dependancy waitingtime rating numberofactions health mana sectionid spelltype spelltargetid xposition yposition zposition xtargetposition ytargetposition ztargetposition'.split()
ACTION_STRINGS='nodetype targetnodetype destination spelltarget'.split()


def reference(data):
    # Semicolons inside paired quotes are string bytes; comments outside are removed.
    clean='\n'.join(re.split(r';(?=(?:[^"]*"[^"]*")*[^"]*$)',line,maxsplit=1)[0] for line in data.decode('latin1').splitlines())
    parser=configparser.ConfigParser(interpolation=None,strict=True)
    parser.read_string(clean)
    config={section.lower():dict(parser[section]) for section in parser.sections()}
    def flag(value):
        return None if value is None else value.lower() in ('true','1')
    def string(value):
        return None if value is None else value.removeprefix('"').removesuffix('"')
    general=config['general'];stats={key:int(general[key]) if key in general else None for key in GENERAL_NUMBERS}
    stats.update(name=string(general['name']),wizardanimfile=string(general.get('wizardanimfile')))
    items={};actions={}
    for section,fields in config.items():
        if section.startswith('item_'):
            items[str(int(section[5:]))]={key:flag(fields.get(key)) for key in ('hasit','researched')}
        if section.startswith('action_'):
            value={key:int(fields[key]) if key in fields else None for key in ACTION_NUMBERS}
            value.update({key:string(fields.get(key)) for key in ACTION_STRINGS})
            value['valid']=flag(fields.get('valid'));actions[str(int(section[7:]))]=value
    def selected(section,prefix):
        return {str(int(k[len(prefix):])):flag(v) for k,v in config.get(section,{}).items() if k.startswith(prefix)}
    return dict(general=stats,validwizardfile=flag(config.get('header',{}).get('validwizardfile')),
                items=items,actions=actions,start_spells=selected('start_spells','spell_'),
                start_objects=selected('start_objects','object_'),start_magic_items=selected('start_magic_items','mitem_'),
                config=config,source_sha256=hashlib.sha256(data).hexdigest())


def compare(binary,root,path):
    data=(root/path).read_bytes()
    result=subprocess.run([str(binary),str(root),str(path)],capture_output=True,text=True)
    assert result.returncode==0, f'{path}: {result.stderr}'
    actual=json.loads(result.stdout);expected=reference(data)
    assert actual==expected, f'Native/reference mismatch: {path}'
    digest=hashlib.sha256(json.dumps(actual,sort_keys=True,separators=(',',':')).encode()).hexdigest()
    return {'path':str(path),'bytes':len(data),'source_sha256':actual['source_sha256'],'decoded_sha256':digest,
            'item_records':len(actual['items']),'spell_entries':len(actual['start_spells']),
            'object_entries':len(actual['start_objects']),'magic_item_entries':len(actual['start_magic_items']),
            'action_records':len(actual['actions'])}


def run(binary,installation):
    with tempfile.TemporaryDirectory(prefix='mnm-wizard-') as temp:
        root=Path(temp)
        fixtures=[b'[GENERAL]\nName="Sparse Wizard"\nMaxMana=100\n',
                  b';quoted comment "\r\n[HEADER]\r\nValidWizardFile=TRUE\r\n[GENERAL]\r\nName="A;B" ; inline\r\nMaxMana=0\r\nDifficultyModifier=-50\r\nWizardAnimFile=Greek Female\r\n[ITEM_35]\r\nHasIt=1\r\nResearched=0\r\n[START_SPELLS]\r\nSPELL_92=FALSE\r\nSPELL_1=TRUE\r\n[START_OBJECTS]\r\nOBJECT_71=1\r\n[START_MAGIC_ITEMS]\r\nMITEM_18=TRUE\r\n[future]\r\nopaque=raw value\r\n']
        full=b'[GENERAL]\nName="All Fields"\n'+b''.join(f'{key}={i-5}\n'.encode() for i,key in enumerate(GENERAL_NUMBERS))
        full+=b'[ACTION_03]\nValid=FALSE\n'+b''.join(f'{key}={100+i}\n'.encode() for i,key in enumerate(ACTION_NUMBERS))
        full+=b''.join(f'{key}=Section\n'.encode() for key in ACTION_STRINGS)
        fixtures.append(full)
        for i,data in enumerate(fixtures):
            p=Path(f'{i}.wzd');(root/p).write_bytes(data);compare(binary,root,p)
        (root/'bad.wzd').write_bytes(fixtures[0]+b'[START_SPELLS]\nSPELL_01=TRUE\nSPELL_1=FALSE\n')
        rejected=subprocess.run([str(binary),str(root),'bad.wzd'],capture_output=True)
        assert rejected.returncode==2
    report={'scope':'Offline native text/data comparison; no original engine execution or gameplay application',
            'synthetic_matches':len(fixtures)}
    if installation:
        paths=sorted(installation.rglob('*.wzd'))
        assert paths, 'No installed WZD inputs'
        evidence=[compare(binary,installation,p.relative_to(installation)) for p in paths]
        report.update(installed_matches=len(evidence),files=evidence,
                      inventory_sha256=hashlib.sha256(json.dumps(evidence,sort_keys=True,separators=(',',':')).encode()).hexdigest())
    return report


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary',type=Path)
    parser.add_argument('--installation',type=Path)
    parser.add_argument('--report',type=Path)
    args=parser.parse_args()
    if args.installation:
        subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)
    try:
        result=run(args.binary.resolve(),args.installation.resolve() if args.installation else None)
        if args.report:
            args.report.parent.mkdir(parents=True,exist_ok=True)
            args.report.write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps({k:v for k,v in result.items() if k!='files'},indent=2))
    finally:
        if args.installation:
            subprocess.run([str(REPO/'tools/original-manifest.sh'),'verify'],check=True)

if __name__=='__main__':
    main()
