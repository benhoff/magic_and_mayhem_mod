#!/usr/bin/env python3
"""One bounded installed font load/use/reload/teardown scout through public menus."""
import importlib.util
import json
from pathlib import Path
import sys
from coverage_evidence import write_report

ROOT=Path(__file__).resolve().parents[1]
PREFIX='research/runtime/font-lifetime-results-20261108/'

def main():
    spec=importlib.util.spec_from_file_location('font_menu_base',ROOT/'tools/run-rendering-copy-branches.py')
    base=importlib.util.module_from_spec(spec);spec.loader.exec_module(base)
    base.PREFIX=PREFIX;base.PLAN=PREFIX+'input-plan-01.json';base.__file__=__file__
    old_configure=base.configure
    def configure(env,flags):
        env=old_configure(env,flags);env['MNM_FONT_LIFETIME']='1';return env
    base.configure=configure
    old_write=base.write_report
    def writer(root,name,raw):
        if str(name).endswith('-acquisition.json'):
            p=json.loads(raw);p['kind']='installed_font_lifetime_acquisition'
            p['newly_passed_requirements']=0;p['font_and_dc_row_closed']=False
            raw=(json.dumps(p,indent=2)+'\n').encode()
        return old_write(root,name,raw)
    base.write_report=writer
    class FontJourney(base.Journey):
        def run(self):
            self.input.activate();self.click('Launch game');self.await_main()
            self.capture('font-main-high','Quick Battle')
            def action(call,screen):
                ack=self.last['ack']+1;call();return self.wait(screen,ack)
            def book(label):
                action(lambda:self.click_id('mainMenuAction0'),18)
                action(lambda:self.click_id('regionEntryAuxiliary0'),27)
                action(lambda:self.click_id('grimoireTab0'),27)
                action(lambda:self.key(label+'-page-down',0xff56),27)
                action(lambda:self.key(label+'-page-up',0xff55),27)
                action(lambda:self.key(label+'-book-close',0xff1b),18)
                action(lambda:self.key(label+'-region-close',0xff1b),3)
            book('high')
            before=action(lambda:self.click('Preferences'),10)
            base.require(before['preferences'][2]==0,'High-resolution initial font context required')
            self.click_id('preferencesRadio2');after=action(lambda:self.click_id('preferencesOk'),3)
            self.capture('font-main-low','Quick Battle')
            self.actions.append(dict(action='font-resolution-reload',preferences=before,returned=after))
            book('low')
            self.capture('font-main-final','Quick Battle');self.click('Quit')
    base.Journey=FontJourney
    return base.main()

if __name__=='__main__':raise SystemExit(main())
