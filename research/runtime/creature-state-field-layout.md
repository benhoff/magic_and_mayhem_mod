# Original creature state composition: direction versus previous motion (NS33)

The setup and action projections previously reused `CreatureExecution.previousMove`
for independent original fields. Their isolated fixtures compared their own byte
projections, but that shared native member could not compose both contracts.

| Original field | Confirmed role | Native owner |
| --- | --- | --- |
|614 | Previous direction saved by510e80 before installing608 | `SegmentInitializationState.previousDirection` |
|16c | Previous-motion flag set/cleared by50db60 action changes | `CreatureExecution.previousMove` |
|170 | Prior movement rate captured by50db60 | `CreatureExecution.previousMoveVariant` |

The setup primitive now writes only `previousDirection`. Its paired fixture packs
and checks all three fields independently. Both action fields retain distinct
initial values through setup; they are not aliases of614 or the separateb73
continuity marker. Evidence is the retained setup/action instruction exports and
the byte-checked original fixtures. Confidence is high within these scoped bodies.

Four prospectively declared runs pass81,920 paired original32/native64/sanitized64
cases: setup24,576, request24,576, displacement8,192 and special callbacks24,576.
The native-only refusal totals are10,5,4 and5 respectively. Each new result has
new immutable identity; earlier source hashes and range evidence remain retained.
All thirteen rebuilt native CTests pass against the separated state.

This corrects an interface for future composition. Actual native World/session,
installed construction, canonical sample/route/RNG state and portable continuation
remain independent pending boundaries. No simulation integration or live
replacement is promoted, and no gameplay balance rule is changed.
