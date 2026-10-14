# Character pricing: selected original positive domain

No-CD build40209ca7. Original calculator `0x004b5d10` consumes three stat-bar
and three talisman edited values and configured binary32 price coefficients.
The native model lives in `reconstruction/campaign/character_cost.*`; original
math error/overflow behavior and live control admission remain separate.

Static instructions select integer `index = value / increment`. Decrease uses
that index, increase uses index+1. Each price is positive-domain truncation of
`initial + (linear * index) ^ exponential`. Three talisman rows share their
coefficient group but use their own edited values. Price refresh runs after each
original action; a successful increment followed by decrement should refund
the same indexed price within this domain.

The private fixture maps the complete pinned executable without code stubs.
Original calculator, x87 power helper0x59bcc0 and truncation helper0x59bee0 remain
active. Runtime math state and x87 control word0x27f are supplied explicitly.
The native model uses double arithmetic with binary32 coefficients and refuses
undefined/out-of-range inputs as an intentional safety policy, separately
registered from recovered positive-domain pricing.

Validation compares installed coefficients and every configured value bound,
including nonmultiples of increments, plus bounded fractional/constant/power-of-two
coefficients. Complete source/contract declarations precede execution; originals
are verified before/after every run. Native input/output refusals are not original
error-path equivalence. Live Character edits, menu ownership/return, general
floating-point edge cases and disk persistence require separate evidence.

[UI49 original comparison](character-pricing-original-20261012.json) passes
1,643 calculator executions and19,716 integer prices, plus12 native refusals.
The source/contract declaration preceded execution and matched the full tested
closure. All original calculator/math bytes remain unchanged. The installed
coefficients are Mana/Health(5/20,0,.6,1.3),Control(1,0,.5,1.4),Talismans
(1,0,22,1.1). Earlier wrong-key and reversed-power-operand hypotheses failed in
unique retained working runs; their declarations and logs were not overwritten.

The native costs remain caller-supplied at the widget boundary. Live adapters
must bind values, coefficients and price schedules to current original menu
ownership and prevent stale or forged grants. This comparison does not enable
first-region Character controls. The shared campaign CMake change leaves prior
Realm evidence historical; its model source is unchanged, and no fresh Realm
comparison is asserted by the pricing result.
