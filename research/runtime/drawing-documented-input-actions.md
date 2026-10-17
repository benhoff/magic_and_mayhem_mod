# Documented original input probes

The bounded XTest runner supports Shift on a mouse click and Space, Insert and
Delete taps. Modifier declarations must be arrays containing zero or one Shift
entry, and are admitted only for clicks. Unsupported or duplicate declarations
refuse before injection. Button down/up return values are checked; held keys
and buttons are released by the existing error/normal cleanup path.

The [publisher manual scan](https://d1.xp.myabandonware.com/f/lvq2/Magic-Mayhem_Manual_Win_EN.pdf),
printed pages 14–15 (PDF page 9), describes Shift-left as a selection toggle and
Space as pause/resume. Printed page 10 describes spell hover descriptions.
The supplied Readme.wri additionally names Insert/Delete pause controls. These
are prospective input definitions; actual game outcomes require independent
observation. Manual SHA-256: c06f9477e7b03cbf5d3e7d0ff5ed67e7cfffdc47c69e9e7b513d55237976991f.
Downloaded provenance and inspected page renders are retained under working/.

The ordinary ground-click probe retains valid no-hit callbacks while the prior
actor latch remains set. Repeated creature clicks also retain the latch; neither
operation supplies deselection evidence. Complete picking/selection/pause and
exact action/frame/source/native parity remain pending.

Current owned X11 fixture passes six cases: exact Shift state on button down/up,
three pause key pairs, four malformed modifier declarations and cancellation
while keys/button are held. All cases leave inputs released. This validates the
input transport independently of engine semantics.

Original probe01 retains stale no-hit candidates at both modifier button-down
points. Probe02 adds a 600ms pointer dwell before each Shift click; original
calls 2/6/10 change the same observed actor +68/count/mode 0→1→0→1, with
matching release and later independent pointer-away states. Images at actions
3/6/9 were inspected: creature/portrait brackets are present, absent, present.
Those images are independently sampled, without exact original frame binding.

Static pause admission binds the four original mapped-key call groups to four
World mode transitions within a diagnostic 500ms bound. Two mode-one intervals
retain the selected World counter while accepted render sequences advance;
active and resumed intervals advance the counter. Selected ANI fields are
reported without inferring all animation or simulation semantics. The 500ms
association and 1000ms minimum interval are diagnostic policies. Six synthetic
guards reject missing/stale hits, identity/release changes, missing pointer-away
states, ambiguous coordinates, moving paused counters, absent active/render
controls, wrong key owners and unbounded key/transition associations.
