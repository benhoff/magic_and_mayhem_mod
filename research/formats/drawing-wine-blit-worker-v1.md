# Selected Wine opcode42worker V1

`wine-blit-worker.bin`header64bytes:magic`MNMWBK01`,version1,rowbytes192,
build0x40209ca7,131072rowcap,opcode42;reservedwordszero. Rows48little-endianu32:
ordinal0,phase1(entry1/return2/terminal4),callID2,opcode3,Win32thread4,tick5,
originalreturnLastError6,exactlifetimeprefix7,destinationID8/generation9,
sourceID10/generation11,destination/source-subresource12/13,destinationbox14..19,
sourcebox20..25,flags26,effects27..32,filter33,reserved34..47zero.

Original command100bytefields are read as owned scalars;resourceaddresses become
opaque IDs only. Entrygeneration values match the selected lifetimejournal at
prefix7;pairedreturn preserves all ownedinputs and recordsoriginalLastError.
Thevoidcdeclhandler runs once outside recorderlocks. Pendingworker countdefers
explicit sessionterminal until observedcallsreturn. Terminalreason2,totalcalls3,
thread4,tick5;otherwordszero. Exactpairedcount/closure required foradmission.

OnlyinstalledWineopcode42dispatchtableentryisobserved.Otheropcode/routes,
commandqueueidentity,sourceownershipandasyncEGLassignmentremainunproved.
This journaldoesnottransportoriginalpixels or claimcompletepresentedframes.
