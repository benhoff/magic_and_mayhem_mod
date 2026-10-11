# Enabled native movie startup

`NR.movie-enabled-startup` owns a bounded integration scenario: both original
intro calls with movies enabled, native Qt completion, then renewed original
frame publication. The existing control-zero movie hook, media V1 wire and
native broker stay separate from complete rendering replacement. Original
post-movie drawing remains active. This scenario does not validate other movie
controls/placement, audible devices, original DirectShow pixels, active World
ownership or complete session takeover.

The runner clones a disposable game and Wine prefix, pins source/staged hashes,
reserves Wine execution and leaves original media read-only. It requires two
installed intro completions and at least three subsequently published frames.
Decoder failures, unsupported records and absent return to presentation fail.
The Qt server timeout accommodates the complete installed clips.

The original requests use `.\fmv/intro0.avi` and `.\fmv/intro1.avi`. The broker
previously rejected that single leading current-directory component, silently
leaving movies to the original fallback. It now admits that safe relative form
while continuing to reject interior `.` and all `..` components.

The enabled-intro scenario passes: 1,777 Intro0 frames and 1,004 Intro1 frames
complete with native status/result 3, then three original frames resume publication.
A final reproduction fingerprints both input movies before execution and verifies
them afterward. Its [current report](native-enabled-movie-startup-20261010.json) passes with stable
source and input hashes; no whole-screen/session
replacement promotion follows from this bounded observation.
