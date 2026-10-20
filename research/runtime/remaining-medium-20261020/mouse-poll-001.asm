
/home/hoff/swdev/magic_and_mayhem_mod/working/worktrees/remaining-medium-20261020/original/Arcane_Nocd/Chaos.exe:     file format pei-i386


Disassembly of section .text:

004a3800 <.text+0xa2800>:
  4a3800:	53                   	push   ebx
  4a3801:	55                   	push   ebp
  4a3802:	56                   	push   esi
  4a3803:	8b 35 88 1a 64 00    	mov    esi,DWORD PTR ds:0x641a88
  4a3809:	57                   	push   edi
  4a380a:	8d 46 ff             	lea    eax,[esi-0x1]
  4a380d:	83 f8 ff             	cmp    eax,0xffffffff
  4a3810:	75 05                	jne    0x4a3817
  4a3812:	b8 1f 00 00 00       	mov    eax,0x1f
  4a3817:	8d 04 80             	lea    eax,[eax+eax*4]
  4a381a:	8d 34 b6             	lea    esi,[esi+esi*4]
  4a381d:	81 e1 ff ff 00 00    	and    ecx,0xffff
  4a3823:	8b ea                	mov    ebp,edx
  4a3825:	8d 3c c5 b8 1a 64 00 	lea    edi,[eax*8+0x641ab8]
  4a382c:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
  4a3830:	8d 34 f5 b8 1a 64 00 	lea    esi,[esi*8+0x641ab8]
  4a3837:	25 ff ff 00 00       	and    eax,0xffff
  4a383c:	89 44 24 14          	mov    DWORD PTR [esp+0x14],eax
  4a3840:	8b d1                	mov    edx,ecx
  4a3842:	89 46 04             	mov    DWORD PTR [esi+0x4],eax
  4a3845:	8b c1                	mov    eax,ecx
  4a3847:	d1 e8                	shr    eax,1
  4a3849:	81 e5 ff ff 00 00    	and    ebp,0xffff
  4a384f:	83 e2 01             	and    edx,0x1
  4a3852:	83 e0 01             	and    eax,0x1
  4a3855:	89 2e                	mov    DWORD PTR [esi],ebp
  4a3857:	89 56 0c             	mov    DWORD PTR [esi+0xc],edx
  4a385a:	89 46 10             	mov    DWORD PTR [esi+0x10],eax
  4a385d:	33 db                	xor    ebx,ebx
  4a385f:	3b 57 0c             	cmp    edx,DWORD PTR [edi+0xc]
  4a3862:	0f 95 c3             	setne  bl
  4a3865:	89 5e 14             	mov    DWORD PTR [esi+0x14],ebx
  4a3868:	8b 5f 10             	mov    ebx,DWORD PTR [edi+0x10]
  4a386b:	33 d2                	xor    edx,edx
  4a386d:	3b c3                	cmp    eax,ebx
  4a386f:	0f 95 c2             	setne  dl
  4a3872:	89 56 18             	mov    DWORD PTR [esi+0x18],edx
  4a3875:	8b 07                	mov    eax,DWORD PTR [edi]
  4a3877:	33 db                	xor    ebx,ebx
  4a3879:	3b e8                	cmp    ebp,eax
  4a387b:	75 0f                	jne    0x4a388c
  4a387d:	8b 44 24 14          	mov    eax,DWORD PTR [esp+0x14]
  4a3881:	8b 57 04             	mov    edx,DWORD PTR [edi+0x4]
  4a3884:	3b c2                	cmp    eax,edx
  4a3886:	75 04                	jne    0x4a388c
  4a3888:	33 c0                	xor    eax,eax
  4a388a:	eb 05                	jmp    0x4a3891
  4a388c:	b8 01 00 00 00       	mov    eax,0x1
  4a3891:	8b d1                	mov    edx,ecx
  4a3893:	6a 12                	push   0x12
  4a3895:	c1 ea 02             	shr    edx,0x2
  4a3898:	c1 e9 03             	shr    ecx,0x3
  4a389b:	83 e2 01             	and    edx,0x1
  4a389e:	83 e1 01             	and    ecx,0x1
  4a38a1:	89 46 08             	mov    DWORD PTR [esi+0x8],eax
  4a38a4:	89 56 1c             	mov    DWORD PTR [esi+0x1c],edx
  4a38a7:	89 4e 20             	mov    DWORD PTR [esi+0x20],ecx
  4a38aa:	ff 15 04 52 5c 00    	call   DWORD PTR ds:0x5c5204
  4a38b0:	33 c9                	xor    ecx,ecx
  4a38b2:	66 85 c0             	test   ax,ax
  4a38b5:	0f 9c c1             	setl   cl
  4a38b8:	89 4e 24             	mov    DWORD PTR [esi+0x24],ecx
  4a38bb:	a1 88 1a 64 00       	mov    eax,ds:0x641a88
  4a38c0:	40                   	inc    eax
  4a38c1:	83 f8 20             	cmp    eax,0x20
  4a38c4:	a3 88 1a 64 00       	mov    ds:0x641a88,eax
  4a38c9:	75 07                	jne    0x4a38d2
  4a38cb:	33 c0                	xor    eax,eax
  4a38cd:	a3 88 1a 64 00       	mov    ds:0x641a88,eax
  4a38d2:	8b 0d bc 1f 64 00    	mov    ecx,DWORD PTR ds:0x641fbc
  4a38d8:	41                   	inc    ecx
  4a38d9:	83 f9 20             	cmp    ecx,0x20
  4a38dc:	89 0d bc 1f 64 00    	mov    DWORD PTR ds:0x641fbc,ecx
  4a38e2:	75 06                	jne    0x4a38ea
  4a38e4:	89 1d bc 1f 64 00    	mov    DWORD PTR ds:0x641fbc,ebx
  4a38ea:	8b 0d 74 1a 64 00    	mov    ecx,DWORD PTR ds:0x641a74
  4a38f0:	3b c1                	cmp    eax,ecx
  4a38f2:	75 12                	jne    0x4a3906
  4a38f4:	41                   	inc    ecx
  4a38f5:	83 f9 20             	cmp    ecx,0x20
  4a38f8:	89 0d 74 1a 64 00    	mov    DWORD PTR ds:0x641a74,ecx
  4a38fe:	75 06                	jne    0x4a3906
  4a3900:	89 1d 74 1a 64 00    	mov    DWORD PTR ds:0x641a74,ebx
  4a3906:	5f                   	pop    edi
  4a3907:	5e                   	pop    esi
  4a3908:	5d                   	pop    ebp
  4a3909:	5b                   	pop    ebx
  4a390a:	c2 04 00             	ret    0x4
  4a390d:	90                   	nop
  4a390e:	90                   	nop
  4a390f:	90                   	nop
  4a3910:	c2 08 00             	ret    0x8
  4a3913:	90                   	nop
  4a3914:	90                   	nop
  4a3915:	90                   	nop
  4a3916:	90                   	nop
  4a3917:	90                   	nop
  4a3918:	90                   	nop
  4a3919:	90                   	nop
  4a391a:	90                   	nop
  4a391b:	90                   	nop
  4a391c:	90                   	nop
  4a391d:	90                   	nop
  4a391e:	90                   	nop
  4a391f:	90                   	nop
  4a3920:	33 c0                	xor    eax,eax
  4a3922:	c3                   	ret
  4a3923:	90                   	nop
  4a3924:	90                   	nop
  4a3925:	90                   	nop
  4a3926:	90                   	nop
  4a3927:	90                   	nop
  4a3928:	90                   	nop
  4a3929:	90                   	nop
  4a392a:	90                   	nop
  4a392b:	90                   	nop
  4a392c:	90                   	nop
  4a392d:	90                   	nop
  4a392e:	90                   	nop
  4a392f:	90                   	nop
  4a3930:	a1 74 1a 64 00       	mov    eax,ds:0x641a74
  4a3935:	57                   	push   edi
  4a3936:	8b f9                	mov    edi,ecx
  4a3938:	3b 05 88 1a 64 00    	cmp    eax,DWORD PTR ds:0x641a88
  4a393e:	75 04                	jne    0x4a3944
  4a3940:	33 c0                	xor    eax,eax
  4a3942:	5f                   	pop    edi
  4a3943:	c3                   	ret
  4a3944:	8d 04 80             	lea    eax,[eax+eax*4]
  4a3947:	56                   	push   esi
  4a3948:	b9 0a 00 00 00       	mov    ecx,0xa
  4a394d:	8d 34 c5 b8 1a 64 00 	lea    esi,[eax*8+0x641ab8]
  4a3954:	f3 a5                	rep movs DWORD PTR es:[edi],DWORD PTR ds:[esi]
  4a3956:	a1 74 1a 64 00       	mov    eax,ds:0x641a74
  4a395b:	5e                   	pop    esi
  4a395c:	40                   	inc    eax
  4a395d:	83 f8 20             	cmp    eax,0x20
  4a3960:	a3 74 1a 64 00       	mov    ds:0x641a74,eax
  4a3965:	75 0a                	jne    0x4a3971
  4a3967:	c7 05 74 1a 64 00 00 00 00 00 	mov    DWORD PTR ds:0x641a74,0x0
  4a3971:	b8 01 00 00 00       	mov    eax,0x1
  4a3976:	5f                   	pop    edi
  4a3977:	c3                   	ret
  4a3978:	90                   	nop
  4a3979:	90                   	nop
  4a397a:	90                   	nop
  4a397b:	90                   	nop
  4a397c:	90                   	nop
  4a397d:	90                   	nop
  4a397e:	90                   	nop
  4a397f:	90                   	nop
  4a3980:	8b 0d 74 1a 64 00    	mov    ecx,DWORD PTR ds:0x641a74
  4a3986:	8b 15 88 1a 64 00    	mov    edx,DWORD PTR ds:0x641a88
  4a398c:	33 c0                	xor    eax,eax
  4a398e:	3b ca                	cmp    ecx,edx
  4a3990:	0f 95 c0             	setne  al
  4a3993:	c3                   	ret
  4a3994:	90                   	nop
  4a3995:	90                   	nop
  4a3996:	90                   	nop
  4a3997:	90                   	nop
  4a3998:	90                   	nop
  4a3999:	90                   	nop
  4a399a:	90                   	nop
  4a399b:	90                   	nop
  4a399c:	90                   	nop
  4a399d:	90                   	nop
  4a399e:	90                   	nop
  4a399f:	90                   	nop
  4a39a0:	a1 c8 04 5e 00       	mov    eax,ds:0x5e04c8
  4a39a5:	56                   	push   esi
  4a39a6:	8b f1                	mov    esi,ecx
  4a39a8:	3b c6                	cmp    eax,esi
  4a39aa:	0f 84 65 02 00 00    	je     0x4a3c15
  4a39b0:	85 f6                	test   esi,esi
  4a39b2:	0f 8c 5d 02 00 00    	jl     0x4a3c15
  4a39b8:	83 fe 17             	cmp    esi,0x17
  4a39bb:	0f 8d 54 02 00 00    	jge    0x4a3c15
  4a39c1:	a1 64 2c 64 00       	mov    eax,ds:0x642c64
  4a39c6:	85 c0                	test   eax,eax
  4a39c8:	0f 84 98 00 00 00    	je     0x4a3a66
  4a39ce:	a1 58 2c 64 00       	mov    eax,ds:0x642c58
  4a39d3:	85 c0                	test   eax,eax
  4a39d5:	0f 84 3a 02 00 00    	je     0x4a3c15
  4a39db:	6a 0a                	push   0xa
  4a39dd:	50                   	push   eax
  4a39de:	ff 15 40 51 5c 00    	call   DWORD PTR ds:0x5c5140
  4a39e4:	85 c0                	test   eax,eax
  4a39e6:	0f 85 29 02 00 00    	jne    0x4a3c15
  4a39ec:	83 fe 16             	cmp    esi,0x16
  4a39ef:	89 35 c8 04 5e 00    	mov    DWORD PTR ds:0x5e04c8,esi
  4a39f5:	74 61                	je     0x4a3a58
  4a39f7:	a3 5c 2c 64 00       	mov    ds:0x642c5c,eax
  4a39fc:	8b d6                	mov    edx,esi
  4a39fe:	8b c6                	mov    eax,esi
  4a3a00:	57                   	push   edi
  4a3a01:	c1 e2 04             	shl    edx,0x4
  4a3a04:	c1 e0 04             	shl    eax,0x4
  4a3a07:	03 d6                	add    edx,esi
  4a3a09:	8b 3d 8c 1a 64 00    	mov    edi,DWORD PTR ds:0x641a8c
  4a3a0f:	03 c6                	add    eax,esi
  4a3a11:	8b 34 95 84 26 64 00 	mov    esi,DWORD PTR [edx*4+0x642684]
  4a3a18:	8b 0c 85 b8 26 64 00 	mov    ecx,DWORD PTR [eax*4+0x6426b8]
  4a3a1f:	8b c6                	mov    eax,esi
  4a3a21:	99                   	cdq
  4a3a22:	f7 ff                	idiv   edi
  4a3a24:	89 0d c0 1f 64 00    	mov    DWORD PTR ds:0x641fc0,ecx
  4a3a2a:	8b c6                	mov    eax,esi
  4a3a2c:	8d 0c 92             	lea    ecx,[edx+edx*4]
  4a3a2f:	99                   	cdq
  4a3a30:	f7 ff                	idiv   edi
  4a3a32:	c1 e1 03             	shl    ecx,0x3
  4a3a35:	89 0d 90 1a 64 00    	mov    DWORD PTR ds:0x641a90,ecx
  4a3a3b:	83 c1 28             	add    ecx,0x28
  4a3a3e:	89 0d 98 1a 64 00    	mov    DWORD PTR ds:0x641a98,ecx
  4a3a44:	5f                   	pop    edi
  4a3a45:	8d 04 80             	lea    eax,[eax+eax*4]
  4a3a48:	c1 e0 03             	shl    eax,0x3
  4a3a4b:	a3 94 1a 64 00       	mov    ds:0x641a94,eax
  4a3a50:	83 c0 28             	add    eax,0x28
  4a3a53:	a3 9c 1a 64 00       	mov    ds:0x641a9c,eax
  4a3a58:	a1 58 2c 64 00       	mov    eax,ds:0x642c58
  4a3a5d:	50                   	push   eax
  4a3a5e:	ff 15 3c 51 5c 00    	call   DWORD PTR ds:0x5c513c
  4a3a64:	5e                   	pop    esi
  4a3a65:	c3                   	ret
  4a3a66:	83 fe 13             	cmp    esi,0x13
  4a3a69:	89 35 c8 04 5e 00    	mov    DWORD PTR ds:0x5e04c8,esi
  4a3a6f:	0f 87 87 01 00 00    	ja     0x4a3bfc
  4a3a75:	ff 24 b5 18 3c 4a 00 	jmp    DWORD PTR [esi*4+0x4a3c18]
  4a3a7c:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3a82:	6a 6c                	push   0x6c
  4a3a84:	51                   	push   ecx
  4a3a85:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3a8b:	50                   	push   eax
  4a3a8c:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3a92:	5e                   	pop    esi
  4a3a93:	c3                   	ret
  4a3a94:	6a 68                	push   0x68
  4a3a96:	e9 66 01 00 00       	jmp    0x4a3c01
  4a3a9b:	a1 68 bb 6c 00       	mov    eax,ds:0x6cbb68
  4a3aa0:	6a 6a                	push   0x6a
  4a3aa2:	50                   	push   eax
  4a3aa3:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3aa9:	50                   	push   eax
  4a3aaa:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3ab0:	5e                   	pop    esi
  4a3ab1:	c3                   	ret
  4a3ab2:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3ab8:	6a 6d                	push   0x6d
  4a3aba:	51                   	push   ecx
  4a3abb:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3ac1:	50                   	push   eax
  4a3ac2:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3ac8:	5e                   	pop    esi
  4a3ac9:	c3                   	ret
  4a3aca:	6a 6b                	push   0x6b
  4a3acc:	e9 30 01 00 00       	jmp    0x4a3c01
  4a3ad1:	a1 68 bb 6c 00       	mov    eax,ds:0x6cbb68
  4a3ad6:	68 92 00 00 00       	push   0x92
  4a3adb:	50                   	push   eax
  4a3adc:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3ae2:	50                   	push   eax
  4a3ae3:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3ae9:	5e                   	pop    esi
  4a3aea:	c3                   	ret
  4a3aeb:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3af1:	68 93 00 00 00       	push   0x93
  4a3af6:	51                   	push   ecx
  4a3af7:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3afd:	50                   	push   eax
  4a3afe:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3b04:	5e                   	pop    esi
  4a3b05:	c3                   	ret
  4a3b06:	68 95 00 00 00       	push   0x95
  4a3b0b:	e9 f1 00 00 00       	jmp    0x4a3c01
  4a3b10:	a1 68 bb 6c 00       	mov    eax,ds:0x6cbb68
  4a3b15:	68 94 00 00 00       	push   0x94
  4a3b1a:	50                   	push   eax
  4a3b1b:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3b21:	50                   	push   eax
  4a3b22:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3b28:	5e                   	pop    esi
  4a3b29:	c3                   	ret
  4a3b2a:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3b30:	68 97 00 00 00       	push   0x97
  4a3b35:	51                   	push   ecx
  4a3b36:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3b3c:	50                   	push   eax
  4a3b3d:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3b43:	5e                   	pop    esi
  4a3b44:	c3                   	ret
  4a3b45:	68 98 00 00 00       	push   0x98
  4a3b4a:	e9 b2 00 00 00       	jmp    0x4a3c01
  4a3b4f:	a1 68 bb 6c 00       	mov    eax,ds:0x6cbb68
  4a3b54:	68 9a 00 00 00       	push   0x9a
  4a3b59:	50                   	push   eax
  4a3b5a:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3b60:	50                   	push   eax
  4a3b61:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3b67:	5e                   	pop    esi
  4a3b68:	c3                   	ret
  4a3b69:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3b6f:	68 99 00 00 00       	push   0x99
  4a3b74:	51                   	push   ecx
  4a3b75:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3b7b:	50                   	push   eax
  4a3b7c:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3b82:	5e                   	pop    esi
  4a3b83:	c3                   	ret
  4a3b84:	68 9b 00 00 00       	push   0x9b
  4a3b89:	eb 76                	jmp    0x4a3c01
  4a3b8b:	a1 68 bb 6c 00       	mov    eax,ds:0x6cbb68
  4a3b90:	68 9c 00 00 00       	push   0x9c
  4a3b95:	50                   	push   eax
  4a3b96:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3b9c:	50                   	push   eax
  4a3b9d:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3ba3:	5e                   	pop    esi
  4a3ba4:	c3                   	ret
  4a3ba5:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3bab:	68 9e 00 00 00       	push   0x9e
  4a3bb0:	51                   	push   ecx
  4a3bb1:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3bb7:	50                   	push   eax
  4a3bb8:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3bbe:	5e                   	pop    esi
  4a3bbf:	c3                   	ret
  4a3bc0:	68 9f 00 00 00       	push   0x9f
  4a3bc5:	eb 3a                	jmp    0x4a3c01
  4a3bc7:	a1 68 bb 6c 00       	mov    eax,ds:0x6cbb68
  4a3bcc:	68 a0 00 00 00       	push   0xa0
  4a3bd1:	50                   	push   eax
  4a3bd2:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3bd8:	50                   	push   eax
  4a3bd9:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3bdf:	5e                   	pop    esi
  4a3be0:	c3                   	ret
  4a3be1:	8b 0d 68 bb 6c 00    	mov    ecx,DWORD PTR ds:0x6cbb68
  4a3be7:	68 a1 00 00 00       	push   0xa1
  4a3bec:	51                   	push   ecx
  4a3bed:	ff 15 14 52 5c 00    	call   DWORD PTR ds:0x5c5214
  4a3bf3:	50                   	push   eax
  4a3bf4:	ff 15 00 52 5c 00    	call   DWORD PTR ds:0x5c5200
  4a3bfa:	5e                   	pop    esi
  4a3bfb:	c3                   	ret
  4a3bfc:	68                   	.byte 0x68
  4a3bfd:	9d                   	popf
	...
