# Original API CPU source witnesses V1

`api-source-bytes.bin`header64bytes contains`MNMSBY01`,version1,rowbytes192,
build0x40209ca7,max4096points,max1024planes,1GiBplane cap,reservedzero.
Rows48u32:ordinal0,phase1(entry1/return2/terminal4),APIentry2,Winthread3,tick4,
source/destinationAPIgeneration5/6,status7,oracleordinal8,planebytes9,
reserved10..15zero,before64bytesourcemetadata16..31,aftermdata32..47.

Status0admits exact800×600RGB565tightrows/960000bytesandidenticalbefore/after
metadata;statuses1invalidoutput/ABI,2unknown/stopped/failure/pendingprimitive/
unsupportedframes,3extent/capacity/readability,4metadatachange,5transfergeometry
retainrefusalswithoutplane. A knownsinglecaller-ownedtag7transferframe mayremain
open;allotherpendingframes refuse. This doesnot freezeoriginalbytes.

Planes`api-source-oracle-NNNNNN.565`areexternaloracles only. Nativeinputstays
ownedproducerparameters/pinnedencodedassets;nooriginalCPU/rasteroutput enters.
EveryobservedAPIentryrequiresentry/returnrows;terminalcontainsreason2,plane
count8,totalbytes9andotherwordszero. Admissionchecksfileextent/hash separately.
Fullimmutableownership/allwriters/worker/EGLsourceassignmentremainunproved.
