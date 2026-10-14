# Grimoire dynamic field source syntax

Build: No-CD `40209ca76705b5db04ea1974543bdec1739c68acdebdbefe2537ed025b8b7168`.
Evidence: [private comparison](../runtime/grimoire-dynamic-comparison-20261013.json)
and [literal identities](../runtime/grimoire-property-static-20261013.json).
Confidence is high for the bounded tested grammar; malformed legacy input and
complete book parsing remain outside the comparison.

Installed Grimoire text uses this form:

```text
~D "Health" 120x10 160x5 92-56-69 177-133-150 HEALTH 10
```

After the `D`, the original parser consumes a quoted byte label, an unsigned
horizontal offset with optional `x` vertical offset (otherwise zero), a
`widthxheight` pair, two groups of three decimal colour numbers, a case-sensitive
property token, and its selector. All six colour numbers are consumed but their
integer results are discarded. The descriptor contains the original default
colour state. The surrounding tokenizer recognizes `D` and `d`; property names
remain case-sensitive. Labels remain original bytes, not UTF-8 strings.

Ordinary properties take one decimal selector: `HEALTH`, `MAGIC_RESIST`,
`INTELLIGENCE`, `WEIGHT`, `VIEW_RANGE`, `FIRE_RANGE`, `GROUND_SPEED`,
`FLYING_SPEED`, `BRAVERY`, `SPELL_RANGE`, `MANA_COST`. `HTH` and `LRC` take
`creaturevopponent`, for example `10v14`; the creature selects the getter receiver
and the opponent is its argument. Human units and combat formula equivalence are
not inferred from the labels.

The source parser consumes whitespace after a successful descriptor and returns
its input pointer one byte before the next input. Its surrounding tokenizer
advances once. Zero `LRC` and zero flying speed return no bar earlier, leaving
the pointer immediately after the selector. These are distinct branches.

The native model deliberately refuses unknown properties, missing syntax or
terminators, embedded NUL, oversized input/labels/numbers, missing combat
backends and selectors outside caller-supplied datasets. These bounds are native
policies; the original is not claimed to enforce them. In particular an unknown
legacy property leaves a default value200/maximum400 bar. Font wrapping,
`~NP` page placement, drawing and live campaign use remain unvalidated here.
