/* Owned fixture text, reserving the original caller RVAs inside this PE image.
 * Wine reserves the enclosing image allocation; VirtualAlloc inside it refuses.
 * The fixture writes only its own padding, never an original executable. */
.text
.space 0xbe000, 0x90
