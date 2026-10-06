# Original RGB565 keyed-copy outputs

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
The unchanged rectangle-keyed wrapper at `0x0058ca90` executes from a private PE
mapping against real Wine built-in IDirectDrawSurface2 system-memory objects.
Its destination interface observes Blt/BltFast, forwards every argument to the
real driver, and returns the actual HRESULT. Source and destination are distinct
real COM objects. No original instruction bytes or input files are patched.

This is an isolated original-routine experiment with controlled caller globals,
not a full game launch or a Windows 1998 driver comparison. Window-coordinate
translation is excluded by leaving the selected window object null. Fullscreen
and no-WAIT globals select the original Blt/BltFast and WAIT branches. The
selected wrapper returns after its API call, so real errors are retained without
substituting an error result or exercising another wrapper's retry loop.

The corpus includes RGB565 exact keys `0`, `0x8000` and `0xffff`, mixed/all/absent
key pixels, full/subrectangle/one-pixel/bottom-row copies, and WAIT pairs through
both APIs. Key mutation cases make two calls on the same source and destination:
the second changes the key to its 16-bit complement and starts from the first
call's complete after state. GetColorKey state is captured before and after each
call. Missing-key cases retain the actual API result and complete pixels.
Indexed keys and duplicate palette colors remain separate pending work.

Evidence: [capture](original-surface-keys-capture-20261006.json),
[portable corpus](../../tests/fixtures/surfaces/original-rgb565-keyed-corpus.json),
and [offline comparison](original-surface-keys-replay-20261006.json).
The gzip corpus records input, forwarded call, descriptors, key state and full
source/destination before/after arrays. Descriptor pointers are zeroed; no
process pointers are replay identities. Catalog pins compressed/raw hashes,
build, entry and oracle. Collector/probe/driver/input/output identities and
manifest verification are retained in the capture record.

The CPU reference derives destination pixels from before state, source rectangle
and observed native key. Native replay uses existing COPY commands for successful
calls; expected source/destination arrays occur only in CHECK diagnostics. Failed
keyed HRESULTs remain captured but are **not** claimed as native API equivalence:
the existing keyed primitive does not represent Surface2 result semantics.
Original retry/Restore, attached clipping, indexed/masked formats, physical driver
compatibility and live replacement remain pending. Native status/checkpoint
policies must not be substituted for the original error path.

Offline reproduction needs neither Wine nor the executable:

```sh
python3 -B tests/test-original-surface-keys.py
xvfb-run -a python3 tools/check-original-surface-keys.py \
  --build working/build/renderer --report working/tests/original-keys-new.json
```

For a new independent capture, use new report and fixture directories:

```sh
xvfb-run -a python3 tools/capture-original-surface-keys.py \
  --fixture-dir working/tests/original-keys-new \
  --report working/tests/original-keys-capture-new.json
```

Capture verifies the original manifest before/after, hash-checks the supported
working executable, and uses a fresh copied Wine prefix. It never launches the
original executable entry point; the harness maps its sections and executes the
selected original wrapper. The harness reserves 32 MiB at the original preferred base with loader-owned
padding and places its code after that padding, preventing Wine heaps/stacks
from occupying the original image range. The private mapping writes only that
reservation. Source files and executable hashes must remain stable
during the run. Evidence/output paths refuse overwrite.
