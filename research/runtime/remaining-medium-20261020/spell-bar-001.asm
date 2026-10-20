
/home/hoff/swdev/magic_and_mayhem_mod/working/worktrees/remaining-medium-20261020/original/Arcane_Nocd/Chaos.exe:     file format pei-i386


Disassembly of section .text:

004d4d60 <.text+0xd3d60>:
  4d4d60:	83 ec 14             	sub    esp,0x14
  4d4d63:	55                   	push   ebp
  4d4d64:	56                   	push   esi
  4d4d65:	8b f1                	mov    esi,ecx
  4d4d67:	57                   	push   edi
  4d4d68:	33 c9                	xor    ecx,ecx
  4d4d6a:	8b 86 18 15 00 00    	mov    eax,DWORD PTR [esi+0x1518]
  4d4d70:	89 4c 24 0c          	mov    DWORD PTR [esp+0xc],ecx
  4d4d74:	3b c1                	cmp    eax,ecx
  4d4d76:	0f 84 86 03 00 00    	je     0x4d5102
  4d4d7c:	8b 6c 24 24          	mov    ebp,DWORD PTR [esp+0x24]
  4d4d80:	8b c5                	mov    eax,ebp
  4d4d82:	c1 e0 05             	shl    eax,0x5
  4d4d85:	03 c5                	add    eax,ebp
  4d4d87:	8d 04 40             	lea    eax,[eax+eax*2]
  4d4d8a:	8d 3c 46             	lea    edi,[esi+eax*2]
  4d4d8d:	8b 84 46 b5 01 00 00 	mov    eax,DWORD PTR [esi+eax*2+0x1b5]
  4d4d94:	83 f8 01             	cmp    eax,0x1
  4d4d97:	75 6e                	jne    0x4d4e07
  4d4d99:	8b 87 b1 01 00 00    	mov    eax,DWORD PTR [edi+0x1b1]
  4d4d9f:	83 f8 10             	cmp    eax,0x10
  4d4da2:	0f 84 5a 03 00 00    	je     0x4d5102
  4d4da8:	83 f8 34             	cmp    eax,0x34
  4d4dab:	0f 84 51 03 00 00    	je     0x4d5102
  4d4db1:	83 f8 33             	cmp    eax,0x33
  4d4db4:	0f 84 48 03 00 00    	je     0x4d5102
  4d4dba:	83 f8 ff             	cmp    eax,0xffffffff
  4d4dbd:	0f 84 3f 03 00 00    	je     0x4d5102
  4d4dc3:	8a 87 bd 01 00 00    	mov    al,BYTE PTR [edi+0x1bd]
  4d4dc9:	89 4c 24 10          	mov    DWORD PTR [esp+0x10],ecx
  4d4dcd:	8b 8e ec 18 00 00    	mov    ecx,DWORD PTR [esi+0x18ec]
  4d4dd3:	c7 44 24 14 08 00 00 00 	mov    DWORD PTR [esp+0x14],0x8
  4d4ddb:	c7 44 24 18 0e 00 00 00 	mov    DWORD PTR [esp+0x18],0xe
  4d4de3:	8a 11                	mov    dl,BYTE PTR [ecx]
  4d4de5:	8d 4c 24 10          	lea    ecx,[esp+0x10]
  4d4de9:	51                   	push   ecx
  4d4dea:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4d4def:	88 54 24 20          	mov    BYTE PTR [esp+0x20],dl
  4d4df3:	88 44 24 21          	mov    BYTE PTR [esp+0x21],al
  4d4df7:	e8 d4 2d 06 00       	call   0x537bd0
  4d4dfc:	5f                   	pop    edi
  4d4dfd:	5e                   	pop    esi
  4d4dfe:	33 c0                	xor    eax,eax
  4d4e00:	5d                   	pop    ebp
  4d4e01:	83 c4 14             	add    esp,0x14
  4d4e04:	c2 04 00             	ret    0x4
  4d4e07:	3b c1                	cmp    eax,ecx
  4d4e09:	0f 85 f3 02 00 00    	jne    0x4d5102
  4d4e0f:	8b 8e 08 19 00 00    	mov    ecx,DWORD PTR [esi+0x1908]
  4d4e15:	53                   	push   ebx
  4d4e16:	83 cb ff             	or     ebx,0xffffffff
  4d4e19:	3b cb                	cmp    ecx,ebx
  4d4e1b:	74 64                	je     0x4d4e81
  4d4e1d:	8b c1                	mov    eax,ecx
  4d4e1f:	c1 e0 05             	shl    eax,0x5
  4d4e22:	03 c1                	add    eax,ecx
  4d4e24:	8d 14 40             	lea    edx,[eax+eax*2]
  4d4e27:	8b 84 56 fc 00 00 00 	mov    eax,DWORD PTR [esi+edx*2+0xfc]
  4d4e2e:	8d 8c 56 fc 00 00 00 	lea    ecx,[esi+edx*2+0xfc]
  4d4e35:	ff 50 24             	call   DWORD PTR [eax+0x24]
  4d4e38:	a1 d5 e6 6d 00       	mov    eax,ds:0x6de6d5
  4d4e3d:	8b 8e 08 19 00 00    	mov    ecx,DWORD PTR [esi+0x1908]
  4d4e43:	f7 d8                	neg    eax
  4d4e45:	1b c0                	sbb    eax,eax
  4d4e47:	83 e0 03             	and    eax,0x3
  4d4e4a:	83 c0 08             	add    eax,0x8
  4d4e4d:	3b c8                	cmp    ecx,eax
  4d4e4f:	7e 2a                	jle    0x4d4e7b
  4d4e51:	2b c8                	sub    ecx,eax
  4d4e53:	8b c1                	mov    eax,ecx
  4d4e55:	c1 e0 05             	shl    eax,0x5
  4d4e58:	03 c1                	add    eax,ecx
  4d4e5a:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4d4e5d:	8d 1c 4e             	lea    ebx,[esi+ecx*2]
  4d4e60:	8b 93 fc 00 00 00    	mov    edx,DWORD PTR [ebx+0xfc]
  4d4e66:	8d 8b fc 00 00 00    	lea    ecx,[ebx+0xfc]
  4d4e6c:	ff 52 10             	call   DWORD PTR [edx+0x10]
  4d4e6f:	8b 43 36             	mov    eax,DWORD PTR [ebx+0x36]
  4d4e72:	8d 4b 36             	lea    ecx,[ebx+0x36]
  4d4e75:	ff 50 10             	call   DWORD PTR [eax+0x10]
  4d4e78:	83 cb ff             	or     ebx,0xffffffff
  4d4e7b:	89 9e 08 19 00 00    	mov    DWORD PTR [esi+0x1908],ebx
  4d4e81:	8b 8e 0c 19 00 00    	mov    ecx,DWORD PTR [esi+0x190c]
  4d4e87:	3b cb                	cmp    ecx,ebx
  4d4e89:	0f 84 87 00 00 00    	je     0x4d4f16
  4d4e8f:	8b c1                	mov    eax,ecx
  4d4e91:	c1 e0 05             	shl    eax,0x5
  4d4e94:	03 c1                	add    eax,ecx
  4d4e96:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4d4e99:	8b 94 4e 96 15 00 00 	mov    edx,DWORD PTR [esi+ecx*2+0x1596]
  4d4ea0:	8d 8c 4e 96 15 00 00 	lea    ecx,[esi+ecx*2+0x1596]
  4d4ea7:	ff 52 24             	call   DWORD PTR [edx+0x24]
  4d4eaa:	8b 86 0c 19 00 00    	mov    eax,DWORD PTR [esi+0x190c]
  4d4eb0:	85 c0                	test   eax,eax
  4d4eb2:	75 1a                	jne    0x4d4ece
  4d4eb4:	83 be 21 19 00 00 02 	cmp    DWORD PTR [esi+0x1921],0x2
  4d4ebb:	72 53                	jb     0x4d4f10
  4d4ebd:	8b 86 5c 16 00 00    	mov    eax,DWORD PTR [esi+0x165c]
  4d4ec3:	8d 8e 5c 16 00 00    	lea    ecx,[esi+0x165c]
  4d4ec9:	ff 50 10             	call   DWORD PTR [eax+0x10]
  4d4ecc:	eb 42                	jmp    0x4d4f10
  4d4ece:	83 f8 01             	cmp    eax,0x1
  4d4ed1:	75 29                	jne    0x4d4efc
  4d4ed3:	8b 96 96 15 00 00    	mov    edx,DWORD PTR [esi+0x1596]
  4d4ed9:	8d 8e 96 15 00 00    	lea    ecx,[esi+0x1596]
  4d4edf:	ff 52 10             	call   DWORD PTR [edx+0x10]
  4d4ee2:	83 be 21 19 00 00 03 	cmp    DWORD PTR [esi+0x1921],0x3
  4d4ee9:	72 25                	jb     0x4d4f10
  4d4eeb:	8b 86 22 17 00 00    	mov    eax,DWORD PTR [esi+0x1722]
  4d4ef1:	8d 8e 22 17 00 00    	lea    ecx,[esi+0x1722]
  4d4ef7:	ff 50 10             	call   DWORD PTR [eax+0x10]
  4d4efa:	eb 14                	jmp    0x4d4f10
  4d4efc:	83 f8 02             	cmp    eax,0x2
  4d4eff:	75 0f                	jne    0x4d4f10
  4d4f01:	8b 96 5c 16 00 00    	mov    edx,DWORD PTR [esi+0x165c]
  4d4f07:	8d 8e 5c 16 00 00    	lea    ecx,[esi+0x165c]
  4d4f0d:	ff 52 10             	call   DWORD PTR [edx+0x10]
  4d4f10:	89 9e 0c 19 00 00    	mov    DWORD PTR [esi+0x190c],ebx
  4d4f16:	39 9f b1 01 00 00    	cmp    DWORD PTR [edi+0x1b1],ebx
  4d4f1c:	74 62                	je     0x4d4f80
  4d4f1e:	8b 87 b5 01 00 00    	mov    eax,DWORD PTR [edi+0x1b5]
  4d4f24:	85 c0                	test   eax,eax
  4d4f26:	75 58                	jne    0x4d4f80
  4d4f28:	b9 78 bb 6c 00       	mov    ecx,0x6cbb78
  4d4f2d:	e8 6e 53 f9 ff       	call   0x46a2a0
  4d4f32:	8b 87 b1 01 00 00    	mov    eax,DWORD PTR [edi+0x1b1]
  4d4f38:	8b 8e ec 18 00 00    	mov    ecx,DWORD PTR [esi+0x18ec]
  4d4f3e:	50                   	push   eax
  4d4f3f:	e8 8c 4c 03 00       	call   0x509bd0
  4d4f44:	8b 97 fc 00 00 00    	mov    edx,DWORD PTR [edi+0xfc]
  4d4f4a:	8d 9f fc 00 00 00    	lea    ebx,[edi+0xfc]
  4d4f50:	8b cb                	mov    ecx,ebx
  4d4f52:	ff 52 20             	call   DWORD PTR [edx+0x20]
  4d4f55:	8b 03                	mov    eax,DWORD PTR [ebx]
  4d4f57:	8b cb                	mov    ecx,ebx
  4d4f59:	ff 50 10             	call   DWORD PTR [eax+0x10]
  4d4f5c:	89 ae 08 19 00 00    	mov    DWORD PTR [esi+0x1908],ebp
  4d4f62:	a1 90 3a 6f 00       	mov    eax,ds:0x6f3a90
  4d4f67:	85 c0                	test   eax,eax
  4d4f69:	74 15                	je     0x4d4f80
  4d4f6b:	8b 8f b1 01 00 00    	mov    ecx,DWORD PTR [edi+0x1b1]
  4d4f71:	6a ff                	push   0xffffffff
  4d4f73:	51                   	push   ecx
  4d4f74:	6a 01                	push   0x1
  4d4f76:	b9 70 3a 6f 00       	mov    ecx,0x6f3a70
  4d4f7b:	e8 e0 70 09 00       	call   0x56c060
  4d4f80:	33 ed                	xor    ebp,ebp
  4d4f82:	8d 9e 96 15 00 00    	lea    ebx,[esi+0x1596]
  4d4f88:	8b 86 21 19 00 00    	mov    eax,DWORD PTR [esi+0x1921]
  4d4f8e:	8b 8e e8 17 00 00    	mov    ecx,DWORD PTR [esi+0x17e8]
  4d4f94:	3b c1                	cmp    eax,ecx
  4d4f96:	72 02                	jb     0x4d4f9a
  4d4f98:	8b c1                	mov    eax,ecx
  4d4f9a:	3b e8                	cmp    ebp,eax
  4d4f9c:	73 35                	jae    0x4d4fd3
  4d4f9e:	8b 93 b5 00 00 00    	mov    edx,DWORD PTR [ebx+0xb5]
  4d4fa4:	8b 87 b1 01 00 00    	mov    eax,DWORD PTR [edi+0x1b1]
  4d4faa:	3b d0                	cmp    edx,eax
  4d4fac:	75 1c                	jne    0x4d4fca
  4d4fae:	8b 03                	mov    eax,DWORD PTR [ebx]
  4d4fb0:	8b cb                	mov    ecx,ebx
  4d4fb2:	ff 50 20             	call   DWORD PTR [eax+0x20]
  4d4fb5:	8b 13                	mov    edx,DWORD PTR [ebx]
  4d4fb7:	8b cb                	mov    ecx,ebx
  4d4fb9:	ff 52 10             	call   DWORD PTR [edx+0x10]
  4d4fbc:	89 ae 0c 19 00 00    	mov    DWORD PTR [esi+0x190c],ebp
  4d4fc2:	c7 44 24 10 01 00 00 00 	mov    DWORD PTR [esp+0x10],0x1
  4d4fca:	45                   	inc    ebp
  4d4fcb:	81 c3 c6 00 00 00    	add    ebx,0xc6
  4d4fd1:	eb b5                	jmp    0x4d4f88
  4d4fd3:	8b 44 24 10          	mov    eax,DWORD PTR [esp+0x10]
  4d4fd7:	5b                   	pop    ebx
  4d4fd8:	85 c0                	test   eax,eax
  4d4fda:	0f 85 22 01 00 00    	jne    0x4d5102
  4d4fe0:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d4fe6:	8d 87 fc 00 00 00    	lea    eax,[edi+0xfc]
  4d4fec:	50                   	push   eax
  4d4fed:	8b c1                	mov    eax,ecx
  4d4fef:	c1 e0 05             	shl    eax,0x5
  4d4ff2:	03 c1                	add    eax,ecx
  4d4ff4:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4d4ff7:	8d 8c 4e 96 15 00 00 	lea    ecx,[esi+ecx*2+0x1596]
  4d4ffe:	e8 bd 68 ff ff       	call   0x4cb8c0
  4d5003:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d5009:	8b c1                	mov    eax,ecx
  4d500b:	c1 e0 05             	shl    eax,0x5
  4d500e:	03 c1                	add    eax,ecx
  4d5010:	8d 14 40             	lea    edx,[eax+eax*2]
  4d5013:	8b 87 b1 01 00 00    	mov    eax,DWORD PTR [edi+0x1b1]
  4d5019:	89 84 56 4b 16 00 00 	mov    DWORD PTR [esi+edx*2+0x164b],eax
  4d5020:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d5026:	8b c1                	mov    eax,ecx
  4d5028:	8b 97 b5 01 00 00    	mov    edx,DWORD PTR [edi+0x1b5]
  4d502e:	c1 e0 05             	shl    eax,0x5
  4d5031:	03 c1                	add    eax,ecx
  4d5033:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4d5036:	89 94 4e 4f 16 00 00 	mov    DWORD PTR [esi+ecx*2+0x164f],edx
  4d503d:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d5043:	8b 97 b1 01 00 00    	mov    edx,DWORD PTR [edi+0x1b1]
  4d5049:	8b c1                	mov    eax,ecx
  4d504b:	c1 e0 05             	shl    eax,0x5
  4d504e:	03 c1                	add    eax,ecx
  4d5050:	83 c2 05             	add    edx,0x5
  4d5053:	8d 04 40             	lea    eax,[eax+eax*2]
  4d5056:	89 94 46 cb 15 00 00 	mov    DWORD PTR [esi+eax*2+0x15cb],edx
  4d505d:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d5063:	8b c1                	mov    eax,ecx
  4d5065:	c1 e0 05             	shl    eax,0x5
  4d5068:	03 c1                	add    eax,ecx
  4d506a:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4d506d:	8b 94 4e 96 15 00 00 	mov    edx,DWORD PTR [esi+ecx*2+0x1596]
  4d5074:	8d 8c 4e 96 15 00 00 	lea    ecx,[esi+ecx*2+0x1596]
  4d507b:	ff 52 10             	call   DWORD PTR [edx+0x10]
  4d507e:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d5084:	8b c1                	mov    eax,ecx
  4d5086:	c1 e0 05             	shl    eax,0x5
  4d5089:	03 c1                	add    eax,ecx
  4d508b:	8b 4c 24 24          	mov    ecx,DWORD PTR [esp+0x24]
  4d508f:	8d 04 40             	lea    eax,[eax+eax*2]
  4d5092:	89 8c 46 57 16 00 00 	mov    DWORD PTR [esi+eax*2+0x1657],ecx
  4d5099:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d509f:	8b c1                	mov    eax,ecx
  4d50a1:	c1 e0 05             	shl    eax,0x5
  4d50a4:	03 c1                	add    eax,ecx
  4d50a6:	8d 14 40             	lea    edx,[eax+eax*2]
  4d50a9:	8b 84 56 96 15 00 00 	mov    eax,DWORD PTR [esi+edx*2+0x1596]
  4d50b0:	8d 8c 56 96 15 00 00 	lea    ecx,[esi+edx*2+0x1596]
  4d50b7:	ff 50 20             	call   DWORD PTR [eax+0x20]
  4d50ba:	8b 8e 18 19 00 00    	mov    ecx,DWORD PTR [esi+0x1918]
  4d50c0:	8b c1                	mov    eax,ecx
  4d50c2:	c1 e0 05             	shl    eax,0x5
  4d50c5:	03 c1                	add    eax,ecx
  4d50c7:	8d 0c 40             	lea    ecx,[eax+eax*2]
  4d50ca:	8b 94 4e 96 15 00 00 	mov    edx,DWORD PTR [esi+ecx*2+0x1596]
  4d50d1:	8d 8c 4e 96 15 00 00 	lea    ecx,[esi+ecx*2+0x1596]
  4d50d8:	ff 52 10             	call   DWORD PTR [edx+0x10]
  4d50db:	8b 86 18 19 00 00    	mov    eax,DWORD PTR [esi+0x1918]
  4d50e1:	8b 8e e8 17 00 00    	mov    ecx,DWORD PTR [esi+0x17e8]
  4d50e7:	89 86 0c 19 00 00    	mov    DWORD PTR [esi+0x190c],eax
  4d50ed:	40                   	inc    eax
  4d50ee:	3b c1                	cmp    eax,ecx
  4d50f0:	89 86 18 19 00 00    	mov    DWORD PTR [esi+0x1918],eax
  4d50f6:	72 0a                	jb     0x4d5102
  4d50f8:	c7 86 18 19 00 00 00 00 00 00 	mov    DWORD PTR [esi+0x1918],0x0
  4d5102:	5f                   	pop    edi
  4d5103:	5e                   	pop    esi
  4d5104:	33 c0                	xor    eax,eax
  4d5106:	5d                   	pop    ebp
  4d5107:	83 c4 14             	add    esp,0x14
  4d510a:	c2 04 00             	ret    0x4
  4d510d:	90                   	nop
  4d510e:	90                   	nop
  4d510f:	90                   	nop
  4d5110:	83 ec 10             	sub    esp,0x10
  4d5113:	53                   	push   ebx
  4d5114:	8b 5c 24 18          	mov    ebx,DWORD PTR [esp+0x18]
  4d5118:	8b c3                	mov    eax,ebx
  4d511a:	56                   	push   esi
  4d511b:	c1 e0 05             	shl    eax,0x5
  4d511e:	03 c3                	add    eax,ebx
  4d5120:	8d 04 40             	lea    eax,[eax+eax*2]
  4d5123:	8d 34 41             	lea    esi,[ecx+eax*2]
  4d5126:	8b 84 41 b5 01 00 00 	mov    eax,DWORD PTR [ecx+eax*2+0x1b5]
  4d512d:	83 f8 01             	cmp    eax,0x1
  4d5130:	75 5a                	jne    0x4d518c
  4d5132:	a1 20 45 64 00       	mov    eax,ds:0x644520
  4d5137:	83 f8 ff             	cmp    eax,0xffffffff
  4d513a:	74 50                	je     0x4d518c
  4d513c:	50                   	push   eax
  4d513d:	b9 80 3a 64 00       	mov    ecx,0x643a80
  4d5142:	c7 44 24 0c 00 00 00 00 	mov    DWORD PTR [esp+0xc],0x0
  4d514a:	c7 44 24 10 09 00 00 00 	mov    DWORD PTR [esp+0x10],0x9
  4d5152:	c7 44 24 14 10 00 00 00 	mov    DWORD PTR [esp+0x14],0x10
  4d515a:	e8 11 17 f9 ff       	call   0x466870
  4d515f:	8a 08                	mov    cl,BYTE PTR [eax]
  4d5161:	8a 96 bd 01 00 00    	mov    dl,BYTE PTR [esi+0x1bd]
  4d5167:	8a 86 b1 01 00 00    	mov    al,BYTE PTR [esi+0x1b1]
  4d516d:	88 4c 24 14          	mov    BYTE PTR [esp+0x14],cl
  4d5171:	8d 4c 24 08          	lea    ecx,[esp+0x8]
  4d5175:	88 54 24 15          	mov    BYTE PTR [esp+0x15],dl
  4d5179:	51                   	push   ecx
  4d517a:	b9 98 28 6a 00       	mov    ecx,0x6a2898
  4d517f:	88 44 24 1a          	mov    BYTE PTR [esp+0x1a],al
  4d5183:	88 5c 24 1b          	mov    BYTE PTR [esp+0x1b],bl
  4d5187:	e8 44 2a 06 00       	call   0x537bd0
  4d518c:	5e                   	pop    esi
  4d518d:	33 c0                	xor    eax,eax
  4d518f:	5b                   	pop    ebx
  4d5190:	83 c4 10             	add    esp,0x10
  4d5193:	c2 04 00             	ret    0x4
  4d5196:	90                   	nop
  4d5197:	90                   	nop
  4d5198:	90                   	nop
  4d5199:	90                   	nop
  4d519a:	90                   	nop
  4d519b:	90                   	nop
  4d519c:	90                   	nop
  4d519d:	90                   	nop
  4d519e:	90                   	nop
  4d519f:	90                   	nop
