"""Apply only the reviewed caller analysis input bindings; preserve acquisition commands."""
from rendering_pilot import require

FLAGS=('--claims','--requirements','--history-build')

def analysis_command(case,review,report):
    command=list(case['comparison_command']);old=review['original_comparison_command'];new=review['comparison_command']
    require(command[:2]==['python3','tools/compare-broader-caller.py']
            and old[:2]==command[:2]
            and new[:2]==['python3','tools/compare-broader-runtime-caller-evidence.py'],
            'Unknown caller comparison adapter')
    require(all(command.count(flag)==old.count(flag)==new.count(flag)==1 for flag in (*FLAGS,'--acquisition','--report')),
            'Missing or duplicated caller analysis argument')
    require(command[command.index('--acquisition')+1]==case['report']
            and command[command.index('--report')+1]==case['comparison'],
            'Caller comparison changed its acquired/output context')
    changed={1,command.index('--report')+1}
    command[1]=new[1];command[command.index('--report')+1]=report
    for flag in FLAGS:
        at=command.index(flag)+1;require(command[at]==old[old.index(flag)+1],
                                       'Caller analysis input differs from reviewed predecessor')
        command[at]=new[new.index(flag)+1];changed.add(at)
    require(all(value==case['comparison_command'][i] for i,value in enumerate(command)if i not in changed),
            'Unreviewed caller action/model/control argument changed')
    return command
