# DRAW-02 held-source rejection diagnostics

Step 2 remains in progress. The combined resource/source-lease capture original01 exits with a new source lease active. Its retained artifacts do not identify the first failed guard. Teardown state is an effect, not evidence of a shutdown race.

The next bounded experiment writes one separate 192-byte failure record before exit99, including reason, API/lease/phase, source getter status and metadata, and readable scalar rectangle inputs. It leaves the successful snapshot journal unchanged. Synthetic negative cases must validate the record before original reproduction.

The PE32 fixture passes all six cases. Negative records identify getter refusal,
invalid metadata, partial rectangle, changed canvas and changed protected bytes
independently. Three decoder tests reject malformed records and retain unknown
rectangle inputs without dereferencing them.

`source-held-refusal.bin` contains a 64-byte MNMSHR01/version1 header and exactly
one 192-byte record. Header fields retain command/completion and prior snapshot
counts. The record owns API/lease/phase/thread/time, source getter result, optional
readable rectangle scalars, destination coordinates, flags and two 64-byte source
metadata samples. It never serializes process pointers or source pixels. The
accepted MNMSHB01 journal is unchanged; refusal stays exit99 and is never admitted
by the capture runner. Original reproduction remains pending.

Two original-active repetitions with the new diagnostic completed normally and
produced no rejection record. Each closes 118 protected pairs and 222 exact
source commands; they retain 5137/5209 resource calls and 335/366 API physical
witnesses respectively. Both use manifest verification and actual normal Quit
exit0. The earlier resource-original01 refusal remains unreproduced and its
cause remains unassigned. Successful repetitions do not retroactively validate
that failed run or establish a shutdown fix.

The second combined run also supplies 104 eligible protected-source/worker/EGL
witnesses that match native replay exactly, with 262 exclusions retained. The
first comparison attempt is preserved as an Xvfb failure with zero frames
compared; the rerun exits0 and compares all104. This validates only the admitted
cohort and does not resolve the earlier refusal or whole DRAW-02 acceptance.
