# Native CD fade gain

The pinned original stop caller0x482010 reads raw volume by calling first-mixer
helper0x482ac0, then converts it at0x4820d8. It bypasses the standalone getter
0x4818b0 already routed to native music. Under the native zero-mixer policy,
the raw local remains0 and the original descending fade steps are skipped.

The native adapter guards the five original bytes`b8 01 78 00 78` and replaces
that MOV with a CALL. The helper preserves flags/registers and LastError, gets
the native per-music level, supplies`level*4369` to the original stack local,
then restores the MOV's magic constant in EAX. The raw slot is56 bytes above
the helper's stack after its CALL return address, flags and eight saved registers.
Original conversion, level steps, busy polling, STOP0 and saved-level restoration
remain in the original caller. No desktop mixer is created or modified.

All original guards are checked before mutation. Fade and volume hooks share
an RX page; query and mixer hooks share another. Protection is restored in
reverse acquisition order. Music feature32 records the guarded fade-gain supply;
the full selected feature mask becomes63. The music wire remains version1/1024
bytes; this adds a feature bit without changing fields or operations.

Original stop/fade caller recovery is independent: bounded scripted MCI, mixer,
volume and positive nonwrapping clock callees compare the unchanged original
body with its native model. Actual callee recursion, driver timing, clock wrap,
speaker waveform and whole playlist semantics remain separate. The PE converter
fixture uses the original stack/conversion instructions with synthetic receiver
and native gain, drift rejection, LastError and page protection checks. A live
campaign is still needed before claiming integration of this hook.
