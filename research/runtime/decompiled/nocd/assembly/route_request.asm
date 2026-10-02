
Chaos.exe:     file format pei-i386


Disassembly of section .text:

00512800 <.text+0x111800>:
  512800:	51                   	push   ecx
  512801:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
  512805:	55                   	push   ebp
  512806:	a1 4c 17 5e 00       	mov    eax,ds:0x5e174c
  51280b:	8b e9                	mov    ebp,ecx
  51280d:	56                   	push   esi
  51280e:	8d 4c 24 08          	lea    ecx,[esp+0x8]
  512812:	57                   	push   edi
  512813:	51                   	push   ecx
  512814:	51                   	push   ecx
  512815:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  512819:	8b cc                	mov    ecx,esp
  51281b:	52                   	push   edx
  51281c:	c6 05 54 03 69 00 01 	mov    BYTE PTR ds:0x690354,0x1
  512823:	e8 88 c6 ef ff       	call   0x40eeb0
  512828:	8b 44 24 20          	mov    eax,DWORD PTR [esp+0x20]
  51282c:	51                   	push   ecx
  51282d:	8b cc                	mov    ecx,esp
  51282f:	50                   	push   eax
  512830:	e8 6b c0 ef ff       	call   0x40e8a0
  512835:	8b 54 24 20          	mov    edx,DWORD PTR [esp+0x20]
  512839:	51                   	push   ecx
  51283a:	8b cc                	mov    ecx,esp
  51283c:	52                   	push   edx
  51283d:	e8 4e ba ef ff       	call   0x40e290
  512842:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
  512846:	b9 48 01 69 00       	mov    ecx,0x690148
  51284b:	50                   	push   eax
  51284c:	55                   	push   ebp
  51284d:	e8 ae 8f 03 00       	call   0x54b800
  512852:	8d bd 6b 09 00 00    	lea    edi,[ebp+0x96b]
  512858:	b9 83 00 00 00       	mov    ecx,0x83
  51285d:	be 48 01 69 00       	mov    esi,0x690148
  512862:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  512864:	8b 85 7b 09 00 00    	mov    eax,DWORD PTR [ebp+0x97b]
  51286a:	33 c9                	xor    ecx,ecx
  51286c:	3b c1                	cmp    eax,ecx
  51286e:	c7 85 8b 0b 00 00 01 	mov    DWORD PTR [ebp+0xb8b],0x1
  512875:	00 00 00 
  512878:	89 8d 03 0d 00 00    	mov    DWORD PTR [ebp+0xd03],ecx
  51287e:	75 06                	jne    0x512886
  512880:	89 8d 8b 0b 00 00    	mov    DWORD PTR [ebp+0xb8b],ecx
  512886:	33 c9                	xor    ecx,ecx
  512888:	5f                   	pop    edi
  512889:	3b c8                	cmp    ecx,eax
  51288b:	5e                   	pop    esi
  51288c:	1b c0                	sbb    eax,eax
  51288e:	5d                   	pop    ebp
  51288f:	f7 d8                	neg    eax
  512891:	59                   	pop    ecx
  512892:	c2 10 00             	ret    0x10
