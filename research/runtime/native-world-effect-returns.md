# World effect raster low16 returns

World stores AX after the selected effect leaves. The native batch predictor now
models zero for backends7/13/14/15/16 only after admission of the declared valid
owned-frame/clip domain. Unknown backend identities refuse. Upper EAX and CPU
flags are outside this recovered return contract.

`tools/test-world-effect-returns.py` executes unchanged580a90/57fe10/581200/
580260/5806f0 instructions over966 retained authored canvases. All67421 selected
calls return low16zero:176additive,356outline,66401tint,323source-placement and
165destination-sampling calls. Their pixels still equal the retained independent
canvases. The default reference entry/output and model refusal controls pass.
The original bodies are unchanged; a source wrapper observes their return values.
Original manifest/hash and input/source bookends pass. Earlier compilation and
sandbox SIGSYS attempts are retained separately and supply no positive proof.

Static inspection shows zero-initialized local status, with zero on the
non-overlap exit. The local4/5 status assignments require vertical row bounds
outside a positive non-wrapping frame, after earlier overlap checks have already
excluded that geometry. This explains the observed zero within this input domain;
it does not establish arbitrary malformed-pointer or wrapped-integer behavior.

These are method returns, separate from pixel reconstruction and live caller
admission. The producer policy records its own owned wire, ABI and GPU evidence.
Source lifetime, natural route reachability and complete World/replacement scope
remain open.
