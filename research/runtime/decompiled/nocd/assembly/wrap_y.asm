
Chaos.exe:     file format pei-i386


Disassembly of section .text:

0040e8a0 <.text+0xd8a0>:
  40e8a0:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
  40e8a4:	8b c1                	mov    eax,ecx
  40e8a6:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  40e8ac:	85 d2                	test   edx,edx
  40e8ae:	7d 09                	jge    0x40e8b9
  40e8b0:	03 d1                	add    edx,ecx
  40e8b2:	78 fc                	js     0x40e8b0
  40e8b4:	89 10                	mov    DWORD PTR [eax],edx
  40e8b6:	c2 04 00             	ret    0x4
  40e8b9:	3b d1                	cmp    edx,ecx
  40e8bb:	72 06                	jb     0x40e8c3
  40e8bd:	2b d1                	sub    edx,ecx
  40e8bf:	3b d1                	cmp    edx,ecx
  40e8c1:	73 fa                	jae    0x40e8bd
  40e8c3:	89 10                	mov    DWORD PTR [eax],edx
  40e8c5:	c2 04 00             	ret    0x4
