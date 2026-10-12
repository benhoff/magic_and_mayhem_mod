# Portable active worker context V1

The64-bytefilemapping`wine-worker-context.bin`contains16little-endianu32words:
magic`MNMWCX01`words0..1,version1word2,bytes64word3,seqlockword4,activeword5,
workerID6,commandID7,submissionID8,Win32thread9,lifetimeprefix10,unknowncommand
flag11,reserved12..15zero. Thepublisher writes oddsequence,fields,then even
sequence withsequentiallyconsistentatomic stores. Readersaccept onlyunchanged
evensequence within8attempts. Inactivefields6..11arezero. Onlyoneobservedworker
mayownactivecontext;competingworkerentryrefuses. Thewirehasnohostpointers/pixels.

Unix`egl-worker-context.bin`header64bytescontains`MNMEWC01`,version1,rowbytes128,
max131072rows;reservedzero. Rows32u32:ordinal0,phase1(before1/after2),EGLcallID2,
UnixPID3/TID4,monotonicNSlo/hi5/6,status7(stable0/missing1/unstable2),64bytewire
snapshot8..23,reserved24..31zero. Failedsnapshotpayloadiszero. Everycallrequires
pairedsameUnixthread andnondecreasingtimes. Stableidenticalactivecontext across
swap isintervalmetadata;Win/Unixthreadrelation/sourceownership/EGLframeassignment
remainseparate until independentlyvalidated. Missing/changedsnapshots areretained.
