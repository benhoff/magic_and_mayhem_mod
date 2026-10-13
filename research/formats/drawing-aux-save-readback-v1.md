# Auxiliary save readback journal v1

`MNMAUX01` is an opt-in little-endian64byte header/256byte row diagnostic.
The header pins NoCD40209ca7,256attempts,source queues30..50,configured source
canvas1(800×600)/destination5(40×40),and3200RGB565bytes per admitted image.
Scalar canvas IDs are checked per sample; this does not prove cross-launch
addresses or a general auxiliary identity. No process pointer enters wire.

Sample words:0ordinal,1kind1,2parentAPIentry,3owner,4tick,5originalresult,
6originalLastError,7surface-interface kind,8refusal reason,9..11extraGetDesc/
Lock/Unlock counts,12..14extraHRESULTs,15image ordinal,16..18width/height/pitch,
19..22bits/R/G/B masks,23pixel bytes,24..55the original128byte source pair,
56..61post-probe destination flags/sequence/queue/ID/dirty/pending metadata,
62..63zero. Failed probes have no image. Successful RGB565images have40×40
packed WORDs copied only while the extra Lock owns its mapped view; padding
is omitted. `aux-save-oracle-NNNNNN.565` is an external output oracle.

Terminal kind4 records reason1/2,owner/tick,attempts/images/refusals/budget skips,
active probe and parent pending counts; remaining words arezero. Mid-call close,
missing Unlock,extra artifacts,unsupported shape/masks/metadata or count drift
refuse complete admission. Unlock failure remains explicit and disallows a claim
that original driver state was restored. Default mode creates no sidecar and
makes no extra driver calls. These are additional diagnostic calls, not baseline
application calls or native rendering inputs.
