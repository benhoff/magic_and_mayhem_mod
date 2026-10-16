# Playable native installed-terrain campaign

The native Qt window offers **New map campaign…** to select the installed game
root. `--asset-root ROOT --map-campaign` starts the same mission directly;
`--asset-root ROOT --load-campaign FILE` reopens it paused. Existing native arena
campaigns and standalone saves retain their own startup/actions. The selected
resource directory is session context, not a host path embedded in checkpoints.

The app stages an immutable resource context, diagnostic presentation and owned
campaign before adopting any of them. Loading resolves the campaign identity,
requires installed resources for the Plains campaign, rebuilds the same revision
and validates expected mission/navigation/health policy. Failed construction or
load retains the previous campaign/presentation. Orders, fixed-tick playback,
Save, Continue/Retry and settlement still go through CampaignSession.

Presentation uses the decoded paired Plains SPR and existing ordinary terrain
submission/depth helpers. View0 uses a bounded cell-centred isometric projection
with32 fine units per XY cell and16 per layer. Sprite origins and opaque masks
are respected; terrain images are decoded once and shared. Terrain remains ordered by the recovered depth helper. Native cell outlines
and raised vector unit markers are diagnostic overlays so tall grass does not
hide interaction targets. Picking checks those same unit hit regions first,
then projected supported-cell diamonds, choosing the foremost admitted cell
where projected cells overlap. Sealed/unsupported cells are excluded. Projectiles
are native overlays. This explicitly replaces the initial opaque-sprite picking
experiment, whose failed execution is retained: tall grass intercepted intended
cell clicks and buried the small glyphs. Logical cells drive glyph positions;
smooth original animation, lighting/fog, original UI and rendered-pixel equivalence
remain outside this milestone.

A five-process scripted journey drives the production buttons, terrain mouse
coordinates and file dialogs. It summons, moves, queues a bolt, saves pending and
in-flight state, compares exact bytes with the independent CLI, exits/reopens,
splits continuation, rejects a checksummed forged mission and corrupt file without
changing the active checkpoint, wins, saves/reopens the settled result unchanged,
and starts the exact arena-finale checkpoint. Existing arena campaign and
standalone window scenarios run separately. Execution reports pin source claims,
asset hashes and original-manifest checks; native gameplay rules/rewards are
unchanged and no original campaign equivalence is claimed.
