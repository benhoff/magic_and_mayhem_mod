
Chaos.exe:     file format pei-i386


Disassembly of section .text:

0040e290 <.text+0xd290>:
  40e290:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
  40e294:	8b c1                	mov    eax,ecx
  40e296:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  40e29c:	85 d2                	test   edx,edx
  40e29e:	7d 09                	jge    0x40e2a9
  40e2a0:	03 d1                	add    edx,ecx
  40e2a2:	78 fc                	js     0x40e2a0
  40e2a4:	89 10                	mov    DWORD PTR [eax],edx
  40e2a6:	c2 04 00             	ret    0x4
  40e2a9:	3b d1                	cmp    edx,ecx
  40e2ab:	72 06                	jb     0x40e2b3
  40e2ad:	2b d1                	sub    edx,ecx
  40e2af:	3b d1                	cmp    edx,ecx
  40e2b1:	73 fa                	jae    0x40e2ad
  40e2b3:	89 10                	mov    DWORD PTR [eax],edx
  40e2b5:	c2 04 00             	ret    0x4
