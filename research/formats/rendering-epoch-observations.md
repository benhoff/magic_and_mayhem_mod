# Native rendering epoch observations

These are opt-in host diagnostics, not a game wire revision. The executable
sources and actual observed bytes must accompany any execution claim.

`MNM_RESIDENT_PROFILE_CAPTURE_DIRECTORY` selects an existing absolute directory.
It conflicts with `MNM_RESIDENT_PROFILE_CAPTURE`. Each resident World channel
writes `worlds-<sha256>.bin`, where the digest covers the UTF-8 absolute channel
path. The existing MNMPROF4 format, queue limits and NewOnly refusal remain.
Different launch-owned channel paths retain separate source corpora. Path
identity here does not establish arbitrary symlink equivalence.

In this mode, each actual command capture has a NewOnly
`commands-N.bin.producer.json` sidecar with schema 1, kind
`command-producer-binding`, the nonzero producer identifier, transport version,
absolute command channel and capture paths, and decimal steady-clock `host_us`.
The binding is emitted after the actual command channel is created. It does not
change the MNMACE01 command record versions or payloads. A duplicate or invalid
binding makes capture incomplete, preserving the first file.

The same opt-in mode records `native-terminal-state.json` in the failed launch's
evidence directory after native-only force-stop completes. Schema 1 binds that
directory, the steady clock, the actual terminal error and counters, and nine
native state predicates: terminal, owners retired, timers stopped, frame cleared,
input detached/suspended, public restart available, retry disabled and fallback
disabled. This is a snapshot of the actual native owner state. Original process
retirement, an observed stability interval, a fresh producer and ordinary final
Quit require separate evidence; the snapshot alone proves none of those.

The failure path saves the resident report before destroying its session. That
report can therefore retain `complete: false` and `finished: false` even when
all accepted records have been written and its queue is empty. Preserve those
flags. Terminal source admission instead requires the actual owner-retirement
snapshot, balanced accepted/written counts, exact stored bytes, and successful
production-reader validation of every record's order, decoded hash and extent.
Session destruction joins the writer. MNMPROF4 has no fabricated completion
record added by this analysis. Fresh ordinary completion still requires the
normal complete/finished report.

The failed native-only summary can likewise retain a false renderer-finish
flag. Use the nine actual native predicates for the terminal interval and the
subsequent verified restart receipt for generation retirement. Neither flag is
rewritten, nor does a failed generation receive ordinary completion credit.

Working JSON sidecars are producer interchange. Citable exact sidecar bytes,
source captures, SQLite samples, native viewports and command streams are retained
through `coverage_evidence.write_report`. A failed producer has no synthetic END
and earns no normal-completion credit. Its independently compared visible prefix
must explicitly retain and exclude any uncredited terminal suffix.
