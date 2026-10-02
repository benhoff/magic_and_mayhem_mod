
Chaos.exe:     file format pei-i386


Disassembly of section .text:

004ec780 <.text+0xeb780>:
  4ec780:	83 ec 10             	sub    esp,0x10
  4ec783:	8b 15 dc 54 6c 00    	mov    edx,DWORD PTR ds:0x6c54dc
  4ec789:	33 c0                	xor    eax,eax
  4ec78b:	89 44 24 0c          	mov    DWORD PTR [esp+0xc],eax
  4ec78f:	89 44 24 00          	mov    DWORD PTR [esp+0x0],eax
  4ec793:	89 44 24 08          	mov    DWORD PTR [esp+0x8],eax
  4ec797:	89 44 24 04          	mov    DWORD PTR [esp+0x4],eax
  4ec79b:	2b ca                	sub    ecx,edx
  4ec79d:	b8 ab aa aa 2a       	mov    eax,0x2aaaaaab
  4ec7a2:	f7 e9                	imul   ecx
  4ec7a4:	d1 fa                	sar    edx,1
  4ec7a6:	8b c2                	mov    eax,edx
  4ec7a8:	53                   	push   ebx
  4ec7a9:	c1 e8 1f             	shr    eax,0x1f
  4ec7ac:	55                   	push   ebp
  4ec7ad:	03 d0                	add    edx,eax
  4ec7af:	56                   	push   esi
  4ec7b0:	8b f2                	mov    esi,edx
  4ec7b2:	8b c6                	mov    eax,esi
  4ec7b4:	57                   	push   edi
  4ec7b5:	99                   	cdq
  4ec7b6:	f7 3d a0 54 6c 00    	idiv   DWORD PTR ds:0x6c54a0
  4ec7bc:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4ec7c0:	8b d8                	mov    ebx,eax
  4ec7c2:	2b 34 9d c2 b8 6c 00 	sub    esi,DWORD PTR [ebx*4+0x6cb8c2]
  4ec7c9:	8b c6                	mov    eax,esi
  4ec7cb:	99                   	cdq
  4ec7cc:	f7 3d 94 54 6c 00    	idiv   DWORD PTR ds:0x6c5494
  4ec7d2:	50                   	push   eax
  4ec7d3:	e8 08 6b f2 ff       	call   0x4132e0
  4ec7d8:	8b 0c 85 42 b9 6c 00 	mov    ecx,DWORD PTR [eax*4+0x6cb942]
  4ec7df:	8b 2d 94 54 6c 00    	mov    ebp,DWORD PTR ds:0x6c5494
  4ec7e5:	2b f1                	sub    esi,ecx
  4ec7e7:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
  4ec7eb:	79 06                	jns    0x4ec7f3
  4ec7ed:	03 f5                	add    esi,ebp
  4ec7ef:	78 fc                	js     0x4ec7ed
  4ec7f1:	eb 0a                	jmp    0x4ec7fd
  4ec7f3:	3b f5                	cmp    esi,ebp
  4ec7f5:	72 06                	jb     0x4ec7fd
  4ec7f7:	2b f5                	sub    esi,ebp
  4ec7f9:	3b f5                	cmp    esi,ebp
  4ec7fb:	73 fa                	jae    0x4ec7f7
  4ec7fd:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4ec801:	8b 15 dc 54 6c 00    	mov    edx,DWORD PTR ds:0x6c54dc
  4ec807:	2b ca                	sub    ecx,edx
  4ec809:	b8 ab aa aa 2a       	mov    eax,0x2aaaaaab
  4ec80e:	f7 e9                	imul   ecx
  4ec810:	d1 fa                	sar    edx,1
  4ec812:	8b ca                	mov    ecx,edx
  4ec814:	89 74 24 1c          	mov    DWORD PTR [esp+0x1c],esi
  4ec818:	c1 e9 1f             	shr    ecx,0x1f
  4ec81b:	03 d1                	add    edx,ecx
  4ec81d:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4ec821:	8b f2                	mov    esi,edx
  4ec823:	8b c6                	mov    eax,esi
  4ec825:	99                   	cdq
  4ec826:	f7 3d a0 54 6c 00    	idiv   DWORD PTR ds:0x6c54a0
  4ec82c:	8b f8                	mov    edi,eax
  4ec82e:	2b 34 bd c2 b8 6c 00 	sub    esi,DWORD PTR [edi*4+0x6cb8c2]
  4ec835:	8b c6                	mov    eax,esi
  4ec837:	99                   	cdq
  4ec838:	f7 fd                	idiv   ebp
  4ec83a:	50                   	push   eax
  4ec83b:	e8 a0 6a f2 ff       	call   0x4132e0
  4ec840:	8b 2c 85 42 b9 6c 00 	mov    ebp,DWORD PTR [eax*4+0x6cb942]
  4ec847:	8d 4c 24 18          	lea    ecx,[esp+0x18]
  4ec84b:	2b f5                	sub    esi,ebp
  4ec84d:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  4ec851:	56                   	push   esi
  4ec852:	e8 f9 1d f2 ff       	call   0x40e650
  4ec857:	51                   	push   ecx
  4ec858:	8d 54 24 14          	lea    edx,[esp+0x14]
  4ec85c:	8b cc                	mov    ecx,esp
  4ec85e:	52                   	push   edx
  4ec85f:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4ec863:	e8 68 20 f2 ff       	call   0x40e8d0
  4ec868:	8d 4c 24 18          	lea    ecx,[esp+0x18]
  4ec86c:	e8 ff 22 f2 ff       	call   0x40eb70
  4ec871:	8b f0                	mov    esi,eax
  4ec873:	51                   	push   ecx
  4ec874:	8d 44 24 20          	lea    eax,[esp+0x20]
  4ec878:	8b cc                	mov    ecx,esp
  4ec87a:	50                   	push   eax
  4ec87b:	e8 40 1a f2 ff       	call   0x40e2c0
  4ec880:	8d 4c 24 1c          	lea    ecx,[esp+0x1c]
  4ec884:	e8 57 1c f2 ff       	call   0x40e4e0
  4ec889:	2b fb                	sub    edi,ebx
  4ec88b:	8d 14 36             	lea    edx,[esi+esi*1]
  4ec88e:	57                   	push   edi
  4ec88f:	8d 0c 00             	lea    ecx,[eax+eax*1]
  4ec892:	e8 79 e3 ff ff       	call   0x4eac10
  4ec897:	5f                   	pop    edi
  4ec898:	8d 04 80             	lea    eax,[eax+eax*4]
  4ec89b:	5e                   	pop    esi
  4ec89c:	5d                   	pop    ebp
  4ec89d:	c1 e0 02             	shl    eax,0x2
  4ec8a0:	5b                   	pop    ebx
  4ec8a1:	83 c4 10             	add    esp,0x10
  4ec8a4:	c2 04 00             	ret    0x4
