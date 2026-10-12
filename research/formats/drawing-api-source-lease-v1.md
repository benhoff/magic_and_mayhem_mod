# Diagnostic API source read-only lease V1

The128byterowjournal`api-source-lease.bin`has64byteheader`MNMSLS01`,version1,
rowbytes128,build0x40209ca7,max4096points;reservedzero. Rows32u32:ordinal0,
phase1(begin1/checkedrelease2/terminal4),APIentry2,Winthread3,tick4,sourceAPIid5,
result6,leaseID7,reserved8..15zero,64bytesourcepoint16..31.

Beginresult highbit marks success;low31bitsaremonotonicleaseID,result1..4are
refusalswithlease0. Release requires exactID/owner/COM/canvas/pixeladdress/extent
and allprotectedregionsstillreadonly,thenrestoresexactpriorprotection. Refused
begins retain release status2/nolease;failedrelease ofacceptedlease refusesrun.
Terminal carries reasonword2,otherunusedzero. Pairedthread/source/ID required.

Sourcepagesareexplicitdiagnosticpermissionpolicy. Coveredrangewalkscommitted
regionsinoneallocationwithoriginalRWprotection,thenRO;sharedheappagesmayfault.
Writefaultrecord`MNMSLFT1`retainslocalfault/instruction/range/ownerforresearchand
refusesexit99. Originalfiles/bytesareunmodified. TheleaseendsatAPIreturn and
`async_egl_completion_covered`staysfalse. Noallroute/ownership/scanoutclaim.
