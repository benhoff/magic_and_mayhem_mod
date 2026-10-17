# Native pixel forwarding guard

Implemented and exercised in an isolated checkout, 2026-10-17.

`MNM_NATIVE_PIXEL_NO_FORWARD=1` adds a synchronous guard to the common
`NativeDrawProducerCountV1` callback used by all six optional producer DLLs.
The callback terminates with exit126 before returning to an original fallback.
It also refuses missing/duplicate/unexpected route installations. The ordinary
partial mode keeps its established forwarding behavior.

The launcher stages this policy explicitly in the hash-bound game manifest;
the runner clears ambient selection and authorizes it only after all six
families and fresh route counters are selected with owned native drawing.
The actual-game harness exposes `--no-pixel-forwarding`, which requires
`--other-pixel-producers`. Refusal reports preserve reason and installation masks.

This enforces one necessary complete-mode boundary. It does not establish that
every original pixel writer uses these six callbacks, complete startup admission,
World bypass, all aliases/readers, full presented-session parity or 60 FPS.
The complete-native release mode remains unavailable while those gates are open.


Eleven actual PE32 cases exercise the common callback. Ordinary partial selectors
reach the stand-in original fallback; strict native success preserves LastError.
Strict forwarding, missing/duplicate/unexpected/uninstalled routes, counters and
invalid selectors terminate before the fallback marker. The new campaign run
uses the same guard; route counters distinguish installed routes from routes
actually exercised. Zero forwarded calls alone cannot establish coverage of an
unexercised route or an unknown original writer.


The successful [NVIDIA storage run](native-completion-native-storage-20261017.json)
records8937native optional-producer calls and zero forwards. It exercises17of43
installed routes;26remain unexercised in that journey. The guard remains enabled
through Save/Load, both display rebuilds, Main Load and normal Quit. There is no
claim that every original writer belongs to those installed routes.
