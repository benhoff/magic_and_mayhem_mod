
/home/hoff/swdev/magic_and_mayhem_mod/working/worktrees/remaining-medium-20261020/original/Arcane_Nocd/Chaos.exe:     file format pei-i386


Disassembly of section .text:

004fdad0 <.text+0xfcad0>:
  4fdad0:	81 ec 30 01 00 00    	sub    esp,0x130
  4fdad6:	53                   	push   ebx
  4fdad7:	55                   	push   ebp
  4fdad8:	56                   	push   esi
  4fdad9:	8b e9                	mov    ebp,ecx
  4fdadb:	be 01 00 00 00       	mov    esi,0x1
  4fdae0:	33 db                	xor    ebx,ebx
  4fdae2:	57                   	push   edi
  4fdae3:	8d 4c 24 2c          	lea    ecx,[esp+0x2c]
  4fdae7:	89 b4 24 e0 00 00 00 	mov    DWORD PTR [esp+0xe0],esi
  4fdaee:	c7 84 24 e4 00 00 00 ff ff ff ff 	mov    DWORD PTR [esp+0xe4],0xffffffff
  4fdaf9:	89 9c 24 e8 00 00 00 	mov    DWORD PTR [esp+0xe8],ebx
  4fdb00:	89 5c 24 20          	mov    DWORD PTR [esp+0x20],ebx
  4fdb04:	89 5c 24 18          	mov    DWORD PTR [esp+0x18],ebx
  4fdb08:	89 5c 24 24          	mov    DWORD PTR [esp+0x24],ebx
  4fdb0c:	89 5c 24 10          	mov    DWORD PTR [esp+0x10],ebx
  4fdb10:	89 5c 24 1c          	mov    DWORD PTR [esp+0x1c],ebx
  4fdb14:	89 5c 24 14          	mov    DWORD PTR [esp+0x14],ebx
  4fdb18:	89 9c 24 ec 00 00 00 	mov    DWORD PTR [esp+0xec],ebx
  4fdb1f:	c7 84 24 f0 00 00 00 04 00 00 00 	mov    DWORD PTR [esp+0xf0],0x4
  4fdb2a:	c7 84 24 f4 00 00 00 21 00 00 00 	mov    DWORD PTR [esp+0xf4],0x21
  4fdb35:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4fdb39:	e8 82 49 00 00       	call   0x5024c0
  4fdb3e:	b9 13 00 00 00       	mov    ecx,0x13
  4fdb43:	b8 0e 00 00 00       	mov    eax,0xe
  4fdb48:	89 8c 24 b0 00 00 00 	mov    DWORD PTR [esp+0xb0],ecx
  4fdb4f:	89 8c 24 9c 00 00 00 	mov    DWORD PTR [esp+0x9c],ecx
  4fdb56:	8a 8c 24 44 01 00 00 	mov    cl,BYTE PTR [esp+0x144]
  4fdb5d:	89 9c 24 a8 00 00 00 	mov    DWORD PTR [esp+0xa8],ebx
  4fdb64:	f6 c1 0f             	test   cl,0xf
  4fdb67:	c7 84 24 ac 00 00 00 06 00 00 00 	mov    DWORD PTR [esp+0xac],0x6
  4fdb72:	89 9c 24 84 00 00 00 	mov    DWORD PTR [esp+0x84],ebx
  4fdb79:	c7 84 24 88 00 00 00 07 00 00 00 	mov    DWORD PTR [esp+0x88],0x7
  4fdb84:	89 84 24 8c 00 00 00 	mov    DWORD PTR [esp+0x8c],eax
  4fdb8b:	89 9c 24 94 00 00 00 	mov    DWORD PTR [esp+0x94],ebx
  4fdb92:	c7 84 24 98 00 00 00 0a 00 00 00 	mov    DWORD PTR [esp+0x98],0xa
  4fdb9d:	89 5c 24 44          	mov    DWORD PTR [esp+0x44],ebx
  4fdba1:	c7 44 24 48 0c 00 00 00 	mov    DWORD PTR [esp+0x48],0xc
  4fdba9:	c7 44 24 4c 14 00 00 00 	mov    DWORD PTR [esp+0x4c],0x14
  4fdbb1:	89 5c 24 64          	mov    DWORD PTR [esp+0x64],ebx
  4fdbb5:	c7 44 24 68 0b 00 00 00 	mov    DWORD PTR [esp+0x68],0xb
  4fdbbd:	89 44 24 6c          	mov    DWORD PTR [esp+0x6c],eax
  4fdbc1:	89 5c 24 74          	mov    DWORD PTR [esp+0x74],ebx
  4fdbc5:	c7 44 24 78 0d 00 00 00 	mov    DWORD PTR [esp+0x78],0xd
  4fdbcd:	89 44 24 7c          	mov    DWORD PTR [esp+0x7c],eax
  4fdbd1:	74 06                	je     0x4fdbd9
  4fdbd3:	89 35 04 1f 6e 00    	mov    DWORD PTR ds:0x6e1f04,esi
  4fdbd9:	a1 e8 03 69 00       	mov    eax,ds:0x6903e8
  4fdbde:	2b c3                	sub    eax,ebx
  4fdbe0:	0f 84 54 15 00 00    	je     0x4ff13a
  4fdbe6:	48                   	dec    eax
  4fdbe7:	0f 84 bb 03 00 00    	je     0x4fdfa8
  4fdbed:	48                   	dec    eax
  4fdbee:	0f 85 c9 15 00 00    	jne    0x4ff1bd
  4fdbf4:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4fdbf9:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4fdbfe:	50                   	push   eax
  4fdbff:	e8 6c 8c f6 ff       	call   0x466870
  4fdc04:	85 c0                	test   eax,eax
  4fdc06:	0f 84 b1 15 00 00    	je     0x4ff1bd
  4fdc0c:	f6 84 24 44 01 00 00 04 	test   BYTE PTR [esp+0x144],0x4
  4fdc14:	0f 84 49 02 00 00    	je     0x4fde63
  4fdc1a:	83 3d c8 04 5e 00 05 	cmp    DWORD PTR ds:0x5e04c8,0x5
  4fdc21:	74 33                	je     0x4fdc56
  4fdc23:	a1 64 bb 6c 00       	mov    eax,ds:0x6cbb64
  4fdc28:	8b 0d 84 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5484
  4fdc2e:	8b 15 88 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5488
  4fdc34:	53                   	push   ebx
  4fdc35:	53                   	push   ebx
  4fdc36:	53                   	push   ebx
  4fdc37:	50                   	push   eax
  4fdc38:	51                   	push   ecx
  4fdc39:	52                   	push   edx
  4fdc3a:	68 f3 03 00 00       	push   0x3f3
  4fdc3f:	b9 78 81 65 00       	mov    ecx,0x658178
  4fdc44:	e8 c7 36 07 00       	call   0x571310
  4fdc49:	5f                   	pop    edi
  4fdc4a:	5e                   	pop    esi
  4fdc4b:	5d                   	pop    ebp
  4fdc4c:	5b                   	pop    ebx
  4fdc4d:	81 c4 30 01 00 00    	add    esp,0x130
  4fdc53:	c2 2c 00             	ret    0x2c
  4fdc56:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4fdc5b:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4fdc60:	50                   	push   eax
  4fdc61:	e8 0a 8c f6 ff       	call   0x466870
  4fdc66:	8b 88 e8 05 00 00    	mov    ecx,DWORD PTR [eax+0x5e8]
  4fdc6c:	83 f9 ff             	cmp    ecx,0xffffffff
  4fdc6f:	75 04                	jne    0x4fdc75
  4fdc71:	0b c1                	or     eax,ecx
  4fdc73:	eb 07                	jmp    0x4fdc7c
  4fdc75:	8b 84 88 70 05 00 00 	mov    eax,DWORD PTR [eax+ecx*4+0x570]
  4fdc7c:	8b 0d 90 3a 6f 00    	mov    ecx,DWORD PTR ds:0x6f3a90
  4fdc82:	89 84 24 bc 00 00 00 	mov    DWORD PTR [esp+0xbc],eax
  4fdc89:	3b cb                	cmp    ecx,ebx
  4fdc8b:	74 1e                	je     0x4fdcab
  4fdc8d:	6a ff                	push   0xffffffff
  4fdc8f:	50                   	push   eax
  4fdc90:	6a 02                	push   0x2
  4fdc92:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fdc97:	e8 14 e5 06 00       	call   0x56c1b0
  4fdc9c:	85 c0                	test   eax,eax
  4fdc9e:	0f 85 19 15 00 00    	jne    0x4ff1bd
  4fdca4:	8b 84 24 bc 00 00 00 	mov    eax,DWORD PTR [esp+0xbc]
  4fdcab:	8d 0c 80             	lea    ecx,[eax+eax*4]
  4fdcae:	8d 0c c9             	lea    ecx,[ecx+ecx*8]
  4fdcb1:	c1 e1 04             	shl    ecx,0x4
  4fdcb4:	8a 84 01 bc 13 6b 00 	mov    al,BYTE PTR [ecx+eax*1+0x6b13bc]
  4fdcbb:	8b 0d 34 51 6a 00    	mov    ecx,DWORD PTR ds:0x6a5134
  4fdcc1:	83 f9 02             	cmp    ecx,0x2
  4fdcc4:	75 17                	jne    0x4fdcdd
  4fdcc6:	a1 00 66 65 00       	mov    eax,ds:0x656600
  4fdccb:	3b c3                	cmp    eax,ebx
  4fdccd:	74 57                	je     0x4fdd26
  4fdccf:	8b 10                	mov    edx,DWORD PTR [eax]
  4fdcd1:	89 94 24 dc 00 00 00 	mov    DWORD PTR [esp+0xdc],edx
  4fdcd8:	e9 0c 01 00 00       	jmp    0x4fdde9
  4fdcdd:	3b ce                	cmp    ecx,esi
  4fdcdf:	75 45                	jne    0x4fdd26
  4fdce1:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fdce7:	3b cb                	cmp    ecx,ebx
  4fdce9:	74 3b                	je     0x4fdd26
  4fdceb:	3c 04                	cmp    al,0x4
  4fdced:	74 10                	je     0x4fdcff
  4fdcef:	3c 08                	cmp    al,0x8
  4fdcf1:	74 0c                	je     0x4fdcff
  4fdcf3:	3c 0c                	cmp    al,0xc
  4fdcf5:	74 08                	je     0x4fdcff
  4fdcf7:	3c 0e                	cmp    al,0xe
  4fdcf9:	74 04                	je     0x4fdcff
  4fdcfb:	3c 0a                	cmp    al,0xa
  4fdcfd:	75 27                	jne    0x4fdd26
  4fdcff:	8b 09                	mov    ecx,DWORD PTR [ecx]
  4fdd01:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4fdd06:	89 8c 24 dc 00 00 00 	mov    DWORD PTR [esp+0xdc],ecx
  4fdd0d:	50                   	push   eax
  4fdd0e:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4fdd13:	e8 58 8b f6 ff       	call   0x466870
  4fdd18:	8b 10                	mov    edx,DWORD PTR [eax]
  4fdd1a:	89 94 24 d8 00 00 00 	mov    DWORD PTR [esp+0xd8],edx
  4fdd21:	e9 dc 00 00 00       	jmp    0x4fde02
  4fdd26:	8b 35 88 54 6c 00    	mov    esi,DWORD PTR ds:0x6c5488
  4fdd2c:	3b f3                	cmp    esi,ebx
  4fdd2e:	0f 8c 89 14 00 00    	jl     0x4ff1bd
  4fdd34:	3b 35 94 54 6c 00    	cmp    esi,DWORD PTR ds:0x6c5494
  4fdd3a:	0f 8d 7d 14 00 00    	jge    0x4ff1bd
  4fdd40:	8b 15 84 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5484
  4fdd46:	3b d3                	cmp    edx,ebx
  4fdd48:	0f 8c 6f 14 00 00    	jl     0x4ff1bd
  4fdd4e:	3b 15 98 54 6c 00    	cmp    edx,DWORD PTR ds:0x6c5498
  4fdd54:	0f 8d 63 14 00 00    	jge    0x4ff1bd
  4fdd5a:	8b 0d 64 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb64
  4fdd60:	3b cb                	cmp    ecx,ebx
  4fdd62:	0f 8c 55 14 00 00    	jl     0x4ff1bd
  4fdd68:	3b 0d 9c 54 6c 00    	cmp    ecx,DWORD PTR ds:0x6c549c
  4fdd6e:	0f 8d 49 14 00 00    	jge    0x4ff1bd
  4fdd74:	8b 04 95 42 b9 6c 00 	mov    eax,DWORD PTR [edx*4+0x6cb942]
  4fdd7b:	8b 2c 8d c2 b8 6c 00 	mov    ebp,DWORD PTR [ecx*4+0x6cb8c2]
  4fdd82:	03 c5                	add    eax,ebp
  4fdd84:	8b 2d dc 54 6c 00    	mov    ebp,DWORD PTR ds:0x6c54dc
  4fdd8a:	03 c6                	add    eax,esi
  4fdd8c:	c1 e6 05             	shl    esi,0x5
  4fdd8f:	8d 3c 40             	lea    edi,[eax+eax*2]
  4fdd92:	33 c0                	xor    eax,eax
  4fdd94:	c1 e2 05             	shl    edx,0x5
  4fdd97:	66 8b 44 bd 00       	mov    ax,WORD PTR [ebp+edi*4+0x0]
  4fdd9c:	c1 e1 04             	shl    ecx,0x4
  4fdd9f:	8d 3c 80             	lea    edi,[eax+eax*4]
  4fdda2:	8d 3c ff             	lea    edi,[edi+edi*8]
  4fdda5:	d1 e7                	shl    edi,1
  4fdda7:	2b f8                	sub    edi,eax
  4fdda9:	a1 0c 66 65 00       	mov    eax,ds:0x65660c
  4fddae:	8d 04 b8             	lea    eax,[eax+edi*4]
  4fddb1:	8b b8 b8 00 00 00    	mov    edi,DWORD PTR [eax+0xb8]
  4fddb7:	03 fe                	add    edi,esi
  4fddb9:	89 bc 24 cc 00 00 00 	mov    DWORD PTR [esp+0xcc],edi
  4fddc0:	8b b0 bc 00 00 00    	mov    esi,DWORD PTR [eax+0xbc]
  4fddc6:	03 f2                	add    esi,edx
  4fddc8:	89 b4 24 d0 00 00 00 	mov    DWORD PTR [esp+0xd0],esi
  4fddcf:	8b 90 c0 00 00 00    	mov    edx,DWORD PTR [eax+0xc0]
  4fddd5:	03 d1                	add    edx,ecx
  4fddd7:	c7 84 24 dc 00 00 00 ff ff ff ff 	mov    DWORD PTR [esp+0xdc],0xffffffff
  4fdde2:	89 94 24 d4 00 00 00 	mov    DWORD PTR [esp+0xd4],edx
  4fdde9:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4fddee:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4fddf3:	50                   	push   eax
  4fddf4:	e8 77 8a f6 ff       	call   0x466870
  4fddf9:	8b 00                	mov    eax,DWORD PTR [eax]
  4fddfb:	89 84 24 d8 00 00 00 	mov    DWORD PTR [esp+0xd8],eax
  4fde02:	8d 8c 24 bc 00 00 00 	lea    ecx,[esp+0xbc]
  4fde09:	89 9c 24 ec 00 00 00 	mov    DWORD PTR [esp+0xec],ebx
  4fde10:	51                   	push   ecx
  4fde11:	8d 8c 24 f0 00 00 00 	lea    ecx,[esp+0xf0]
  4fde18:	c7 84 24 f4 00 00 00 04 00 00 00 	mov    DWORD PTR [esp+0xf4],0x4
  4fde23:	c7 84 24 f8 00 00 00 21 00 00 00 	mov    DWORD PTR [esp+0xf8],0x21
  4fde2e:	e8 6d 61 03 00       	call   0x533fa0
  4fde33:	8d 94 24 ec 00 00 00 	lea    edx,[esp+0xec]
  4fde3a:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fde3f:	52                   	push   edx
  4fde40:	e8 8b 9d 03 00       	call   0x537bd0
  4fde45:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fde4b:	74 16                	je     0x4fde63
  4fde4d:	8b 84 24 bc 00 00 00 	mov    eax,DWORD PTR [esp+0xbc]
  4fde54:	6a ff                	push   0xffffffff
  4fde56:	50                   	push   eax
  4fde57:	6a 02                	push   0x2
  4fde59:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fde5e:	e8 fd e1 06 00       	call   0x56c060
  4fde63:	f6 84 24 44 01 00 00 01 	test   BYTE PTR [esp+0x144],0x1
  4fde6b:	0f 84 4c 13 00 00    	je     0x4ff1bd
  4fde71:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fde77:	3b cb                	cmp    ecx,ebx
  4fde79:	0f 84 ef 00 00 00    	je     0x4fdf6e
  4fde7f:	39 99 e4 00 00 00    	cmp    DWORD PTR [ecx+0xe4],ebx
  4fde85:	0f 84 ca 00 00 00    	je     0x4fdf55
  4fde8b:	39 9c 24 68 01 00 00 	cmp    DWORD PTR [esp+0x168],ebx
  4fde92:	74 46                	je     0x4fdeda
  4fde94:	39 59 68             	cmp    DWORD PTR [ecx+0x68],ebx
  4fde97:	74 07                	je     0x4fdea0
  4fde99:	e8 52 96 00 00       	call   0x5074f0
  4fde9e:	eb 05                	jmp    0x4fdea5
  4fdea0:	e8 2b 95 00 00       	call   0x5073d0
  4fdea5:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fdeab:	74 1b                	je     0x4fdec8
  4fdead:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fdeb3:	8b 91 a8 00 00 00    	mov    edx,DWORD PTR [ecx+0xa8]
  4fdeb9:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fdebe:	52                   	push   edx
  4fdebf:	6a ff                	push   0xffffffff
  4fdec1:	6a 03                	push   0x3
  4fdec3:	e8 98 e1 06 00       	call   0x56c060
  4fdec8:	89 1d 70 bb 6c 00    	mov    DWORD PTR ds:0x6cbb70,ebx
  4fdece:	c7 05 e8 03 69 00 01 00 00 00 	mov    DWORD PTR ds:0x6903e8,0x1
  4fded8:	eb 7b                	jmp    0x4fdf55
  4fdeda:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fdedf:	8b 35 58 ef 6d 00    	mov    esi,DWORD PTR ds:0x6def58
  4fdee5:	33 ff                	xor    edi,edi
  4fdee7:	3b c3                	cmp    eax,ebx
  4fdee9:	76 2c                	jbe    0x4fdf17
  4fdeeb:	39 5e 04             	cmp    DWORD PTR [esi+0x4],ebx
  4fdeee:	74 17                	je     0x4fdf07
  4fdef0:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fdef3:	75 05                	jne    0x4fdefa
  4fdef5:	39 5e 70             	cmp    DWORD PTR [esi+0x70],ebx
  4fdef8:	74 0d                	je     0x4fdf07
  4fdefa:	8b ce                	mov    ecx,esi
  4fdefc:	e8 ef 95 00 00       	call   0x5074f0
  4fdf01:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fdf07:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fdf0c:	47                   	inc    edi
  4fdf0d:	81 c6 4b 0e 00 00    	add    esi,0xe4b
  4fdf13:	3b f8                	cmp    edi,eax
  4fdf15:	72 d4                	jb     0x4fdeeb
  4fdf17:	89 1d 7c f1 6d 00    	mov    DWORD PTR ds:0x6df17c,ebx
  4fdf1d:	e8 ae 94 00 00       	call   0x5073d0
  4fdf22:	a1 90 3a 6f 00       	mov    eax,ds:0x6f3a90
  4fdf27:	89 1d 70 bb 6c 00    	mov    DWORD PTR ds:0x6cbb70,ebx
  4fdf2d:	3b c3                	cmp    eax,ebx
  4fdf2f:	c7 05 e8 03 69 00 01 00 00 00 	mov    DWORD PTR ds:0x6903e8,0x1
  4fdf39:	74 1a                	je     0x4fdf55
  4fdf3b:	a1 00 66 65 00       	mov    eax,ds:0x656600
  4fdf40:	8b 88 a8 00 00 00    	mov    ecx,DWORD PTR [eax+0xa8]
  4fdf46:	51                   	push   ecx
  4fdf47:	6a ff                	push   0xffffffff
  4fdf49:	6a 03                	push   0x3
  4fdf4b:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fdf50:	e8 0b e1 06 00       	call   0x56c060
  4fdf55:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4fdf5a:	6a ff                	push   0xffffffff
  4fdf5c:	50                   	push   eax
  4fdf5d:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4fdf62:	e8 09 89 f6 ff       	call   0x466870
  4fdf67:	8b c8                	mov    ecx,eax
  4fdf69:	e8 62 bc 00 00       	call   0x509bd0
  4fdf6e:	38 1d d4 e6 6d 00    	cmp    BYTE PTR ds:0x6de6d4,bl
  4fdf74:	75 0c                	jne    0x4fdf82
  4fdf76:	39 1d 0c 1f 6e 00    	cmp    DWORD PTR ds:0x6e1f0c,ebx
  4fdf7c:	0f 84 3b 12 00 00    	je     0x4ff1bd
  4fdf82:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4fdf87:	6a ff                	push   0xffffffff
  4fdf89:	50                   	push   eax
  4fdf8a:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4fdf8f:	e8 dc 88 f6 ff       	call   0x466870
  4fdf94:	8b c8                	mov    ecx,eax
  4fdf96:	e8 35 bc 00 00       	call   0x509bd0
  4fdf9b:	5f                   	pop    edi
  4fdf9c:	5e                   	pop    esi
  4fdf9d:	5d                   	pop    ebp
  4fdf9e:	5b                   	pop    ebx
  4fdf9f:	81 c4 30 01 00 00    	add    esp,0x130
  4fdfa5:	c2 2c 00             	ret    0x2c
  4fdfa8:	f6 c1 01             	test   cl,0x1
  4fdfab:	0f 84 e3 01 00 00    	je     0x4fe194
  4fdfb1:	a1 34 51 6a 00       	mov    eax,ds:0x6a5134
  4fdfb6:	48                   	dec    eax
  4fdfb7:	0f 84 84 01 00 00    	je     0x4fe141
  4fdfbd:	48                   	dec    eax
  4fdfbe:	74 0c                	je     0x4fdfcc
  4fdfc0:	48                   	dec    eax
  4fdfc1:	0f 84 7a 01 00 00    	je     0x4fe141
  4fdfc7:	e9 c8 01 00 00       	jmp    0x4fe194
  4fdfcc:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fdfd2:	39 99 e4 00 00 00    	cmp    DWORD PTR [ecx+0xe4],ebx
  4fdfd8:	0f 84 b6 01 00 00    	je     0x4fe194
  4fdfde:	8b 84 24 64 01 00 00 	mov    eax,DWORD PTR [esp+0x164]
  4fdfe5:	3b c3                	cmp    eax,ebx
  4fdfe7:	8b 41 68             	mov    eax,DWORD PTR [ecx+0x68]
  4fdfea:	74 4a                	je     0x4fe036
  4fdfec:	3b c3                	cmp    eax,ebx
  4fdfee:	74 0a                	je     0x4fdffa
  4fdff0:	e8 fb 94 00 00       	call   0x5074f0
  4fdff5:	e9 94 01 00 00       	jmp    0x4fe18e
  4fdffa:	e8 d1 93 00 00       	call   0x5073d0
  4fdfff:	a1 90 3a 6f 00       	mov    eax,ds:0x6f3a90
  4fe004:	c7 05 7c f1 6d 00 01 00 00 00 	mov    DWORD PTR ds:0x6df17c,0x1
  4fe00e:	3b c3                	cmp    eax,ebx
  4fe010:	0f 84 78 01 00 00    	je     0x4fe18e
  4fe016:	8b 15 00 66 65 00    	mov    edx,DWORD PTR ds:0x656600
  4fe01c:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fe021:	8b 82 a8 00 00 00    	mov    eax,DWORD PTR [edx+0xa8]
  4fe027:	50                   	push   eax
  4fe028:	6a ff                	push   0xffffffff
  4fe02a:	6a 03                	push   0x3
  4fe02c:	e8 2f e0 06 00       	call   0x56c060
  4fe031:	e9 58 01 00 00       	jmp    0x4fe18e
  4fe036:	3b c3                	cmp    eax,ebx
  4fe038:	0f 84 8c 00 00 00    	je     0x4fe0ca
  4fe03e:	39 1d 7c f1 6d 00    	cmp    DWORD PTR ds:0x6df17c,ebx
  4fe044:	74 74                	je     0x4fe0ba
  4fe046:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fe04b:	8b 35 58 ef 6d 00    	mov    esi,DWORD PTR ds:0x6def58
  4fe051:	33 ff                	xor    edi,edi
  4fe053:	3b c3                	cmp    eax,ebx
  4fe055:	76 2c                	jbe    0x4fe083
  4fe057:	39 5e 04             	cmp    DWORD PTR [esi+0x4],ebx
  4fe05a:	74 11                	je     0x4fe06d
  4fe05c:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fe05f:	75 05                	jne    0x4fe066
  4fe061:	39 5e 70             	cmp    DWORD PTR [esi+0x70],ebx
  4fe064:	74 07                	je     0x4fe06d
  4fe066:	8b ce                	mov    ecx,esi
  4fe068:	e8 83 94 00 00       	call   0x5074f0
  4fe06d:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fe072:	47                   	inc    edi
  4fe073:	81 c6 4b 0e 00 00    	add    esi,0xe4b
  4fe079:	3b f8                	cmp    edi,eax
  4fe07b:	72 da                	jb     0x4fe057
  4fe07d:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fe083:	89 1d 7c f1 6d 00    	mov    DWORD PTR ds:0x6df17c,ebx
  4fe089:	e8 42 93 00 00       	call   0x5073d0
  4fe08e:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fe094:	0f 84 fa 00 00 00    	je     0x4fe194
  4fe09a:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fe0a0:	8b 91 a8 00 00 00    	mov    edx,DWORD PTR [ecx+0xa8]
  4fe0a6:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fe0ab:	52                   	push   edx
  4fe0ac:	6a ff                	push   0xffffffff
  4fe0ae:	6a 03                	push   0x3
  4fe0b0:	e8 ab df 06 00       	call   0x56c060
  4fe0b5:	e9 da 00 00 00       	jmp    0x4fe194
  4fe0ba:	51                   	push   ecx
  4fe0bb:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe0c0:	e8 db cc 02 00       	call   0x52ada0
  4fe0c5:	e9 ca 00 00 00       	jmp    0x4fe194
  4fe0ca:	51                   	push   ecx
  4fe0cb:	b9 78 81 65 00       	mov    ecx,0x658178
  4fe0d0:	e8 cb 2c 07 00       	call   0x570da0
  4fe0d5:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fe0da:	8b 35 58 ef 6d 00    	mov    esi,DWORD PTR ds:0x6def58
  4fe0e0:	33 ff                	xor    edi,edi
  4fe0e2:	3b c3                	cmp    eax,ebx
  4fe0e4:	76 26                	jbe    0x4fe10c
  4fe0e6:	39 5e 04             	cmp    DWORD PTR [esi+0x4],ebx
  4fe0e9:	74 11                	je     0x4fe0fc
  4fe0eb:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fe0ee:	75 05                	jne    0x4fe0f5
  4fe0f0:	39 5e 70             	cmp    DWORD PTR [esi+0x70],ebx
  4fe0f3:	74 07                	je     0x4fe0fc
  4fe0f5:	8b ce                	mov    ecx,esi
  4fe0f7:	e8 f4 93 00 00       	call   0x5074f0
  4fe0fc:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fe101:	47                   	inc    edi
  4fe102:	81 c6 4b 0e 00 00    	add    esi,0xe4b
  4fe108:	3b f8                	cmp    edi,eax
  4fe10a:	72 da                	jb     0x4fe0e6
  4fe10c:	8b 0d 00 66 65 00    	mov    ecx,DWORD PTR ds:0x656600
  4fe112:	89 1d 7c f1 6d 00    	mov    DWORD PTR ds:0x6df17c,ebx
  4fe118:	e8 b3 92 00 00       	call   0x5073d0
  4fe11d:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fe123:	74 69                	je     0x4fe18e
  4fe125:	a1 00 66 65 00       	mov    eax,ds:0x656600
  4fe12a:	8b 88 a8 00 00 00    	mov    ecx,DWORD PTR [eax+0xa8]
  4fe130:	51                   	push   ecx
  4fe131:	6a ff                	push   0xffffffff
  4fe133:	6a 03                	push   0x3
  4fe135:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fe13a:	e8 21 df 06 00       	call   0x56c060
  4fe13f:	eb 4d                	jmp    0x4fe18e
  4fe141:	38 1d d4 e6 6d 00    	cmp    BYTE PTR ds:0x6de6d4,bl
  4fe147:	75 08                	jne    0x4fe151
  4fe149:	39 1d 0c 1f 6e 00    	cmp    DWORD PTR ds:0x6e1f0c,ebx
  4fe14f:	74 3d                	je     0x4fe18e
  4fe151:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fe156:	8b 35 58 ef 6d 00    	mov    esi,DWORD PTR ds:0x6def58
  4fe15c:	33 ff                	xor    edi,edi
  4fe15e:	3b c3                	cmp    eax,ebx
  4fe160:	76 26                	jbe    0x4fe188
  4fe162:	39 5e 04             	cmp    DWORD PTR [esi+0x4],ebx
  4fe165:	74 11                	je     0x4fe178
  4fe167:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fe16a:	75 05                	jne    0x4fe171
  4fe16c:	39 5e 70             	cmp    DWORD PTR [esi+0x70],ebx
  4fe16f:	74 07                	je     0x4fe178
  4fe171:	8b ce                	mov    ecx,esi
  4fe173:	e8 78 93 00 00       	call   0x5074f0
  4fe178:	a1 80 f1 6d 00       	mov    eax,ds:0x6df180
  4fe17d:	47                   	inc    edi
  4fe17e:	81 c6 4b 0e 00 00    	add    esi,0xe4b
  4fe184:	3b f8                	cmp    edi,eax
  4fe186:	72 da                	jb     0x4fe162
  4fe188:	89 1d 7c f1 6d 00    	mov    DWORD PTR ds:0x6df17c,ebx
  4fe18e:	89 1d 70 bb 6c 00    	mov    DWORD PTR ds:0x6cbb70,ebx
  4fe194:	f6 84 24 44 01 00 00 04 	test   BYTE PTR [esp+0x144],0x4
  4fe19c:	0f 84 1b 10 00 00    	je     0x4ff1bd
  4fe1a2:	a1 34 51 6a 00       	mov    eax,ds:0x6a5134
  4fe1a7:	48                   	dec    eax
  4fe1a8:	0f 84 d9 06 00 00    	je     0x4fe887
  4fe1ae:	48                   	dec    eax
  4fe1af:	74 14                	je     0x4fe1c5
  4fe1b1:	48                   	dec    eax
  4fe1b2:	0f 84 cf 06 00 00    	je     0x4fe887
  4fe1b8:	5f                   	pop    edi
  4fe1b9:	5e                   	pop    esi
  4fe1ba:	5d                   	pop    ebp
  4fe1bb:	5b                   	pop    ebx
  4fe1bc:	81 c4 30 01 00 00    	add    esp,0x130
  4fe1c2:	c2 2c 00             	ret    0x2c
  4fe1c5:	a1 c8 04 5e 00       	mov    eax,ds:0x5e04c8
  4fe1ca:	3b c3                	cmp    eax,ebx
  4fe1cc:	0f 85 23 01 00 00    	jne    0x4fe2f5
  4fe1d2:	8b 15 00 66 65 00    	mov    edx,DWORD PTR ds:0x656600
  4fe1d8:	39 9a e4 00 00 00    	cmp    DWORD PTR [edx+0xe4],ebx
  4fe1de:	0f 84 d9 0f 00 00    	je     0x4ff1bd
  4fe1e4:	b8 20 62 50 00       	mov    eax,0x506220
  4fe1e9:	33 c9                	xor    ecx,ecx
  4fe1eb:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4fe1f0:	a1 58 ef 6d 00       	mov    eax,ds:0x6def58
  4fe1f5:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4fe1fb:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe200:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4fe206:	a3 64 ef 6d 00       	mov    ds:0x6def64,eax
  4fe20b:	e8 00 c5 02 00       	call   0x52a710
  4fe210:	8b f0                	mov    esi,eax
  4fe212:	3b f3                	cmp    esi,ebx
  4fe214:	0f 84 a3 0f 00 00    	je     0x4ff1bd
  4fe21a:	8b 3d 04 52 5c 00    	mov    edi,DWORD PTR ds:0x5c5204
  4fe220:	bd 0e 00 00 00       	mov    ebp,0xe
  4fe225:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fe228:	0f 84 a6 00 00 00    	je     0x4fe2d4
  4fe22e:	6a 10                	push   0x10
  4fe230:	ff d7                	call   edi
  4fe232:	66 85 c0             	test   ax,ax
  4fe235:	7d 33                	jge    0x4fe26a
  4fe237:	8b 15 00 66 65 00    	mov    edx,DWORD PTR ds:0x656600
  4fe23d:	89 5c 24 64          	mov    DWORD PTR [esp+0x64],ebx
  4fe241:	c7 44 24 68 0b 00 00 00 	mov    DWORD PTR [esp+0x68],0xb
  4fe249:	89 6c 24 6c          	mov    DWORD PTR [esp+0x6c],ebp
  4fe24d:	8a 0e                	mov    cl,BYTE PTR [esi]
  4fe24f:	88 4c 24 70          	mov    BYTE PTR [esp+0x70],cl
  4fe253:	8a 02                	mov    al,BYTE PTR [edx]
  4fe255:	8d 4c 24 64          	lea    ecx,[esp+0x64]
  4fe259:	88 44 24 71          	mov    BYTE PTR [esp+0x71],al
  4fe25d:	51                   	push   ecx
  4fe25e:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe263:	e8 68 99 03 00       	call   0x537bd0
  4fe268:	eb 6a                	jmp    0x4fe2d4
  4fe26a:	a1 00 66 65 00       	mov    eax,ds:0x656600
  4fe26f:	89 9c 24 84 00 00 00 	mov    DWORD PTR [esp+0x84],ebx
  4fe276:	c7 84 24 88 00 00 00 07 00 00 00 	mov    DWORD PTR [esp+0x88],0x7
  4fe281:	89 ac 24 8c 00 00 00 	mov    DWORD PTR [esp+0x8c],ebp
  4fe288:	8a 16                	mov    dl,BYTE PTR [esi]
  4fe28a:	88 94 24 90 00 00 00 	mov    BYTE PTR [esp+0x90],dl
  4fe291:	8a 08                	mov    cl,BYTE PTR [eax]
  4fe293:	8d 94 24 84 00 00 00 	lea    edx,[esp+0x84]
  4fe29a:	88 8c 24 91 00 00 00 	mov    BYTE PTR [esp+0x91],cl
  4fe2a1:	52                   	push   edx
  4fe2a2:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe2a7:	e8 24 99 03 00       	call   0x537bd0
  4fe2ac:	56                   	push   esi
  4fe2ad:	b9 78 81 65 00       	mov    ecx,0x658178
  4fe2b2:	e8 59 2b 07 00       	call   0x570e10
  4fe2b7:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fe2bd:	74 15                	je     0x4fe2d4
  4fe2bf:	8b 86 a8 00 00 00    	mov    eax,DWORD PTR [esi+0xa8]
  4fe2c5:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fe2ca:	50                   	push   eax
  4fe2cb:	6a ff                	push   0xffffffff
  4fe2cd:	6a 06                	push   0x6
  4fe2cf:	e8 8c dd 06 00       	call   0x56c060
  4fe2d4:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe2d9:	e8 32 c4 02 00       	call   0x52a710
  4fe2de:	8b f0                	mov    esi,eax
  4fe2e0:	3b f3                	cmp    esi,ebx
  4fe2e2:	0f 85 3d ff ff ff    	jne    0x4fe225
  4fe2e8:	5f                   	pop    edi
  4fe2e9:	5e                   	pop    esi
  4fe2ea:	5d                   	pop    ebp
  4fe2eb:	5b                   	pop    ebx
  4fe2ec:	81 c4 30 01 00 00    	add    esp,0x130
  4fe2f2:	c2 2c 00             	ret    0x2c
  4fe2f5:	83 f8 15             	cmp    eax,0x15
  4fe2f8:	0f 85 92 00 00 00    	jne    0x4fe390
  4fe2fe:	33 c9                	xor    ecx,ecx
  4fe300:	b8 20 62 50 00       	mov    eax,0x506220
  4fe305:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4fe30b:	8b 0d 58 ef 6d 00    	mov    ecx,DWORD PTR ds:0x6def58
  4fe311:	89 0d 64 ef 6d 00    	mov    DWORD PTR ds:0x6def64,ecx
  4fe317:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe31c:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4fe322:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4fe327:	e8 e4 c3 02 00       	call   0x52a710
  4fe32c:	3b c3                	cmp    eax,ebx
  4fe32e:	0f 84 89 0e 00 00    	je     0x4ff1bd
  4fe334:	bf 0e 00 00 00       	mov    edi,0xe
  4fe339:	be 0d 00 00 00       	mov    esi,0xd
  4fe33e:	39 58 68             	cmp    DWORD PTR [eax+0x68],ebx
  4fe341:	74 32                	je     0x4fe375
  4fe343:	89 5c 24 74          	mov    DWORD PTR [esp+0x74],ebx
  4fe347:	89 74 24 78          	mov    DWORD PTR [esp+0x78],esi
  4fe34b:	89 7c 24 7c          	mov    DWORD PTR [esp+0x7c],edi
  4fe34f:	8a 10                	mov    dl,BYTE PTR [eax]
  4fe351:	a1 00 66 65 00       	mov    eax,ds:0x656600
  4fe356:	88 94 24 80 00 00 00 	mov    BYTE PTR [esp+0x80],dl
  4fe35d:	8d 54 24 74          	lea    edx,[esp+0x74]
  4fe361:	8a 08                	mov    cl,BYTE PTR [eax]
  4fe363:	52                   	push   edx
  4fe364:	88 8c 24 85 00 00 00 	mov    BYTE PTR [esp+0x85],cl
  4fe36b:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe370:	e8 5b 98 03 00       	call   0x537bd0
  4fe375:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe37a:	e8 91 c3 02 00       	call   0x52a710
  4fe37f:	3b c3                	cmp    eax,ebx
  4fe381:	75 bb                	jne    0x4fe33e
  4fe383:	5f                   	pop    edi
  4fe384:	5e                   	pop    esi
  4fe385:	5d                   	pop    ebp
  4fe386:	5b                   	pop    ebx
  4fe387:	81 c4 30 01 00 00    	add    esp,0x130
  4fe38d:	c2 2c 00             	ret    0x2c
  4fe390:	8b 15 88 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5488
  4fe396:	3b d3                	cmp    edx,ebx
  4fe398:	0f 8c 1f 0e 00 00    	jl     0x4ff1bd
  4fe39e:	3b 15 94 54 6c 00    	cmp    edx,DWORD PTR ds:0x6c5494
  4fe3a4:	0f 8d 13 0e 00 00    	jge    0x4ff1bd
  4fe3aa:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fe3af:	3b c3                	cmp    eax,ebx
  4fe3b1:	0f 8c 06 0e 00 00    	jl     0x4ff1bd
  4fe3b7:	3b 05 98 54 6c 00    	cmp    eax,DWORD PTR ds:0x6c5498
  4fe3bd:	0f 8d fa 0d 00 00    	jge    0x4ff1bd
  4fe3c3:	8b 0d 64 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb64
  4fe3c9:	3b cb                	cmp    ecx,ebx
  4fe3cb:	0f 8c ec 0d 00 00    	jl     0x4ff1bd
  4fe3d1:	3b 0d 9c 54 6c 00    	cmp    ecx,DWORD PTR ds:0x6c549c
  4fe3d7:	0f 8d e0 0d 00 00    	jge    0x4ff1bd
  4fe3dd:	8b 04 85 42 b9 6c 00 	mov    eax,DWORD PTR [eax*4+0x6cb942]
  4fe3e4:	8b 34 8d c2 b8 6c 00 	mov    esi,DWORD PTR [ecx*4+0x6cb8c2]
  4fe3eb:	8b 0d dc 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c54dc
  4fe3f1:	03 c6                	add    eax,esi
  4fe3f3:	03 c2                	add    eax,edx
  4fe3f5:	8d 04 40             	lea    eax,[eax+eax*2]
  4fe3f8:	8d 0c 81             	lea    ecx,[ecx+eax*4]
  4fe3fb:	33 c0                	xor    eax,eax
  4fe3fd:	89 4d 79             	mov    DWORD PTR [ebp+0x79],ecx
  4fe400:	66 8b 01             	mov    ax,WORD PTR [ecx]
  4fe403:	8d 0c 80             	lea    ecx,[eax+eax*4]
  4fe406:	8d 14 c9             	lea    edx,[ecx+ecx*8]
  4fe409:	d1 e2                	shl    edx,1
  4fe40b:	2b d0                	sub    edx,eax
  4fe40d:	a1 0c 66 65 00       	mov    eax,ds:0x65660c
  4fe412:	8b 8c 90 94 00 00 00 	mov    ecx,DWORD PTR [eax+edx*4+0x94]
  4fe419:	a1 64 bb 6c 00       	mov    eax,ds:0x6cbb64
  4fe41e:	83 f9 10             	cmp    ecx,0x10
  4fe421:	8d 48 01             	lea    ecx,[eax+0x1]
  4fe424:	74 02                	je     0x4fe428
  4fe426:	8b c8                	mov    ecx,eax
  4fe428:	3b c8                	cmp    ecx,eax
  4fe42a:	89 4c 24 28          	mov    DWORD PTR [esp+0x28],ecx
  4fe42e:	74 46                	je     0x4fe476
  4fe430:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fe435:	3b c3                	cmp    eax,ebx
  4fe437:	0f 8c 80 0d 00 00    	jl     0x4ff1bd
  4fe43d:	3b 05 94 54 6c 00    	cmp    eax,DWORD PTR ds:0x6c5494
  4fe443:	0f 8d 74 0d 00 00    	jge    0x4ff1bd
  4fe449:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fe44e:	3b c3                	cmp    eax,ebx
  4fe450:	0f 8c 67 0d 00 00    	jl     0x4ff1bd
  4fe456:	3b 05 98 54 6c 00    	cmp    eax,DWORD PTR ds:0x6c5498
  4fe45c:	0f 8d 5b 0d 00 00    	jge    0x4ff1bd
  4fe462:	3b cb                	cmp    ecx,ebx
  4fe464:	0f 8c 53 0d 00 00    	jl     0x4ff1bd
  4fe46a:	3b 0d 9c 54 6c 00    	cmp    ecx,DWORD PTR ds:0x6c549c
  4fe470:	0f 8d 47 0d 00 00    	jge    0x4ff1bd
  4fe476:	33 c9                	xor    ecx,ecx
  4fe478:	b8 01 00 00 00       	mov    eax,0x1
  4fe47d:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4fe483:	8b 0d 58 ef 6d 00    	mov    ecx,DWORD PTR ds:0x6def58
  4fe489:	89 44 24 58          	mov    DWORD PTR [esp+0x58],eax
  4fe48d:	89 44 24 60          	mov    DWORD PTR [esp+0x60],eax
  4fe491:	b8 20 62 50 00       	mov    eax,0x506220
  4fe496:	89 0d 64 ef 6d 00    	mov    DWORD PTR ds:0x6def64,ecx
  4fe49c:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe4a1:	89 5c 24 5c          	mov    DWORD PTR [esp+0x5c],ebx
  4fe4a5:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4fe4ab:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4fe4b0:	e8 5b c2 02 00       	call   0x52a710
  4fe4b5:	8b f0                	mov    esi,eax
  4fe4b7:	3b f3                	cmp    esi,ebx
  4fe4b9:	0f 84 fe 0c 00 00    	je     0x4ff1bd
  4fe4bf:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fe4c2:	0f 84 3e 03 00 00    	je     0x4fe806
  4fe4c8:	8b 96 6b 09 00 00    	mov    edx,DWORD PTR [esi+0x96b]
  4fe4ce:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fe4d3:	3b d0                	cmp    edx,eax
  4fe4d5:	75 26                	jne    0x4fe4fd
  4fe4d7:	8b 86 6f 09 00 00    	mov    eax,DWORD PTR [esi+0x96f]
  4fe4dd:	8b 0d 84 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5484
  4fe4e3:	3b c1                	cmp    eax,ecx
  4fe4e5:	75 16                	jne    0x4fe4fd
  4fe4e7:	8b 4c 24 28          	mov    ecx,DWORD PTR [esp+0x28]
  4fe4eb:	8b 86 73 09 00 00    	mov    eax,DWORD PTR [esi+0x973]
  4fe4f1:	3b c1                	cmp    eax,ecx
  4fe4f3:	75 08                	jne    0x4fe4fd
  4fe4f5:	c7 44 24 5c 01 00 00 00 	mov    DWORD PTR [esp+0x5c],0x1
  4fe4fd:	8b 86 a8 00 00 00    	mov    eax,DWORD PTR [esi+0xa8]
  4fe503:	8d 14 c0             	lea    edx,[eax+eax*8]
  4fe506:	8d 0c 90             	lea    ecx,[eax+edx*4]
  4fe509:	8d 0c 89             	lea    ecx,[ecx+ecx*4]
  4fe50c:	8d ac c8 80 5f 6a 00 	lea    ebp,[eax+ecx*8+0x6a5f80]
  4fe513:	8b 44 24 5c          	mov    eax,DWORD PTR [esp+0x5c]
  4fe517:	3b c3                	cmp    eax,ebx
  4fe519:	0f 84 dc 01 00 00    	je     0x4fe6fb
  4fe51f:	39 9e 04 01 00 00    	cmp    DWORD PTR [esi+0x104],ebx
  4fe525:	0f 84 d0 01 00 00    	je     0x4fe6fb
  4fe52b:	6a 10                	push   0x10
  4fe52d:	ff 15 04 52 5c 00    	call   DWORD PTR ds:0x5c5204
  4fe533:	66 85 c0             	test   ax,ax
  4fe536:	0f 8c eb 02 00 00    	jl     0x4fe827
  4fe53c:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fe541:	8d 4c 24 20          	lea    ecx,[esp+0x20]
  4fe545:	50                   	push   eax
  4fe546:	e8 05 01 f1 ff       	call   0x40e650
  4fe54b:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4fe54f:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fe554:	50                   	push   eax
  4fe555:	8d 4c 24 28          	lea    ecx,[esp+0x28]
  4fe559:	e8 82 4d f1 ff       	call   0x4132e0
  4fe55e:	89 44 24 24          	mov    DWORD PTR [esp+0x24],eax
  4fe562:	a1 64 bb 6c 00       	mov    eax,ds:0x6cbb64
  4fe567:	51                   	push   ecx
  4fe568:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4fe56c:	8b cc                	mov    ecx,esp
  4fe56e:	8d 54 24 28          	lea    edx,[esp+0x28]
  4fe572:	51                   	push   ecx
  4fe573:	89 01                	mov    DWORD PTR [ecx],eax
  4fe575:	8b cc                	mov    ecx,esp
  4fe577:	52                   	push   edx
  4fe578:	e8 53 03 f1 ff       	call   0x40e8d0
  4fe57d:	51                   	push   ecx
  4fe57e:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4fe582:	8b cc                	mov    ecx,esp
  4fe584:	50                   	push   eax
  4fe585:	e8 36 fd f0 ff       	call   0x40e2c0
  4fe58a:	8b ce                	mov    ecx,esi
  4fe58c:	e8 af 63 f1 ff       	call   0x414940
  4fe591:	8b 0d 64 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb64
  4fe597:	3b c3                	cmp    eax,ebx
  4fe599:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
  4fe59d:	75 17                	jne    0x4fe5b6
  4fe59f:	8b 55 40             	mov    edx,DWORD PTR [ebp+0x40]
  4fe5a2:	03 d1                	add    edx,ecx
  4fe5a4:	3b c2                	cmp    eax,edx
  4fe5a6:	73 0e                	jae    0x4fe5b6
  4fe5a8:	8b 15 80 17 5e 00    	mov    edx,DWORD PTR ds:0x5e1780
  4fe5ae:	4a                   	dec    edx
  4fe5af:	3b c2                	cmp    eax,edx
  4fe5b1:	7d 03                	jge    0x4fe5b6
  4fe5b3:	40                   	inc    eax
  4fe5b4:	eb b1                	jmp    0x4fe567
  4fe5b6:	8b 55 40             	mov    edx,DWORD PTR [ebp+0x40]
  4fe5b9:	03 d1                	add    edx,ecx
  4fe5bb:	3b c2                	cmp    eax,edx
  4fe5bd:	73 57                	jae    0x4fe616
  4fe5bf:	8b 0d 80 17 5e 00    	mov    ecx,DWORD PTR ds:0x5e1780
  4fe5c5:	49                   	dec    ecx
  4fe5c6:	3b c1                	cmp    eax,ecx
  4fe5c8:	7d 4c                	jge    0x4fe616
  4fe5ca:	51                   	push   ecx
  4fe5cb:	8d 4c 24 20          	lea    ecx,[esp+0x20]
  4fe5cf:	8b fc                	mov    edi,esp
  4fe5d1:	e8 aa 62 f1 ff       	call   0x414880
  4fe5d6:	51                   	push   ecx
  4fe5d7:	8d 54 24 2c          	lea    edx,[esp+0x2c]
  4fe5db:	40                   	inc    eax
  4fe5dc:	8b cc                	mov    ecx,esp
  4fe5de:	52                   	push   edx
  4fe5df:	89 07                	mov    DWORD PTR [edi],eax
  4fe5e1:	e8 ea 02 f1 ff       	call   0x40e8d0
  4fe5e6:	51                   	push   ecx
  4fe5e7:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4fe5eb:	8b cc                	mov    ecx,esp
  4fe5ed:	50                   	push   eax
  4fe5ee:	e8 cd fc f0 ff       	call   0x40e2c0
  4fe5f3:	8b ce                	mov    ecx,esi
  4fe5f5:	e8 46 63 f1 ff       	call   0x414940
  4fe5fa:	3b c3                	cmp    eax,ebx
  4fe5fc:	74 18                	je     0x4fe616
  4fe5fe:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
  4fe602:	8b 3d 64 bb 6c 00    	mov    edi,DWORD PTR ds:0x6cbb64
  4fe608:	40                   	inc    eax
  4fe609:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
  4fe60d:	8b 4d 40             	mov    ecx,DWORD PTR [ebp+0x40]
  4fe610:	03 cf                	add    ecx,edi
  4fe612:	3b c1                	cmp    eax,ecx
  4fe614:	72 a9                	jb     0x4fe5bf
  4fe616:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4fe61a:	8d 44 24 24          	lea    eax,[esp+0x24]
  4fe61e:	52                   	push   edx
  4fe61f:	8d 4c 24 24          	lea    ecx,[esp+0x24]
  4fe623:	50                   	push   eax
  4fe624:	8d 94 24 18 01 00 00 	lea    edx,[esp+0x118]
  4fe62b:	51                   	push   ecx
  4fe62c:	52                   	push   edx
  4fe62d:	8b ce                	mov    ecx,esi
  4fe62f:	e8 3c f3 f4 ff       	call   0x44d970
  4fe634:	50                   	push   eax
  4fe635:	8b 06                	mov    eax,DWORD PTR [esi]
  4fe637:	50                   	push   eax
  4fe638:	b9 90 54 6c 00       	mov    ecx,0x6c5490
  4fe63d:	e8 ee 6b ff ff       	call   0x4f5230
  4fe642:	85 c0                	test   eax,eax
  4fe644:	0f 84 bc 01 00 00    	je     0x4fe806
  4fe64a:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4fe64e:	8d 54 24 10          	lea    edx,[esp+0x10]
  4fe652:	51                   	push   ecx
  4fe653:	8b 4c 24 64          	mov    ecx,DWORD PTR [esp+0x64]
  4fe657:	8d 44 24 1c          	lea    eax,[esp+0x1c]
  4fe65b:	52                   	push   edx
  4fe65c:	50                   	push   eax
  4fe65d:	51                   	push   ecx
  4fe65e:	51                   	push   ecx
  4fe65f:	8d 54 24 30          	lea    edx,[esp+0x30]
  4fe663:	8b cc                	mov    ecx,esp
  4fe665:	52                   	push   edx
  4fe666:	e8 55 08 f1 ff       	call   0x40eec0
  4fe66b:	51                   	push   ecx
  4fe66c:	8d 44 24 3c          	lea    eax,[esp+0x3c]
  4fe670:	8b cc                	mov    ecx,esp
  4fe672:	50                   	push   eax
  4fe673:	e8 58 02 f1 ff       	call   0x40e8d0
  4fe678:	51                   	push   ecx
  4fe679:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  4fe67d:	8b cc                	mov    ecx,esp
  4fe67f:	52                   	push   edx
  4fe680:	e8 3b fc f0 ff       	call   0x40e2c0
  4fe685:	8b ce                	mov    ecx,esi
  4fe687:	e8 f4 97 01 00       	call   0x517e80
  4fe68c:	66 8b 4c 24 18       	mov    cx,WORD PTR [esp+0x18]
  4fe691:	66 8b 54 24 10       	mov    dx,WORD PTR [esp+0x10]
  4fe696:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4fe69a:	c7 44 24 30 05 00 00 00 	mov    DWORD PTR [esp+0x30],0x5
  4fe6a2:	c7 44 24 34 15 00 00 00 	mov    DWORD PTR [esp+0x34],0x15
  4fe6aa:	88 5c 24 40          	mov    BYTE PTR [esp+0x40],bl
  4fe6ae:	88 5c 24 3f          	mov    BYTE PTR [esp+0x3f],bl
  4fe6b2:	8a 06                	mov    al,BYTE PTR [esi]
  4fe6b4:	66 89 4c 24 39       	mov    WORD PTR [esp+0x39],cx
  4fe6b9:	8d 4c 24 2c          	lea    ecx,[esp+0x2c]
  4fe6bd:	88 44 24 38          	mov    BYTE PTR [esp+0x38],al
  4fe6c1:	66 8b 44 24 14       	mov    ax,WORD PTR [esp+0x14]
  4fe6c6:	51                   	push   ecx
  4fe6c7:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe6cc:	89 5c 24 64          	mov    DWORD PTR [esp+0x64],ebx
  4fe6d0:	66 89 54 24 3f       	mov    WORD PTR [esp+0x3f],dx
  4fe6d5:	66 89 44 24 41       	mov    WORD PTR [esp+0x41],ax
  4fe6da:	e8 f1 94 03 00       	call   0x537bd0
  4fe6df:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fe6e5:	0f 84 1b 01 00 00    	je     0x4fe806
  4fe6eb:	8b 96 a8 00 00 00    	mov    edx,DWORD PTR [esi+0xa8]
  4fe6f1:	52                   	push   edx
  4fe6f2:	6a ff                	push   0xffffffff
  4fe6f4:	6a 05                	push   0x5
  4fe6f6:	e9 01 01 00 00       	jmp    0x4fe7fc
  4fe6fb:	8d 44 24 14          	lea    eax,[esp+0x14]
  4fe6ff:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4fe703:	50                   	push   eax
  4fe704:	8b 44 24 5c          	mov    eax,DWORD PTR [esp+0x5c]
  4fe708:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4fe70c:	51                   	push   ecx
  4fe70d:	52                   	push   edx
  4fe70e:	8b 54 24 34          	mov    edx,DWORD PTR [esp+0x34]
  4fe712:	50                   	push   eax
  4fe713:	51                   	push   ecx
  4fe714:	8b cc                	mov    ecx,esp
  4fe716:	52                   	push   edx
  4fe717:	e8 94 07 f1 ff       	call   0x40eeb0
  4fe71c:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fe721:	51                   	push   ecx
  4fe722:	8b cc                	mov    ecx,esp
  4fe724:	50                   	push   eax
  4fe725:	e8 76 01 f1 ff       	call   0x40e8a0
  4fe72a:	8b 15 88 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5488
  4fe730:	51                   	push   ecx
  4fe731:	8b cc                	mov    ecx,esp
  4fe733:	52                   	push   edx
  4fe734:	e8 57 fb f0 ff       	call   0x40e290
  4fe739:	8b ce                	mov    ecx,esi
  4fe73b:	e8 10 93 01 00       	call   0x517a50
  4fe740:	6a 10                	push   0x10
  4fe742:	89 5c 24 5c          	mov    DWORD PTR [esp+0x5c],ebx
  4fe746:	ff 15 04 52 5c 00    	call   DWORD PTR ds:0x5c5204
  4fe74c:	66 85 c0             	test   ax,ax
  4fe74f:	7d 49                	jge    0x4fe79a
  4fe751:	66 8b 4c 24 18       	mov    cx,WORD PTR [esp+0x18]
  4fe756:	66 8b 54 24 10       	mov    dx,WORD PTR [esp+0x10]
  4fe75b:	89 5c 24 44          	mov    DWORD PTR [esp+0x44],ebx
  4fe75f:	c7 44 24 48 0c 00 00 00 	mov    DWORD PTR [esp+0x48],0xc
  4fe767:	c7 44 24 4c 14 00 00 00 	mov    DWORD PTR [esp+0x4c],0x14
  4fe76f:	8a 06                	mov    al,BYTE PTR [esi]
  4fe771:	66 89 4c 24 51       	mov    WORD PTR [esp+0x51],cx
  4fe776:	8d 4c 24 44          	lea    ecx,[esp+0x44]
  4fe77a:	88 44 24 50          	mov    BYTE PTR [esp+0x50],al
  4fe77e:	66 8b 44 24 14       	mov    ax,WORD PTR [esp+0x14]
  4fe783:	51                   	push   ecx
  4fe784:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe789:	66 89 54 24 57       	mov    WORD PTR [esp+0x57],dx
  4fe78e:	66 89 44 24 59       	mov    WORD PTR [esp+0x59],ax
  4fe793:	e8 38 94 03 00       	call   0x537bd0
  4fe798:	eb 6c                	jmp    0x4fe806
  4fe79a:	66 8b 44 24 18       	mov    ax,WORD PTR [esp+0x18]
  4fe79f:	66 8b 4c 24 10       	mov    cx,WORD PTR [esp+0x10]
  4fe7a4:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4fe7a8:	c7 44 24 30 05 00 00 00 	mov    DWORD PTR [esp+0x30],0x5
  4fe7b0:	c7 44 24 34 15 00 00 00 	mov    DWORD PTR [esp+0x34],0x15
  4fe7b8:	88 5c 24 40          	mov    BYTE PTR [esp+0x40],bl
  4fe7bc:	88 5c 24 3f          	mov    BYTE PTR [esp+0x3f],bl
  4fe7c0:	8a 16                	mov    dl,BYTE PTR [esi]
  4fe7c2:	66 89 44 24 39       	mov    WORD PTR [esp+0x39],ax
  4fe7c7:	88 54 24 38          	mov    BYTE PTR [esp+0x38],dl
  4fe7cb:	66 8b 54 24 14       	mov    dx,WORD PTR [esp+0x14]
  4fe7d0:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4fe7d4:	66 89 4c 24 3b       	mov    WORD PTR [esp+0x3b],cx
  4fe7d9:	50                   	push   eax
  4fe7da:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe7df:	66 89 54 24 41       	mov    WORD PTR [esp+0x41],dx
  4fe7e4:	e8 e7 93 03 00       	call   0x537bd0
  4fe7e9:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fe7ef:	74 15                	je     0x4fe806
  4fe7f1:	8b 8e a8 00 00 00    	mov    ecx,DWORD PTR [esi+0xa8]
  4fe7f7:	51                   	push   ecx
  4fe7f8:	6a ff                	push   0xffffffff
  4fe7fa:	6a 04                	push   0x4
  4fe7fc:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fe801:	e8 5a d8 06 00       	call   0x56c060
  4fe806:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe80b:	e8 00 bf 02 00       	call   0x52a710
  4fe810:	8b f0                	mov    esi,eax
  4fe812:	3b f3                	cmp    esi,ebx
  4fe814:	0f 85 a5 fc ff ff    	jne    0x4fe4bf
  4fe81a:	5f                   	pop    edi
  4fe81b:	5e                   	pop    esi
  4fe81c:	5d                   	pop    ebp
  4fe81d:	5b                   	pop    ebx
  4fe81e:	81 c4 30 01 00 00    	add    esp,0x130
  4fe824:	c2 2c 00             	ret    0x2c
  4fe827:	8d 54 24 14          	lea    edx,[esp+0x14]
  4fe82b:	8d 44 24 10          	lea    eax,[esp+0x10]
  4fe82f:	52                   	push   edx
  4fe830:	8b 54 24 5c          	mov    edx,DWORD PTR [esp+0x5c]
  4fe834:	8d 4c 24 1c          	lea    ecx,[esp+0x1c]
  4fe838:	50                   	push   eax
  4fe839:	8b 44 24 30          	mov    eax,DWORD PTR [esp+0x30]
  4fe83d:	51                   	push   ecx
  4fe83e:	52                   	push   edx
  4fe83f:	51                   	push   ecx
  4fe840:	8b cc                	mov    ecx,esp
  4fe842:	50                   	push   eax
  4fe843:	e8 68 06 f1 ff       	call   0x40eeb0
  4fe848:	8b 15 84 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5484
  4fe84e:	51                   	push   ecx
  4fe84f:	8b cc                	mov    ecx,esp
  4fe851:	52                   	push   edx
  4fe852:	e8 49 00 f1 ff       	call   0x40e8a0
  4fe857:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fe85c:	51                   	push   ecx
  4fe85d:	8b cc                	mov    ecx,esp
  4fe85f:	50                   	push   eax
  4fe860:	e8 2b fa f0 ff       	call   0x40e290
  4fe865:	8b ce                	mov    ecx,esi
  4fe867:	e8 e4 91 01 00       	call   0x517a50
  4fe86c:	89 5c 24 44          	mov    DWORD PTR [esp+0x44],ebx
  4fe870:	c7 44 24 48 0c 00 00 00 	mov    DWORD PTR [esp+0x48],0xc
  4fe878:	c7 44 24 4c 14 00 00 00 	mov    DWORD PTR [esp+0x4c],0x14
  4fe880:	8a 0e                	mov    cl,BYTE PTR [esi]
  4fe882:	e9 75 08 00 00       	jmp    0x4ff0fc
  4fe887:	39 1d c8 04 5e 00    	cmp    DWORD PTR ds:0x5e04c8,ebx
  4fe88d:	0f 85 ba 00 00 00    	jne    0x4fe94d
  4fe893:	b8 20 62 50 00       	mov    eax,0x506220
  4fe898:	33 c9                	xor    ecx,ecx
  4fe89a:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4fe89f:	a1 58 ef 6d 00       	mov    eax,ds:0x6def58
  4fe8a4:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4fe8aa:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe8af:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4fe8b5:	a3 64 ef 6d 00       	mov    ds:0x6def64,eax
  4fe8ba:	e8 51 be 02 00       	call   0x52a710
  4fe8bf:	3b c3                	cmp    eax,ebx
  4fe8c1:	0f 84 f6 08 00 00    	je     0x4ff1bd
  4fe8c7:	bf 13 00 00 00       	mov    edi,0x13
  4fe8cc:	be 0a 00 00 00       	mov    esi,0xa
  4fe8d1:	39 58 68             	cmp    DWORD PTR [eax+0x68],ebx
  4fe8d4:	74 5c                	je     0x4fe932
  4fe8d6:	66 8b 15 88 54 6c 00 	mov    dx,WORD PTR ds:0x6c5488
  4fe8dd:	89 9c 24 94 00 00 00 	mov    DWORD PTR [esp+0x94],ebx
  4fe8e4:	89 b4 24 98 00 00 00 	mov    DWORD PTR [esp+0x98],esi
  4fe8eb:	89 bc 24 9c 00 00 00 	mov    DWORD PTR [esp+0x9c],edi
  4fe8f2:	8a 08                	mov    cl,BYTE PTR [eax]
  4fe8f4:	66 a1 84 54 6c 00    	mov    ax,ds:0x6c5484
  4fe8fa:	88 8c 24 a0 00 00 00 	mov    BYTE PTR [esp+0xa0],cl
  4fe901:	66 8b 0d 64 bb 6c 00 	mov    cx,WORD PTR ds:0x6cbb64
  4fe908:	66 89 94 24 a1 00 00 00 	mov    WORD PTR [esp+0xa1],dx
  4fe910:	8d 94 24 94 00 00 00 	lea    edx,[esp+0x94]
  4fe917:	66 89 8c 24 a5 00 00 00 	mov    WORD PTR [esp+0xa5],cx
  4fe91f:	52                   	push   edx
  4fe920:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fe925:	66 89 84 24 a7 00 00 00 	mov    WORD PTR [esp+0xa7],ax
  4fe92d:	e8 9e 92 03 00       	call   0x537bd0
  4fe932:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe937:	e8 d4 bd 02 00       	call   0x52a710
  4fe93c:	3b c3                	cmp    eax,ebx
  4fe93e:	75 91                	jne    0x4fe8d1
  4fe940:	5f                   	pop    edi
  4fe941:	5e                   	pop    esi
  4fe942:	5d                   	pop    ebp
  4fe943:	5b                   	pop    ebx
  4fe944:	81 c4 30 01 00 00    	add    esp,0x130
  4fe94a:	c2 2c 00             	ret    0x2c
  4fe94d:	8b 15 88 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5488
  4fe953:	3b d3                	cmp    edx,ebx
  4fe955:	0f 8c 62 08 00 00    	jl     0x4ff1bd
  4fe95b:	3b 15 94 54 6c 00    	cmp    edx,DWORD PTR ds:0x6c5494
  4fe961:	0f 8d 56 08 00 00    	jge    0x4ff1bd
  4fe967:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fe96c:	3b c3                	cmp    eax,ebx
  4fe96e:	0f 8c 49 08 00 00    	jl     0x4ff1bd
  4fe974:	3b 05 98 54 6c 00    	cmp    eax,DWORD PTR ds:0x6c5498
  4fe97a:	0f 8d 3d 08 00 00    	jge    0x4ff1bd
  4fe980:	8b 0d 64 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb64
  4fe986:	3b cb                	cmp    ecx,ebx
  4fe988:	0f 8c 2f 08 00 00    	jl     0x4ff1bd
  4fe98e:	3b 0d 9c 54 6c 00    	cmp    ecx,DWORD PTR ds:0x6c549c
  4fe994:	0f 8d 23 08 00 00    	jge    0x4ff1bd
  4fe99a:	8b 04 85 42 b9 6c 00 	mov    eax,DWORD PTR [eax*4+0x6cb942]
  4fe9a1:	8b 3c 8d c2 b8 6c 00 	mov    edi,DWORD PTR [ecx*4+0x6cb8c2]
  4fe9a8:	8b 0d dc 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c54dc
  4fe9ae:	03 c7                	add    eax,edi
  4fe9b0:	03 c2                	add    eax,edx
  4fe9b2:	8d 04 40             	lea    eax,[eax+eax*2]
  4fe9b5:	8d 0c 81             	lea    ecx,[ecx+eax*4]
  4fe9b8:	89 4d 79             	mov    DWORD PTR [ebp+0x79],ecx
  4fe9bb:	a1 c8 04 5e 00       	mov    eax,ds:0x5e04c8
  4fe9c0:	83 f8 12             	cmp    eax,0x12
  4fe9c3:	0f 85 bc 00 00 00    	jne    0x4fea85
  4fe9c9:	8b 15 58 ef 6d 00    	mov    edx,DWORD PTR ds:0x6def58
  4fe9cf:	33 c9                	xor    ecx,ecx
  4fe9d1:	b8 20 62 50 00       	mov    eax,0x506220
  4fe9d6:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4fe9dc:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fe9e1:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4fe9e7:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4fe9ec:	89 15 64 ef 6d 00    	mov    DWORD PTR ds:0x6def64,edx
  4fe9f2:	e8 19 bd 02 00       	call   0x52a710
  4fe9f7:	3b c3                	cmp    eax,ebx
  4fe9f9:	0f 84 be 07 00 00    	je     0x4ff1bd
  4fe9ff:	bf 13 00 00 00       	mov    edi,0x13
  4fea04:	be 06 00 00 00       	mov    esi,0x6
  4fea09:	39 58 68             	cmp    DWORD PTR [eax+0x68],ebx
  4fea0c:	74 5c                	je     0x4fea6a
  4fea0e:	66 8b 0d 88 54 6c 00 	mov    cx,WORD PTR ds:0x6c5488
  4fea15:	66 8b 15 84 54 6c 00 	mov    dx,WORD PTR ds:0x6c5484
  4fea1c:	89 9c 24 a8 00 00 00 	mov    DWORD PTR [esp+0xa8],ebx
  4fea23:	89 b4 24 ac 00 00 00 	mov    DWORD PTR [esp+0xac],esi
  4fea2a:	89 bc 24 b0 00 00 00 	mov    DWORD PTR [esp+0xb0],edi
  4fea31:	8a 00                	mov    al,BYTE PTR [eax]
  4fea33:	66 89 8c 24 b5 00 00 00 	mov    WORD PTR [esp+0xb5],cx
  4fea3b:	8d 8c 24 a8 00 00 00 	lea    ecx,[esp+0xa8]
  4fea42:	88 84 24 b4 00 00 00 	mov    BYTE PTR [esp+0xb4],al
  4fea49:	66 a1 64 bb 6c 00    	mov    ax,ds:0x6cbb64
  4fea4f:	51                   	push   ecx
  4fea50:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fea55:	66 89 94 24 bb 00 00 00 	mov    WORD PTR [esp+0xbb],dx
  4fea5d:	66 89 84 24 bd 00 00 00 	mov    WORD PTR [esp+0xbd],ax
  4fea65:	e8 66 91 03 00       	call   0x537bd0
  4fea6a:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fea6f:	e8 9c bc 02 00       	call   0x52a710
  4fea74:	3b c3                	cmp    eax,ebx
  4fea76:	75 91                	jne    0x4fea09
  4fea78:	5f                   	pop    edi
  4fea79:	5e                   	pop    esi
  4fea7a:	5d                   	pop    ebp
  4fea7b:	5b                   	pop    ebx
  4fea7c:	81 c4 30 01 00 00    	add    esp,0x130
  4fea82:	c2 2c 00             	ret    0x2c
  4fea85:	83 f8 11             	cmp    eax,0x11
  4fea88:	0f 85 72 01 00 00    	jne    0x4fec00
  4fea8e:	8b 15 58 ef 6d 00    	mov    edx,DWORD PTR ds:0x6def58
  4fea94:	33 c9                	xor    ecx,ecx
  4fea96:	b8 20 62 50 00       	mov    eax,0x506220
  4fea9b:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4feaa1:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4feaa6:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4feaac:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4feab1:	89 15 64 ef 6d 00    	mov    DWORD PTR ds:0x6def64,edx
  4feab7:	e8 54 bc 02 00       	call   0x52a710
  4feabc:	8b f0                	mov    esi,eax
  4feabe:	3b f3                	cmp    esi,ebx
  4feac0:	0f 84 f7 06 00 00    	je     0x4ff1bd
  4feac6:	8b 3d 04 52 5c 00    	mov    edi,DWORD PTR ds:0x5c5204
  4feacc:	bd 15 00 00 00       	mov    ebp,0x15
  4fead1:	39 5e 68             	cmp    DWORD PTR [esi+0x68],ebx
  4fead4:	0f 84 05 01 00 00    	je     0x4febdf
  4feada:	8d 44 24 14          	lea    eax,[esp+0x14]
  4feade:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4feae2:	50                   	push   eax
  4feae3:	a1 a4 be 6d 00       	mov    eax,ds:0x6dbea4
  4feae8:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4feaec:	51                   	push   ecx
  4feaed:	52                   	push   edx
  4feaee:	8b 50 10             	mov    edx,DWORD PTR [eax+0x10]
  4feaf1:	6a 01                	push   0x1
  4feaf3:	51                   	push   ecx
  4feaf4:	8b cc                	mov    ecx,esp
  4feaf6:	52                   	push   edx
  4feaf7:	e8 b4 03 f1 ff       	call   0x40eeb0
  4feafc:	a1 a4 be 6d 00       	mov    eax,ds:0x6dbea4
  4feb01:	51                   	push   ecx
  4feb02:	8b cc                	mov    ecx,esp
  4feb04:	8b 50 0c             	mov    edx,DWORD PTR [eax+0xc]
  4feb07:	52                   	push   edx
  4feb08:	e8 93 fd f0 ff       	call   0x40e8a0
  4feb0d:	a1 a4 be 6d 00       	mov    eax,ds:0x6dbea4
  4feb12:	51                   	push   ecx
  4feb13:	8b cc                	mov    ecx,esp
  4feb15:	8b 50 08             	mov    edx,DWORD PTR [eax+0x8]
  4feb18:	52                   	push   edx
  4feb19:	e8 72 f7 f0 ff       	call   0x40e290
  4feb1e:	8b ce                	mov    ecx,esi
  4feb20:	e8 2b 8f 01 00       	call   0x517a50
  4feb25:	6a 10                	push   0x10
  4feb27:	ff d7                	call   edi
  4feb29:	66 85 c0             	test   ax,ax
  4feb2c:	7d 49                	jge    0x4feb77
  4feb2e:	66 8b 4c 24 18       	mov    cx,WORD PTR [esp+0x18]
  4feb33:	66 8b 54 24 10       	mov    dx,WORD PTR [esp+0x10]
  4feb38:	89 5c 24 44          	mov    DWORD PTR [esp+0x44],ebx
  4feb3c:	c7 44 24 48 0c 00 00 00 	mov    DWORD PTR [esp+0x48],0xc
  4feb44:	c7 44 24 4c 14 00 00 00 	mov    DWORD PTR [esp+0x4c],0x14
  4feb4c:	8a 06                	mov    al,BYTE PTR [esi]
  4feb4e:	66 89 4c 24 51       	mov    WORD PTR [esp+0x51],cx
  4feb53:	8d 4c 24 44          	lea    ecx,[esp+0x44]
  4feb57:	88 44 24 50          	mov    BYTE PTR [esp+0x50],al
  4feb5b:	66 8b 44 24 14       	mov    ax,WORD PTR [esp+0x14]
  4feb60:	51                   	push   ecx
  4feb61:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4feb66:	66 89 54 24 57       	mov    WORD PTR [esp+0x57],dx
  4feb6b:	66 89 44 24 59       	mov    WORD PTR [esp+0x59],ax
  4feb70:	e8 5b 90 03 00       	call   0x537bd0
  4feb75:	eb 68                	jmp    0x4febdf
  4feb77:	66 8b 44 24 18       	mov    ax,WORD PTR [esp+0x18]
  4feb7c:	66 8b 4c 24 10       	mov    cx,WORD PTR [esp+0x10]
  4feb81:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4feb85:	c7 44 24 30 05 00 00 00 	mov    DWORD PTR [esp+0x30],0x5
  4feb8d:	89 6c 24 34          	mov    DWORD PTR [esp+0x34],ebp
  4feb91:	88 5c 24 40          	mov    BYTE PTR [esp+0x40],bl
  4feb95:	88 5c 24 3f          	mov    BYTE PTR [esp+0x3f],bl
  4feb99:	8a 16                	mov    dl,BYTE PTR [esi]
  4feb9b:	66 89 44 24 39       	mov    WORD PTR [esp+0x39],ax
  4feba0:	88 54 24 38          	mov    BYTE PTR [esp+0x38],dl
  4feba4:	66 8b 54 24 14       	mov    dx,WORD PTR [esp+0x14]
  4feba9:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4febad:	66 89 4c 24 3b       	mov    WORD PTR [esp+0x3b],cx
  4febb2:	50                   	push   eax
  4febb3:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4febb8:	66 89 54 24 41       	mov    WORD PTR [esp+0x41],dx
  4febbd:	e8 0e 90 03 00       	call   0x537bd0
  4febc2:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4febc8:	74 15                	je     0x4febdf
  4febca:	8b 8e a8 00 00 00    	mov    ecx,DWORD PTR [esi+0xa8]
  4febd0:	51                   	push   ecx
  4febd1:	6a ff                	push   0xffffffff
  4febd3:	6a 04                	push   0x4
  4febd5:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4febda:	e8 81 d4 06 00       	call   0x56c060
  4febdf:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4febe4:	e8 27 bb 02 00       	call   0x52a710
  4febe9:	8b f0                	mov    esi,eax
  4febeb:	3b f3                	cmp    esi,ebx
  4febed:	0f 85 de fe ff ff    	jne    0x4fead1
  4febf3:	5f                   	pop    edi
  4febf4:	5e                   	pop    esi
  4febf5:	5d                   	pop    ebp
  4febf6:	5b                   	pop    ebx
  4febf7:	81 c4 30 01 00 00    	add    esp,0x130
  4febfd:	c2 2c 00             	ret    0x2c
  4fec00:	33 c0                	xor    eax,eax
  4fec02:	66 8b 01             	mov    ax,WORD PTR [ecx]
  4fec05:	8d 0c 80             	lea    ecx,[eax+eax*4]
  4fec08:	8d 14 c9             	lea    edx,[ecx+ecx*8]
  4fec0b:	d1 e2                	shl    edx,1
  4fec0d:	2b d0                	sub    edx,eax
  4fec0f:	a1 0c 66 65 00       	mov    eax,ds:0x65660c
  4fec14:	8b 8c 90 94 00 00 00 	mov    ecx,DWORD PTR [eax+edx*4+0x94]
  4fec1b:	a1 64 bb 6c 00       	mov    eax,ds:0x6cbb64
  4fec20:	83 f9 10             	cmp    ecx,0x10
  4fec23:	8d 70 01             	lea    esi,[eax+0x1]
  4fec26:	74 02                	je     0x4fec2a
  4fec28:	8b f0                	mov    esi,eax
  4fec2a:	3b f0                	cmp    esi,eax
  4fec2c:	89 74 24 28          	mov    DWORD PTR [esp+0x28],esi
  4fec30:	74 46                	je     0x4fec78
  4fec32:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fec37:	3b c3                	cmp    eax,ebx
  4fec39:	0f 8c 7e 05 00 00    	jl     0x4ff1bd
  4fec3f:	3b 05 94 54 6c 00    	cmp    eax,DWORD PTR ds:0x6c5494
  4fec45:	0f 8d 72 05 00 00    	jge    0x4ff1bd
  4fec4b:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fec50:	3b c3                	cmp    eax,ebx
  4fec52:	0f 8c 65 05 00 00    	jl     0x4ff1bd
  4fec58:	3b 05 98 54 6c 00    	cmp    eax,DWORD PTR ds:0x6c5498
  4fec5e:	0f 8d 59 05 00 00    	jge    0x4ff1bd
  4fec64:	3b f3                	cmp    esi,ebx
  4fec66:	0f 8c 51 05 00 00    	jl     0x4ff1bd
  4fec6c:	3b 35 9c 54 6c 00    	cmp    esi,DWORD PTR ds:0x6c549c
  4fec72:	0f 8d 45 05 00 00    	jge    0x4ff1bd
  4fec78:	33 c9                	xor    ecx,ecx
  4fec7a:	b8 01 00 00 00       	mov    eax,0x1
  4fec7f:	89 0d 6c ef 6d 00    	mov    DWORD PTR ds:0x6def6c,ecx
  4fec85:	8b 0d 58 ef 6d 00    	mov    ecx,DWORD PTR ds:0x6def58
  4fec8b:	89 44 24 58          	mov    DWORD PTR [esp+0x58],eax
  4fec8f:	89 44 24 5c          	mov    DWORD PTR [esp+0x5c],eax
  4fec93:	b8 20 62 50 00       	mov    eax,0x506220
  4fec98:	89 0d 64 ef 6d 00    	mov    DWORD PTR ds:0x6def64,ecx
  4fec9e:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4feca3:	89 5c 24 60          	mov    DWORD PTR [esp+0x60],ebx
  4feca7:	89 1d 60 ef 6d 00    	mov    DWORD PTR ds:0x6def60,ebx
  4fecad:	a3 68 ef 6d 00       	mov    ds:0x6def68,eax
  4fecb2:	e8 59 ba 02 00       	call   0x52a710
  4fecb7:	8b e8                	mov    ebp,eax
  4fecb9:	3b eb                	cmp    ebp,ebx
  4fecbb:	0f 84 fc 04 00 00    	je     0x4ff1bd
  4fecc1:	39 5d 68             	cmp    DWORD PTR [ebp+0x68],ebx
  4fecc4:	0f 84 a4 02 00 00    	je     0x4fef6e
  4fecca:	8b 95 6b 09 00 00    	mov    edx,DWORD PTR [ebp+0x96b]
  4fecd0:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fecd5:	3b d0                	cmp    edx,eax
  4fecd7:	75 20                	jne    0x4fecf9
  4fecd9:	8b 85 6f 09 00 00    	mov    eax,DWORD PTR [ebp+0x96f]
  4fecdf:	8b 0d 84 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5484
  4fece5:	3b c1                	cmp    eax,ecx
  4fece7:	75 10                	jne    0x4fecf9
  4fece9:	39 b5 73 09 00 00    	cmp    DWORD PTR [ebp+0x973],esi
  4fecef:	75 08                	jne    0x4fecf9
  4fecf1:	c7 44 24 60 01 00 00 00 	mov    DWORD PTR [esp+0x60],0x1
  4fecf9:	8b 85 a8 00 00 00    	mov    eax,DWORD PTR [ebp+0xa8]
  4fecff:	8d 0c c0             	lea    ecx,[eax+eax*8]
  4fed02:	8d 0c 88             	lea    ecx,[eax+ecx*4]
  4fed05:	8d 14 89             	lea    edx,[ecx+ecx*4]
  4fed08:	8d bc d0 80 5f 6a 00 	lea    edi,[eax+edx*8+0x6a5f80]
  4fed0f:	8b 44 24 60          	mov    eax,DWORD PTR [esp+0x60]
  4fed13:	3b c3                	cmp    eax,ebx
  4fed15:	0f 84 74 02 00 00    	je     0x4fef8f
  4fed1b:	39 9d 04 01 00 00    	cmp    DWORD PTR [ebp+0x104],ebx
  4fed21:	0f 84 68 02 00 00    	je     0x4fef8f
  4fed27:	6a 10                	push   0x10
  4fed29:	ff 15 04 52 5c 00    	call   DWORD PTR ds:0x5c5204
  4fed2f:	66 85 c0             	test   ax,ax
  4fed32:	0f 8c 6c 03 00 00    	jl     0x4ff0a4
  4fed38:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fed3d:	8b 0d 94 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5494
  4fed43:	3b c3                	cmp    eax,ebx
  4fed45:	7d 06                	jge    0x4fed4d
  4fed47:	03 c1                	add    eax,ecx
  4fed49:	78 fc                	js     0x4fed47
  4fed4b:	eb 0a                	jmp    0x4fed57
  4fed4d:	3b c1                	cmp    eax,ecx
  4fed4f:	72 06                	jb     0x4fed57
  4fed51:	2b c1                	sub    eax,ecx
  4fed53:	3b c1                	cmp    eax,ecx
  4fed55:	73 fa                	jae    0x4fed51
  4fed57:	8b 0d 98 54 6c 00    	mov    ecx,DWORD PTR ds:0x6c5498
  4fed5d:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4fed61:	a1 84 54 6c 00       	mov    eax,ds:0x6c5484
  4fed66:	3b c3                	cmp    eax,ebx
  4fed68:	7d 06                	jge    0x4fed70
  4fed6a:	03 c1                	add    eax,ecx
  4fed6c:	78 fc                	js     0x4fed6a
  4fed6e:	eb 0a                	jmp    0x4fed7a
  4fed70:	3b c1                	cmp    eax,ecx
  4fed72:	72 06                	jb     0x4fed7a
  4fed74:	2b c1                	sub    eax,ecx
  4fed76:	3b c1                	cmp    eax,ecx
  4fed78:	73 fa                	jae    0x4fed74
  4fed7a:	89 44 24 24          	mov    DWORD PTR [esp+0x24],eax
  4fed7e:	a1 64 bb 6c 00       	mov    eax,ds:0x6cbb64
  4fed83:	51                   	push   ecx
  4fed84:	89 44 24 20          	mov    DWORD PTR [esp+0x20],eax
  4fed88:	8b cc                	mov    ecx,esp
  4fed8a:	51                   	push   ecx
  4fed8b:	89 01                	mov    DWORD PTR [ecx],eax
  4fed8d:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4fed91:	8b cc                	mov    ecx,esp
  4fed93:	50                   	push   eax
  4fed94:	e8 37 fb f0 ff       	call   0x40e8d0
  4fed99:	51                   	push   ecx
  4fed9a:	8d 54 24 2c          	lea    edx,[esp+0x2c]
  4fed9e:	8b cc                	mov    ecx,esp
  4feda0:	52                   	push   edx
  4feda1:	e8 1a f5 f0 ff       	call   0x40e2c0
  4feda6:	8b cd                	mov    ecx,ebp
  4feda8:	e8 93 5b f1 ff       	call   0x414940
  4fedad:	8b 0d 64 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb64
  4fedb3:	3b c3                	cmp    eax,ebx
  4fedb5:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
  4fedb9:	75 17                	jne    0x4fedd2
  4fedbb:	8b 57 40             	mov    edx,DWORD PTR [edi+0x40]
  4fedbe:	03 d1                	add    edx,ecx
  4fedc0:	3b c2                	cmp    eax,edx
  4fedc2:	73 0e                	jae    0x4fedd2
  4fedc4:	8b 15 80 17 5e 00    	mov    edx,DWORD PTR ds:0x5e1780
  4fedca:	4a                   	dec    edx
  4fedcb:	3b c2                	cmp    eax,edx
  4fedcd:	7d 03                	jge    0x4fedd2
  4fedcf:	40                   	inc    eax
  4fedd0:	eb b1                	jmp    0x4fed83
  4fedd2:	8b 57 40             	mov    edx,DWORD PTR [edi+0x40]
  4fedd5:	03 d1                	add    edx,ecx
  4fedd7:	3b c2                	cmp    eax,edx
  4fedd9:	73 57                	jae    0x4fee32
  4feddb:	8b 0d 80 17 5e 00    	mov    ecx,DWORD PTR ds:0x5e1780
  4fede1:	49                   	dec    ecx
  4fede2:	3b c1                	cmp    eax,ecx
  4fede4:	7d 4c                	jge    0x4fee32
  4fede6:	51                   	push   ecx
  4fede7:	8d 4c 24 20          	lea    ecx,[esp+0x20]
  4fedeb:	8b f4                	mov    esi,esp
  4feded:	e8 8e 5a f1 ff       	call   0x414880
  4fedf2:	51                   	push   ecx
  4fedf3:	8d 54 24 2c          	lea    edx,[esp+0x2c]
  4fedf7:	40                   	inc    eax
  4fedf8:	8b cc                	mov    ecx,esp
  4fedfa:	52                   	push   edx
  4fedfb:	89 06                	mov    DWORD PTR [esi],eax
  4fedfd:	e8 ce fa f0 ff       	call   0x40e8d0
  4fee02:	51                   	push   ecx
  4fee03:	8d 44 24 2c          	lea    eax,[esp+0x2c]
  4fee07:	8b cc                	mov    ecx,esp
  4fee09:	50                   	push   eax
  4fee0a:	e8 b1 f4 f0 ff       	call   0x40e2c0
  4fee0f:	8b cd                	mov    ecx,ebp
  4fee11:	e8 2a 5b f1 ff       	call   0x414940
  4fee16:	3b c3                	cmp    eax,ebx
  4fee18:	74 18                	je     0x4fee32
  4fee1a:	8b 44 24 1c          	mov    eax,DWORD PTR [esp+0x1c]
  4fee1e:	8b 35 64 bb 6c 00    	mov    esi,DWORD PTR ds:0x6cbb64
  4fee24:	40                   	inc    eax
  4fee25:	89 44 24 1c          	mov    DWORD PTR [esp+0x1c],eax
  4fee29:	8b 4f 40             	mov    ecx,DWORD PTR [edi+0x40]
  4fee2c:	03 ce                	add    ecx,esi
  4fee2e:	3b c1                	cmp    eax,ecx
  4fee30:	72 a9                	jb     0x4feddb
  4fee32:	38 9d 23 07 00 00    	cmp    BYTE PTR [ebp+0x723],bl
  4fee38:	75 09                	jne    0x4fee43
  4fee3a:	83 bd ec 05 00 00 0e 	cmp    DWORD PTR [ebp+0x5ec],0xe
  4fee41:	75 05                	jne    0x4fee48
  4fee43:	bb 01 00 00 00       	mov    ebx,0x1
  4fee48:	8b 85 ac 00 00 00    	mov    eax,DWORD PTR [ebp+0xac]
  4fee4e:	8b 8d a8 00 00 00    	mov    ecx,DWORD PTR [ebp+0xa8]
  4fee54:	8b b5 04 01 00 00    	mov    esi,DWORD PTR [ebp+0x104]
  4fee5a:	53                   	push   ebx
  4fee5b:	8b 50 44             	mov    edx,DWORD PTR [eax+0x44]
  4fee5e:	8b 78 08             	mov    edi,DWORD PTR [eax+0x8]
  4fee61:	8b 40 0c             	mov    eax,DWORD PTR [eax+0xc]
  4fee64:	33 db                	xor    ebx,ebx
  4fee66:	51                   	push   ecx
  4fee67:	53                   	push   ebx
  4fee68:	52                   	push   edx
  4fee69:	56                   	push   esi
  4fee6a:	57                   	push   edi
  4fee6b:	50                   	push   eax
  4fee6c:	8d 8c 24 44 01 00 00 	lea    ecx,[esp+0x144]
  4fee73:	e8 d8 f3 f0 ff       	call   0x40e250
  4fee78:	b9 05 00 00 00       	mov    ecx,0x5
  4fee7d:	8b f0                	mov    esi,eax
  4fee7f:	8d bc 24 10 01 00 00 	lea    edi,[esp+0x110]
  4fee86:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4fee8a:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4fee8c:	8d 44 24 24          	lea    eax,[esp+0x24]
  4fee90:	52                   	push   edx
  4fee91:	8d 4c 24 24          	lea    ecx,[esp+0x24]
  4fee95:	50                   	push   eax
  4fee96:	8b 45 00             	mov    eax,DWORD PTR [ebp+0x0]
  4fee99:	8d 94 24 18 01 00 00 	lea    edx,[esp+0x118]
  4feea0:	51                   	push   ecx
  4feea1:	52                   	push   edx
  4feea2:	50                   	push   eax
  4feea3:	b9 90 54 6c 00       	mov    ecx,0x6c5490
  4feea8:	66 a5                	movs   WORD PTR es:[edi],WORD PTR ds:[esi]
  4feeaa:	e8 81 63 ff ff       	call   0x4f5230
  4feeaf:	85 c0                	test   eax,eax
  4feeb1:	0f 84 b3 00 00 00    	je     0x4fef6a
  4feeb7:	8d 4c 24 14          	lea    ecx,[esp+0x14]
  4feebb:	8d 54 24 10          	lea    edx,[esp+0x10]
  4feebf:	51                   	push   ecx
  4feec0:	8b 4c 24 60          	mov    ecx,DWORD PTR [esp+0x60]
  4feec4:	8d 44 24 1c          	lea    eax,[esp+0x1c]
  4feec8:	52                   	push   edx
  4feec9:	50                   	push   eax
  4feeca:	51                   	push   ecx
  4feecb:	51                   	push   ecx
  4feecc:	8d 54 24 30          	lea    edx,[esp+0x30]
  4feed0:	8b cc                	mov    ecx,esp
  4feed2:	52                   	push   edx
  4feed3:	e8 e8 ff f0 ff       	call   0x40eec0
  4feed8:	51                   	push   ecx
  4feed9:	8d 44 24 3c          	lea    eax,[esp+0x3c]
  4feedd:	8b cc                	mov    ecx,esp
  4feedf:	50                   	push   eax
  4feee0:	e8 eb f9 f0 ff       	call   0x40e8d0
  4feee5:	51                   	push   ecx
  4feee6:	8d 54 24 3c          	lea    edx,[esp+0x3c]
  4feeea:	8b cc                	mov    ecx,esp
  4feeec:	52                   	push   edx
  4feeed:	e8 ce f3 f0 ff       	call   0x40e2c0
  4feef2:	8b cd                	mov    ecx,ebp
  4feef4:	e8 87 8f 01 00       	call   0x517e80
  4feef9:	66 8b 4c 24 18       	mov    cx,WORD PTR [esp+0x18]
  4feefe:	66 8b 54 24 10       	mov    dx,WORD PTR [esp+0x10]
  4fef03:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4fef07:	c7 44 24 30 05 00 00 00 	mov    DWORD PTR [esp+0x30],0x5
  4fef0f:	c7 44 24 34 15 00 00 00 	mov    DWORD PTR [esp+0x34],0x15
  4fef17:	88 5c 24 40          	mov    BYTE PTR [esp+0x40],bl
  4fef1b:	88 5c 24 3f          	mov    BYTE PTR [esp+0x3f],bl
  4fef1f:	8a 45 00             	mov    al,BYTE PTR [ebp+0x0]
  4fef22:	66 89 4c 24 39       	mov    WORD PTR [esp+0x39],cx
  4fef27:	8d 4c 24 2c          	lea    ecx,[esp+0x2c]
  4fef2b:	88 44 24 38          	mov    BYTE PTR [esp+0x38],al
  4fef2f:	66 8b 44 24 14       	mov    ax,WORD PTR [esp+0x14]
  4fef34:	51                   	push   ecx
  4fef35:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4fef3a:	89 5c 24 60          	mov    DWORD PTR [esp+0x60],ebx
  4fef3e:	66 89 54 24 3f       	mov    WORD PTR [esp+0x3f],dx
  4fef43:	66 89 44 24 41       	mov    WORD PTR [esp+0x41],ax
  4fef48:	e8 83 8c 03 00       	call   0x537bd0
  4fef4d:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4fef53:	74 15                	je     0x4fef6a
  4fef55:	8b 95 a8 00 00 00    	mov    edx,DWORD PTR [ebp+0xa8]
  4fef5b:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4fef60:	52                   	push   edx
  4fef61:	6a ff                	push   0xffffffff
  4fef63:	6a 05                	push   0x5
  4fef65:	e8 f6 d0 06 00       	call   0x56c060
  4fef6a:	8b 74 24 28          	mov    esi,DWORD PTR [esp+0x28]
  4fef6e:	b9 58 ef 6d 00       	mov    ecx,0x6def58
  4fef73:	e8 98 b7 02 00       	call   0x52a710
  4fef78:	8b e8                	mov    ebp,eax
  4fef7a:	3b eb                	cmp    ebp,ebx
  4fef7c:	0f 85 3f fd ff ff    	jne    0x4fecc1
  4fef82:	5f                   	pop    edi
  4fef83:	5e                   	pop    esi
  4fef84:	5d                   	pop    ebp
  4fef85:	5b                   	pop    ebx
  4fef86:	81 c4 30 01 00 00    	add    esp,0x130
  4fef8c:	c2 2c 00             	ret    0x2c
  4fef8f:	8d 44 24 14          	lea    eax,[esp+0x14]
  4fef93:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4fef97:	50                   	push   eax
  4fef98:	8b 44 24 5c          	mov    eax,DWORD PTR [esp+0x5c]
  4fef9c:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4fefa0:	51                   	push   ecx
  4fefa1:	52                   	push   edx
  4fefa2:	50                   	push   eax
  4fefa3:	51                   	push   ecx
  4fefa4:	8b cc                	mov    ecx,esp
  4fefa6:	56                   	push   esi
  4fefa7:	e8 04 ff f0 ff       	call   0x40eeb0
  4fefac:	8b 15 84 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5484
  4fefb2:	51                   	push   ecx
  4fefb3:	8b cc                	mov    ecx,esp
  4fefb5:	52                   	push   edx
  4fefb6:	e8 e5 f8 f0 ff       	call   0x40e8a0
  4fefbb:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4fefc0:	51                   	push   ecx
  4fefc1:	8b cc                	mov    ecx,esp
  4fefc3:	50                   	push   eax
  4fefc4:	e8 c7 f2 f0 ff       	call   0x40e290
  4fefc9:	8b cd                	mov    ecx,ebp
  4fefcb:	e8 80 8a 01 00       	call   0x517a50
  4fefd0:	6a 10                	push   0x10
  4fefd2:	89 5c 24 5c          	mov    DWORD PTR [esp+0x5c],ebx
  4fefd6:	ff 15 04 52 5c 00    	call   DWORD PTR ds:0x5c5204
  4fefdc:	66 85 c0             	test   ax,ax
  4fefdf:	7d 4d                	jge    0x4ff02e
  4fefe1:	66 8b 54 24 18       	mov    dx,WORD PTR [esp+0x18]
  4fefe6:	66 8b 44 24 10       	mov    ax,WORD PTR [esp+0x10]
  4fefeb:	89 5c 24 44          	mov    DWORD PTR [esp+0x44],ebx
  4fefef:	c7 44 24 48 0c 00 00 00 	mov    DWORD PTR [esp+0x48],0xc
  4feff7:	c7 44 24 4c 14 00 00 00 	mov    DWORD PTR [esp+0x4c],0x14
  4fefff:	8a 4d 00             	mov    cl,BYTE PTR [ebp+0x0]
  4ff002:	88 4c 24 50          	mov    BYTE PTR [esp+0x50],cl
  4ff006:	66 8b 4c 24 14       	mov    cx,WORD PTR [esp+0x14]
  4ff00b:	66 89 54 24 51       	mov    WORD PTR [esp+0x51],dx
  4ff010:	8d 54 24 44          	lea    edx,[esp+0x44]
  4ff014:	66 89 4c 24 55       	mov    WORD PTR [esp+0x55],cx
  4ff019:	52                   	push   edx
  4ff01a:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4ff01f:	66 89 44 24 57       	mov    WORD PTR [esp+0x57],ax
  4ff024:	e8 a7 8b 03 00       	call   0x537bd0
  4ff029:	e9 40 ff ff ff       	jmp    0x4fef6e
  4ff02e:	66 8b 4c 24 18       	mov    cx,WORD PTR [esp+0x18]
  4ff033:	66 8b 54 24 10       	mov    dx,WORD PTR [esp+0x10]
  4ff038:	89 5c 24 2c          	mov    DWORD PTR [esp+0x2c],ebx
  4ff03c:	c7 44 24 30 05 00 00 00 	mov    DWORD PTR [esp+0x30],0x5
  4ff044:	c7 44 24 34 15 00 00 00 	mov    DWORD PTR [esp+0x34],0x15
  4ff04c:	88 5c 24 40          	mov    BYTE PTR [esp+0x40],bl
  4ff050:	88 5c 24 3f          	mov    BYTE PTR [esp+0x3f],bl
  4ff054:	8a 45 00             	mov    al,BYTE PTR [ebp+0x0]
  4ff057:	66 89 4c 24 39       	mov    WORD PTR [esp+0x39],cx
  4ff05c:	8d 4c 24 2c          	lea    ecx,[esp+0x2c]
  4ff060:	88 44 24 38          	mov    BYTE PTR [esp+0x38],al
  4ff064:	66 8b 44 24 14       	mov    ax,WORD PTR [esp+0x14]
  4ff069:	51                   	push   ecx
  4ff06a:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4ff06f:	66 89 54 24 3f       	mov    WORD PTR [esp+0x3f],dx
  4ff074:	66 89 44 24 41       	mov    WORD PTR [esp+0x41],ax
  4ff079:	e8 52 8b 03 00       	call   0x537bd0
  4ff07e:	39 1d 90 3a 6f 00    	cmp    DWORD PTR ds:0x6f3a90,ebx
  4ff084:	0f 84 e4 fe ff ff    	je     0x4fef6e
  4ff08a:	8b 95 a8 00 00 00    	mov    edx,DWORD PTR [ebp+0xa8]
  4ff090:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4ff095:	52                   	push   edx
  4ff096:	6a ff                	push   0xffffffff
  4ff098:	6a 04                	push   0x4
  4ff09a:	e8 c1 cf 06 00       	call   0x56c060
  4ff09f:	e9 ca fe ff ff       	jmp    0x4fef6e
  4ff0a4:	8d 44 24 14          	lea    eax,[esp+0x14]
  4ff0a8:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4ff0ac:	50                   	push   eax
  4ff0ad:	8b 44 24 5c          	mov    eax,DWORD PTR [esp+0x5c]
  4ff0b1:	8d 54 24 1c          	lea    edx,[esp+0x1c]
  4ff0b5:	51                   	push   ecx
  4ff0b6:	52                   	push   edx
  4ff0b7:	50                   	push   eax
  4ff0b8:	51                   	push   ecx
  4ff0b9:	8b cc                	mov    ecx,esp
  4ff0bb:	56                   	push   esi
  4ff0bc:	e8 ef fd f0 ff       	call   0x40eeb0
  4ff0c1:	8b 15 84 54 6c 00    	mov    edx,DWORD PTR ds:0x6c5484
  4ff0c7:	51                   	push   ecx
  4ff0c8:	8b cc                	mov    ecx,esp
  4ff0ca:	52                   	push   edx
  4ff0cb:	e8 d0 f7 f0 ff       	call   0x40e8a0
  4ff0d0:	a1 88 54 6c 00       	mov    eax,ds:0x6c5488
  4ff0d5:	51                   	push   ecx
  4ff0d6:	8b cc                	mov    ecx,esp
  4ff0d8:	50                   	push   eax
  4ff0d9:	e8 b2 f1 f0 ff       	call   0x40e290
  4ff0de:	8b cd                	mov    ecx,ebp
  4ff0e0:	e8 6b 89 01 00       	call   0x517a50
  4ff0e5:	89 5c 24 44          	mov    DWORD PTR [esp+0x44],ebx
  4ff0e9:	c7 44 24 48 0c 00 00 00 	mov    DWORD PTR [esp+0x48],0xc
  4ff0f1:	c7 44 24 4c 14 00 00 00 	mov    DWORD PTR [esp+0x4c],0x14
  4ff0f9:	8a 4d 00             	mov    cl,BYTE PTR [ebp+0x0]
  4ff0fc:	66                   	data16
  4ff0fd:	8b                   	.byte 0x8b
  4ff0fe:	54                   	push   esp
  4ff0ff:	24                   	.byte 0x24
