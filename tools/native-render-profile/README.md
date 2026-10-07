# Headless native-renderer profile

This diagnostic uses NVIDIA EGL device contexts without X, Wayland or a desktop.
It builds the unchanged native decoder, consumer and GlBlitter sources, presents
shared GPU leases into an RGBA framebuffer, and compares the same captured prefix
with Mesa llvmpipe. It refuses a missing requested EGL vendor or unexpected GL
vendor. The adapter rejects windows and swapchains. It is a profiling tool,
separate from the application platform backend and legacy producer.

Requires the installed Qt 6.11+ Gui/Core private development modules, desktop
OpenGL/EGL and an accessible NVIDIA device. The QPA adapter uses private Qt
interfaces: rebuild against the installed Qt version after any Qt upgrade.
Generated builds, raw reports and CPU samples stay under `working/`.

```sh
cmake -S tools/native-render-profile -B working/build/native-render-profile \
  -DCMAKE_BUILD_TYPE=Release
cmake --build working/build/native-render-profile --parallel 4
python3 tools/native-render-profile/run.py PATH_TO_COMMAND_ARCHIVE \
  --replays 12 --perf
```

The runner removes display and inherited driver overrides from its children,
selects each EGL vendor explicitly, and records the GL renderer. Device access
may require running outside a restricted filesystem/device sandbox. It does not
change permissions, system drivers, the desktop or original files.

Only exact archive bytes through the final complete PRESENT before a terminal
marker are selected; no command is rewritten. Trailing partial frame work is
excluded. The streaming decoder and consumer check sequence, ownership and
formats. Two warmup replays precede twelve measured replays in each of four
hardware/software and serial/throughput combinations. An independent 2x2 RGB565
fixture checks full final RGBA bytes, including keyed copy transparency and a
later update; hardware/software capture output must also match exactly. Every
command and PRESENT is executed; cleanup must retire all owned native surfaces.
Failure/refusal, arbitrary formats, oversized streams and other devices are
outside this execution's validation scope.

Serial frame completion measures CPU submission plus its remaining GPU drain.
Backend GPU timestamps span native drawing through shared-context sampling;
the interval includes CPU feeding gaps and synchronization, so it is not pure
shader busy time. Throughput avoids timer queries and per-frame `glFinish`, then
waits once for completed output at the end of each replay. Decode, context/shader
construction, cleanup and one diagnostic final-frame readback are outside the
reported render timings. CPU sampling includes process startup, warmup, replay
and diagnostic cleanup; its inclusive caller percentages overlap.

The first retained execution replayed a 15-frame startup capture containing
3,987 commands and 3,898 uploads. On the TITAN RTX, queued completion throughput
was 96.3 frames/sec median. After the initial checkpoint, serial completion was
10.66 ms median, 13.83 ms p95 and 18.75 ms maximum; remaining wait after CPU
submission was 0.033 ms median. Matching llvmpipe completion was 7.74 ms median,
9.59 ms p95, with 132.9 frames/sec queued throughput. Final pixels match across
all modes. About 70% of sampled CPU time was under `QOpenGLContext::makeCurrent`
and 84% under native surface update. Context batching is an optimization candidate,
not an implemented or measured optimization. EGL timings do not establish the
same costs for desktop GLX.

These are finite-workload renderer capacity and service time, not live-game FPS
or input-to-screen latency. Battles, producer/IPC queue age, full Qt widgets,
compositor/vsync/scanout and physical input/display remain unmeasured. The earlier
Xvfb live observation's approximately20fps includes a different pipeline and
pacing, and cannot be directly compared with this unpaced EGL replay.

The durable [execution record](../../research/runtime/native-render-headless-profile-20261007.json)
contains full timings, source/artifact fingerprints, CPU summaries and the exact
compressed prefix. To reproduce its input without original media:

```sh
python3 - <<'PY'
import base64, json, pathlib, zlib
r = json.loads(pathlib.Path('research/runtime/native-render-headless-profile-20261007.json').read_text())
p = pathlib.Path('working/headless-profile-capture.bin')
assert not p.exists()
p.write_bytes(zlib.decompress(base64.b64decode(r['workload_zlib_base64'])))
PY
python3 tools/native-render-profile/run.py working/headless-profile-capture.bin \
  --replays 12 --perf
```
