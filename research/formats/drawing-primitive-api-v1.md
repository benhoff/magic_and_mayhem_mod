# Primitive point and API identity journal V1

Prospective diagnostic format, 2026-10-17. See the exact DWORD layouts in
`protocols/include/mnm/drawing_primitive_point_v1.h`. The read-only PE32 stdcall
export is `SceneDrawingPrimitivePointV1(out, 192)`. Wrong size/null output
refuses; valid output preserves LastError. Point state distinguishes active,
no active wrapper, stopped and unavailable recording. IDs are run-local.

`api-primitive-identities.bin` uses magic MNMPAPI1, a 64-byte header and
256-byte rows, at most 786433 including its terminal. Three rows per observed
surface API retain entry, original return and completion separately. Every
point has its own QPC; concurrent point acquisition and journal writes need
not have globally sorted clocks. Per-call clocks must advance. All unused
fields are zero; closed API identities/results and the independent lifetime
journal are mandatory crosschecks for a fully bound diagnostic analysis.

No pointer, pixel plane, permission to access memory, or ownership promise is
exported. The full request header is parameter metadata, not COPY source bytes.
