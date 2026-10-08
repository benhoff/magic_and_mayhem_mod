#!/usr/bin/env python3
"""Optional CDP browser regression checks; needs an existing browser and websockets.

Start the dashboard separately and an installed headless Chromium with
--remote-debugging-port=9331, then pass a new --output directory.
"""
import argparse
import asyncio
import base64
import json
from pathlib import Path
from urllib.request import urlopen
import websockets

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('--url',default='http://127.0.0.1:8786')
parser.add_argument('--devtools-url',default='http://127.0.0.1:9331')
parser.add_argument('--output',type=Path,required=True)
args=parser.parse_args()
URL=args.url.rstrip('/')
OUT=args.output
OUT.mkdir(parents=True,exist_ok=False)

async def main():
    tabs=json.load(urlopen(args.devtools_url.rstrip('/')+'/json/list'))
    tab=next(t for t in tabs if t['type']=='page')
    async with websockets.connect(tab['webSocketDebuggerUrl'],max_size=20_000_000) as ws:
        seq=0;errors=[];checks=[]
        async def rpc(method,params=None):
            nonlocal seq
            seq+=1;current=seq
            await ws.send(json.dumps({'id':current,'method':method,'params':params or {}}))
            while True:
                message=json.loads(await ws.recv())
                if message.get('method')=='Runtime.exceptionThrown':errors.append(message['params'])
                if message.get('id')==current:
                    if 'error' in message:raise RuntimeError(message['error'])
                    return message.get('result',{})
        async def evaluate(js):
            result=await rpc('Runtime.evaluate',{'expression':js,'returnByValue':True,'awaitPromise':True})
            if 'exceptionDetails' in result:raise RuntimeError(result['exceptionDetails'])
            return result['result'].get('value')
        async def wait(js):
            for _ in range(120):
                if await evaluate(js):return
                await asyncio.sleep(.2)
            raise AssertionError('Timeout: '+js)
        async def check(label,js):
            assert await evaluate(js),label
            checks.append(label)
        await rpc('Runtime.enable')
        await rpc('Page.enable')
        await rpc('Emulation.setDeviceMetricsOverride',{'width':1440,'height':1100,'deviceScaleFactor':1,'mobile':False})
        await rpc('Page.navigate',{'url':URL})
        await wait("typeof data !== 'undefined' && !!data && !!document.querySelector('[data-render-milestone]')")
        await check('native rendering is the default permanent view',"state.view==='rendering' && document.querySelector('nav [data-view=rendering]').getAttribute('aria-current')==='page' && document.querySelectorAll('[data-render-milestone]').length===data.tracks.find(t=>t.id==='rendering').milestones.length && document.querySelector('#kind').closest('.scope-control').hidden")
        await check('native focus keeps live ownership and initial canvas blockers visible',"document.querySelector('.render-boundary').textContent.includes('Original drawing remains active') && document.querySelector('#view-content').textContent.includes('Recover the initial World canvas') && document.querySelector('[data-render-milestone=word-takeover]').textContent.includes('fully in-bounds') && document.querySelector('[data-render-milestone=complete-replacement]').textContent.includes('Not Started')")
        await check('rendering milestone states match registered contracts',"data.tracks.find(t=>t.id==='rendering').milestones.every(m=>document.querySelector('[data-render-milestone='+m.id+'] .badge').textContent===titleCase(milestoneLabels[m.state]))")
        await check('current comparison follows exact execution claims',
                    "data.behaviors.filter(b=>b.kind==='recovered' && b.status.comparison==='recorded').every(b=>(b.comparison_freshness==='current')===(b.current_validation.comparison.state==='current'))")
        await check('historical source freshness cannot substitute for current validation',
                    "data.behaviors.some(b=>b.status.comparison==='recorded' && b.comparison_source_freshness==='current' && b.comparison_freshness==='pending') && !!gapLabels.validation && !!gapActions.validation")
        await evaluate("document.querySelector('[data-behavior=\"RS.world-initial-state\"]').click()")
        await check('native focus opens exact behavior evidence and stage boundaries',"document.querySelector('#detail').open && document.querySelector('#detail-body').textContent.includes(behaviorById.get('RS.world-initial-state').scope) && !!document.querySelector('#detail-body .evidence-item')")
        await evaluate("document.querySelector('#close-detail').click();document.querySelector('[data-track-work=rendering]').click()")
        await check('rendering gaps include original and native contracts',"state.view==='work' && state.track==='rendering' && state.kind==='all' && scopedBehaviors().some(b=>b.id==='RS.world-initial-state') && scopedBehaviors().some(b=>b.id==='NR.world-live-view')")
        await rpc('Page.navigate',{'url':URL+'/#view=rendering'})
        await wait("typeof data !== 'undefined' && !!data && !!document.querySelector('[data-render-milestone]')")
        await check('native rendering bookmark restores the focus view',"state.view==='rendering'")
        focus=await rpc('Page.captureScreenshot',{'format':'png','captureBeyondViewport':False})
        (OUT/'native-rendering-desktop.png').write_bytes(base64.b64decode(focus['data']))
        await evaluate("document.querySelector('[data-view=tracks]').click()")
        await check('default view shows all major tracks with named milestone percentages',"state.view==='tracks' && document.querySelector('[data-track-card=movement] .track-caption strong').textContent===data.tracks.find(t=>t.id==='movement').progress.count+' / '+data.tracks.find(t=>t.id==='movement').progress.total+' demonstrated'")
        await check('audio card follows its current registered roadmap',"document.querySelector('[data-track-card=audio] .track-caption strong').textContent===data.tracks.find(t=>t.id==='audio').progress.count+' / '+data.tracks.find(t=>t.id==='audio').progress.total+' demonstrated'")
        await evaluate("document.querySelector('[data-track-detail=movement]').click()")
        await check('track dialog shows concrete next milestones and historical evidence boundary',"document.querySelector('#detail').open && document.querySelector('#detail-body').textContent.includes('Original orders & scheduling') && document.querySelector('#detail-body').textContent.includes('historical') && document.querySelectorAll('.milestone-detail').length===data.tracks.find(t=>t.id==='movement').milestones.length")
        await evaluate("document.querySelector('[data-track-work=movement]').click()")
        await check('track drilldown includes native policies and recovered contracts',"state.track==='movement' && state.kind==='all' && state.view==='work' && scopedBehaviors().every(b=>data.tracks.find(t=>t.id==='movement').behaviors.includes(b.id)) && scopedBehaviors().some(b=>b.kind==='native_policy')")
        await evaluate("document.querySelector('[data-clear]').click();document.querySelector('#kind').value='recovered';document.querySelector('#kind').dispatchEvent(new Event('change',{bubbles:true}));document.querySelector('[data-view=overview]').click()")
        await check('baseline scoped percentage matches register',"document.querySelectorAll('.metric')[1].querySelector('.metric-value').textContent === (data.summary.recovered.scoped.percent).toFixed(1)+'%'")
        await check('binary percentage matches global function links',"document.querySelectorAll('.metric')[0].querySelector('.metric-value').textContent === data.binary.builds[0].functions.percent.toFixed(1)+'%'")
        await evaluate("document.querySelector('[data-go=work][data-gap=partial]').click()")
        await check('partial work view has correct total and scope',"state.gap==='partial' && document.querySelector('.work-chip.active strong').textContent === String(scopedBehaviors().filter(b=>b.status.implementation==='partial').length) && !!document.querySelector('.feature-scope')")
        await evaluate("document.querySelector('#gap-filter').value='unimplemented'; document.querySelector('#gap-filter').dispatchEvent(new Event('change',{bubbles:true})); document.querySelector('#search').value='MV.scheduler'; document.querySelector('#search').dispatchEvent(new Event('input',{bubbles:true}))")
        await wait("state.query==='MV.scheduler' && document.querySelectorAll('#view-content [data-behavior]').length === 1")
        await evaluate("document.querySelector('#view-content [data-behavior]').click()")
        await check('feature detail shows exact scope, research, addresses and gaps',"document.querySelector('#detail').open && document.querySelector('#detail-body').textContent.includes(behaviorById.get('MV.scheduler').scope) && document.querySelectorAll('#detail-body a[href^=\"/files\"]').length>0 && document.querySelector('#detail-body').textContent.includes('No original comparison')")
        path=await evaluate("behaviorById.get('MV.scheduler').documents[0]")
        assert urlopen(URL+'/files?path='+path).status==200
        checks.append('registered research file opens')
        await rpc('Input.dispatchKeyEvent',{'type':'keyDown','key':'Escape','code':'Escape','windowsVirtualKeyCode':27})
        await check('Escape closes detail dialog',"!document.querySelector('#detail').open")
        await evaluate("document.querySelector('[data-view=binary]').click();document.querySelector('[data-clear]').click();document.querySelector('#search').value='0x00401041';document.querySelector('#search').dispatchEvent(new Event('input',{bubbles:true}))")
        await wait("state.query==='0x00401041' && document.querySelectorAll('#view-content tbody tr').length===1")
        await check('interior hexadecimal address finds owning function',"document.querySelector('[data-function]').textContent==='0x00401040'")
        await evaluate("document.querySelector('[data-function]').click()")
        await check('function details include direct call edges and limits',"document.querySelector('#detail-body').textContent.includes('Direct callees') && document.querySelector('#detail-body').textContent.includes('not stable runtime pointers')")
        await evaluate("document.querySelector('#close-detail').click();document.querySelector('[data-view=overview]').click();document.querySelector('#kind').value='native_policy';document.querySelector('#kind').dispatchEvent(new Event('change',{bubbles:true}))")
        await check('native policies exclude original-comparison denominator',"document.querySelectorAll('.metric')[2].querySelector('.metric-value').textContent==='N/A' && document.querySelectorAll('.metric')[3].querySelector('.metric-value').textContent==='N/A'")
        await evaluate("document.querySelector('#kind').value='recovered';document.querySelector('#kind').dispatchEvent(new Event('change',{bubbles:true}));document.querySelector('[data-subsystem=movement]').click()")
        await check('subsystem drilldown filters work list',"state.subsystem==='movement' && state.view==='work' && document.querySelectorAll('#view-content tbody tr').length>0")
        await evaluate("document.querySelector('[data-clear]').click();document.querySelector('[data-view=behaviors]').click()")
        await check('behavior pagination limits large lists',"document.querySelectorAll('#view-content tbody tr').length===25")
        await evaluate("document.querySelector('[data-page=\"1\"]').click()")
        await check('next page works',"state.page===1 && document.querySelector('.pagination').textContent.includes('Page 2')")
        await evaluate("document.querySelector('#refresh').click()")
        await wait("!document.querySelector('#refresh').disabled")
        await check('refresh rebuilds current snapshot without losing filters',"state.page===1 && state.view==='behaviors' && document.querySelector('#snapshot-time').textContent.includes('Snapshot')")
        await evaluate("document.querySelector('[data-view=tracks]').click()")
        await rpc('Emulation.setDeviceMetricsOverride',{'width':1440,'height':1100,'deviceScaleFactor':1,'mobile':False})
        await asyncio.sleep(.3)
        desktop=await rpc('Page.captureScreenshot',{'format':'png','captureBeyondViewport':False})
        (OUT/'desktop.png').write_bytes(base64.b64decode(desktop['data']))
        await rpc('Emulation.setDeviceMetricsOverride',{'width':390,'height':844,'deviceScaleFactor':1,'mobile':True})
        await asyncio.sleep(.3)
        await check('mobile viewport has no horizontal page overflow',"document.documentElement.scrollWidth<=window.innerWidth")
        await evaluate("document.querySelector('[data-view=rendering]').click()")
        await check('native rendering mobile view fits and exposes roadmap and research',"document.documentElement.scrollWidth<=window.innerWidth && document.querySelectorAll('[data-render-milestone]').length===data.tracks.find(t=>t.id==='rendering').milestones.length && document.querySelector('#view-content').textContent.includes('Research & remaining work')")
        native_mobile=await rpc('Page.captureScreenshot',{'format':'png','captureBeyondViewport':False})
        (OUT/'native-rendering-mobile.png').write_bytes(base64.b64decode(native_mobile['data']))
        await evaluate("document.querySelector('[data-track-detail=rendering]').click()")
        await check('mobile track dialog fits viewport and exposes replacement boundary',"document.querySelector('#detail').open && document.querySelector('#detail').getBoundingClientRect().width<=window.innerWidth && document.querySelector('#detail-body').textContent.includes('observation does not mean native replacement')")
        await evaluate("document.querySelector('#close-detail').click()")
        mobile=await rpc('Page.captureScreenshot',{'format':'png','captureBeyondViewport':False})
        (OUT/'mobile.png').write_bytes(base64.b64decode(mobile['data']))
        await check('no uncaught browser errors',str(not errors).lower())
        result={'url':URL,'checks':checks,'browser_errors':errors,'register_sha256':await evaluate('data.register_sha256'),'recorded_at':await evaluate('new Date().toISOString()'),'screenshots':['native-rendering-desktop.png','native-rendering-mobile.png','desktop.png','mobile.png']}
        (OUT/'result.json').write_text(json.dumps(result,indent=2)+'\n')
        print(json.dumps(result,indent=2))

asyncio.run(main())
