# Actual engine update identities in drawing acquisition

Reuse the existing pinned memory-only engine cadence observer in separately
enabled original DRAW02 capture. Normal original Quit flushes its trace after
API/EGL capture closes. Decode actual accepted-update/world-counter windows,
retaining non-advancing windows and excluding the incomplete final one.
Contiguous update counts do not bind producer queues or prove all presentations
owned. Synthetic guards precede original execution; all complete semantic
requirements and the120window owned-source requirement remain pending.

`--engine-cadence` builds and adds a separately pinned `MnmEngineCadence.dll`
import to disposable executable staging, preserving all original sections. The
runner sets its capture path only for this opt-in. The existing observer guards
all fifteen instruction sites, preserves original calls/stores and retains
bounded events in private memory. Original clock, game-speed and drawing bodies
remain active. API/EGL journals close at Main, then the original Main Quit action
runs and the launcher must exit normally; cadence writes occur on DLL detach.
Shutdown occurs outside the completed physical acquisition interval.

The window decoder uses the cadence trace’s QPC frequency and owner thread,
checks selected call nesting and original return callers, and starts a window
at each accepted-update marker. A complete window requires the next accepted
marker; the last window is retained as explicitly excluded. Advancement requires
one observed world-counter store with exact unsigned delta one; the sampled mode
byte is retained without inferring pause policy. Sequence and counter continuity
both constrain consecutive runs. Non-advancing windows and queue records outside
complete windows stay visible. Selected queue callers must be the pinned
`004fd4b0` or `004fd559` return addresses, and queue counter values must match
the window. A count or caller match alone does not bind producer source identity.

Six synthetic tests pass 120 complete windows plus final exclusion, non-advancing
run breaks, wrap, sequence gaps, observed advancement without inferred mode policy
and malformed counter/caller/nesting refusals. Original capture and exact
producer/update/presentation binding remain separate milestones.

Original run 01 exits through original Quit and closes 68 complete consecutive
counter/sequence-advancing windows, each with one selected World queue; final
sequence 69 is excluded. There are no sampled non-advancing windows in this run.
API/EGL acquisition retains 332/331 physical witnesses. The source recorder
still closes only 16 returns. Exact producer/update binding and 120 contiguous
owned/presented windows remain open. The next increment binds a queue’s cadence
identity to its producer entry, then extends bounded acquisition and validates
its decoders before using a longer original recording.
