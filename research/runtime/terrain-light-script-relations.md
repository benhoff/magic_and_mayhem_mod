# Script relation production and reveal

The pinned no-CD function 0056a8d0 consumes owner/character names from an
action record. The sentinel string is read at 005fc874. Action+2cc/+2ec
selects a named creature owner change; +32c/+30c disables relation18,
+34c/+30c enables it, and +36c/+30c changes share20 according to +38c.
Pair setters precede enumeration. Active, light-enabled creatures for either
owner receive ordered cleanup and conditional reveal services. Character
reveal also requires the opposite owner's directed relation18 and share20
bytes toward the current player, an enabled character and a valid index.

The reconstructed caller retains separate owner name resolution, lifecycle,
visibility handle, notification and script completion services. The fixture
records their call order and controlled effects. It executes the complete
original body in a private PE32 mapping, preserving its bytes, and compares
native and ASan/UBSan output over 2,048 combinations of optional names, owner
indices, character presence, source activity, source owner, control states and
raw directed relations. Null/malformed guest structures are outside this
valid-input contract. This comparison does not recover the service bodies.

The sharing name at +36c requires its partner at +30c. The original's
missing-partner branch enters character reveal using uninitialized owner locals;
script-debug02 and script-debug-events02 retain the observed isolated fault.
The valid comparison domain supplies both names. Native execution explicitly
refuses the malformed pair before mutating state.
