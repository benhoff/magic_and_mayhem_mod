
Chaos.exe:     file format pei-i386


Disassembly of section .text:

0054b800 <.text+0x14a800>:
  54b800:	81 ec d4 00 00 00    	sub    esp,0xd4
  54b806:	53                   	push   ebx
  54b807:	55                   	push   ebp
  54b808:	56                   	push   esi
  54b809:	33 db                	xor    ebx,ebx
  54b80b:	57                   	push   edi
  54b80c:	53                   	push   ebx
  54b80d:	53                   	push   ebx
  54b80e:	53                   	push   ebx
  54b80f:	53                   	push   ebx
  54b810:	8b e9                	mov    ebp,ecx
  54b812:	53                   	push   ebx
  54b813:	53                   	push   ebx
  54b814:	53                   	push   ebx
  54b815:	8d 8c 24 9c 00 00 00 	lea    ecx,[esp+0x9c]
  54b81c:	89 9c 24 90 00 00 00 	mov    DWORD PTR [esp+0x90],ebx
  54b823:	89 9c 24 94 00 00 00 	mov    DWORD PTR [esp+0x94],ebx
  54b82a:	89 9c 24 98 00 00 00 	mov    DWORD PTR [esp+0x98],ebx
  54b831:	e8 1a 2a ec ff       	call   0x40e250
  54b836:	8b 94 24 e8 00 00 00 	mov    edx,DWORD PTR [esp+0xe8]
  54b83d:	8b 84 24 ec 00 00 00 	mov    eax,DWORD PTR [esp+0xec]
  54b844:	b9 83 00 00 00       	mov    ecx,0x83
  54b849:	8b fd                	mov    edi,ebp
  54b84b:	8d b2 6b 09 00 00    	lea    esi,[edx+0x96b]
  54b851:	89 44 24 70          	mov    DWORD PTR [esp+0x70],eax
  54b855:	8b 84 24 f4 00 00 00 	mov    eax,DWORD PTR [esp+0xf4]
  54b85c:	89 54 24 6c          	mov    DWORD PTR [esp+0x6c],edx
  54b860:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  54b862:	8b b4 24 f8 00 00 00 	mov    esi,DWORD PTR [esp+0xf8]
  54b869:	8b 8c 24 f0 00 00 00 	mov    ecx,DWORD PTR [esp+0xf0]
  54b870:	89 84 24 9a 00 00 00 	mov    DWORD PTR [esp+0x9a],eax
  54b877:	8b 04 85 42 b9 6c 00 	mov    eax,DWORD PTR [eax*4+0x6cb942]
  54b87e:	8b 3c b5 c2 b8 6c 00 	mov    edi,DWORD PTR [esi*4+0x6cb8c2]
  54b885:	89 8c 24 96 00 00 00 	mov    DWORD PTR [esp+0x96],ecx
  54b88c:	03 c7                	add    eax,edi
  54b88e:	89 b4 24 9e 00 00 00 	mov    DWORD PTR [esp+0x9e],esi
  54b895:	03 c1                	add    eax,ecx
  54b897:	8b 0d dc 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c54dc
  54b89d:	8b 72 10             	mov    esi,DWORD PTR [edx+0x10]
  54b8a0:	89 9c 24 a2 00 00 00 	mov    DWORD PTR [esp+0xa2],ebx
  54b8a7:	8d 04 40             	lea    eax,[eax+eax*2]
  54b8aa:	89 9c 24 b2 00 00 00 	mov    DWORD PTR [esp+0xb2],ebx
  54b8b1:	8b 3c b5 c2 b8 6c 00 	mov    edi,DWORD PTR [esi*4+0x6cb8c2]
  54b8b8:	8b 72 08             	mov    esi,DWORD PTR [edx+0x8]
  54b8bb:	8d 04 81             	lea    eax,[ecx+eax*4]
  54b8be:	89 9c 24 b6 00 00 00 	mov    DWORD PTR [esp+0xb6],ebx
  54b8c5:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
  54b8c9:	8b 42 0c             	mov    eax,DWORD PTR [edx+0xc]
  54b8cc:	89 5c 24 34          	mov    DWORD PTR [esp+0x34],ebx
  54b8d0:	89 5c 24 24          	mov    DWORD PTR [esp+0x24],ebx
  54b8d4:	8b 04 85 42 b9 6c 00 	mov    eax,DWORD PTR [eax*4+0x6cb942]
  54b8db:	89 5c 24 1c          	mov    DWORD PTR [esp+0x1c],ebx
  54b8df:	03 c7                	add    eax,edi
  54b8e1:	89 5c 24 30          	mov    DWORD PTR [esp+0x30],ebx
  54b8e5:	03 c6                	add    eax,esi
  54b8e7:	89 5c 24 3c          	mov    DWORD PTR [esp+0x3c],ebx
  54b8eb:	89 5c 24 68          	mov    DWORD PTR [esp+0x68],ebx
  54b8ef:	8d 14 40             	lea    edx,[eax+eax*2]
  54b8f2:	8d 04 91             	lea    eax,[ecx+edx*4]
  54b8f5:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  54b8f9:	8a 85 0c 02 00 00    	mov    al,BYTE PTR [ebp+0x20c]
  54b8ff:	84 c0                	test   al,al
  54b901:	0f 84 bc 00 00 00    	je     0x54b9c3
  54b907:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  54b90b:	8d b5 21 02 00 00    	lea    esi,[ebp+0x221]
  54b911:	51                   	push   ecx
  54b912:	88 9d 0c 02 00 00    	mov    BYTE PTR [ebp+0x20c],bl
  54b918:	8b bd 25 02 00 00    	mov    edi,DWORD PTR [ebp+0x225]
  54b91e:	8b ce                	mov    ecx,esi
  54b920:	e8 2b 46 ef ff       	call   0x43ff50
  54b925:	8b 00                	mov    eax,DWORD PTR [eax]
  54b927:	57                   	push   edi
  54b928:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  54b92c:	50                   	push   eax
  54b92d:	52                   	push   edx
  54b92e:	8b ce                	mov    ecx,esi
  54b930:	e8 2b 47 ef ff       	call   0x440060
  54b935:	8b 85 35 02 00 00    	mov    eax,DWORD PTR [ebp+0x235]
  54b93b:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  54b93f:	8d bd 31 02 00 00    	lea    edi,[ebp+0x231]
  54b945:	51                   	push   ecx
  54b946:	8b cf                	mov    ecx,edi
  54b948:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
  54b94c:	e8 0f 46 ef ff       	call   0x43ff60
  54b951:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
  54b955:	8b 00                	mov    eax,DWORD PTR [eax]
  54b957:	52                   	push   edx
  54b958:	50                   	push   eax
  54b959:	8d 44 24 40          	lea    eax,[esp+0x40]
  54b95d:	8b cf                	mov    ecx,edi
  54b95f:	50                   	push   eax
  54b960:	e8 8b 47 ef ff       	call   0x4400f0
  54b965:	8d 4c 24 20          	lea    ecx,[esp+0x20]
  54b969:	8d 54 24 18          	lea    edx,[esp+0x18]
  54b96d:	51                   	push   ecx
  54b96e:	52                   	push   edx
  54b96f:	8d 4c 24 58          	lea    ecx,[esp+0x58]
  54b973:	89 5c 24 20          	mov    DWORD PTR [esp+0x20],ebx
  54b977:	e8 54 48 ef ff       	call   0x4401d0
  54b97c:	50                   	push   eax
  54b97d:	8d 4c 24 48          	lea    ecx,[esp+0x48]
  54b981:	e8 ea 4a ef ff       	call   0x440470
  54b986:	8d 44 24 44          	lea    eax,[esp+0x44]
  54b98a:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  54b98e:	50                   	push   eax
  54b98f:	51                   	push   ecx
  54b990:	8b ce                	mov    ecx,esi
  54b992:	e8 a9 46 ef ff       	call   0x440040
  54b997:	8d 54 24 20          	lea    edx,[esp+0x20]
  54b99b:	8b cf                	mov    ecx,edi
  54b99d:	52                   	push   edx
  54b99e:	e8 dd 46 ef ff       	call   0x440080
  54b9a3:	89 18                	mov    DWORD PTR [eax],ebx
  54b9a5:	8b 44 24 2c          	mov    eax,DWORD PTR [esp+0x2c]
  54b9a9:	8b 4c 24 20          	mov    ecx,DWORD PTR [esp+0x20]
  54b9ad:	50                   	push   eax
  54b9ae:	e8 cd 0d fa ff       	call   0x4ec780
  54b9b3:	8b 4c 24 20          	mov    ecx,DWORD PTR [esp+0x20]
  54b9b7:	89 85 11 02 00 00    	mov    DWORD PTR [ebp+0x211],eax
  54b9bd:	89 8d 15 02 00 00    	mov    DWORD PTR [ebp+0x215],ecx
  54b9c3:	39 9d 2d 02 00 00    	cmp    DWORD PTR [ebp+0x22d],ebx
  54b9c9:	0f 84 bf 01 00 00    	je     0x54bb8e
  54b9cf:	8d 54 24 5c          	lea    edx,[esp+0x5c]
  54b9d3:	8d b5 21 02 00 00    	lea    esi,[ebp+0x221]
  54b9d9:	52                   	push   edx
  54b9da:	8b ce                	mov    ecx,esi
  54b9dc:	e8 6f 45 ef ff       	call   0x43ff50
  54b9e1:	8b 38                	mov    edi,DWORD PTR [eax]
  54b9e3:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  54b9e7:	50                   	push   eax
  54b9e8:	8d 8d 31 02 00 00    	lea    ecx,[ebp+0x231]
  54b9ee:	e8 8d 46 ef ff       	call   0x440080
  54b9f3:	8b 4f 0c             	mov    ecx,DWORD PTR [edi+0xc]
  54b9f6:	8b 10                	mov    edx,DWORD PTR [eax]
  54b9f8:	3b ca                	cmp    ecx,edx
  54b9fa:	0f 8d 8e 01 00 00    	jge    0x54bb8e
  54ba00:	8d 54 24 60          	lea    edx,[esp+0x60]
  54ba04:	8b ce                	mov    ecx,esi
  54ba06:	52                   	push   edx
  54ba07:	e8 44 45 ef ff       	call   0x43ff50
  54ba0c:	8b 00                	mov    eax,DWORD PTR [eax]
  54ba0e:	8d 4c 24 64          	lea    ecx,[esp+0x64]
  54ba12:	51                   	push   ecx
  54ba13:	8b ce                	mov    ecx,esi
  54ba15:	8b 40 0c             	mov    eax,DWORD PTR [eax+0xc]
  54ba18:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
  54ba1c:	e8 2f 45 ef ff       	call   0x43ff50
  54ba21:	8b 00                	mov    eax,DWORD PTR [eax]
  54ba23:	8b 8d 45 02 00 00    	mov    ecx,DWORD PTR [ebp+0x245]
  54ba29:	8d bd 41 02 00 00    	lea    edi,[ebp+0x241]
  54ba2f:	8b 50 10             	mov    edx,DWORD PTR [eax+0x10]
  54ba32:	8b 85 49 02 00 00    	mov    eax,DWORD PTR [ebp+0x249]
  54ba38:	50                   	push   eax
  54ba39:	51                   	push   ecx
  54ba3a:	8b cf                	mov    ecx,edi
  54ba3c:	89 54 24 18          	mov    DWORD PTR [esp+0x18],edx
  54ba40:	e8 fb 46 ef ff       	call   0x440140
  54ba45:	8b 94 24 fc 00 00 00 	mov    edx,DWORD PTR [esp+0xfc]
  54ba4c:	8d 44 24 10          	lea    eax,[esp+0x10]
  54ba50:	52                   	push   edx
  54ba51:	50                   	push   eax
  54ba52:	8d 8d 31 02 00 00    	lea    ecx,[ebp+0x231]
  54ba58:	e8 23 46 ef ff       	call   0x440080
  54ba5d:	83 c0 08             	add    eax,0x8
  54ba60:	8d 4c 24 6c          	lea    ecx,[esp+0x6c]
  54ba64:	50                   	push   eax
  54ba65:	51                   	push   ecx
  54ba66:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
  54ba6a:	57                   	push   edi
  54ba6b:	e8 70 00 fa ff       	call   0x4ebae0
  54ba70:	8b 94 24 fc 00 00 00 	mov    edx,DWORD PTR [esp+0xfc]
  54ba77:	39 1a                	cmp    DWORD PTR [edx],ebx
  54ba79:	0f 8e 16 01 00 00    	jle    0x54bb95
  54ba7f:	8d 44 24 58          	lea    eax,[esp+0x58]
  54ba83:	8b ce                	mov    ecx,esi
  54ba85:	50                   	push   eax
  54ba86:	e8 c5 44 ef ff       	call   0x43ff50
  54ba8b:	8b 00                	mov    eax,DWORD PTR [eax]
  54ba8d:	8d 4c 24 40          	lea    ecx,[esp+0x40]
  54ba91:	50                   	push   eax
  54ba92:	51                   	push   ecx
  54ba93:	8b ce                	mov    ecx,esi
  54ba95:	e8 26 70 f1 ff       	call   0x462ac0
  54ba9a:	8b 54 24 2c          	mov    edx,DWORD PTR [esp+0x2c]
  54ba9e:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
  54baa2:	52                   	push   edx
  54baa3:	e8 d8 0c fa ff       	call   0x4ec780
  54baa8:	8b b5 45 02 00 00    	mov    esi,DWORD PTR [ebp+0x245]
  54baae:	89 44 24 4c          	mov    DWORD PTR [esp+0x4c],eax
  54bab2:	8b 85 49 02 00 00    	mov    eax,DWORD PTR [ebp+0x249]
  54bab8:	89 74 24 18          	mov    DWORD PTR [esp+0x18],esi
  54babc:	3b f0                	cmp    esi,eax
  54babe:	0f 84 be 00 00 00    	je     0x54bb82
  54bac4:	8d 5e 04             	lea    ebx,[esi+0x4]
  54bac7:	8b 44 24 2c          	mov    eax,DWORD PTR [esp+0x2c]
  54bacb:	8b 0b                	mov    ecx,DWORD PTR [ebx]
  54bacd:	50                   	push   eax
  54bace:	e8 ad 0c fa ff       	call   0x4ec780
  54bad3:	8b 0e                	mov    ecx,DWORD PTR [esi]
  54bad5:	8b 7c 24 4c          	mov    edi,DWORD PTR [esp+0x4c]
  54bad9:	2b cf                	sub    ecx,edi
  54badb:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  54badf:	03 c8                	add    ecx,eax
  54bae1:	8b 44 24 38          	mov    eax,DWORD PTR [esp+0x38]
  54bae5:	03 c8                	add    ecx,eax
  54bae7:	53                   	push   ebx
  54bae8:	89 4c 24 2c          	mov    DWORD PTR [esp+0x2c],ecx
  54baec:	8d 8d 31 02 00 00    	lea    ecx,[ebp+0x231]
  54baf2:	e8 89 45 ef ff       	call   0x440080
  54baf7:	8b 4c 24 28          	mov    ecx,DWORD PTR [esp+0x28]
  54bafb:	8b 10                	mov    edx,DWORD PTR [eax]
  54bafd:	3b ca                	cmp    ecx,edx
  54baff:	7d 67                	jge    0x54bb68
  54bb01:	89 08                	mov    DWORD PTR [eax],ecx
  54bb03:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
  54bb07:	8d 73 04             	lea    esi,[ebx+0x4]
  54bb0a:	8d 78 08             	lea    edi,[eax+0x8]
  54bb0d:	b9 07 00 00 00       	mov    ecx,0x7
  54bb12:	89 50 04             	mov    DWORD PTR [eax+0x4],edx
  54bb15:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  54bb17:	8d 44 24 28          	lea    eax,[esp+0x28]
  54bb1b:	53                   	push   ebx
  54bb1c:	50                   	push   eax
  54bb1d:	8d 8c 24 c4 00 00 00 	lea    ecx,[esp+0xc4]
  54bb24:	e8 a7 46 ef ff       	call   0x4401d0
  54bb29:	50                   	push   eax
  54bb2a:	8d 4c 24 54          	lea    ecx,[esp+0x54]
  54bb2e:	e8 3d 49 ef ff       	call   0x440470
  54bb33:	8d 4c 24 50          	lea    ecx,[esp+0x50]
  54bb37:	8d 54 24 44          	lea    edx,[esp+0x44]
  54bb3b:	51                   	push   ecx
  54bb3c:	52                   	push   edx
  54bb3d:	8d 8d 21 02 00 00    	lea    ecx,[ebp+0x221]
  54bb43:	e8 f8 44 ef ff       	call   0x440040
  54bb48:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
  54bb4c:	8b 8d 11 02 00 00    	mov    ecx,DWORD PTR [ebp+0x211]
  54bb52:	3b c1                	cmp    eax,ecx
  54bb54:	7d 0e                	jge    0x54bb64
  54bb56:	89 85 11 02 00 00    	mov    DWORD PTR [ebp+0x211],eax
  54bb5c:	8b 03                	mov    eax,DWORD PTR [ebx]
  54bb5e:	89 85 15 02 00 00    	mov    DWORD PTR [ebp+0x215],eax
  54bb64:	8b 74 24 18          	mov    esi,DWORD PTR [esp+0x18]
  54bb68:	8b 85 49 02 00 00    	mov    eax,DWORD PTR [ebp+0x249]
  54bb6e:	83 c6 24             	add    esi,0x24
  54bb71:	83 c3 24             	add    ebx,0x24
  54bb74:	3b f0                	cmp    esi,eax
  54bb76:	89 74 24 18          	mov    DWORD PTR [esp+0x18],esi
  54bb7a:	0f 85 47 ff ff ff    	jne    0x54bac7
  54bb80:	33 db                	xor    ebx,ebx
  54bb82:	39 9d 2d 02 00 00    	cmp    DWORD PTR [ebp+0x22d],ebx
  54bb88:	0f 85 41 fe ff ff    	jne    0x54b9cf
  54bb8e:	c6 85 0c 02 00 00 01 	mov    BYTE PTR [ebp+0x20c],0x1
  54bb95:	8b 95 59 02 00 00    	mov    edx,DWORD PTR [ebp+0x259]
  54bb9b:	8b 85 55 02 00 00    	mov    eax,DWORD PTR [ebp+0x255]
  54bba1:	52                   	push   edx
  54bba2:	8d 8d 51 02 00 00    	lea    ecx,[ebp+0x251]
  54bba8:	50                   	push   eax
  54bba9:	e8 e2 45 ef ff       	call   0x440190
  54bbae:	8b 8d 15 02 00 00    	mov    ecx,DWORD PTR [ebp+0x215]
  54bbb4:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
  54bbb8:	8d 54 24 10          	lea    edx,[esp+0x10]
  54bbbc:	8d 8d 31 02 00 00    	lea    ecx,[ebp+0x231]
  54bbc2:	52                   	push   edx
  54bbc3:	e8 b8 44 ef ff       	call   0x440080
  54bbc8:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
  54bbcc:	8d 70 08             	lea    esi,[eax+0x8]
  54bbcf:	89 8c 24 c4 00 00 00 	mov    DWORD PTR [esp+0xc4],ecx
  54bbd6:	b9 07 00 00 00       	mov    ecx,0x7
  54bbdb:	8d bc 24 c8 00 00 00 	lea    edi,[esp+0xc8]
  54bbe2:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  54bbe6:	8b 85 59 02 00 00    	mov    eax,DWORD PTR [ebp+0x259]
  54bbec:	8d 94 24 c4 00 00 00 	lea    edx,[esp+0xc4]
  54bbf3:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  54bbf5:	8d b5 51 02 00 00    	lea    esi,[ebp+0x251]
  54bbfb:	52                   	push   edx
  54bbfc:	50                   	push   eax
  54bbfd:	8b ce                	mov    ecx,esi
  54bbff:	e8 5c 46 ef ff       	call   0x440260
  54bc04:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
  54bc08:	8b 44 24 20          	mov    eax,DWORD PTR [esp+0x20]
  54bc0c:	3b c8                	cmp    ecx,eax
  54bc0e:	74 0d                	je     0x54bc1d
  54bc10:	8b 54 24 14          	mov    edx,DWORD PTR [esp+0x14]
  54bc14:	8b 42 04             	mov    eax,DWORD PTR [edx+0x4]
  54bc17:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
  54bc1b:	eb 9b                	jmp    0x54bbb8
  54bc1d:	8d 4c 24 44          	lea    ecx,[esp+0x44]
  54bc21:	51                   	push   ecx
  54bc22:	8b ce                	mov    ecx,esi
  54bc24:	e8 57 45 ef ff       	call   0x440180
  54bc29:	8b 10                	mov    edx,DWORD PTR [eax]
  54bc2b:	8d 85 15 02 00 00    	lea    eax,[ebp+0x215]
  54bc31:	50                   	push   eax
  54bc32:	8d 8d 31 02 00 00    	lea    ecx,[ebp+0x231]
  54bc38:	89 95 61 02 00 00    	mov    DWORD PTR [ebp+0x261],edx
  54bc3e:	e8 3d 44 ef ff       	call   0x440080
  54bc43:	8b 00                	mov    eax,DWORD PTR [eax]
  54bc45:	8b 8c 24 f0 00 00 00 	mov    ecx,DWORD PTR [esp+0xf0]
  54bc4c:	8b 94 24 f4 00 00 00 	mov    edx,DWORD PTR [esp+0xf4]
  54bc53:	89 85 65 02 00 00    	mov    DWORD PTR [ebp+0x265],eax
  54bc59:	8b 84 24 f8 00 00 00 	mov    eax,DWORD PTR [esp+0xf8]
  54bc60:	8d bd 61 02 00 00    	lea    edi,[ebp+0x261]
  54bc66:	89 4d 00             	mov    DWORD PTR [ebp+0x0],ecx
  54bc69:	89 55 04             	mov    DWORD PTR [ebp+0x4],edx
  54bc6c:	89 45 08             	mov    DWORD PTR [ebp+0x8],eax
  54bc6f:	8b 0f                	mov    ecx,DWORD PTR [edi]
  54bc71:	8d 54 24 44          	lea    edx,[esp+0x44]
  54bc75:	89 4c 24 14          	mov    DWORD PTR [esp+0x14],ecx
  54bc79:	52                   	push   edx
  54bc7a:	8d 8d 51 02 00 00    	lea    ecx,[ebp+0x251]
  54bc80:	33 f6                	xor    esi,esi
  54bc82:	e8 c9 01 00 00       	call   0x54be50
  54bc87:	8b 4c 24 14          	mov    ecx,DWORD PTR [esp+0x14]
  54bc8b:	8b 10                	mov    edx,DWORD PTR [eax]
  54bc8d:	3b ca                	cmp    ecx,edx
  54bc8f:	74 12                	je     0x54bca3
  54bc91:	8b 37                	mov    esi,DWORD PTR [edi]
  54bc93:	8d 54 24 40          	lea    edx,[esp+0x40]
  54bc97:	53                   	push   ebx
  54bc98:	52                   	push   edx
  54bc99:	8b cf                	mov    ecx,edi
  54bc9b:	83 ee 20             	sub    esi,0x20
  54bc9e:	e8 ed 40 ee ff       	call   0x42fd90
  54bca3:	8b 0e                	mov    ecx,DWORD PTR [esi]
  54bca5:	8b 3d dc 54 6c 00    	mov    edi,DWORD PTR ds:0x6c54dc
  54bcab:	2b cf                	sub    ecx,edi
  54bcad:	b8 ab aa aa 2a       	mov    eax,0x2aaaaaab
  54bcb2:	f7 e9                	imul   ecx
  54bcb4:	d1 fa                	sar    edx,1
  54bcb6:	8b c2                	mov    eax,edx
  54bcb8:	8d 4c 24 30          	lea    ecx,[esp+0x30]
  54bcbc:	c1 e8 1f             	shr    eax,0x1f
  54bcbf:	03 d0                	add    edx,eax
  54bcc1:	8b f2                	mov    esi,edx
  54bcc3:	8b c6                	mov    eax,esi
  54bcc5:	99                   	cdq
  54bcc6:	f7 3d a0 54 6c 00    	idiv   DWORD PTR ds:0x6c54a0
  54bccc:	8b d8                	mov    ebx,eax
  54bcce:	2b 34 9d c2 b8 6c 00 	sub    esi,DWORD PTR [ebx*4+0x6cb8c2]
  54bcd5:	8b c6                	mov    eax,esi
  54bcd7:	99                   	cdq
  54bcd8:	f7 3d 94 54 6c 00    	idiv   DWORD PTR ds:0x6c5494
  54bcde:	50                   	push   eax
  54bcdf:	e8 fc 75 ec ff       	call   0x4132e0
  54bce4:	8b 0c 85 42 b9 6c 00 	mov    ecx,DWORD PTR [eax*4+0x6cb942]
  54bceb:	89 44 24 30          	mov    DWORD PTR [esp+0x30],eax
  54bcef:	2b f1                	sub    esi,ecx
  54bcf1:	8d 4c 24 24          	lea    ecx,[esp+0x24]
  54bcf5:	56                   	push   esi
  54bcf6:	e8 55 29 ec ff       	call   0x40e650
  54bcfb:	8b 8c 24 e8 00 00 00 	mov    ecx,DWORD PTR [esp+0xe8]
  54bd02:	89 44 24 24          	mov    DWORD PTR [esp+0x24],eax
  54bd06:	8d 75 14             	lea    esi,[ebp+0x14]
  54bd09:	8b 91 08 06 00 00    	mov    edx,DWORD PTR [ecx+0x608]
  54bd0f:	c7 45 10 00 00 00 00 	mov    DWORD PTR [ebp+0x10],0x0
  54bd16:	89 54 24 28          	mov    DWORD PTR [esp+0x28],edx
  54bd1a:	8b bd 61 02 00 00    	mov    edi,DWORD PTR [ebp+0x261]
  54bd20:	8d 44 24 44          	lea    eax,[esp+0x44]
  54bd24:	50                   	push   eax
  54bd25:	8d 8d 51 02 00 00    	lea    ecx,[ebp+0x251]
  54bd2b:	e8 20 01 00 00       	call   0x54be50
  54bd30:	3b 38                	cmp    edi,DWORD PTR [eax]
  54bd32:	0f 84 fc 00 00 00    	je     0x54be34
  54bd38:	8b bd 61 02 00 00    	mov    edi,DWORD PTR [ebp+0x261]
  54bd3e:	8d 4c 24 40          	lea    ecx,[esp+0x40]
  54bd42:	6a 00                	push   0x0
  54bd44:	51                   	push   ecx
  54bd45:	8d 8d 61 02 00 00    	lea    ecx,[ebp+0x261]
  54bd4b:	83 ef 20             	sub    edi,0x20
  54bd4e:	e8 3d 40 ee ff       	call   0x42fd90
  54bd53:	85 ff                	test   edi,edi
  54bd55:	0f 84 d9 00 00 00    	je     0x54be34
  54bd5b:	83 7d 10 10          	cmp    DWORD PTR [ebp+0x10],0x10
  54bd5f:	0f 83 cf 00 00 00    	jae    0x54be34
  54bd65:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  54bd69:	8d 44 24 1c          	lea    eax,[esp+0x1c]
  54bd6d:	52                   	push   edx
  54bd6e:	8b 17                	mov    edx,DWORD PTR [edi]
  54bd70:	8d 4c 24 38          	lea    ecx,[esp+0x38]
  54bd74:	50                   	push   eax
  54bd75:	51                   	push   ecx
  54bd76:	52                   	push   edx
  54bd77:	b9 90 54 6c 00       	mov    ecx,0x6c5490
  54bd7c:	e8 0f 42 ef ff       	call   0x43ff90
  54bd81:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
  54bd85:	8b 44 24 34          	mov    eax,DWORD PTR [esp+0x34]
  54bd89:	8b 54 24 3c          	mov    edx,DWORD PTR [esp+0x3c]
  54bd8d:	89 4e 04             	mov    DWORD PTR [esi+0x4],ecx
  54bd90:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  54bd94:	89 06                	mov    DWORD PTR [esi],eax
  54bd96:	3b c1                	cmp    eax,ecx
  54bd98:	8b 4c 24 30          	mov    ecx,DWORD PTR [esp+0x30]
  54bd9c:	89 56 08             	mov    DWORD PTR [esi+0x8],edx
  54bd9f:	75 10                	jne    0x54bdb1
  54bda1:	39 4c 24 1c          	cmp    DWORD PTR [esp+0x1c],ecx
  54bda5:	75 0a                	jne    0x54bdb1
  54bda7:	8b 44 24 28          	mov    eax,DWORD PTR [esp+0x28]
  54bdab:	8b ca                	mov    ecx,edx
  54bdad:	2b cb                	sub    ecx,ebx
  54bdaf:	eb 35                	jmp    0x54bde6
  54bdb1:	51                   	push   ecx
  54bdb2:	8b c4                	mov    eax,esp
  54bdb4:	89 08                	mov    DWORD PTR [eax],ecx
  54bdb6:	8d 4c 24 20          	lea    ecx,[esp+0x20]
  54bdba:	e8 b1 2d ec ff       	call   0x40eb70
  54bdbf:	51                   	push   ecx
  54bdc0:	8d 54 24 28          	lea    edx,[esp+0x28]
  54bdc4:	8b cc                	mov    ecx,esp
  54bdc6:	52                   	push   edx
  54bdc7:	8b d8                	mov    ebx,eax
  54bdc9:	e8 f2 24 ec ff       	call   0x40e2c0
  54bdce:	8d 4c 24 38          	lea    ecx,[esp+0x38]
  54bdd2:	e8 09 27 ec ff       	call   0x40e4e0
  54bdd7:	8b c8                	mov    ecx,eax
  54bdd9:	8b d3                	mov    edx,ebx
  54bddb:	e8 00 f0 f9 ff       	call   0x4eade0
  54bde0:	8b 54 24 3c          	mov    edx,DWORD PTR [esp+0x3c]
  54bde4:	33 c9                	xor    ecx,ecx
  54bde6:	89 46 0c             	mov    DWORD PTR [esi+0xc],eax
  54bde9:	89 4e 10             	mov    DWORD PTR [esi+0x10],ecx
  54bdec:	8b 7f 04             	mov    edi,DWORD PTR [edi+0x4]
  54bdef:	83 ff 01             	cmp    edi,0x1
  54bdf2:	89 7e 18             	mov    DWORD PTR [esi+0x18],edi
  54bdf5:	74 10                	je     0x54be07
  54bdf7:	8b cf                	mov    ecx,edi
  54bdf9:	83 f9 02             	cmp    ecx,0x2
  54bdfc:	74 09                	je     0x54be07
  54bdfe:	83 f9 03             	cmp    ecx,0x3
  54be01:	74 04                	je     0x54be07
  54be03:	33 c9                	xor    ecx,ecx
  54be05:	eb 05                	jmp    0x54be0c
  54be07:	b9 01 00 00 00       	mov    ecx,0x1
  54be0c:	89 4e 14             	mov    DWORD PTR [esi+0x14],ecx
  54be0f:	8b 4c 24 34          	mov    ecx,DWORD PTR [esp+0x34]
  54be13:	8b 7d 10             	mov    edi,DWORD PTR [ebp+0x10]
  54be16:	89 4c 24 24          	mov    DWORD PTR [esp+0x24],ecx
  54be1a:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
  54be1e:	47                   	inc    edi
  54be1f:	89 7d 10             	mov    DWORD PTR [ebp+0x10],edi
  54be22:	83 c6 1c             	add    esi,0x1c
  54be25:	89 4c 24 30          	mov    DWORD PTR [esp+0x30],ecx
  54be29:	8b da                	mov    ebx,edx
  54be2b:	89 44 24 28          	mov    DWORD PTR [esp+0x28],eax
  54be2f:	e9 e6 fe ff ff       	jmp    0x54bd1a
  54be34:	5f                   	pop    edi
  54be35:	c7 45 0c 00 00 00 00 	mov    DWORD PTR [ebp+0xc],0x0
  54be3c:	5e                   	pop    esi
  54be3d:	5d                   	pop    ebp
  54be3e:	5b                   	pop    ebx
  54be3f:	81 c4 d4 00 00 00    	add    esp,0xd4
  54be45:	c2 18 00             	ret    0x18
