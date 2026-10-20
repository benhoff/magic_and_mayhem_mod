
original/Arcane_Nocd/Chaos.exe:     file format pei-i386


Disassembly of section .text:

004a4e20 <.text+0xa3e20>:
  4a4e20:	81 3d 68 2c 64 00 90 	cmp    DWORD PTR ds:0x642c68,0x190
  4a4e27:	01 00 00 
  4a4e2a:	72 05                	jb     0x4a4e31
  4a4e2c:	33 c0                	xor    eax,eax
  4a4e2e:	c2 08 00             	ret    0x8
  4a4e31:	8b 54 24 04          	mov    edx,DWORD PTR [esp+0x4]
  4a4e35:	56                   	push   esi
  4a4e36:	8d 41 11             	lea    eax,[ecx+0x11]
  4a4e39:	8b 32                	mov    esi,DWORD PTR [edx]
  4a4e3b:	89 30                	mov    DWORD PTR [eax],esi
  4a4e3d:	8b 72 04             	mov    esi,DWORD PTR [edx+0x4]
  4a4e40:	89 70 04             	mov    DWORD PTR [eax+0x4],esi
  4a4e43:	8b 72 08             	mov    esi,DWORD PTR [edx+0x8]
  4a4e46:	89 70 08             	mov    DWORD PTR [eax+0x8],esi
  4a4e49:	5e                   	pop    esi
  4a4e4a:	8b 52 0c             	mov    edx,DWORD PTR [edx+0xc]
  4a4e4d:	89 50 0c             	mov    DWORD PTR [eax+0xc],edx
  4a4e50:	8b 44 24 08          	mov    eax,DWORD PTR [esp+0x8]
  4a4e54:	89 41 08             	mov    DWORD PTR [ecx+0x8],eax
  4a4e57:	8b 15 68 2c 64 00    	mov    edx,DWORD PTR ds:0x642c68
  4a4e5d:	89 51 04             	mov    DWORD PTR [ecx+0x4],edx
  4a4e60:	a1 68 2c 64 00       	mov    eax,ds:0x642c68
  4a4e65:	40                   	inc    eax
  4a4e66:	a3 68 2c 64 00       	mov    ds:0x642c68,eax
  4a4e6b:	8b 41 04             	mov    eax,DWORD PTR [ecx+0x4]
  4a4e6e:	89 0c 85 34 20 64 00 	mov    DWORD PTR [eax*4+0x642034],ecx
  4a4e75:	b8 01 00 00 00       	mov    eax,0x1
  4a4e7a:	c2 08 00             	ret    0x8
  4a4e7d:	90                   	nop
  4a4e7e:	90                   	nop
  4a4e7f:	90                   	nop
  4a4e80:	a1 68 2c 64 00       	mov    eax,ds:0x642c68
  4a4e85:	85 c0                	test   eax,eax
  4a4e87:	74 3b                	je     0x4a4ec4
  4a4e89:	53                   	push   ebx
  4a4e8a:	48                   	dec    eax
  4a4e8b:	56                   	push   esi
  4a4e8c:	57                   	push   edi
  4a4e8d:	8d 3c 85 34 20 64 00 	lea    edi,[eax*4+0x642034]
  4a4e94:	8b 17                	mov    edx,DWORD PTR [edi]
  4a4e96:	8b 72 08             	mov    esi,DWORD PTR [edx+0x8]
  4a4e99:	85 f6                	test   esi,esi
  4a4e9b:	74 1d                	je     0x4a4eba
  4a4e9d:	8b 31                	mov    esi,DWORD PTR [ecx]
  4a4e9f:	8b 5a 11             	mov    ebx,DWORD PTR [edx+0x11]
  4a4ea2:	3b f3                	cmp    esi,ebx
  4a4ea4:	7c 14                	jl     0x4a4eba
  4a4ea6:	3b 72 19             	cmp    esi,DWORD PTR [edx+0x19]
  4a4ea9:	7d 0f                	jge    0x4a4eba
  4a4eab:	8b 71 04             	mov    esi,DWORD PTR [ecx+0x4]
  4a4eae:	8b 5a 15             	mov    ebx,DWORD PTR [edx+0x15]
  4a4eb1:	3b f3                	cmp    esi,ebx
  4a4eb3:	7c 05                	jl     0x4a4eba
  4a4eb5:	3b 72 1d             	cmp    esi,DWORD PTR [edx+0x1d]
  4a4eb8:	7c 06                	jl     0x4a4ec0
  4a4eba:	48                   	dec    eax
  4a4ebb:	83 ef 04             	sub    edi,0x4
  4a4ebe:	eb d4                	jmp    0x4a4e94
  4a4ec0:	5f                   	pop    edi
  4a4ec1:	5e                   	pop    esi
  4a4ec2:	5b                   	pop    ebx
  4a4ec3:	c3                   	ret
  4a4ec4:	83 c8 ff             	or     eax,0xffffffff
  4a4ec7:	c3                   	ret
  4a4ec8:	90                   	nop
  4a4ec9:	90                   	nop
  4a4eca:	90                   	nop
  4a4ecb:	90                   	nop
  4a4ecc:	90                   	nop
  4a4ecd:	90                   	nop
  4a4ece:	90                   	nop
  4a4ecf:	90                   	nop
  4a4ed0:	a1 68 2c 64 00       	mov    eax,ds:0x642c68
  4a4ed5:	56                   	push   esi
  4a4ed6:	33 f6                	xor    esi,esi
  4a4ed8:	3b c6                	cmp    eax,esi
  4a4eda:	74 7d                	je     0x4a4f59
  4a4edc:	76 7b                	jbe    0x4a4f59
  4a4ede:	8b 15 cc 04 5e 00    	mov    edx,DWORD PTR ds:0x5e04cc
  4a4ee4:	53                   	push   ebx
  4a4ee5:	55                   	push   ebp
  4a4ee6:	57                   	push   edi
  4a4ee7:	8b d8                	mov    ebx,eax
  4a4ee9:	83 cf ff             	or     edi,0xffffffff
  4a4eec:	8d 48 ff             	lea    ecx,[eax-0x1]
  4a4eef:	a1 34 20 64 00       	mov    eax,ds:0x642034
  4a4ef4:	3b 50 04             	cmp    edx,DWORD PTR [eax+0x4]
  4a4ef7:	75 06                	jne    0x4a4eff
  4a4ef9:	89 3d cc 04 5e 00    	mov    DWORD PTR ds:0x5e04cc,edi
  4a4eff:	3b ce                	cmp    ecx,esi
  4a4f01:	89 70 08             	mov    DWORD PTR [eax+0x8],esi
  4a4f04:	74 1b                	je     0x4a4f21
  4a4f06:	76 19                	jbe    0x4a4f21
  4a4f08:	b8 34 20 64 00       	mov    eax,0x642034
  4a4f0d:	8b d1                	mov    edx,ecx
  4a4f0f:	8b 48 04             	mov    ecx,DWORD PTR [eax+0x4]
  4a4f12:	89 08                	mov    DWORD PTR [eax],ecx
  4a4f14:	83 c0 04             	add    eax,0x4
  4a4f17:	8b 69 04             	mov    ebp,DWORD PTR [ecx+0x4]
  4a4f1a:	4d                   	dec    ebp
  4a4f1b:	4a                   	dec    edx
  4a4f1c:	89 69 04             	mov    DWORD PTR [ecx+0x4],ebp
  4a4f1f:	75 ee                	jne    0x4a4f0f
  4a4f21:	8b 15 cc 04 5e 00    	mov    edx,DWORD PTR ds:0x5e04cc
  4a4f27:	3b d6                	cmp    edx,esi
  4a4f29:	75 04                	jne    0x4a4f2f
  4a4f2b:	8b d7                	mov    edx,edi
  4a4f2d:	eb 05                	jmp    0x4a4f34
  4a4f2f:	3b d7                	cmp    edx,edi
  4a4f31:	74 07                	je     0x4a4f3a
  4a4f33:	4a                   	dec    edx
  4a4f34:	89 15 cc 04 5e 00    	mov    DWORD PTR ds:0x5e04cc,edx
  4a4f3a:	a1 68 2c 64 00       	mov    eax,ds:0x642c68
  4a4f3f:	48                   	dec    eax
  4a4f40:	4b                   	dec    ebx
  4a4f41:	a3 68 2c 64 00       	mov    ds:0x642c68,eax
  4a4f46:	75 a4                	jne    0x4a4eec
  4a4f48:	89 3d cc 04 5e 00    	mov    DWORD PTR ds:0x5e04cc,edi
  4a4f4e:	5f                   	pop    edi
  4a4f4f:	5d                   	pop    ebp
  4a4f50:	89 35 68 2c 64 00    	mov    DWORD PTR ds:0x642c68,esi
  4a4f56:	5b                   	pop    ebx
  4a4f57:	5e                   	pop    esi
  4a4f58:	c3                   	ret
  4a4f59:	89 35 68 2c 64 00    	mov    DWORD PTR ds:0x642c68,esi
  4a4f5f:	c7 05 cc 04 5e 00 ff 	mov    DWORD PTR ds:0x5e04cc,0xffffffff
  4a4f66:	ff ff ff 
  4a4f69:	5e                   	pop    esi
  4a4f6a:	c3                   	ret
  4a4f6b:	90                   	nop
  4a4f6c:	90                   	nop
  4a4f6d:	90                   	nop
  4a4f6e:	90                   	nop
  4a4f6f:	90                   	nop
  4a4f70:	83 f9 ff             	cmp    ecx,0xffffffff
  4a4f73:	0f 84 84 00 00 00    	je     0x4a4ffd
  4a4f79:	8b 15 68 2c 64 00    	mov    edx,DWORD PTR ds:0x642c68
  4a4f7f:	53                   	push   ebx
  4a4f80:	8b 1d cc 04 5e 00    	mov    ebx,DWORD PTR ds:0x5e04cc
  4a4f86:	8d 04 8d 34 20 64 00 	lea    eax,[ecx*4+0x642034]
  4a4f8d:	56                   	push   esi
  4a4f8e:	8b 30                	mov    esi,DWORD PTR [eax]
  4a4f90:	2b d1                	sub    edx,ecx
  4a4f92:	4a                   	dec    edx
  4a4f93:	3b 5e 04             	cmp    ebx,DWORD PTR [esi+0x4]
  4a4f96:	75 0a                	jne    0x4a4fa2
  4a4f98:	c7 05 cc 04 5e 00 ff 	mov    DWORD PTR ds:0x5e04cc,0xffffffff
  4a4f9f:	ff ff ff 
  4a4fa2:	85 d2                	test   edx,edx
  4a4fa4:	c7 46 08 00 00 00 00 	mov    DWORD PTR [esi+0x8],0x0
  4a4fab:	74 1c                	je     0x4a4fc9
  4a4fad:	03 d1                	add    edx,ecx
  4a4faf:	3b ca                	cmp    ecx,edx
  4a4fb1:	73 16                	jae    0x4a4fc9
  4a4fb3:	2b d1                	sub    edx,ecx
  4a4fb5:	8b f2                	mov    esi,edx
  4a4fb7:	8b 50 04             	mov    edx,DWORD PTR [eax+0x4]
  4a4fba:	89 10                	mov    DWORD PTR [eax],edx
  4a4fbc:	83 c0 04             	add    eax,0x4
  4a4fbf:	8b 5a 04             	mov    ebx,DWORD PTR [edx+0x4]
  4a4fc2:	4b                   	dec    ebx
  4a4fc3:	4e                   	dec    esi
  4a4fc4:	89 5a 04             	mov    DWORD PTR [edx+0x4],ebx
  4a4fc7:	75 ee                	jne    0x4a4fb7
  4a4fc9:	a1 cc 04 5e 00       	mov    eax,ds:0x5e04cc
  4a4fce:	5e                   	pop    esi
  4a4fcf:	3b c1                	cmp    eax,ecx
  4a4fd1:	5b                   	pop    ebx
  4a4fd2:	75 16                	jne    0x4a4fea
  4a4fd4:	a1 68 2c 64 00       	mov    eax,ds:0x642c68
  4a4fd9:	c7 05 cc 04 5e 00 ff 	mov    DWORD PTR ds:0x5e04cc,0xffffffff
  4a4fe0:	ff ff ff 
  4a4fe3:	48                   	dec    eax
  4a4fe4:	a3 68 2c 64 00       	mov    ds:0x642c68,eax
  4a4fe9:	c3                   	ret
  4a4fea:	72 0b                	jb     0x4a4ff7
  4a4fec:	83 f8 ff             	cmp    eax,0xffffffff
  4a4fef:	74 06                	je     0x4a4ff7
  4a4ff1:	48                   	dec    eax
  4a4ff2:	a3 cc 04 5e 00       	mov    ds:0x5e04cc,eax
  4a4ff7:	ff 0d 68 2c 64 00    	dec    DWORD PTR ds:0x642c68
  4a4ffd:	c3                   	ret
  4a4ffe:	90                   	nop
  4a4fff:	90                   	nop
