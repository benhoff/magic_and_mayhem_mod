# Packed opcode4 cast command

Build: No-CD40209ca7.534000 reads a33-byte command and updates a48-byte descriptor.
Offsets below are bytes from the command start. Fields are unaligned little-endian;
use byte decoding rather than a shared C++ object layout. Instruction/export evidence
and original comparison are recorded in [runtime decoder research](../runtime/cast-descriptor-expansion.md).

| Command offset | Size | Descriptor word | Operation |
| --- | ---: | ---: | --- |
|0c|1|0|Unsigned expansion |
|0d,0f,11,13,15,17|2 each|1..6|Unsigned little-endian expansion |
|19,1a|1 each|7,8|Unsigned expansion;ff becomesffffffff |
|1c|4|10|Copy bits |
|20|1|11|Unsigned expansion |

Descriptor word9 is caller-owned and untouched. Command+00..0b and+1b are not read
by this function. Header validation, opaque field meanings, cast admission and
network/recursive buffer handling are separate contracts; this structure alone
does not establish them. Confidence is high for these exact byte mappings.
