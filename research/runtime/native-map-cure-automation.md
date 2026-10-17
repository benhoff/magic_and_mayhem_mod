# Automated Plains/Cure campaign integration

This integration joins the independently tested authored Plains crop campaign
and shared combat/Cure runtime. It is native policy, with no change to the
existing cost (3 mana), magnitude (8 health), or timing. Original campaign rules,
full-map behavior, pixel matching, physical audio and live replacement remain
outside this scope.

The Cure presentation previously assumed an 8×8×3 arena. The bounded runtime
now owns validated mission dimensions (4..16 by 4..16 by 2..32, at most 4096
cells), uses those dimensions for spatial projection, and refuses a restored
presentation whose dimensions differ from the bound navigation. The Plains
crop uses 8×8×23; the finale uses the original 8×8×3 profile. Default arena
payloads retain their original dimensions and serialization.

Map resources accept an explicit Cure option. Its spell roster and both initial
mission snapshots participate in content identity. Existing no-Cure map content
retains its identity. Loading selects the candidate profile from the saved
player roster, then validates the entire identity and resource-bound checkpoint
before adoption. The existing New map campaign action retains the prior profile;
`mnm-map-campaign-cli ASSET_ROOT create-cure OUTPUT` creates the enabled profile.
Start it with the CLI, then load it through the window's campaign load action.

`tools/test-native-map-campaign.py ASSET_ROOT --cure` is the combined automated
journey. Run it under the main checkout storage guard. It declares coverage
before execution, retains immutable reports, verifies original inputs before
and after, runs the existing suite and map/presentation/window checks, then
moves a damaged test Sentinel while casting Cure, saves pending and mid-ANI
state, restarts in a separate Qt process, and compares complete checkpoint bytes
with headless continuation. The fixture damages only test-owned state; no
production debug action or health override is added. Single debit and single
heal are checked separately. It then finishes the battle, reopens the settled
result unchanged, and continues into the arena finale. Sanitized contracts cover
Cure, shared effects, mission state and the 23-layer fixture. Visual taste and
physical input devices are outside these functional checks.

Execution results are registered separately; this document is not execution
proof. Failed attempts remain in `working/tests/map-campaign/` and do not promote
coverage status. Historical evidence hashes remain unchanged after integration.
