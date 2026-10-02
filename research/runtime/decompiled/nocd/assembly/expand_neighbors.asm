
Chaos.exe:     file format pei-i386


Disassembly of section .text:

004ebae0 <.text+0xeaae0>:
  4ebae0:	81 ec 8c 00 00 00    	sub    esp,0x8c
  4ebae6:	8b 84 24 9c 00 00 00 	mov    eax,DWORD PTR [esp+0x9c]
  4ebaed:	53                   	push   ebx
  4ebaee:	8b d9                	mov    ebx,ecx
  4ebaf0:	55                   	push   ebp
  4ebaf1:	8b 08                	mov    ecx,DWORD PTR [eax]
  4ebaf3:	33 ed                	xor    ebp,ebp
  4ebaf5:	56                   	push   esi
  4ebaf6:	3b cd                	cmp    ecx,ebp
  4ebaf8:	57                   	push   edi
  4ebaf9:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4ebafd:	0f 8e 69 0c 00 00    	jle    0x4ec76c
  4ebb03:	8b 15 dc 54 6c 00    	mov    edx,DWORD PTR ds:0x6c54dc
  4ebb09:	8b cb                	mov    ecx,ebx
  4ebb0b:	2b ca                	sub    ecx,edx
  4ebb0d:	b8 ab aa aa 2a       	mov    eax,0x2aaaaaab
  4ebb12:	f7 e9                	imul   ecx
  4ebb14:	d1 fa                	sar    edx,1
  4ebb16:	8b ca                	mov    ecx,edx
  4ebb18:	89 6c 24 18          	mov    DWORD PTR [esp+0x18],ebp
  4ebb1c:	c1 e9 1f             	shr    ecx,0x1f
  4ebb1f:	03 d1                	add    edx,ecx
  4ebb21:	89 6c 24 1c          	mov    DWORD PTR [esp+0x1c],ebp
  4ebb25:	8b f2                	mov    esi,edx
  4ebb27:	89 6c 24 10          	mov    DWORD PTR [esp+0x10],ebp
  4ebb2b:	8b c6                	mov    eax,esi
  4ebb2d:	89 6c 24 14          	mov    DWORD PTR [esp+0x14],ebp
  4ebb31:	99                   	cdq
  4ebb32:	f7 3d a0 54 6c 00    	idiv   DWORD PTR ds:0x6c54a0
  4ebb38:	c7 44 24 44 05 00 00 	mov    DWORD PTR [esp+0x44],0x5
  4ebb3f:	00 
  4ebb40:	89 6c 24 50          	mov    DWORD PTR [esp+0x50],ebp
  4ebb44:	89 6c 24 54          	mov    DWORD PTR [esp+0x54],ebp
  4ebb48:	89 6c 24 58          	mov    DWORD PTR [esp+0x58],ebp
  4ebb4c:	89 6c 24 5c          	mov    DWORD PTR [esp+0x5c],ebp
  4ebb50:	89 44 24 24          	mov    DWORD PTR [esp+0x24],eax
  4ebb54:	8b 0c 85 c2 b8 6c 00 	mov    ecx,DWORD PTR [eax*4+0x6cb8c2]
  4ebb5b:	2b f1                	sub    esi,ecx
  4ebb5d:	8d 4c 24 1c          	lea    ecx,[esp+0x1c]
  4ebb61:	8b c6                	mov    eax,esi
  4ebb63:	99                   	cdq
  4ebb64:	f7 3d 94 54 6c 00    	idiv   DWORD PTR ds:0x6c5494
  4ebb6a:	50                   	push   eax
  4ebb6b:	e8 70 77 f2 ff       	call   0x4132e0
  4ebb70:	8b 14 85 42 b9 6c 00 	mov    edx,DWORD PTR [eax*4+0x6cb942]
  4ebb77:	8d 4c 24 18          	lea    ecx,[esp+0x18]
  4ebb7b:	2b f2                	sub    esi,edx
  4ebb7d:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
  4ebb81:	56                   	push   esi
  4ebb82:	e8 c9 2a f2 ff       	call   0x40e650
  4ebb87:	8b b4 24 a4 00 00 00 	mov    esi,DWORD PTR [esp+0xa4]
  4ebb8e:	89 44 24 18          	mov    DWORD PTR [esp+0x18],eax
  4ebb92:	39 6e 3a             	cmp    DWORD PTR [esi+0x3a],ebp
  4ebb95:	0f 84 6c 01 00 00    	je     0x4ebd07
  4ebb9b:	8b 46 42             	mov    eax,DWORD PTR [esi+0x42]
  4ebb9e:	83 f8 ff             	cmp    eax,0xffffffff
  4ebba1:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4ebba5:	74 19                	je     0x4ebbc0
  4ebba7:	db 44 24 20          	fild   DWORD PTR [esp+0x20]
  4ebbab:	8b 94 24 a8 00 00 00 	mov    edx,DWORD PTR [esp+0xa8]
  4ebbb2:	d8 5a 10             	fcomp  DWORD PTR [edx+0x10]
  4ebbb5:	df e0                	fnstsw ax
  4ebbb7:	f6 c4 41             	test   ah,0x41
  4ebbba:	0f 85 a3 0b 00 00    	jne    0x4ec763
  4ebbc0:	33 ff                	xor    edi,edi
  4ebbc2:	eb 04                	jmp    0x4ebbc8
  4ebbc4:	8b 5c 24 2c          	mov    ebx,DWORD PTR [esp+0x2c]
  4ebbc8:	8b 8f 08 71 5c 00    	mov    ecx,DWORD PTR [edi+0x5c7108]
  4ebbce:	33 c0                	xor    eax,eax
  4ebbd0:	66 8b 43 08          	mov    ax,WORD PTR [ebx+0x8]
  4ebbd4:	85 c8                	test   eax,ecx
  4ebbd6:	0f 84 1a 01 00 00    	je     0x4ebcf6
  4ebbdc:	8b 87 c0 70 5c 00    	mov    eax,DWORD PTR [edi+0x5c70c0]
  4ebbe2:	8b 54 24 18          	mov    edx,DWORD PTR [esp+0x18]
  4ebbe6:	03 c2                	add    eax,edx
  4ebbe8:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4ebbec:	50                   	push   eax
  4ebbed:	e8 5e 2a f2 ff       	call   0x40e650
  4ebbf2:	8b 5c 24 1c          	mov    ebx,DWORD PTR [esp+0x1c]
  4ebbf6:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
  4ebbfa:	8b 87 d8 70 5c 00    	mov    eax,DWORD PTR [edi+0x5c70d8]
  4ebc00:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4ebc04:	03 c3                	add    eax,ebx
  4ebc06:	50                   	push   eax
  4ebc07:	e8 d4 76 f2 ff       	call   0x4132e0
  4ebc0c:	8b 9f f0 70 5c 00    	mov    ebx,DWORD PTR [edi+0x5c70f0]
  4ebc12:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4ebc16:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  4ebc1a:	8b 46 3e             	mov    eax,DWORD PTR [esi+0x3e]
  4ebc1d:	03 d9                	add    ebx,ecx
  4ebc1f:	83 f8 ff             	cmp    eax,0xffffffff
  4ebc22:	74 57                	je     0x4ebc7b
  4ebc24:	8b 6e 0c             	mov    ebp,DWORD PTR [esi+0xc]
  4ebc27:	51                   	push   ecx
  4ebc28:	8d 54 24 14          	lea    edx,[esp+0x14]
  4ebc2c:	8b cc                	mov    ecx,esp
  4ebc2e:	52                   	push   edx
  4ebc2f:	e8 8c 26 f2 ff       	call   0x40e2c0
  4ebc34:	8b cd                	mov    ecx,ebp
  4ebc36:	e8 55 64 f5 ff       	call   0x442090
  4ebc3b:	51                   	push   ecx
  4ebc3c:	8b e8                	mov    ebp,eax
  4ebc3e:	8b 46 10             	mov    eax,DWORD PTR [esi+0x10]
  4ebc41:	8d 54 24 18          	lea    edx,[esp+0x18]
  4ebc45:	8b cc                	mov    ecx,esp
  4ebc47:	52                   	push   edx
  4ebc48:	89 44 24 28          	mov    DWORD PTR [esp+0x28],eax
  4ebc4c:	e8 7f 2c f2 ff       	call   0x40e8d0
  4ebc51:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4ebc55:	e8 46 64 f5 ff       	call   0x4420a0
  4ebc5a:	8b c8                	mov    ecx,eax
  4ebc5c:	8b d5                	mov    edx,ebp
  4ebc5e:	0f af c8             	imul   ecx,eax
  4ebc61:	0f af d5             	imul   edx,ebp
  4ebc64:	03 ca                	add    ecx,edx
  4ebc66:	89 4c 24 20          	mov    DWORD PTR [esp+0x20],ecx
  4ebc6a:	db 44 24 20          	fild   DWORD PTR [esp+0x20]
  4ebc6e:	d9 fa                	fsqrt
  4ebc70:	e8 6b 02 0b 00       	call   0x59bee0
  4ebc75:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4ebc79:	33 ed                	xor    ebp,ebp
  4ebc7b:	8b 46 3e             	mov    eax,DWORD PTR [esi+0x3e]
  4ebc7e:	83 f8 ff             	cmp    eax,0xffffffff
  4ebc81:	74 06                	je     0x4ebc89
  4ebc83:	39 44 24 20          	cmp    DWORD PTR [esp+0x20],eax
  4ebc87:	7d 6d                	jge    0x4ebcf6
  4ebc89:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
  4ebc8d:	8b 0c 9d c2 b8 6c 00 	mov    ecx,DWORD PTR [ebx*4+0x6cb8c2]
  4ebc94:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
  4ebc98:	8b 04 85 42 b9 6c 00 	mov    eax,DWORD PTR [eax*4+0x6cb942]
  4ebc9f:	03 c1                	add    eax,ecx
  4ebca1:	03 c2                	add    eax,edx
  4ebca3:	8b 15 dc 54 6c 00    	mov    edx,DWORD PTR ds:0x6c54dc
  4ebca9:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4ebcac:	8d 04 8a             	lea    eax,[edx+ecx*4]
  4ebcaf:	89 44 24 40          	mov    DWORD PTR [esp+0x40],eax
  4ebcb3:	66 39 28             	cmp    WORD PTR [eax],bp
  4ebcb6:	74 3e                	je     0x4ebcf6
  4ebcb8:	8b 8c 24 a8 00 00 00 	mov    ecx,DWORD PTR [esp+0xa8]
  4ebcbf:	8b 87 20 71 5c 00    	mov    eax,DWORD PTR [edi+0x5c7120]
  4ebcc5:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
  4ebcc9:	8b 51 14             	mov    edx,DWORD PTR [ecx+0x14]
  4ebccc:	d9 41 10             	fld    DWORD PTR [ecx+0x10]
  4ebccf:	8b 8c 24 a0 00 00 00 	mov    ecx,DWORD PTR [esp+0xa0]
  4ebcd6:	03 d0                	add    edx,eax
  4ebcd8:	d8 05 88 53 5c 00    	fadd   DWORD PTR ds:0x5c5388
  4ebcde:	8d 44 24 3c          	lea    eax,[esp+0x3c]
  4ebce2:	89 54 24 58          	mov    DWORD PTR [esp+0x58],edx
  4ebce6:	8b 51 08             	mov    edx,DWORD PTR [ecx+0x8]
  4ebce9:	50                   	push   eax
  4ebcea:	6a 01                	push   0x1
  4ebcec:	52                   	push   edx
  4ebced:	d9 5c 24 60          	fstp   DWORD PTR [esp+0x60]
  4ebcf1:	e8 ea 67 01 00       	call   0x5024e0
  4ebcf6:	83 c7 04             	add    edi,0x4
  4ebcf9:	83 ff 18             	cmp    edi,0x18
  4ebcfc:	0f 8c c2 fe ff ff    	jl     0x4ebbc4
  4ebd02:	e9 5c 0a 00 00       	jmp    0x4ec763
  4ebd07:	39 2e                	cmp    DWORD PTR [esi],ebp
  4ebd09:	75 1f                	jne    0x4ebd2a
  4ebd0b:	8b 46 04             	mov    eax,DWORD PTR [esi+0x4]
  4ebd0e:	8b 80 a8 00 00 00    	mov    eax,DWORD PTR [eax+0xa8]
  4ebd14:	8d 0c c0             	lea    ecx,[eax+eax*8]
  4ebd17:	8d 0c 88             	lea    ecx,[eax+ecx*4]
  4ebd1a:	8d 14 89             	lea    edx,[ecx+ecx*4]
  4ebd1d:	8d 84 d0 80 5f 6a 00 	lea    eax,[eax+edx*8+0x6a5f80]
  4ebd24:	89 44 24 60          	mov    DWORD PTR [esp+0x60],eax
  4ebd28:	eb 39                	jmp    0x4ebd63
  4ebd2a:	8d 4e 18             	lea    ecx,[esi+0x18]
  4ebd2d:	8d 54 24 24          	lea    edx,[esp+0x24]
  4ebd31:	51                   	push   ecx
  4ebd32:	51                   	push   ecx
  4ebd33:	8b cc                	mov    ecx,esp
  4ebd35:	52                   	push   edx
  4ebd36:	e8 85 31 f2 ff       	call   0x40eec0
  4ebd3b:	51                   	push   ecx
  4ebd3c:	8d 44 24 28          	lea    eax,[esp+0x28]
  4ebd40:	8b cc                	mov    ecx,esp
  4ebd42:	50                   	push   eax
  4ebd43:	e8 88 2b f2 ff       	call   0x40e8d0
  4ebd48:	51                   	push   ecx
  4ebd49:	8d 54 24 28          	lea    edx,[esp+0x28]
  4ebd4d:	8b cc                	mov    ecx,esp
  4ebd4f:	52                   	push   edx
  4ebd50:	e8 6b 25 f2 ff       	call   0x40e2c0
  4ebd55:	b9 90 54 6c 00       	mov    ecx,0x6c5490
  4ebd5a:	e8 d1 85 00 00       	call   0x4f4330
  4ebd5f:	89 44 24 60          	mov    DWORD PTR [esp+0x60],eax
  4ebd63:	39 2e                	cmp    DWORD PTR [esi],ebp
  4ebd65:	0f 84 a4 01 00 00    	je     0x4ebf0f
  4ebd6b:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  4ebd71:	8b c1                	mov    eax,ecx
  4ebd73:	99                   	cdq
  4ebd74:	2b c2                	sub    eax,edx
  4ebd76:	8b f8                	mov    edi,eax
  4ebd78:	a1 98 54 6c 00       	mov    eax,ds:0x6c5498
  4ebd7d:	99                   	cdq
  4ebd7e:	2b c2                	sub    eax,edx
  4ebd80:	8b 56 0c             	mov    edx,DWORD PTR [esi+0xc]
  4ebd83:	8b e8                	mov    ebp,eax
  4ebd85:	8b 46 2e             	mov    eax,DWORD PTR [esi+0x2e]
  4ebd88:	2b c2                	sub    eax,edx
  4ebd8a:	89 54 24 20          	mov    DWORD PTR [esp+0x20],edx
  4ebd8e:	99                   	cdq
  4ebd8f:	8b d8                	mov    ebx,eax
  4ebd91:	33 da                	xor    ebx,edx
  4ebd93:	d1 ff                	sar    edi,1
  4ebd95:	2b da                	sub    ebx,edx
  4ebd97:	89 bc 24 80 00 00 00 	mov    DWORD PTR [esp+0x80],edi
  4ebd9e:	d1 fd                	sar    ebp,1
  4ebda0:	3b df                	cmp    ebx,edi
  4ebda2:	89 ac 24 84 00 00 00 	mov    DWORD PTR [esp+0x84],ebp
  4ebda9:	7e 04                	jle    0x4ebdaf
  4ebdab:	2b cb                	sub    ecx,ebx
  4ebdad:	8b d9                	mov    ebx,ecx
  4ebdaf:	8b 46 32             	mov    eax,DWORD PTR [esi+0x32]
  4ebdb2:	8b 56 10             	mov    edx,DWORD PTR [esi+0x10]
  4ebdb5:	2b c2                	sub    eax,edx
  4ebdb7:	99                   	cdq
  4ebdb8:	33 c2                	xor    eax,edx
  4ebdba:	2b c2                	sub    eax,edx
  4ebdbc:	3b c5                	cmp    eax,ebp
  4ebdbe:	89 44 24 28          	mov    DWORD PTR [esp+0x28],eax
  4ebdc2:	7e 0c                	jle    0x4ebdd0
  4ebdc4:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  4ebdca:	2b c8                	sub    ecx,eax
  4ebdcc:	89 4c 24 28          	mov    DWORD PTR [esp+0x28],ecx
  4ebdd0:	8b 46 36             	mov    eax,DWORD PTR [esi+0x36]
  4ebdd3:	8b 56 14             	mov    edx,DWORD PTR [esi+0x14]
  4ebdd6:	2b c2                	sub    eax,edx
  4ebdd8:	51                   	push   ecx
  4ebdd9:	99                   	cdq
  4ebdda:	33 c2                	xor    eax,edx
  4ebddc:	8b cc                	mov    ecx,esp
  4ebdde:	2b c2                	sub    eax,edx
  4ebde0:	c7 44 24 74 00 00 00 	mov    DWORD PTR [esp+0x74],0x0
  4ebde7:	00 
  4ebde8:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
  4ebdec:	8d 44 24 1c          	lea    eax,[esp+0x1c]
  4ebdf0:	50                   	push   eax
  4ebdf1:	e8 ca 24 f2 ff       	call   0x40e2c0
  4ebdf6:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4ebdfa:	e8 91 62 f5 ff       	call   0x442090
  4ebdff:	99                   	cdq
  4ebe00:	33 c2                	xor    eax,edx
  4ebe02:	2b c2                	sub    eax,edx
  4ebe04:	3b c7                	cmp    eax,edi
  4ebe06:	7e 0a                	jle    0x4ebe12
  4ebe08:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  4ebe0e:	2b c8                	sub    ecx,eax
  4ebe10:	8b c1                	mov    eax,ecx
  4ebe12:	3b c3                	cmp    eax,ebx
  4ebe14:	7f 3c                	jg     0x4ebe52
  4ebe16:	8b 56 2e             	mov    edx,DWORD PTR [esi+0x2e]
  4ebe19:	51                   	push   ecx
  4ebe1a:	8d 44 24 1c          	lea    eax,[esp+0x1c]
  4ebe1e:	8b cc                	mov    ecx,esp
  4ebe20:	50                   	push   eax
  4ebe21:	89 54 24 28          	mov    DWORD PTR [esp+0x28],edx
  4ebe25:	e8 96 24 f2 ff       	call   0x40e2c0
  4ebe2a:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4ebe2e:	e8 5d 62 f5 ff       	call   0x442090
  4ebe33:	99                   	cdq
  4ebe34:	33 c2                	xor    eax,edx
  4ebe36:	2b c2                	sub    eax,edx
  4ebe38:	3b c7                	cmp    eax,edi
  4ebe3a:	7e 0a                	jle    0x4ebe46
  4ebe3c:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  4ebe42:	2b c8                	sub    ecx,eax
  4ebe44:	8b c1                	mov    eax,ecx
  4ebe46:	3b c3                	cmp    eax,ebx
  4ebe48:	7f 08                	jg     0x4ebe52
  4ebe4a:	c7 44 24 70 01 00 00 	mov    DWORD PTR [esp+0x70],0x1
  4ebe51:	00 
  4ebe52:	8b 7e 10             	mov    edi,DWORD PTR [esi+0x10]
  4ebe55:	51                   	push   ecx
  4ebe56:	8d 54 24 20          	lea    edx,[esp+0x20]
  4ebe5a:	8b cc                	mov    ecx,esp
  4ebe5c:	52                   	push   edx
  4ebe5d:	c7 44 24 74 00 00 00 	mov    DWORD PTR [esp+0x74],0x0
  4ebe64:	00 
  4ebe65:	e8 66 2a f2 ff       	call   0x40e8d0
  4ebe6a:	8b cf                	mov    ecx,edi
  4ebe6c:	e8 2f 62 f5 ff       	call   0x4420a0
  4ebe71:	99                   	cdq
  4ebe72:	33 c2                	xor    eax,edx
  4ebe74:	2b c2                	sub    eax,edx
  4ebe76:	3b c5                	cmp    eax,ebp
  4ebe78:	7e 0a                	jle    0x4ebe84
  4ebe7a:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  4ebe80:	2b c8                	sub    ecx,eax
  4ebe82:	8b c1                	mov    eax,ecx
  4ebe84:	3b 44 24 28          	cmp    eax,DWORD PTR [esp+0x28]
  4ebe88:	7f 38                	jg     0x4ebec2
  4ebe8a:	8b 7e 32             	mov    edi,DWORD PTR [esi+0x32]
  4ebe8d:	51                   	push   ecx
  4ebe8e:	8d 54 24 20          	lea    edx,[esp+0x20]
  4ebe92:	8b cc                	mov    ecx,esp
  4ebe94:	52                   	push   edx
  4ebe95:	e8 36 2a f2 ff       	call   0x40e8d0
  4ebe9a:	8b cf                	mov    ecx,edi
  4ebe9c:	e8 ff 61 f5 ff       	call   0x4420a0
  4ebea1:	99                   	cdq
  4ebea2:	33 c2                	xor    eax,edx
  4ebea4:	2b c2                	sub    eax,edx
  4ebea6:	3b c5                	cmp    eax,ebp
  4ebea8:	7e 0a                	jle    0x4ebeb4
  4ebeaa:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  4ebeb0:	2b c8                	sub    ecx,eax
  4ebeb2:	8b c1                	mov    eax,ecx
  4ebeb4:	3b 44 24 28          	cmp    eax,DWORD PTR [esp+0x28]
  4ebeb8:	7f 08                	jg     0x4ebec2
  4ebeba:	c7 44 24 6c 01 00 00 	mov    DWORD PTR [esp+0x6c],0x1
  4ebec1:	00 
  4ebec2:	8b 6c 24 24          	mov    ebp,DWORD PTR [esp+0x24]
  4ebec6:	8b 56 14             	mov    edx,DWORD PTR [esi+0x14]
  4ebec9:	8b c5                	mov    eax,ebp
  4ebecb:	8b 4c 24 38          	mov    ecx,DWORD PTR [esp+0x38]
  4ebecf:	2b c2                	sub    eax,edx
  4ebed1:	c7 44 24 7c 00 00 00 	mov    DWORD PTR [esp+0x7c],0x0
  4ebed8:	00 
  4ebed9:	99                   	cdq
  4ebeda:	33 c2                	xor    eax,edx
  4ebedc:	2b c2                	sub    eax,edx
  4ebede:	3b c1                	cmp    eax,ecx
  4ebee0:	7f 18                	jg     0x4ebefa
  4ebee2:	8b c5                	mov    eax,ebp
  4ebee4:	8b 6e 36             	mov    ebp,DWORD PTR [esi+0x36]
  4ebee7:	2b c5                	sub    eax,ebp
  4ebee9:	99                   	cdq
  4ebeea:	33 c2                	xor    eax,edx
  4ebeec:	2b c2                	sub    eax,edx
  4ebeee:	3b c1                	cmp    eax,ecx
  4ebef0:	7f 08                	jg     0x4ebefa
  4ebef2:	c7 44 24 7c 01 00 00 	mov    DWORD PTR [esp+0x7c],0x1
  4ebef9:	00 
  4ebefa:	8b 54 24 28          	mov    edx,DWORD PTR [esp+0x28]
  4ebefe:	43                   	inc    ebx
  4ebeff:	42                   	inc    edx
  4ebf00:	41                   	inc    ecx
  4ebf01:	89 5c 24 78          	mov    DWORD PTR [esp+0x78],ebx
  4ebf05:	89 54 24 28          	mov    DWORD PTR [esp+0x28],edx
  4ebf09:	89 4c 24 38          	mov    DWORD PTR [esp+0x38],ecx
  4ebf0d:	33 ed                	xor    ebp,ebp
  4ebf0f:	c7 44 24 74 8c 17 5e 	mov    DWORD PTR [esp+0x74],0x5e178c
  4ebf16:	00 
  4ebf17:	89 6c 24 20          	mov    DWORD PTR [esp+0x20],ebp
  4ebf1b:	8b 7c 24 74          	mov    edi,DWORD PTR [esp+0x74]
  4ebf1f:	8b 6c 24 18          	mov    ebp,DWORD PTR [esp+0x18]
  4ebf23:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4ebf27:	8b 07                	mov    eax,DWORD PTR [edi]
  4ebf29:	83 c7 04             	add    edi,0x4
  4ebf2c:	03 c5                	add    eax,ebp
  4ebf2e:	50                   	push   eax
  4ebf2f:	e8 1c 27 f2 ff       	call   0x40e650
  4ebf34:	8b 54 24 1c          	mov    edx,DWORD PTR [esp+0x1c]
  4ebf38:	89 44 24 10          	mov    DWORD PTR [esp+0x10],eax
  4ebf3c:	8b 07                	mov    eax,DWORD PTR [edi]
  4ebf3e:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4ebf42:	03 c2                	add    eax,edx
  4ebf44:	83 c7 04             	add    edi,0x4
  4ebf47:	50                   	push   eax
  4ebf48:	e8 93 73 f2 ff       	call   0x4132e0
  4ebf4d:	8b 37                	mov    esi,DWORD PTR [edi]
  4ebf4f:	8b 5c 24 24          	mov    ebx,DWORD PTR [esp+0x24]
  4ebf53:	03 f3                	add    esi,ebx
  4ebf55:	83 c7 04             	add    edi,0x4
  4ebf58:	85 f6                	test   esi,esi
  4ebf5a:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  4ebf5e:	89 7c 24 74          	mov    DWORD PTR [esp+0x74],edi
  4ebf62:	0f 8c e9 07 00 00    	jl     0x4ec751
  4ebf68:	3b 35 9c 54 6c 00    	cmp    esi,DWORD PTR ds:0x6c549c
  4ebf6e:	0f 8d dd 07 00 00    	jge    0x4ec751
  4ebf74:	8b 9c 24 a4 00 00 00 	mov    ebx,DWORD PTR [esp+0xa4]
  4ebf7b:	83 3b 00             	cmp    DWORD PTR [ebx],0x0
  4ebf7e:	0f 84 89 01 00 00    	je     0x4ec10d
  4ebf84:	8b 44 24 70          	mov    eax,DWORD PTR [esp+0x70]
  4ebf88:	85 c0                	test   eax,eax
  4ebf8a:	75 6f                	jne    0x4ebffb
  4ebf8c:	8b 7b 0c             	mov    edi,DWORD PTR [ebx+0xc]
  4ebf8f:	51                   	push   ecx
  4ebf90:	8d 54 24 14          	lea    edx,[esp+0x14]
  4ebf94:	8b cc                	mov    ecx,esp
  4ebf96:	52                   	push   edx
  4ebf97:	e8 24 23 f2 ff       	call   0x40e2c0
  4ebf9c:	8b cf                	mov    ecx,edi
  4ebf9e:	e8 ed 60 f5 ff       	call   0x442090
  4ebfa3:	8b ac 24 80 00 00 00 	mov    ebp,DWORD PTR [esp+0x80]
  4ebfaa:	99                   	cdq
  4ebfab:	33 c2                	xor    eax,edx
  4ebfad:	2b c2                	sub    eax,edx
  4ebfaf:	3b c5                	cmp    eax,ebp
  4ebfb1:	7e 0a                	jle    0x4ebfbd
  4ebfb3:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  4ebfb9:	2b c8                	sub    ecx,eax
  4ebfbb:	8b c1                	mov    eax,ecx
  4ebfbd:	3b 44 24 78          	cmp    eax,DWORD PTR [esp+0x78]
  4ebfc1:	0f 8f 8a 07 00 00    	jg     0x4ec751
  4ebfc7:	8b 7b 2e             	mov    edi,DWORD PTR [ebx+0x2e]
  4ebfca:	51                   	push   ecx
  4ebfcb:	8d 54 24 14          	lea    edx,[esp+0x14]
  4ebfcf:	8b cc                	mov    ecx,esp
  4ebfd1:	52                   	push   edx
  4ebfd2:	e8 e9 22 f2 ff       	call   0x40e2c0
  4ebfd7:	8b cf                	mov    ecx,edi
  4ebfd9:	e8 b2 60 f5 ff       	call   0x442090
  4ebfde:	99                   	cdq
  4ebfdf:	33 c2                	xor    eax,edx
  4ebfe1:	2b c2                	sub    eax,edx
  4ebfe3:	3b c5                	cmp    eax,ebp
  4ebfe5:	7e 0a                	jle    0x4ebff1
  4ebfe7:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  4ebfed:	2b c8                	sub    ecx,eax
  4ebfef:	8b c1                	mov    eax,ecx
  4ebff1:	3b 44 24 78          	cmp    eax,DWORD PTR [esp+0x78]
  4ebff5:	0f 8f 56 07 00 00    	jg     0x4ec751
  4ebffb:	8b 44 24 6c          	mov    eax,DWORD PTR [esp+0x6c]
  4ebfff:	85 c0                	test   eax,eax
  4ec001:	75 6f                	jne    0x4ec072
  4ec003:	8b 7b 10             	mov    edi,DWORD PTR [ebx+0x10]
  4ec006:	51                   	push   ecx
  4ec007:	8d 54 24 18          	lea    edx,[esp+0x18]
  4ec00b:	8b cc                	mov    ecx,esp
  4ec00d:	52                   	push   edx
  4ec00e:	e8 bd 28 f2 ff       	call   0x40e8d0
  4ec013:	8b cf                	mov    ecx,edi
  4ec015:	e8 86 60 f5 ff       	call   0x4420a0
  4ec01a:	8b ac 24 84 00 00 00 	mov    ebp,DWORD PTR [esp+0x84]
  4ec021:	99                   	cdq
  4ec022:	33 c2                	xor    eax,edx
  4ec024:	2b c2                	sub    eax,edx
  4ec026:	3b c5                	cmp    eax,ebp
  4ec028:	7e 0a                	jle    0x4ec034
  4ec02a:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  4ec030:	2b c8                	sub    ecx,eax
  4ec032:	8b c1                	mov    eax,ecx
  4ec034:	3b 44 24 28          	cmp    eax,DWORD PTR [esp+0x28]
  4ec038:	0f 8f 13 07 00 00    	jg     0x4ec751
  4ec03e:	8b 7b 32             	mov    edi,DWORD PTR [ebx+0x32]
  4ec041:	51                   	push   ecx
  4ec042:	8d 54 24 18          	lea    edx,[esp+0x18]
  4ec046:	8b cc                	mov    ecx,esp
  4ec048:	52                   	push   edx
  4ec049:	e8 82 28 f2 ff       	call   0x40e8d0
  4ec04e:	8b cf                	mov    ecx,edi
  4ec050:	e8 4b 60 f5 ff       	call   0x4420a0
  4ec055:	99                   	cdq
  4ec056:	33 c2                	xor    eax,edx
  4ec058:	2b c2                	sub    eax,edx
  4ec05a:	3b c5                	cmp    eax,ebp
  4ec05c:	7e 0a                	jle    0x4ec068
  4ec05e:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  4ec064:	2b c8                	sub    ecx,eax
  4ec066:	8b c1                	mov    eax,ecx
  4ec068:	3b 44 24 28          	cmp    eax,DWORD PTR [esp+0x28]
  4ec06c:	0f 8f df 06 00 00    	jg     0x4ec751
  4ec072:	8b 44 24 7c          	mov    eax,DWORD PTR [esp+0x7c]
  4ec076:	85 c0                	test   eax,eax
  4ec078:	75 2c                	jne    0x4ec0a6
  4ec07a:	8b 4b 14             	mov    ecx,DWORD PTR [ebx+0x14]
  4ec07d:	8b c6                	mov    eax,esi
  4ec07f:	2b c1                	sub    eax,ecx
  4ec081:	8b 4c 24 38          	mov    ecx,DWORD PTR [esp+0x38]
  4ec085:	99                   	cdq
  4ec086:	33 c2                	xor    eax,edx
  4ec088:	2b c2                	sub    eax,edx
  4ec08a:	3b c1                	cmp    eax,ecx
  4ec08c:	0f 8f bf 06 00 00    	jg     0x4ec751
  4ec092:	8b 53 36             	mov    edx,DWORD PTR [ebx+0x36]
  4ec095:	8b c6                	mov    eax,esi
  4ec097:	2b c2                	sub    eax,edx
  4ec099:	99                   	cdq
  4ec09a:	33 c2                	xor    eax,edx
  4ec09c:	2b c2                	sub    eax,edx
  4ec09e:	3b c1                	cmp    eax,ecx
  4ec0a0:	0f 8f ab 06 00 00    	jg     0x4ec751
  4ec0a6:	8b 54 24 60          	mov    edx,DWORD PTR [esp+0x60]
  4ec0aa:	8d 44 24 30          	lea    eax,[esp+0x30]
  4ec0ae:	52                   	push   edx
  4ec0af:	8d 4b 18             	lea    ecx,[ebx+0x18]
  4ec0b2:	50                   	push   eax
  4ec0b3:	51                   	push   ecx
  4ec0b4:	51                   	push   ecx
  4ec0b5:	8b cc                	mov    ecx,esp
  4ec0b7:	56                   	push   esi
  4ec0b8:	e8 f3 2d f2 ff       	call   0x40eeb0
  4ec0bd:	51                   	push   ecx
  4ec0be:	8d 54 24 28          	lea    edx,[esp+0x28]
  4ec0c2:	8b cc                	mov    ecx,esp
  4ec0c4:	52                   	push   edx
  4ec0c5:	e8 06 28 f2 ff       	call   0x40e8d0
  4ec0ca:	51                   	push   ecx
  4ec0cb:	8d 44 24 28          	lea    eax,[esp+0x28]
  4ec0cf:	8b cc                	mov    ecx,esp
  4ec0d1:	50                   	push   eax
  4ec0d2:	e8 e9 21 f2 ff       	call   0x40e2c0
  4ec0d7:	51                   	push   ecx
  4ec0d8:	8d 54 24 40          	lea    edx,[esp+0x40]
  4ec0dc:	8b cc                	mov    ecx,esp
  4ec0de:	52                   	push   edx
  4ec0df:	e8 dc 2d f2 ff       	call   0x40eec0
  4ec0e4:	51                   	push   ecx
  4ec0e5:	8d 44 24 3c          	lea    eax,[esp+0x3c]
  4ec0e9:	8b cc                	mov    ecx,esp
  4ec0eb:	50                   	push   eax
  4ec0ec:	e8 df 27 f2 ff       	call   0x40e8d0
  4ec0f1:	51                   	push   ecx
  4ec0f2:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  4ec0f6:	8b cc                	mov    ecx,esp
  4ec0f8:	52                   	push   edx
  4ec0f9:	e8 c2 21 f2 ff       	call   0x40e2c0
  4ec0fe:	b9 90 54 6c 00       	mov    ecx,0x6c5490
  4ec103:	e8 88 78 00 00       	call   0x4f3990
  4ec108:	e9 92 00 00 00       	jmp    0x4ec19f
  4ec10d:	8b 4b 08             	mov    ecx,DWORD PTR [ebx+0x8]
  4ec110:	85 c9                	test   ecx,ecx
  4ec112:	75 32                	jne    0x4ec146
  4ec114:	8b 4c 24 10          	mov    ecx,DWORD PTR [esp+0x10]
  4ec118:	8b 53 2e             	mov    edx,DWORD PTR [ebx+0x2e]
  4ec11b:	3b ca                	cmp    ecx,edx
  4ec11d:	75 23                	jne    0x4ec142
  4ec11f:	3b 43 32             	cmp    eax,DWORD PTR [ebx+0x32]
  4ec122:	75 1e                	jne    0x4ec142
  4ec124:	3b 73 36             	cmp    esi,DWORD PTR [ebx+0x36]
  4ec127:	75 19                	jne    0x4ec142
  4ec129:	8b 53 04             	mov    edx,DWORD PTR [ebx+0x4]
  4ec12c:	8b ba 03 0d 00 00    	mov    edi,DWORD PTR [edx+0xd03]
  4ec132:	85 ff                	test   edi,edi
  4ec134:	75 0c                	jne    0x4ec142
  4ec136:	39 4c 24 18          	cmp    DWORD PTR [esp+0x18],ecx
  4ec13a:	74 0a                	je     0x4ec146
  4ec13c:	39 44 24 1c          	cmp    DWORD PTR [esp+0x1c],eax
  4ec140:	74 04                	je     0x4ec146
  4ec142:	6a 00                	push   0x0
  4ec144:	eb 02                	jmp    0x4ec148
  4ec146:	6a 01                	push   0x1
  4ec148:	8d 44 24 34          	lea    eax,[esp+0x34]
  4ec14c:	50                   	push   eax
  4ec14d:	51                   	push   ecx
  4ec14e:	8b cc                	mov    ecx,esp
  4ec150:	56                   	push   esi
  4ec151:	e8 5a 2d f2 ff       	call   0x40eeb0
  4ec156:	51                   	push   ecx
  4ec157:	8d 54 24 24          	lea    edx,[esp+0x24]
  4ec15b:	8b cc                	mov    ecx,esp
  4ec15d:	52                   	push   edx
  4ec15e:	e8 6d 27 f2 ff       	call   0x40e8d0
  4ec163:	51                   	push   ecx
  4ec164:	8d 44 24 24          	lea    eax,[esp+0x24]
  4ec168:	8b cc                	mov    ecx,esp
  4ec16a:	50                   	push   eax
  4ec16b:	e8 50 21 f2 ff       	call   0x40e2c0
  4ec170:	51                   	push   ecx
  4ec171:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  4ec175:	8b cc                	mov    ecx,esp
  4ec177:	52                   	push   edx
  4ec178:	e8 43 2d f2 ff       	call   0x40eec0
  4ec17d:	51                   	push   ecx
  4ec17e:	8d 44 24 38          	lea    eax,[esp+0x38]
  4ec182:	8b cc                	mov    ecx,esp
  4ec184:	50                   	push   eax
  4ec185:	e8 46 27 f2 ff       	call   0x40e8d0
  4ec18a:	51                   	push   ecx
  4ec18b:	8d 54 24 38          	lea    edx,[esp+0x38]
  4ec18f:	8b cc                	mov    ecx,esp
  4ec191:	52                   	push   edx
  4ec192:	e8 29 21 f2 ff       	call   0x40e2c0
  4ec197:	8b 4b 04             	mov    ecx,DWORD PTR [ebx+0x4]
  4ec19a:	e8 c1 81 02 00       	call   0x514360
  4ec19f:	85 c0                	test   eax,eax
  4ec1a1:	0f 84 aa 05 00 00    	je     0x4ec751
  4ec1a7:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
  4ec1ab:	8b 3c b5 c2 b8 6c 00 	mov    edi,DWORD PTR [esi*4+0x6cb8c2]
  4ec1b2:	8b 54 24 10          	mov    edx,DWORD PTR [esp+0x10]
  4ec1b6:	8b 04 85 42 b9 6c 00 	mov    eax,DWORD PTR [eax*4+0x6cb942]
  4ec1bd:	03 c7                	add    eax,edi
  4ec1bf:	8b fe                	mov    edi,esi
  4ec1c1:	03 c2                	add    eax,edx
  4ec1c3:	8b 15 dc 54 6c 00    	mov    edx,DWORD PTR ds:0x6c54dc
  4ec1c9:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4ec1cc:	8d 04 8a             	lea    eax,[edx+ecx*4]
  4ec1cf:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4ec1d3:	51                   	push   ecx
  4ec1d4:	2b f9                	sub    edi,ecx
  4ec1d6:	8d 54 24 20          	lea    edx,[esp+0x20]
  4ec1da:	8b cc                	mov    ecx,esp
  4ec1dc:	52                   	push   edx
  4ec1dd:	89 44 24 48          	mov    DWORD PTR [esp+0x48],eax
  4ec1e1:	e8 ea 26 f2 ff       	call   0x40e8d0
  4ec1e6:	8d 4c 24 18          	lea    ecx,[esp+0x18]
  4ec1ea:	e8 81 29 f2 ff       	call   0x40eb70
  4ec1ef:	8b e8                	mov    ebp,eax
  4ec1f1:	51                   	push   ecx
  4ec1f2:	8d 44 24 1c          	lea    eax,[esp+0x1c]
  4ec1f6:	8b cc                	mov    ecx,esp
  4ec1f8:	50                   	push   eax
  4ec1f9:	e8 c2 20 f2 ff       	call   0x40e2c0
  4ec1fe:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4ec202:	e8 d9 22 f2 ff       	call   0x40e4e0
  4ec207:	57                   	push   edi
  4ec208:	8d 54 2d 00          	lea    edx,[ebp+ebp*1+0x0]
  4ec20c:	8d 0c 00             	lea    ecx,[eax+eax*1]
  4ec20f:	e8 fc e9 ff ff       	call   0x4eac10
  4ec214:	8b 0b                	mov    ecx,DWORD PTR [ebx]
  4ec216:	89 44 24 34          	mov    DWORD PTR [esp+0x34],eax
  4ec21a:	85 c9                	test   ecx,ecx
  4ec21c:	74 2a                	je     0x4ec248
  4ec21e:	8b 54 24 30          	mov    edx,DWORD PTR [esp+0x30]
  4ec222:	8d 0c c5 00 00 00 00 	lea    ecx,[eax*8+0x0]
  4ec229:	33 c0                	xor    eax,eax
  4ec22b:	89 4c 24 3c          	mov    DWORD PTR [esp+0x3c],ecx
  4ec22f:	89 54 24 44          	mov    DWORD PTR [esp+0x44],edx
  4ec233:	89 44 24 48          	mov    DWORD PTR [esp+0x48],eax
  4ec237:	89 44 24 4c          	mov    DWORD PTR [esp+0x4c],eax
  4ec23b:	89 44 24 50          	mov    DWORD PTR [esp+0x50],eax
  4ec23f:	89 44 24 54          	mov    DWORD PTR [esp+0x54],eax
  4ec243:	e9 8c 02 00 00       	jmp    0x4ec4d4
  4ec248:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
  4ec24c:	8b 4c 24 18          	mov    ecx,DWORD PTR [esp+0x18]
  4ec250:	3b c1                	cmp    eax,ecx
  4ec252:	8b 4c 24 1c          	mov    ecx,DWORD PTR [esp+0x1c]
  4ec256:	75 22                	jne    0x4ec27a
  4ec258:	39 4c 24 14          	cmp    DWORD PTR [esp+0x14],ecx
  4ec25c:	75 1c                	jne    0x4ec27a
  4ec25e:	8b 8c 24 a8 00 00 00 	mov    ecx,DWORD PTR [esp+0xa8]
  4ec265:	8b 54 24 24          	mov    edx,DWORD PTR [esp+0x24]
  4ec269:	8b c6                	mov    eax,esi
  4ec26b:	8b 79 04             	mov    edi,DWORD PTR [ecx+0x4]
  4ec26e:	2b c2                	sub    eax,edx
  4ec270:	89 7c 24 68          	mov    DWORD PTR [esp+0x68],edi
  4ec274:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
  4ec278:	eb 43                	jmp    0x4ec2bd
  4ec27a:	51                   	push   ecx
  4ec27b:	8b c4                	mov    eax,esp
  4ec27d:	89 08                	mov    DWORD PTR [eax],ecx
  4ec27f:	8d 4c 24 18          	lea    ecx,[esp+0x18]
  4ec283:	e8 e8 28 f2 ff       	call   0x40eb70
  4ec288:	51                   	push   ecx
  4ec289:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4ec28d:	8b cc                	mov    ecx,esp
  4ec28f:	52                   	push   edx
  4ec290:	8b f8                	mov    edi,eax
  4ec292:	e8 29 20 f2 ff       	call   0x40e2c0
  4ec297:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4ec29b:	e8 40 22 f2 ff       	call   0x40e4e0
  4ec2a0:	8b 8c 24 a8 00 00 00 	mov    ecx,DWORD PTR [esp+0xa8]
  4ec2a7:	8d 04 78             	lea    eax,[eax+edi*2]
  4ec2aa:	03 f8                	add    edi,eax
  4ec2ac:	33 c0                	xor    eax,eax
  4ec2ae:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
  4ec2b2:	8b 3c bd c4 41 5e 00 	mov    edi,DWORD PTR [edi*4+0x5e41c4]
  4ec2b9:	89 7c 24 68          	mov    DWORD PTR [esp+0x68],edi
  4ec2bd:	3b 41 08             	cmp    eax,DWORD PTR [ecx+0x8]
  4ec2c0:	75 2f                	jne    0x4ec2f1
  4ec2c2:	8b 41 04             	mov    eax,DWORD PTR [ecx+0x4]
  4ec2c5:	2b c7                	sub    eax,edi
  4ec2c7:	83 c0 08             	add    eax,0x8
  4ec2ca:	25 07 00 00 80       	and    eax,0x80000007
  4ec2cf:	79 05                	jns    0x4ec2d6
  4ec2d1:	48                   	dec    eax
  4ec2d2:	83 c8 f8             	or     eax,0xfffffff8
  4ec2d5:	40                   	inc    eax
  4ec2d6:	83 f8 04             	cmp    eax,0x4
  4ec2d9:	7e 03                	jle    0x4ec2de
  4ec2db:	83 c0 f8             	add    eax,0xfffffff8
  4ec2de:	99                   	cdq
  4ec2df:	33 c2                	xor    eax,edx
  4ec2e1:	2b c2                	sub    eax,edx
  4ec2e3:	83 f8 01             	cmp    eax,0x1
  4ec2e6:	7f 09                	jg     0x4ec2f1
  4ec2e8:	8b 49 0c             	mov    ecx,DWORD PTR [ecx+0xc]
  4ec2eb:	89 4c 24 64          	mov    DWORD PTR [esp+0x64],ecx
  4ec2ef:	eb 08                	jmp    0x4ec2f9
  4ec2f1:	c7 44 24 64 00 00 00 	mov    DWORD PTR [esp+0x64],0x0
  4ec2f8:	00 
  4ec2f9:	8d 94 24 98 00 00 00 	lea    edx,[esp+0x98]
  4ec300:	8b ce                	mov    ecx,esi
  4ec302:	52                   	push   edx
  4ec303:	8b 54 24 28          	mov    edx,DWORD PTR [esp+0x28]
  4ec307:	8d 44 24 68          	lea    eax,[esp+0x68]
  4ec30b:	2b ca                	sub    ecx,edx
  4ec30d:	50                   	push   eax
  4ec30e:	51                   	push   ecx
  4ec30f:	51                   	push   ecx
  4ec310:	8d 54 24 2c          	lea    edx,[esp+0x2c]
  4ec314:	8b cc                	mov    ecx,esp
  4ec316:	52                   	push   edx
  4ec317:	e8 b4 25 f2 ff       	call   0x40e8d0
  4ec31c:	8d 4c 24 24          	lea    ecx,[esp+0x24]
  4ec320:	e8 4b 28 f2 ff       	call   0x40eb70
  4ec325:	50                   	push   eax
  4ec326:	51                   	push   ecx
  4ec327:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4ec32b:	8b cc                	mov    ecx,esp
  4ec32d:	50                   	push   eax
  4ec32e:	e8 8d 1f f2 ff       	call   0x40e2c0
  4ec333:	8d 4c 24 24          	lea    ecx,[esp+0x24]
  4ec337:	e8 a4 21 f2 ff       	call   0x40e4e0
  4ec33c:	8b 4c 24 40          	mov    ecx,DWORD PTR [esp+0x40]
  4ec340:	50                   	push   eax
  4ec341:	51                   	push   ecx
  4ec342:	51                   	push   ecx
  4ec343:	8d 54 24 40          	lea    edx,[esp+0x40]
  4ec347:	8b cc                	mov    ecx,esp
  4ec349:	52                   	push   edx
  4ec34a:	e8 71 2b f2 ff       	call   0x40eec0
  4ec34f:	51                   	push   ecx
  4ec350:	8d 44 24 3c          	lea    eax,[esp+0x3c]
  4ec354:	8b cc                	mov    ecx,esp
  4ec356:	50                   	push   eax
  4ec357:	e8 74 25 f2 ff       	call   0x40e8d0
  4ec35c:	51                   	push   ecx
  4ec35d:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  4ec361:	8b cc                	mov    ecx,esp
  4ec363:	52                   	push   edx
  4ec364:	e8 57 1f f2 ff       	call   0x40e2c0
  4ec369:	8b 4c 24 64          	mov    ecx,DWORD PTR [esp+0x64]
  4ec36d:	e8 be ec ff ff       	call   0x4eb030
  4ec372:	8b 4b 04             	mov    ecx,DWORD PTR [ebx+0x4]
  4ec375:	50                   	push   eax
  4ec376:	e8 35 42 03 00       	call   0x5205b0
  4ec37b:	8b 44 24 24          	mov    eax,DWORD PTR [esp+0x24]
  4ec37f:	8b fe                	mov    edi,esi
  4ec381:	2b f8                	sub    edi,eax
  4ec383:	51                   	push   ecx
  4ec384:	8d 44 24 20          	lea    eax,[esp+0x20]
  4ec388:	8b cc                	mov    ecx,esp
  4ec38a:	50                   	push   eax
  4ec38b:	e8 40 25 f2 ff       	call   0x40e8d0
  4ec390:	8d 4c 24 18          	lea    ecx,[esp+0x18]
  4ec394:	e8 d7 27 f2 ff       	call   0x40eb70
  4ec399:	51                   	push   ecx
  4ec39a:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4ec39e:	8b cc                	mov    ecx,esp
  4ec3a0:	52                   	push   edx
  4ec3a1:	8b e8                	mov    ebp,eax
  4ec3a3:	e8 18 1f f2 ff       	call   0x40e2c0
  4ec3a8:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4ec3ac:	e8 2f 21 f2 ff       	call   0x40e4e0
  4ec3b1:	57                   	push   edi
  4ec3b2:	8d 54 2d 00          	lea    edx,[ebp+ebp*1+0x0]
  4ec3b6:	8d 0c 00             	lea    ecx,[eax+eax*1]
  4ec3b9:	e8 52 e8 ff ff       	call   0x4eac10
  4ec3be:	8b 4c 24 60          	mov    ecx,DWORD PTR [esp+0x60]
  4ec3c2:	8b 6c 24 64          	mov    ebp,DWORD PTR [esp+0x64]
  4ec3c6:	33 d2                	xor    edx,edx
  4ec3c8:	0f af 81 89 05 00 00 	imul   eax,DWORD PTR [ecx+0x589]
  4ec3cf:	c1 e0 02             	shl    eax,0x2
  4ec3d2:	f7 f5                	div    ebp
  4ec3d4:	89 44 24 3c          	mov    DWORD PTR [esp+0x3c],eax
  4ec3d8:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
  4ec3dc:	83 f8 02             	cmp    eax,0x2
  4ec3df:	74 17                	je     0x4ec3f8
  4ec3e1:	83 f8 03             	cmp    eax,0x3
  4ec3e4:	74 12                	je     0x4ec3f8
  4ec3e6:	83 f8 01             	cmp    eax,0x1
  4ec3e9:	74 18                	je     0x4ec403
  4ec3eb:	8b 54 24 3c          	mov    edx,DWORD PTR [esp+0x3c]
  4ec3ef:	8d 0c 12             	lea    ecx,[edx+edx*1]
  4ec3f2:	89 4c 24 3c          	mov    DWORD PTR [esp+0x3c],ecx
  4ec3f6:	eb 0b                	jmp    0x4ec403
  4ec3f8:	8b 4c 24 3c          	mov    ecx,DWORD PTR [esp+0x3c]
  4ec3fc:	8d 14 49             	lea    edx,[ecx+ecx*2]
  4ec3ff:	89 54 24 3c          	mov    DWORD PTR [esp+0x3c],edx
  4ec403:	8b 4b 04             	mov    ecx,DWORD PTR [ebx+0x4]
  4ec406:	8b 89 a8 00 00 00    	mov    ecx,DWORD PTR [ecx+0xa8]
  4ec40c:	8d 14 c9             	lea    edx,[ecx+ecx*8]
  4ec40f:	8d 14 91             	lea    edx,[ecx+edx*4]
  4ec412:	8d 14 92             	lea    edx,[edx+edx*4]
  4ec415:	8b bc d1 2d 65 6a 00 	mov    edi,DWORD PTR [ecx+edx*8+0x6a652d]
  4ec41c:	85 ff                	test   edi,edi
  4ec41e:	74 51                	je     0x4ec471
  4ec420:	8b 4c 24 14          	mov    ecx,DWORD PTR [esp+0x14]
  4ec424:	8b 1c b5 c2 b8 6c 00 	mov    ebx,DWORD PTR [esi*4+0x6cb8c2]
  4ec42b:	8b 3d dc 54 6c 00    	mov    edi,DWORD PTR ds:0x6c54dc
  4ec431:	8b 14 8d 42 b9 6c 00 	mov    edx,DWORD PTR [ecx*4+0x6cb942]
  4ec438:	8b ca                	mov    ecx,edx
  4ec43a:	03 cb                	add    ecx,ebx
  4ec43c:	8b 5c 24 10          	mov    ebx,DWORD PTR [esp+0x10]
  4ec440:	03 cb                	add    ecx,ebx
  4ec442:	8d 0c 49             	lea    ecx,[ecx+ecx*2]
  4ec445:	f6 44 8f 0a 10       	test   BYTE PTR [edi+ecx*4+0xa],0x10
  4ec44a:	75 1a                	jne    0x4ec466
  4ec44c:	83 fe 01             	cmp    esi,0x1
  4ec44f:	7e 20                	jle    0x4ec471
  4ec451:	8b 34 b5 be b8 6c 00 	mov    esi,DWORD PTR [esi*4+0x6cb8be]
  4ec458:	03 f2                	add    esi,edx
  4ec45a:	03 f3                	add    esi,ebx
  4ec45c:	8d 0c 76             	lea    ecx,[esi+esi*2]
  4ec45f:	f6 44 8f 0a 10       	test   BYTE PTR [edi+ecx*4+0xa],0x10
  4ec464:	74 0b                	je     0x4ec471
  4ec466:	8b 4c 24 3c          	mov    ecx,DWORD PTR [esp+0x3c]
  4ec46a:	8d 14 49             	lea    edx,[ecx+ecx*2]
  4ec46d:	89 54 24 3c          	mov    DWORD PTR [esp+0x3c],edx
  4ec471:	89 44 24 44          	mov    DWORD PTR [esp+0x44],eax
  4ec475:	8b 44 24 68          	mov    eax,DWORD PTR [esp+0x68]
  4ec479:	89 44 24 48          	mov    DWORD PTR [esp+0x48],eax
  4ec47d:	8b 44 24 34          	mov    eax,DWORD PTR [esp+0x34]
  4ec481:	8b 4c 24 2c          	mov    ecx,DWORD PTR [esp+0x2c]
  4ec485:	89 6c 24 50          	mov    DWORD PTR [esp+0x50],ebp
  4ec489:	8d 14 c0             	lea    edx,[eax+eax*8]
  4ec48c:	89 4c 24 4c          	mov    DWORD PTR [esp+0x4c],ecx
  4ec490:	c1 e2 08             	shl    edx,0x8
  4ec493:	33 c0                	xor    eax,eax
  4ec495:	89 94 24 88 00 00 00 	mov    DWORD PTR [esp+0x88],edx
  4ec49c:	89 84 24 8c 00 00 00 	mov    DWORD PTR [esp+0x8c],eax
  4ec4a3:	8d 0c ad 00 00 00 00 	lea    ecx,[ebp*4+0x0]
  4ec4aa:	df ac 24 88 00 00 00 	fild   QWORD PTR [esp+0x88]
  4ec4b1:	89 8c 24 90 00 00 00 	mov    DWORD PTR [esp+0x90],ecx
  4ec4b8:	89 84 24 94 00 00 00 	mov    DWORD PTR [esp+0x94],eax
  4ec4bf:	8b 94 24 a8 00 00 00 	mov    edx,DWORD PTR [esp+0xa8]
  4ec4c6:	da b4 24 90 00 00 00 	fidiv  DWORD PTR [esp+0x90]
  4ec4cd:	d8 42 10             	fadd   DWORD PTR [edx+0x10]
  4ec4d0:	d9 5c 24 54          	fstp   DWORD PTR [esp+0x54]
  4ec4d4:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec4db:	b8 39 8e e3 38       	mov    eax,0x38e38e39
  4ec4e0:	8b 6f 08             	mov    ebp,DWORD PTR [edi+0x8]
  4ec4e3:	8b 4f 0c             	mov    ecx,DWORD PTR [edi+0xc]
  4ec4e6:	2b cd                	sub    ecx,ebp
  4ec4e8:	8b dd                	mov    ebx,ebp
  4ec4ea:	f7 e9                	imul   ecx
  4ec4ec:	c1 fa 03             	sar    edx,0x3
  4ec4ef:	8b c2                	mov    eax,edx
  4ec4f1:	c1 e8 1f             	shr    eax,0x1f
  4ec4f4:	03 d0                	add    edx,eax
  4ec4f6:	83 fa 01             	cmp    edx,0x1
  4ec4f9:	0f 83 38 01 00 00    	jae    0x4ec637
  4ec4ff:	8b 77 04             	mov    esi,DWORD PTR [edi+0x4]
  4ec502:	85 f6                	test   esi,esi
  4ec504:	74 1c                	je     0x4ec522
  4ec506:	8b cd                	mov    ecx,ebp
  4ec508:	b8 39 8e e3 38       	mov    eax,0x38e38e39
  4ec50d:	2b ce                	sub    ecx,esi
  4ec50f:	f7 e9                	imul   ecx
  4ec511:	c1 fa 03             	sar    edx,0x3
  4ec514:	8b ca                	mov    ecx,edx
  4ec516:	c1 e9 1f             	shr    ecx,0x1f
  4ec519:	03 d1                	add    edx,ecx
  4ec51b:	83 fa 01             	cmp    edx,0x1
  4ec51e:	8b ca                	mov    ecx,edx
  4ec520:	77 05                	ja     0x4ec527
  4ec522:	b9 01 00 00 00       	mov    ecx,0x1
  4ec527:	85 f6                	test   esi,esi
  4ec529:	75 04                	jne    0x4ec52f
  4ec52b:	33 d2                	xor    edx,edx
  4ec52d:	eb 13                	jmp    0x4ec542
  4ec52f:	2b ee                	sub    ebp,esi
  4ec531:	b8 39 8e e3 38       	mov    eax,0x38e38e39
  4ec536:	f7 ed                	imul   ebp
  4ec538:	c1 fa 03             	sar    edx,0x3
  4ec53b:	8b c2                	mov    eax,edx
  4ec53d:	c1 e8 1f             	shr    eax,0x1f
  4ec540:	03 d0                	add    edx,eax
  4ec542:	8d 04 0a             	lea    eax,[edx+ecx*1]
  4ec545:	85 c0                	test   eax,eax
  4ec547:	89 44 24 34          	mov    DWORD PTR [esp+0x34],eax
  4ec54b:	7d 02                	jge    0x4ec54f
  4ec54d:	33 c0                	xor    eax,eax
  4ec54f:	8d 0c c0             	lea    ecx,[eax+eax*8]
  4ec552:	c1 e1 02             	shl    ecx,0x2
  4ec555:	51                   	push   ecx
  4ec556:	e8 f5 b2 0a 00       	call   0x597850
  4ec55b:	8b 57 04             	mov    edx,DWORD PTR [edi+0x4]
  4ec55e:	83 c4 04             	add    esp,0x4
  4ec561:	3b d3                	cmp    edx,ebx
  4ec563:	89 44 24 2c          	mov    DWORD PTR [esp+0x2c],eax
  4ec567:	74 20                	je     0x4ec589
  4ec569:	85 c0                	test   eax,eax
  4ec56b:	74 12                	je     0x4ec57f
  4ec56d:	b9 09 00 00 00       	mov    ecx,0x9
  4ec572:	8b f2                	mov    esi,edx
  4ec574:	8b f8                	mov    edi,eax
  4ec576:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec578:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec57f:	83 c2 24             	add    edx,0x24
  4ec582:	83 c0 24             	add    eax,0x24
  4ec585:	3b d3                	cmp    edx,ebx
  4ec587:	75 e0                	jne    0x4ec569
  4ec589:	85 c0                	test   eax,eax
  4ec58b:	74 14                	je     0x4ec5a1
  4ec58d:	b9 09 00 00 00       	mov    ecx,0x9
  4ec592:	8d 74 24 3c          	lea    esi,[esp+0x3c]
  4ec596:	8b f8                	mov    edi,eax
  4ec598:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec59a:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec5a1:	8b 6f 08             	mov    ebp,DWORD PTR [edi+0x8]
  4ec5a4:	8d 50 24             	lea    edx,[eax+0x24]
  4ec5a7:	3b dd                	cmp    ebx,ebp
  4ec5a9:	74 28                	je     0x4ec5d3
  4ec5ab:	8b ca                	mov    ecx,edx
  4ec5ad:	2b c8                	sub    ecx,eax
  4ec5af:	8d 5c 19 dc          	lea    ebx,[ecx+ebx*1-0x24]
  4ec5b3:	85 d2                	test   edx,edx
  4ec5b5:	74 12                	je     0x4ec5c9
  4ec5b7:	b9 09 00 00 00       	mov    ecx,0x9
  4ec5bc:	8b f3                	mov    esi,ebx
  4ec5be:	8b fa                	mov    edi,edx
  4ec5c0:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec5c2:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec5c9:	83 c3 24             	add    ebx,0x24
  4ec5cc:	83 c2 24             	add    edx,0x24
  4ec5cf:	3b dd                	cmp    ebx,ebp
  4ec5d1:	75 e0                	jne    0x4ec5b3
  4ec5d3:	8b 47 04             	mov    eax,DWORD PTR [edi+0x4]
  4ec5d6:	50                   	push   eax
  4ec5d7:	89 44 24 6c          	mov    DWORD PTR [esp+0x6c],eax
  4ec5db:	e8 90 b2 0a 00       	call   0x597870
  4ec5e0:	8b 44 24 38          	mov    eax,DWORD PTR [esp+0x38]
  4ec5e4:	8b 74 24 30          	mov    esi,DWORD PTR [esp+0x30]
  4ec5e8:	83 c4 04             	add    esp,0x4
  4ec5eb:	8d 14 c0             	lea    edx,[eax+eax*8]
  4ec5ee:	8d 04 96             	lea    eax,[esi+edx*4]
  4ec5f1:	89 47 0c             	mov    DWORD PTR [edi+0xc],eax
  4ec5f4:	8b 47 04             	mov    eax,DWORD PTR [edi+0x4]
  4ec5f7:	85 c0                	test   eax,eax
  4ec5f9:	75 14                	jne    0x4ec60f
  4ec5fb:	33 d2                	xor    edx,edx
  4ec5fd:	89 77 04             	mov    DWORD PTR [edi+0x4],esi
  4ec600:	8d 54 d2 09          	lea    edx,[edx+edx*8+0x9]
  4ec604:	8d 04 96             	lea    eax,[esi+edx*4]
  4ec607:	89 47 08             	mov    DWORD PTR [edi+0x8],eax
  4ec60a:	e9 42 01 00 00       	jmp    0x4ec751
  4ec60f:	8b 4f 08             	mov    ecx,DWORD PTR [edi+0x8]
  4ec612:	89 77 04             	mov    DWORD PTR [edi+0x4],esi
  4ec615:	2b c8                	sub    ecx,eax
  4ec617:	b8 39 8e e3 38       	mov    eax,0x38e38e39
  4ec61c:	f7 e9                	imul   ecx
  4ec61e:	c1 fa 03             	sar    edx,0x3
  4ec621:	8b ca                	mov    ecx,edx
  4ec623:	c1 e9 1f             	shr    ecx,0x1f
  4ec626:	03 d1                	add    edx,ecx
  4ec628:	8d 54 d2 09          	lea    edx,[edx+edx*8+0x9]
  4ec62c:	8d 04 96             	lea    eax,[esi+edx*4]
  4ec62f:	89 47 08             	mov    DWORD PTR [edi+0x8],eax
  4ec632:	e9 1a 01 00 00       	jmp    0x4ec751
  4ec637:	8b cd                	mov    ecx,ebp
  4ec639:	b8 39 8e e3 38       	mov    eax,0x38e38e39
  4ec63e:	2b cb                	sub    ecx,ebx
  4ec640:	f7 e9                	imul   ecx
  4ec642:	c1 fa 03             	sar    edx,0x3
  4ec645:	8b ca                	mov    ecx,edx
  4ec647:	c1 e9 1f             	shr    ecx,0x1f
  4ec64a:	03 d1                	add    edx,ecx
  4ec64c:	83 fa 01             	cmp    edx,0x1
  4ec64f:	0f 83 88 00 00 00    	jae    0x4ec6dd
  4ec655:	3b dd                	cmp    ebx,ebp
  4ec657:	8d 43 24             	lea    eax,[ebx+0x24]
  4ec65a:	74 23                	je     0x4ec67f
  4ec65c:	8d 50 dc             	lea    edx,[eax-0x24]
  4ec65f:	85 c0                	test   eax,eax
  4ec661:	74 12                	je     0x4ec675
  4ec663:	b9 09 00 00 00       	mov    ecx,0x9
  4ec668:	8b f2                	mov    esi,edx
  4ec66a:	8b f8                	mov    edi,eax
  4ec66c:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec66e:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec675:	83 c2 24             	add    edx,0x24
  4ec678:	83 c0 24             	add    eax,0x24
  4ec67b:	3b d5                	cmp    edx,ebp
  4ec67d:	75 e0                	jne    0x4ec65f
  4ec67f:	8b 77 08             	mov    esi,DWORD PTR [edi+0x8]
  4ec682:	b8 39 8e e3 38       	mov    eax,0x38e38e39
  4ec687:	8b ce                	mov    ecx,esi
  4ec689:	2b cb                	sub    ecx,ebx
  4ec68b:	f7 e9                	imul   ecx
  4ec68d:	c1 fa 03             	sar    edx,0x3
  4ec690:	8b c2                	mov    eax,edx
  4ec692:	c1 e8 1f             	shr    eax,0x1f
  4ec695:	03 d0                	add    edx,eax
  4ec697:	b8 01 00 00 00       	mov    eax,0x1
  4ec69c:	2b c2                	sub    eax,edx
  4ec69e:	8b d6                	mov    edx,esi
  4ec6a0:	74 1e                	je     0x4ec6c0
  4ec6a2:	85 d2                	test   edx,edx
  4ec6a4:	74 14                	je     0x4ec6ba
  4ec6a6:	b9 09 00 00 00       	mov    ecx,0x9
  4ec6ab:	8d 74 24 3c          	lea    esi,[esp+0x3c]
  4ec6af:	8b fa                	mov    edi,edx
  4ec6b1:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec6b3:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec6ba:	83 c2 24             	add    edx,0x24
  4ec6bd:	48                   	dec    eax
  4ec6be:	75 e2                	jne    0x4ec6a2
  4ec6c0:	8b 47 08             	mov    eax,DWORD PTR [edi+0x8]
  4ec6c3:	3b d8                	cmp    ebx,eax
  4ec6c5:	74 7f                	je     0x4ec746
  4ec6c7:	8b fb                	mov    edi,ebx
  4ec6c9:	83 c3 24             	add    ebx,0x24
  4ec6cc:	b9 09 00 00 00       	mov    ecx,0x9
  4ec6d1:	8d 74 24 3c          	lea    esi,[esp+0x3c]
  4ec6d5:	3b d8                	cmp    ebx,eax
  4ec6d7:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec6d9:	75 ec                	jne    0x4ec6c7
  4ec6db:	eb 69                	jmp    0x4ec746
  4ec6dd:	8d 45 dc             	lea    eax,[ebp-0x24]
  4ec6e0:	8b d5                	mov    edx,ebp
  4ec6e2:	3b c5                	cmp    eax,ebp
  4ec6e4:	89 44 24 34          	mov    DWORD PTR [esp+0x34],eax
  4ec6e8:	74 20                	je     0x4ec70a
  4ec6ea:	85 d2                	test   edx,edx
  4ec6ec:	74 12                	je     0x4ec700
  4ec6ee:	b9 09 00 00 00       	mov    ecx,0x9
  4ec6f3:	8b f0                	mov    esi,eax
  4ec6f5:	8b fa                	mov    edi,edx
  4ec6f7:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec6f9:	8b bc 24 a0 00 00 00 	mov    edi,DWORD PTR [esp+0xa0]
  4ec700:	83 c0 24             	add    eax,0x24
  4ec703:	83 c2 24             	add    edx,0x24
  4ec706:	3b c5                	cmp    eax,ebp
  4ec708:	75 e0                	jne    0x4ec6ea
  4ec70a:	8b 57 08             	mov    edx,DWORD PTR [edi+0x8]
  4ec70d:	8d 42 dc             	lea    eax,[edx-0x24]
  4ec710:	3b d8                	cmp    ebx,eax
  4ec712:	74 15                	je     0x4ec729
  4ec714:	83 e8 24             	sub    eax,0x24
  4ec717:	83 ea 24             	sub    edx,0x24
  4ec71a:	b9 09 00 00 00       	mov    ecx,0x9
  4ec71f:	8b f0                	mov    esi,eax
  4ec721:	8b fa                	mov    edi,edx
  4ec723:	3b c3                	cmp    eax,ebx
  4ec725:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec727:	75 eb                	jne    0x4ec714
  4ec729:	8d 53 24             	lea    edx,[ebx+0x24]
  4ec72c:	8b c3                	mov    eax,ebx
  4ec72e:	3b da                	cmp    ebx,edx
  4ec730:	74 14                	je     0x4ec746
  4ec732:	8b f8                	mov    edi,eax
  4ec734:	83 c0 24             	add    eax,0x24
  4ec737:	b9 09 00 00 00       	mov    ecx,0x9
  4ec73c:	8d 74 24 3c          	lea    esi,[esp+0x3c]
  4ec740:	3b c2                	cmp    eax,edx
  4ec742:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4ec744:	75 ec                	jne    0x4ec732
  4ec746:	8b 84 24 a0 00 00 00 	mov    eax,DWORD PTR [esp+0xa0]
  4ec74d:	83 40 08 24          	add    DWORD PTR [eax+0x8],0x24
  4ec751:	8b 44 24 20          	mov    eax,DWORD PTR [esp+0x20]
  4ec755:	40                   	inc    eax
  4ec756:	83 f8 1a             	cmp    eax,0x1a
  4ec759:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4ec75d:	0f 8c b8 f7 ff ff    	jl     0x4ebf1b
  4ec763:	8b 84 24 ac 00 00 00 	mov    eax,DWORD PTR [esp+0xac]
  4ec76a:	ff 08                	dec    DWORD PTR [eax]
  4ec76c:	5f                   	pop    edi
  4ec76d:	5e                   	pop    esi
  4ec76e:	5d                   	pop    ebp
  4ec76f:	5b                   	pop    ebx
  4ec770:	81 c4 8c 00 00 00    	add    esp,0x8c
  4ec776:	c2 10 00             	ret    0x10
