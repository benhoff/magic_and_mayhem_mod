# Native fade pixel-loop splice

The fixed No-CD build's `58ed80` wrapper calls `58b660` Lock, reads height,
and enters its loop at `58edb0`. The loop's six-byte entry is
`53 8b 47 10 33 db`; the native admitted branch resumes at `58ede5`, before
the original COM Unlock. The wrapper and its callbacks remain original.

Recovered positive-dimension behavior uses `floor(width/2)*height` contiguous
DWORDs, irrespective of pitch. In WORD terms each value becomes
`(word>>1)&7bef` for zero global `6e1f88`, otherwise `(word>>1)&7def`.
Odd widths leave a trailing physical span unchanged, rather than skipping each
row's final pixel. This distinction is covered by padded and odd-width cases.

The loop finishes with EBP=height, EDX=0, ECX=pixels+processedBytes,
EAX=the original Lock pointer, and EBX restored. Its final equality CMP sets
PF/ZF and clears CF/AF/SF/OF. DF and other flags retain their incoming values.
The wrapper reuses its saved ECX stack slot for the Lock pointer; caller ECX
therefore does not simply retain its original surface-object value.

`fade_entry.S` saves GPRs, flags and the complete aligned FX region around
admission/native execution. The bounded kernel performs integer operations only.
Native refusal forwards the original loop exactly once before any pixel writes.
Bounds and writable lease admission are intentional native policy: a valid
original buffer outside those bounds continues through the original loop.
Failed original Lock/null pointer behavior is outside the admitted domain.

The comparison executes the unchanged pinned wrapper in a private PE mapping,
supplying only the external Lock/Unlock adapters. It then patches the interior
entry by script and compares the actual route and entry against that baseline.
Each case compares full physical pixels and guards, GPRs/stack, defined flags,
all saved FX bytes, Unlock arguments/entry state/count, and original fallback
counts. It seeds two finite active x87 values, defined XMM bytes and both DF
states; the resolver deliberately clobbers FP/XMM scratch state. These synthetic
adapters are not evidence of DirectDraw lifetime or installed live replacement.

The complete fallback loop must be relocated: its outer back-edge targets
`58edb1`, inside the patched six-byte entry. The retained first private failure
caught that mistake; the corrected trampoline copies all 53 self-contained loop
bytes and jumps to the original Unlock tail. No failed report is overwritten.

[Caller-state comparison](fade-loop-state-comparison-20261010.json) passes
19,440 executions: 3,888 native completions and 15,552 original-once fallbacks,
17,541,440 physical WORD comparisons, and sanitizer checks of all 65,536 values
under four format inputs. The two DF states run with and without the diagnostic
completion wrapper; both resolver and completion deliberately clobber FX state.
This does not establish arbitrary FP/unmasked exception or callback failure
behavior beyond the stated corpus.

[Synthetic Win32 admission](canvas-writer-admission-comparison-20261010.json)
passes default-off, invalid-mode and mismatched-signature refusal before patch;
15 atomic phase/dimension/pitch/mask/local-pointer/page/budget refusals; 128
native completions; and unchanged LastError. Read-only, guard, and mixed readable/
inaccessible page ranges are included. The selected owner thread and nested-call
refusals remain explicit policy, with thread races outside the tested domain.

[Installed comparison](fade-writer-live-comparison-20261010.json) runs a fresh
disposable Main/Single Player/Quick Battle/Map2/start request under private
Wine/Xvfb. All 25 native fade loops bypass their original bodies; every full
physical plane matches independent unchanged-PE replay, for 12,000,000 compared
WORDs and no forwards. This is partial native CPU fade replacement on the
original Lock lease. VirtualQuery establishes mapped writable ranges, not a
general proof of driver allocation/lifetime. It does not transfer complete
surface/frame ownership or suppress other producers.

The separate `MnmCanvasWriters.dll` is opt-in and leaves the ordinary launch
unchanged. This experiment stages only the menu action adapter and fade DLL;
it does not load the glyph/scene observer. The capture decoder rejects incomplete
and malformed records. Its bounded 128-record/256 MiB before/after diagnostics
feed only independent validation, never a native frame consumer.

Reproduce with a prospective declaration and a private display:

```sh
python3 tools/draft-coverage-claims.py --behavior RS.fade-live-pixels \
  --scenario native-fade-menu-startup-20261010 --output working/tests/fade-live-next.json
xvfb-run -a -s '-screen 0 1280x1024x24' \
  python3 tools/capture-fade-writer-game.py --claims working/tests/fade-live-next.json
```

Images, panels, glyphs, failed original Lock behavior and complete renderer
replacement remain outside this installed behavior's scope.
