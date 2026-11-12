# Scene actor observation v1

This opt-in, passive sidecar observes the selected `0x40209ca7` build at
`0x5002a0` queue entry. It is separate from the rendering wire. It does not
call original sprite helpers or change simulation, animation or draw arguments.
Enable `MNM_SCENE_ACTORS=1` only with the continuous, resident, publicly bound
World producer. Files use the scene capture directory and `scene-actors-0001.bin`.

All integers are little-endian DWORDs. The authoritative sizes and budgets are
in `protocols/include/mnm/scene_actor_observation_v1.h`; the implementation is
`runtime/scene/scene_actor_observation.h`. `tools/scene_actor_observation.py`
rejects incomplete streams, explicit loss, foreign owners and invalid bounds.

The 64-byte stream header contains magic `MNMSAO01`, version, header size,
build, frame budget, actor size, draw size and byte budget, then zero reserves.
Every frame begins with 32 DWORDs:

| Words | Observation |
| --- | --- |
| 0–3 | Frame bytes, ordinal, completed-source generation low/high |
| 4–8 | Source nonce low/high, engine thread, tick, World counter |
| 9–14 | Queue view/count, pool epoch/capacity, active actors, matching draws |
| 15–19 | Gameplay journal ordinal, source QUEUE_ENTRY ordinal, canvas ID, width/height |
| 20–24 | Queue auxiliary values, raw view, validity bits, payload CRC32C |
| 25–31 | Zero reserves |

Validity bit 0 means the bounded pool was readable; bit 1 means the same engine
thread supplied a gameplay point. The gameplay getter copies the journal's
already-written ordinal; it does not emit another event. The pool epoch changes
only when base/capacity changes. It does **not** establish actor allocation
generations or distinguish reused slots.

Each actor has 64 DWORDs: slot/type/owner/active/health/mana (0–5), XYZ (6–8),
behavior/action/target (9–11), valid-role mask (12), footprint/mode/facing
(13–15), per-frame body/first-child/overlay tokens (16–18), reserve (19), raw
44-byte body/child/overlay ANI records (20–52), reserve (53), SPR inclusive
bounds (54–56), effective frame indices (57–59), then reserves. Unreadable
roles are absent. This observation applies the selected helper's inclusive
bound and frame-zero fallback without invoking its mutating initializer.

Each matched draw has 20 DWORDs: actor slot, role, queue ordinal, token, depth,
XY, draw kind, raw flags, two queue scalars, frame byte count, normalized frame
CRC32C, width/height, origin XY, effective frame index, indexed-palette flag,
reserve. The palette pointer DWORD at byte 28 is zeroed for the frame checksum.
Tokens deduplicate pointers only inside one frame and never serialize pointers.
Matching one frame to several roles/actors is retained as ambiguity, not proof
that those actors own every matched draw.

The 64-byte terminal contains `MNMSAOE1`, version/size, emitted frame count,
preceding bytes, loss code, owner, then reserves. Foreign-thread access, budget
exhaustion, invalid queue or write failure prevents complete admission. CRC32C
detects corruption; it is not authentication. Disabled capture creates no file.

The owned-memory C fixture executes the production encoder without loading
original artifacts. Those controls establish the wire and refusals only.
Attachment placement, actual combat and delivery to a native viewport require
separate original-runtime captures and independent output comparisons.
