"""Reviewed current World families and exact historical primary domains."""
from rendering_pilot import require

FAMILIES={
 'quick':dict(prefix='rendering-normal-results-20261031',controller='run-rendering-acceptance-pilot.py',comparison='check-rendering-acceptance-pilot.py'),
 'storage':dict(prefix='rendering-normal-results-20261031',controller='run-rendering-acceptance-pilot.py',comparison='check-rendering-acceptance-pilot.py'),
 'caller':dict(prefix='caller-state-installed-results-20261108',controller='run-rendering-caller-state.py',comparison='check-rendering-caller-state.py'),
 'interaction':dict(prefix='rendering-interaction-results-20261107',controller='run-rendering-interaction.py',comparison='check-rendering-interaction.py'),
 'viewport':dict(prefix='viewport-cursor-results-20261108',controller='run-rendering-viewport-cursor.py',comparison='check-rendering-viewport-cursor.py'),
 'attachment':dict(prefix='attachment-combat-results-20261107',controller='run-scene-attachment.py',comparison='check-scene-attachment.py'),
 'combat':dict(prefix='rendering-normal-results-20261031',controller='run-rendering-acceptance-pilot.py',comparison='check-rendering-acceptance-pilot.py'),
 'ui':dict(prefix='rendering-ui-results-20261107',controller='run-rendering-ui.py',low_controller='run-rendering-ui-low.py',comparison='check-rendering-ui.py'),
 'hotspot':dict(prefix='cursor-hotspot-results-20261108',controller='run-rendering-cursor-hotspot.py',comparison='check-rendering-cursor-hotspot.py'),
 'picking':dict(prefix='picking-closeout-results-20261109',controller='run-rendering-picking-closeout.py',comparison='check-rendering-picking-closeout.py'),
 'pause':dict(prefix='rendering-pause-results-20261107',controller='run-rendering-pause.py',low_controller='run-rendering-pause-low.py',comparison='check-rendering-pause.py',low_comparison='check-rendering-pause-low.py'),
 'victory':dict(prefix='rendering-progression-results-20261101',controller='run-rendering-progression.py',comparison='check-rendering-progression.py'),
 'defeat':dict(prefix='rendering-progression-results-20261101',controller='run-rendering-progression.py',comparison='check-rendering-progression.py'),
 'font':dict(prefix='font-dc-lifetime-results-20261108',controller='run-font-dc-lifetime.py',comparison='check-font-lifetime.py'),
 'palette':dict(prefix='palette-cycling-results-20261108',controller='run-palette-cycle.py',comparison='check-palette-cycle.py')}

RETAINED_SOFTWARE={('quick',800):'wp10-software-quick-01-acquisition.json',
                   ('quick',640):'wp10-software-low-02-acquisition.json',
                   ('storage',800):'wp10-software-storage-01-acquisition.json'}


def family_of(record,acquired):
    scenario=acquired.get('scenario')
    scenario_names={'normal-quick':'quick','normal-storage':'storage','normal-combat':'combat',
                    'normal-attachment':'attachment','normal-font-dc':'font','normal-palette':'palette',
                    'victory':'victory','defeat':'defeat'}
    if scenario in scenario_names:return scenario_names[scenario]
    matched=[name for name,value in FAMILIES.items()if record.startswith('research/runtime/'+value['prefix']+'/')]
    require(len(matched)==1,'Unknown/ambiguous historical World family: '+record)
    return matched[0]


def initial_extent(comparison):
    domain=comparison['world_domain']
    require(domain['verified']and domain['worlds'],'Missing historical World domain')
    first=domain['worlds'][0];size=[first['width'],first['height']]
    require(size in([640,480],[800,600]),'Unreviewed original extent')
    return size
