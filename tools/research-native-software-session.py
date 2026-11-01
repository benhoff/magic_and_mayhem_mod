#!/usr/bin/env python3
"""Research-only caller for the actual --software-rendering application branch.

Delegate the existing bounded strict-player driver unchanged. Record the exact
single application command augmented with its already supported software flag.
No result, renderer state, oracle, gameplay or admission policy is substituted.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys

from coverage_evidence import Reader, write_report

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--research-report',required=True)
    parser.add_argument('arguments',nargs=argparse.REMAINDER)
    args=parser.parse_args();arguments=args.arguments[1:] if args.arguments[:1]==['--'] else args.arguments
    if (ROOT/args.research_report).exists() or '--native-only' not in arguments or '--prefix-template' not in arguments or '--report' not in arguments:
        parser.error('Require a new research report and strict private-prefix delegate arguments')
    delegate_name=arguments[arguments.index('--report')+1]
    with Reader(ROOT) as reader:
        sources={n:reader.digest(n) for n in ['tools/research-native-software-session.py','tools/test-native-owned-campaign.py',
            'tools/run-native-owned.py','tools/native_only_contract.py','apps/qt-shell/main.cpp']}
    commands=[];original_popen=subprocess.Popen
    def popen(command,*positional,**settings):
        if isinstance(command,(list,tuple)) and command and str(command[0])==str(ROOT/'tools/run-native-owned.py'):
            if '--software-rendering' in command or '--' not in command:
                raise ValueError('Unexpected software research command')
            command=[*command,'--software-rendering'];commands.append(list(map(str,command)))
        return original_popen(command,*positional,**settings)
    report=dict(schema=1,success=False,sources=sources,commands=commands,
        supported_player_accepted=False,independent_presented_history=False,
        scope='Actual software application flag through unchanged strict-driver/owner admission and normal outcomes. Explicit research caller; public wrapper/storage admission and complete requirement acceptance remain separate.')
    try:
        spec=importlib.util.spec_from_file_location('software_research_delegate',ROOT/'tools/test-native-owned-campaign.py')
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        subprocess.Popen=popen;sys.argv=[str(ROOT/'tools/test-native-owned-campaign.py'),*arguments]
        code=module.main()
        with Reader(ROOT) as reader:
            value=reader.json(delegate_name)
            report.update(delegate=dict(record=delegate_name,sha256=reader.digest(delegate_name),success=value['success']),
                          success=code==0 and value['success'] is True and len(commands)==1,
                          software_flag_forwarded=len(commands)==1 and '--software-rendering' in commands[0])
    except Exception as error:
        report['error']=str(error)
    finally:
        subprocess.Popen=original_popen
        with Reader(ROOT) as reader:report['sources_stable']=all(reader.digest(n)==h for n,h in sources.items())
        report['success'] &= report['sources_stable']
        digest=write_report(ROOT,args.research_report,(json.dumps(report,indent=2)+'\n').encode())
        print(json.dumps(dict(success=report['success'],sha256=digest,error=report.get('error'))))
    return 0 if report['success'] else 1


if __name__=='__main__':raise SystemExit(main())
