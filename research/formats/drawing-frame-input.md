# Frame and interaction witness, version 1

`MNM_ACCEPTANCE_FRAME_INPUT=1` adds an optional read-only witness to the existing
public gameplay acceptance route. The launcher sets a fresh private
`MNM_DRAW_FRAME_INPUT_FILE` and independently tracks pool slot zero. No original
method, input poll, simulation tick or picking call is added. Original Region
callbacks still run once with the original arguments, result and LastError;
their existing floating-point save/restore envelope also covers observation.
The frame sampler additionally saves/restores the complete512-byte FX state at
both Region and World boundaries and restores the supplied LastError on return.

The file starts with `MNMIFR01`, then little-endian DWORDs `1,384`. Complete
384-byte records append synchronously. At most 8191 records are admissible;
the 8192nd sets the loss bit and terminates observation. A stopped reader rejects
loss, a partial final record, changed owners/nonces, reversed generations and
unpaired callbacks. A live reader may temporarily admit a pending callback for
progress observation; this cannot pass the stopped evidence gate.

| DWORDs | Meaning |
|---|---|
| 0–7 | Ordinal, callback call ID (zero for World), kind (1 World return, 2 Region entry, 3 Region return), observed World-return ordinal, original tick, thread, callback mask, validity |
| 8–55 | Existing Region V3 scalar layout, including mouse arguments, original result/LastError, hit class/draw index, validated numeric pool/slot/type/owner/activity/health, camera +31 and +39 through +4d, mode, cursor type, selection flags and independently tracked slot |
| 56–63 | Completed source identity: version 1, bytes 32, NoCD build tag, generation low/high, session nonce low/high, actual queue owner |
| 64–80 | Completed native GetCursorPos observation result followed by its 64-byte value |
| 81–84 | Cursor-global readability, saved-background validity, last X/Y at 641aa0/641aa4 |
| 85–88 | Tracked actor pose readability and raw +8/+c/+10 words |
| 89–92 | Draw-history readability and raw +754/+758/+75c words |
| 93–94 | Nominal-resolution flag readability and raw DWORD at 6de6d5 |
| 95 | Reserved zero |

Validity bits 0–7 retain the Region meanings. Bit 8 admits the exact source
identity. Bit 31 signals overflow and always refuses evidence.

`SceneWorldFrameObservationV1(owner,out,32)` exposes a completed original queue's
generation and session nonce. Queue entry invalidates the previous observation;
successful queue return admits the current identity before publishing RETURNED.
Only the actual calling thread may request its own completed identity. This does
not assert that every scalar was read inside the producer: the witness observes
state at named Region/World boundaries. The finite acceptance join additionally
requires the observed orientation to equal the owned camera orientation of each
of three settled, actually presented source generations. Pan changes are checked
in both raw camera fields and the owned source map center; the transform between
those two representations is not recovered by this witness. Hover hit windows
end before the following owned original selection callback entry. Native
ordinals/ticks must belong to the following completed action; physical release
occurs within1s and the completed host action within6s of the hover marker.
This retains original hit-update transients and requires
three actual hit frames plus the exact callback coordinates. The presentation join
follows those exact generations through live import leases with at most500ms
after the settled host marker; host timestamp overlap is not an atomic frame
snapshot contract. Transient states are
retained without being relabeled as producer inputs.

`RenderCursorObservationV1(owner,out,64)` exposes only a completed native
GetCursorPos result on the same calling thread. Its DWORDs are version, size,
build, valid flag, owner, poll ordinal, admitted input-channel sequence, tick,
returned X/Y, desktop-coordinate flag, API result, original caller, then three
reserved zeros. This is a separate interface from DrawingInputPointV1 and has
no Drawing API journal-prefix claim. Each owner has a bounded private slot;
foreign readers cannot inspect it. Every new cursor call invalidates the previous
observation before attempting input admission, so fallback cannot expose a stale
native point. Exhausting the 16 owner slots refuses observation. It never polls
again to satisfy a reader and never changes the input result.

Pool IDs record observed numeric base changes. They are not original allocation
generations and cannot detect same-type, same-owner slot recycling between
observations. The finite case requires a living type-zero, owner-zero wizard,
the same observed pool/slot around each actual selection callback and retained
selection after moving the pointer away. Broader lifetime/selection semantics
remain separate.

Cursor freshness uses actual call order after the released original source
boundary, with three distinct completed native polls. Poll-to-World-return ages
remain measurements. A software capture contains a3082ms interval despite the
correct newly consumed point; no response-deadline or production attribution is
inferred. Policy-specific samples use their public scaling mode. Every actual
World paint must also obey the global Sharp→public-transition→Integer sequence.
A repaint of the same source generation during the authorised scaling transition
is retained and independently compared; it is not mislabeled as the prior mode.
