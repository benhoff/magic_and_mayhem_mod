
/home/hoff/swdev/magic_and_mayhem_mod/working/worktrees/scene-attachment-combat-20261107/original/Arcane_Nocd/Chaos.exe:     file format pei-i386


Disassembly of section .text:

004fa9b0 <.text+0xf99b0>:
  4fa9b0:	17                   	pop    %ss
  4fa9b1:	8d 0c c9             	lea    (%ecx,%ecx,8),%ecx
  4fa9b4:	8b 0c 8a             	mov    (%edx,%ecx,4),%ecx
  4fa9b7:	89 8e e9 01 00 00    	mov    %ecx,0x1e9(%esi)
  4fa9bd:	a3 00 66 65 00       	mov    %eax,0x656600
  4fa9c2:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
  4fa9c6:	8d 54 24 34          	lea    0x34(%esp),%edx
  4fa9ca:	8d 44 24 30          	lea    0x30(%esp),%eax
  4fa9ce:	52                   	push   %edx
  4fa9cf:	50                   	push   %eax
  4fa9d0:	e8 7b c8 00 00       	call   0x507250
  4fa9d5:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
  4fa9d9:	8b 81 e4 00 00 00    	mov    0xe4(%ecx),%eax
  4fa9df:	85 c0                	test   %eax,%eax
  4fa9e1:	8b c1                	mov    %ecx,%eax
  4fa9e3:	0f 84 9a 00 00 00    	je     0x4faa83
  4fa9e9:	8b 88 24 01 00 00    	mov    0x124(%eax),%ecx
  4fa9ef:	85 c9                	test   %ecx,%ecx
  4fa9f1:	0f 84 8c 00 00 00    	je     0x4faa83
  4fa9f7:	8b 90 30 01 00 00    	mov    0x130(%eax),%edx
  4fa9fd:	8b 80 48 01 00 00    	mov    0x148(%eax),%eax
  4faa03:	8b 80 10 01 00 00    	mov    0x110(%eax),%eax
  4faa09:	8b 4a 04             	mov    0x4(%edx),%ecx
  4faa0c:	89 4c 24 58          	mov    %ecx,0x58(%esp)
  4faa10:	8b 10                	mov    (%eax),%edx
  4faa12:	3b 4a 0c             	cmp    0xc(%edx),%ecx
  4faa15:	76 08                	jbe    0x4faa1f
  4faa17:	c7 44 24 58 00 00 00 	movl   $0x0,0x58(%esp)
  4faa1e:	00 
  4faa1f:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4faa23:	8b 96 9d 00 00 00    	mov    0x9d(%esi),%edx
  4faa29:	6a 00                	push   $0x0
  4faa2b:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faa30:	6a 00                	push   $0x0
  4faa32:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faa37:	6a 1f                	push   $0x1f
  4faa39:	83 c1 20             	add    $0x20,%ecx
  4faa3c:	6a 00                	push   $0x0
  4faa3e:	51                   	push   %ecx
  4faa3f:	8b 4c 24 2c          	mov    0x2c(%esp),%ecx
  4faa43:	8d 54 0a 07          	lea    0x7(%edx,%ecx,1),%edx
  4faa47:	8b 4c 24 50          	mov    0x50(%esp),%ecx
  4faa4b:	52                   	push   %edx
  4faa4c:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faa50:	03 cd                	add    %ebp,%ecx
  4faa52:	03 d3                	add    %ebx,%edx
  4faa54:	51                   	push   %ecx
  4faa55:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4faa59:	52                   	push   %edx
  4faa5a:	8b 54 24 3c          	mov    0x3c(%esp),%edx
  4faa5e:	51                   	push   %ecx
  4faa5f:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4faa63:	52                   	push   %edx
  4faa64:	8b 50 08             	mov    0x8(%eax),%edx
  4faa67:	51                   	push   %ecx
  4faa68:	8b 8c 24 8c 00 00 00 	mov    0x8c(%esp),%ecx
  4faa6f:	8b 14 8a             	mov    (%edx,%ecx,4),%edx
  4faa72:	8b 48 0c             	mov    0xc(%eax),%ecx
  4faa75:	03 d1                	add    %ecx,%edx
  4faa77:	8b cf                	mov    %edi,%ecx
  4faa79:	52                   	push   %edx
  4faa7a:	e8 b1 52 00 00       	call   0x4ffd30
  4faa7f:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4faa83:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4faa89:	85 c9                	test   %ecx,%ecx
  4faa8b:	0f 84 43 02 00 00    	je     0x4facd4
  4faa91:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
  4faa95:	e8 56 54 02 00       	call   0x51fef0
  4faa9a:	83 f8 01             	cmp    $0x1,%eax
  4faa9d:	0f 85 9e 00 00 00    	jne    0x4fab41
  4faaa3:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
  4faaa7:	8b 81 d9 07 00 00    	mov    0x7d9(%ecx),%eax
  4faaad:	8b 89 f1 07 00 00    	mov    0x7f1(%ecx),%ecx
  4faab3:	8b d0                	mov    %eax,%edx
  4faab5:	89 84 24 80 00 00 00 	mov    %eax,0x80(%esp)
  4faabc:	8b 89 10 01 00 00    	mov    0x110(%ecx),%ecx
  4faac2:	8b 42 04             	mov    0x4(%edx),%eax
  4faac5:	8b 11                	mov    (%ecx),%edx
  4faac7:	3b 42 0c             	cmp    0xc(%edx),%eax
  4faaca:	76 02                	jbe    0x4faace
  4faacc:	33 c0                	xor    %eax,%eax
  4faace:	8b 51 08             	mov    0x8(%ecx),%edx
  4faad1:	6a 00                	push   $0x0
  4faad3:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faad8:	6a 00                	push   $0x0
  4faada:	8b 04 82             	mov    (%edx,%eax,4),%eax
  4faadd:	8b 51 0c             	mov    0xc(%ecx),%edx
  4faae0:	8b 8c 24 8c 00 00 00 	mov    0x8c(%esp),%ecx
  4faae7:	03 c2                	add    %edx,%eax
  4faae9:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faaee:	6a 1f                	push   $0x1f
  4faaf0:	8b 51 08             	mov    0x8(%ecx),%edx
  4faaf3:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4faaf6:	89 94 24 24 01 00 00 	mov    %edx,0x124(%esp)
  4faafd:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fab01:	83 c2 20             	add    $0x20,%edx
  4fab04:	6a 00                	push   $0x0
  4fab06:	52                   	push   %edx
  4fab07:	8b 96 9d 00 00 00    	mov    0x9d(%esi),%edx
  4fab0d:	83 c2 08             	add    $0x8,%edx
  4fab10:	52                   	push   %edx
  4fab11:	8b 54 24 54          	mov    0x54(%esp),%edx
  4fab15:	03 ca                	add    %edx,%ecx
  4fab17:	8b 94 24 30 01 00 00 	mov    0x130(%esp),%edx
  4fab1e:	03 cd                	add    %ebp,%ecx
  4fab20:	51                   	push   %ecx
  4fab21:	8b 4c 24 54          	mov    0x54(%esp),%ecx
  4fab25:	03 d1                	add    %ecx,%edx
  4fab27:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4fab2b:	03 d3                	add    %ebx,%edx
  4fab2d:	52                   	push   %edx
  4fab2e:	8b 54 24 3c          	mov    0x3c(%esp),%edx
  4fab32:	51                   	push   %ecx
  4fab33:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4fab37:	52                   	push   %edx
  4fab38:	51                   	push   %ecx
  4fab39:	50                   	push   %eax
  4fab3a:	8b cf                	mov    %edi,%ecx
  4fab3c:	e8 ef 51 00 00       	call   0x4ffd30
  4fab41:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fab45:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fab4b:	85 c9                	test   %ecx,%ecx
  4fab4d:	0f 84 81 01 00 00    	je     0x4facd4
  4fab53:	8b 88 89 08 00 00    	mov    0x889(%eax),%ecx
  4fab59:	85 c9                	test   %ecx,%ecx
  4fab5b:	0f 84 b1 00 00 00    	je     0x4fac12
  4fab61:	8b 90 5d 08 00 00    	mov    0x85d(%eax),%edx
  4fab67:	8b 80 75 08 00 00    	mov    0x875(%eax),%eax
  4fab6d:	8b ca                	mov    %edx,%ecx
  4fab6f:	89 94 24 80 00 00 00 	mov    %edx,0x80(%esp)
  4fab76:	8b 51 04             	mov    0x4(%ecx),%edx
  4fab79:	8b 88 10 01 00 00    	mov    0x110(%eax),%ecx
  4fab7f:	8b 01                	mov    (%ecx),%eax
  4fab81:	3b 50 0c             	cmp    0xc(%eax),%edx
  4fab84:	76 02                	jbe    0x4fab88
  4fab86:	33 d2                	xor    %edx,%edx
  4fab88:	8b 41 08             	mov    0x8(%ecx),%eax
  4fab8b:	6a 00                	push   $0x0
  4fab8d:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fab92:	6a 00                	push   $0x0
  4fab94:	8b 04 90             	mov    (%eax,%edx,4),%eax
  4fab97:	8b 51 0c             	mov    0xc(%ecx),%edx
  4fab9a:	8b 8c 24 8c 00 00 00 	mov    0x8c(%esp),%ecx
  4faba1:	03 c2                	add    %edx,%eax
  4faba3:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faba8:	6a 1f                	push   $0x1f
  4fabaa:	8b 51 08             	mov    0x8(%ecx),%edx
  4fabad:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4fabb0:	89 94 24 e8 00 00 00 	mov    %edx,0xe8(%esp)
  4fabb7:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fabbb:	83 c2 20             	add    $0x20,%edx
  4fabbe:	6a 00                	push   $0x0
  4fabc0:	89 8c 24 f0 00 00 00 	mov    %ecx,0xf0(%esp)
  4fabc7:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4fabcd:	52                   	push   %edx
  4fabce:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4fabd2:	8d 4c 11 09          	lea    0x9(%ecx,%edx,1),%ecx
  4fabd6:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fabda:	51                   	push   %ecx
  4fabdb:	8b 8c 24 f8 00 00 00 	mov    0xf8(%esp),%ecx
  4fabe2:	03 ca                	add    %edx,%ecx
  4fabe4:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fabe8:	03 cd                	add    %ebp,%ecx
  4fabea:	51                   	push   %ecx
  4fabeb:	8b 8c 24 f8 00 00 00 	mov    0xf8(%esp),%ecx
  4fabf2:	03 ca                	add    %edx,%ecx
  4fabf4:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fabf8:	03 cb                	add    %ebx,%ecx
  4fabfa:	51                   	push   %ecx
  4fabfb:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fabff:	52                   	push   %edx
  4fac00:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fac04:	51                   	push   %ecx
  4fac05:	52                   	push   %edx
  4fac06:	50                   	push   %eax
  4fac07:	8b cf                	mov    %edi,%ecx
  4fac09:	e8 22 51 00 00       	call   0x4ffd30
  4fac0e:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fac12:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fac18:	85 c9                	test   %ecx,%ecx
  4fac1a:	0f 84 b4 00 00 00    	je     0x4facd4
  4fac20:	8b 88 d1 08 00 00    	mov    0x8d1(%eax),%ecx
  4fac26:	85 c9                	test   %ecx,%ecx
  4fac28:	0f 84 a6 00 00 00    	je     0x4facd4
  4fac2e:	8b 0d 68 f1 6d 00    	mov    0x6df168,%ecx
  4fac34:	a1 50 f1 6d 00       	mov    0x6df150,%eax
  4fac39:	8b 89 10 01 00 00    	mov    0x110(%ecx),%ecx
  4fac3f:	8b 40 04             	mov    0x4(%eax),%eax
  4fac42:	8b 11                	mov    (%ecx),%edx
  4fac44:	3b 42 0c             	cmp    0xc(%edx),%eax
  4fac47:	76 02                	jbe    0x4fac4b
  4fac49:	33 c0                	xor    %eax,%eax
  4fac4b:	8b 51 08             	mov    0x8(%ecx),%edx
  4fac4e:	6a 00                	push   $0x0
  4fac50:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fac55:	6a 00                	push   $0x0
  4fac57:	8b 04 82             	mov    (%edx,%eax,4),%eax
  4fac5a:	8b 51 0c             	mov    0xc(%ecx),%edx
  4fac5d:	8b 0d 50 f1 6d 00    	mov    0x6df150,%ecx
  4fac63:	03 c2                	add    %edx,%eax
  4fac65:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fac6a:	6a 1f                	push   $0x1f
  4fac6c:	8b 51 08             	mov    0x8(%ecx),%edx
  4fac6f:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4fac72:	89 94 24 d0 00 00 00 	mov    %edx,0xd0(%esp)
  4fac79:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fac7d:	83 c2 20             	add    $0x20,%edx
  4fac80:	6a 00                	push   $0x0
  4fac82:	89 8c 24 d8 00 00 00 	mov    %ecx,0xd8(%esp)
  4fac89:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4fac8f:	52                   	push   %edx
  4fac90:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4fac94:	8d 4c 11 0a          	lea    0xa(%ecx,%edx,1),%ecx
  4fac98:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fac9c:	51                   	push   %ecx
  4fac9d:	8b 8c 24 e0 00 00 00 	mov    0xe0(%esp),%ecx
  4faca4:	03 ca                	add    %edx,%ecx
  4faca6:	8b 54 24 50          	mov    0x50(%esp),%edx
  4facaa:	03 cd                	add    %ebp,%ecx
  4facac:	51                   	push   %ecx
  4facad:	8b 8c 24 e0 00 00 00 	mov    0xe0(%esp),%ecx
  4facb4:	03 ca                	add    %edx,%ecx
  4facb6:	8b 54 24 44          	mov    0x44(%esp),%edx
  4facba:	03 cb                	add    %ebx,%ecx
  4facbc:	51                   	push   %ecx
  4facbd:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4facc1:	52                   	push   %edx
  4facc2:	8b 54 24 44          	mov    0x44(%esp),%edx
  4facc6:	51                   	push   %ecx
  4facc7:	52                   	push   %edx
  4facc8:	50                   	push   %eax
  4facc9:	8b cf                	mov    %edi,%ecx
  4faccb:	e8 60 50 00 00       	call   0x4ffd30
  4facd0:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4facd4:	8b 88 bd 08 00 00    	mov    0x8bd(%eax),%ecx
  4facda:	85 c9                	test   %ecx,%ecx
  4facdc:	0f 84 b1 00 00 00    	je     0x4fad93
  4face2:	8b 88 91 08 00 00    	mov    0x891(%eax),%ecx
  4face8:	8b 80 a9 08 00 00    	mov    0x8a9(%eax),%eax
  4facee:	89 8c 24 80 00 00 00 	mov    %ecx,0x80(%esp)
  4facf5:	8b d1                	mov    %ecx,%edx
  4facf7:	8b 88 10 01 00 00    	mov    0x110(%eax),%ecx
  4facfd:	8b 52 04             	mov    0x4(%edx),%edx
  4fad00:	8b 01                	mov    (%ecx),%eax
  4fad02:	3b 50 0c             	cmp    0xc(%eax),%edx
  4fad05:	76 02                	jbe    0x4fad09
  4fad07:	33 d2                	xor    %edx,%edx
  4fad09:	8b 41 08             	mov    0x8(%ecx),%eax
  4fad0c:	6a 00                	push   $0x0
  4fad0e:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fad13:	6a 00                	push   $0x0
  4fad15:	8b 04 90             	mov    (%eax,%edx,4),%eax
  4fad18:	8b 51 0c             	mov    0xc(%ecx),%edx
  4fad1b:	8b 8c 24 8c 00 00 00 	mov    0x8c(%esp),%ecx
  4fad22:	03 c2                	add    %edx,%eax
  4fad24:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fad29:	6a 1f                	push   $0x1f
  4fad2b:	8b 51 08             	mov    0x8(%ecx),%edx
  4fad2e:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4fad31:	89 94 24 18 01 00 00 	mov    %edx,0x118(%esp)
  4fad38:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fad3c:	83 c2 20             	add    $0x20,%edx
  4fad3f:	6a 00                	push   $0x0
  4fad41:	89 8c 24 20 01 00 00 	mov    %ecx,0x120(%esp)
  4fad48:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4fad4e:	52                   	push   %edx
  4fad4f:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4fad53:	8d 4c 11 0b          	lea    0xb(%ecx,%edx,1),%ecx
  4fad57:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fad5b:	51                   	push   %ecx
  4fad5c:	8b 8c 24 28 01 00 00 	mov    0x128(%esp),%ecx
  4fad63:	03 ca                	add    %edx,%ecx
  4fad65:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fad69:	03 cd                	add    %ebp,%ecx
  4fad6b:	51                   	push   %ecx
  4fad6c:	8b 8c 24 28 01 00 00 	mov    0x128(%esp),%ecx
  4fad73:	03 ca                	add    %edx,%ecx
  4fad75:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fad79:	03 cb                	add    %ebx,%ecx
  4fad7b:	51                   	push   %ecx
  4fad7c:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fad80:	52                   	push   %edx
  4fad81:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fad85:	51                   	push   %ecx
  4fad86:	52                   	push   %edx
  4fad87:	50                   	push   %eax
  4fad88:	8b cf                	mov    %edi,%ecx
  4fad8a:	e8 a1 4f 00 00       	call   0x4ffd30
  4fad8f:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fad93:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fad99:	85 c9                	test   %ecx,%ecx
  4fad9b:	0f 84 c7 04 00 00    	je     0x4fb268
  4fada1:	8b 88 80 07 00 00    	mov    0x780(%eax),%ecx
  4fada7:	85 c9                	test   %ecx,%ecx
  4fada9:	0f 84 8b 00 00 00    	je     0x4fae3a
  4fadaf:	a1 60 f0 6d 00       	mov    0x6df060,%eax
  4fadb4:	8b 15 78 f0 6d 00    	mov    0x6df078,%edx
  4fadba:	8b 48 04             	mov    0x4(%eax),%ecx
  4fadbd:	8b 82 10 01 00 00    	mov    0x110(%edx),%eax
  4fadc3:	89 4c 24 58          	mov    %ecx,0x58(%esp)
  4fadc7:	8b 10                	mov    (%eax),%edx
  4fadc9:	3b 4a 0c             	cmp    0xc(%edx),%ecx
  4fadcc:	76 08                	jbe    0x4fadd6
  4fadce:	c7 44 24 58 00 00 00 	movl   $0x0,0x58(%esp)
  4fadd5:	00 
  4fadd6:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fadda:	8b 96 9d 00 00 00    	mov    0x9d(%esi),%edx
  4fade0:	6a 00                	push   $0x0
  4fade2:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fade7:	6a 00                	push   $0x0
  4fade9:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fadee:	6a 1f                	push   $0x1f
  4fadf0:	83 c1 20             	add    $0x20,%ecx
  4fadf3:	6a 00                	push   $0x0
  4fadf5:	51                   	push   %ecx
  4fadf6:	8b 4c 24 2c          	mov    0x2c(%esp),%ecx
  4fadfa:	8d 54 0a 0c          	lea    0xc(%edx,%ecx,1),%edx
  4fadfe:	8b 4c 24 50          	mov    0x50(%esp),%ecx
  4fae02:	52                   	push   %edx
  4fae03:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fae07:	03 cd                	add    %ebp,%ecx
  4fae09:	03 d3                	add    %ebx,%edx
  4fae0b:	51                   	push   %ecx
  4fae0c:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4fae10:	52                   	push   %edx
  4fae11:	8b 54 24 3c          	mov    0x3c(%esp),%edx
  4fae15:	51                   	push   %ecx
  4fae16:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4fae1a:	52                   	push   %edx
  4fae1b:	8b 50 08             	mov    0x8(%eax),%edx
  4fae1e:	51                   	push   %ecx
  4fae1f:	8b 8c 24 8c 00 00 00 	mov    0x8c(%esp),%ecx
  4fae26:	8b 14 8a             	mov    (%edx,%ecx,4),%edx
  4fae29:	8b 48 0c             	mov    0xc(%eax),%ecx
  4fae2c:	03 d1                	add    %ecx,%edx
  4fae2e:	8b cf                	mov    %edi,%ecx
  4fae30:	52                   	push   %edx
  4fae31:	e8 fa 4e 00 00       	call   0x4ffd30
  4fae36:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fae3a:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fae40:	85 c9                	test   %ecx,%ecx
  4fae42:	0f 84 20 04 00 00    	je     0x4fb268
  4fae48:	8b 88 d5 08 00 00    	mov    0x8d5(%eax),%ecx
  4fae4e:	85 c9                	test   %ecx,%ecx
  4fae50:	0f 84 a6 00 00 00    	je     0x4faefc
  4fae56:	8b 0d a8 f0 6d 00    	mov    0x6df0a8,%ecx
  4fae5c:	a1 90 f0 6d 00       	mov    0x6df090,%eax
  4fae61:	8b 89 10 01 00 00    	mov    0x110(%ecx),%ecx
  4fae67:	8b 40 04             	mov    0x4(%eax),%eax
  4fae6a:	8b 11                	mov    (%ecx),%edx
  4fae6c:	3b 42 0c             	cmp    0xc(%edx),%eax
  4fae6f:	76 02                	jbe    0x4fae73
  4fae71:	33 c0                	xor    %eax,%eax
  4fae73:	8b 51 08             	mov    0x8(%ecx),%edx
  4fae76:	6a 00                	push   $0x0
  4fae78:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fae7d:	6a 00                	push   $0x0
  4fae7f:	8b 04 82             	mov    (%edx,%eax,4),%eax
  4fae82:	8b 51 0c             	mov    0xc(%ecx),%edx
  4fae85:	8b 0d 90 f0 6d 00    	mov    0x6df090,%ecx
  4fae8b:	03 c2                	add    %edx,%eax
  4fae8d:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fae92:	6a 02                	push   $0x2
  4fae94:	8b 51 08             	mov    0x8(%ecx),%edx
  4fae97:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4fae9a:	89 94 24 00 01 00 00 	mov    %edx,0x100(%esp)
  4faea1:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faea5:	83 c2 20             	add    $0x20,%edx
  4faea8:	6a 00                	push   $0x0
  4faeaa:	89 8c 24 08 01 00 00 	mov    %ecx,0x108(%esp)
  4faeb1:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4faeb7:	52                   	push   %edx
  4faeb8:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4faebc:	8d 4c 11 0d          	lea    0xd(%ecx,%edx,1),%ecx
  4faec0:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faec4:	51                   	push   %ecx
  4faec5:	8b 8c 24 10 01 00 00 	mov    0x110(%esp),%ecx
  4faecc:	03 ca                	add    %edx,%ecx
  4faece:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faed2:	03 cd                	add    %ebp,%ecx
  4faed4:	51                   	push   %ecx
  4faed5:	8b 8c 24 10 01 00 00 	mov    0x110(%esp),%ecx
  4faedc:	03 ca                	add    %edx,%ecx
  4faede:	8b 54 24 44          	mov    0x44(%esp),%edx
  4faee2:	03 cb                	add    %ebx,%ecx
  4faee4:	51                   	push   %ecx
  4faee5:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4faee9:	52                   	push   %edx
  4faeea:	8b 54 24 44          	mov    0x44(%esp),%edx
  4faeee:	51                   	push   %ecx
  4faeef:	52                   	push   %edx
  4faef0:	50                   	push   %eax
  4faef1:	8b cf                	mov    %edi,%ecx
  4faef3:	e8 38 4e 00 00       	call   0x4ffd30
  4faef8:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4faefc:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4faf02:	85 c9                	test   %ecx,%ecx
  4faf04:	0f 84 5e 03 00 00    	je     0x4fb268
  4faf0a:	8b 88 63 09 00 00    	mov    0x963(%eax),%ecx
  4faf10:	85 c9                	test   %ecx,%ecx
  4faf12:	0f 84 a6 00 00 00    	je     0x4fafbe
  4faf18:	8b 0d 08 f1 6d 00    	mov    0x6df108,%ecx
  4faf1e:	a1 f0 f0 6d 00       	mov    0x6df0f0,%eax
  4faf23:	8b 89 10 01 00 00    	mov    0x110(%ecx),%ecx
  4faf29:	8b 40 04             	mov    0x4(%eax),%eax
  4faf2c:	8b 11                	mov    (%ecx),%edx
  4faf2e:	3b 42 0c             	cmp    0xc(%edx),%eax
  4faf31:	76 02                	jbe    0x4faf35
  4faf33:	33 c0                	xor    %eax,%eax
  4faf35:	8b 51 08             	mov    0x8(%ecx),%edx
  4faf38:	6a 00                	push   $0x0
  4faf3a:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faf3f:	6a 00                	push   $0x0
  4faf41:	8b 04 82             	mov    (%edx,%eax,4),%eax
  4faf44:	8b 51 0c             	mov    0xc(%ecx),%edx
  4faf47:	8b 0d f0 f0 6d 00    	mov    0x6df0f0,%ecx
  4faf4d:	03 c2                	add    %edx,%eax
  4faf4f:	68 d0 8a ff ff       	push   $0xffff8ad0
  4faf54:	6a 02                	push   $0x2
  4faf56:	8b 51 08             	mov    0x8(%ecx),%edx
  4faf59:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4faf5c:	89 94 24 dc 00 00 00 	mov    %edx,0xdc(%esp)
  4faf63:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faf67:	83 c2 20             	add    $0x20,%edx
  4faf6a:	6a 00                	push   $0x0
  4faf6c:	89 8c 24 e4 00 00 00 	mov    %ecx,0xe4(%esp)
  4faf73:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4faf79:	52                   	push   %edx
  4faf7a:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4faf7e:	8d 4c 11 0e          	lea    0xe(%ecx,%edx,1),%ecx
  4faf82:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faf86:	51                   	push   %ecx
  4faf87:	8b 8c 24 ec 00 00 00 	mov    0xec(%esp),%ecx
  4faf8e:	03 ca                	add    %edx,%ecx
  4faf90:	8b 54 24 50          	mov    0x50(%esp),%edx
  4faf94:	03 cd                	add    %ebp,%ecx
  4faf96:	51                   	push   %ecx
  4faf97:	8b 8c 24 ec 00 00 00 	mov    0xec(%esp),%ecx
  4faf9e:	03 ca                	add    %edx,%ecx
  4fafa0:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fafa4:	03 cb                	add    %ebx,%ecx
  4fafa6:	51                   	push   %ecx
  4fafa7:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fafab:	52                   	push   %edx
  4fafac:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fafb0:	51                   	push   %ecx
  4fafb1:	52                   	push   %edx
  4fafb2:	50                   	push   %eax
  4fafb3:	8b cf                	mov    %edi,%ecx
  4fafb5:	e8 76 4d 00 00       	call   0x4ffd30
  4fafba:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fafbe:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fafc4:	85 c9                	test   %ecx,%ecx
  4fafc6:	0f 84 9c 02 00 00    	je     0x4fb268
  4fafcc:	8a 88 62 09 00 00    	mov    0x962(%eax),%cl
  4fafd2:	84 c9                	test   %cl,%cl
  4fafd4:	0f 84 a6 00 00 00    	je     0x4fb080
  4fafda:	8b 0d d8 f0 6d 00    	mov    0x6df0d8,%ecx
  4fafe0:	a1 c0 f0 6d 00       	mov    0x6df0c0,%eax
  4fafe5:	8b 89 10 01 00 00    	mov    0x110(%ecx),%ecx
  4fafeb:	8b 40 04             	mov    0x4(%eax),%eax
  4fafee:	8b 11                	mov    (%ecx),%edx
  4faff0:	3b 42 0c             	cmp    0xc(%edx),%eax
  4faff3:	76 02                	jbe    0x4faff7
  4faff5:	33 c0                	xor    %eax,%eax
  4faff7:	8b 51 08             	mov    0x8(%ecx),%edx
  4faffa:	6a 00                	push   $0x0
  4faffc:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fb001:	6a 00                	push   $0x0
  4fb003:	8b 04 82             	mov    (%edx,%eax,4),%eax
  4fb006:	8b 51 0c             	mov    0xc(%ecx),%edx
  4fb009:	8b 0d c0 f0 6d 00    	mov    0x6df0c0,%ecx
  4fb00f:	03 c2                	add    %edx,%eax
  4fb011:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fb016:	6a 1f                	push   $0x1f
  4fb018:	8b 51 08             	mov    0x8(%ecx),%edx
  4fb01b:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4fb01e:	89 94 24 f4 00 00 00 	mov    %edx,0xf4(%esp)
  4fb025:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fb029:	83 c2 20             	add    $0x20,%edx
  4fb02c:	6a 00                	push   $0x0
  4fb02e:	89 8c 24 fc 00 00 00 	mov    %ecx,0xfc(%esp)
  4fb035:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4fb03b:	52                   	push   %edx
  4fb03c:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4fb040:	8d 4c 11 0f          	lea    0xf(%ecx,%edx,1),%ecx
  4fb044:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fb048:	51                   	push   %ecx
  4fb049:	8b 8c 24 04 01 00 00 	mov    0x104(%esp),%ecx
  4fb050:	03 ca                	add    %edx,%ecx
  4fb052:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fb056:	03 cd                	add    %ebp,%ecx
  4fb058:	51                   	push   %ecx
  4fb059:	8b 8c 24 04 01 00 00 	mov    0x104(%esp),%ecx
  4fb060:	03 ca                	add    %edx,%ecx
  4fb062:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fb066:	03 cb                	add    %ebx,%ecx
  4fb068:	51                   	push   %ecx
  4fb069:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fb06d:	52                   	push   %edx
  4fb06e:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fb072:	51                   	push   %ecx
  4fb073:	52                   	push   %edx
  4fb074:	50                   	push   %eax
  4fb075:	8b cf                	mov    %edi,%ecx
  4fb077:	e8 b4 4c 00 00       	call   0x4ffd30
  4fb07c:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fb080:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fb086:	85 c9                	test   %ecx,%ecx
  4fb088:	0f 84 da 01 00 00    	je     0x4fb268
  4fb08e:	8b 88 8c 07 00 00    	mov    0x78c(%eax),%ecx
  4fb094:	85 c9                	test   %ecx,%ecx
  4fb096:	0f 84 a6 00 00 00    	je     0x4fb142
  4fb09c:	8b 0d 38 f1 6d 00    	mov    0x6df138,%ecx
  4fb0a2:	a1 20 f1 6d 00       	mov    0x6df120,%eax
  4fb0a7:	8b 89 10 01 00 00    	mov    0x110(%ecx),%ecx
  4fb0ad:	8b 40 04             	mov    0x4(%eax),%eax
  4fb0b0:	8b 11                	mov    (%ecx),%edx
  4fb0b2:	3b 42 0c             	cmp    0xc(%edx),%eax
  4fb0b5:	76 02                	jbe    0x4fb0b9
  4fb0b7:	33 c0                	xor    %eax,%eax
  4fb0b9:	8b 51 08             	mov    0x8(%ecx),%edx
  4fb0bc:	6a 00                	push   $0x0
  4fb0be:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fb0c3:	6a 00                	push   $0x0
  4fb0c5:	8b 04 82             	mov    (%edx,%eax,4),%eax
  4fb0c8:	8b 51 0c             	mov    0xc(%ecx),%edx
  4fb0cb:	8b 0d 20 f1 6d 00    	mov    0x6df120,%ecx
  4fb0d1:	03 c2                	add    %edx,%eax
  4fb0d3:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fb0d8:	6a 1f                	push   $0x1f
  4fb0da:	8b 51 08             	mov    0x8(%ecx),%edx
  4fb0dd:	8b 49 0c             	mov    0xc(%ecx),%ecx
  4fb0e0:	89 94 24 0c 01 00 00 	mov    %edx,0x10c(%esp)
  4fb0e7:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fb0eb:	83 c2 20             	add    $0x20,%edx
  4fb0ee:	6a 00                	push   $0x0
  4fb0f0:	89 8c 24 14 01 00 00 	mov    %ecx,0x114(%esp)
  4fb0f7:	8b 8e 9d 00 00 00    	mov    0x9d(%esi),%ecx
  4fb0fd:	52                   	push   %edx
  4fb0fe:	8b 54 24 2c          	mov    0x2c(%esp),%edx
  4fb102:	8d 4c 11 10          	lea    0x10(%ecx,%edx,1),%ecx
  4fb106:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fb10a:	51                   	push   %ecx
  4fb10b:	8b 8c 24 1c 01 00 00 	mov    0x11c(%esp),%ecx
  4fb112:	03 ca                	add    %edx,%ecx
  4fb114:	8b 54 24 50          	mov    0x50(%esp),%edx
  4fb118:	03 cd                	add    %ebp,%ecx
  4fb11a:	51                   	push   %ecx
  4fb11b:	8b 8c 24 1c 01 00 00 	mov    0x11c(%esp),%ecx
  4fb122:	03 ca                	add    %edx,%ecx
  4fb124:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fb128:	03 cb                	add    %ebx,%ecx
  4fb12a:	51                   	push   %ecx
  4fb12b:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fb12f:	52                   	push   %edx
  4fb130:	8b 54 24 44          	mov    0x44(%esp),%edx
  4fb134:	51                   	push   %ecx
  4fb135:	52                   	push   %edx
  4fb136:	50                   	push   %eax
  4fb137:	8b cf                	mov    %edi,%ecx
  4fb139:	e8 f2 4b 00 00       	call   0x4ffd30
  4fb13e:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fb142:	8b 88 e4 00 00 00    	mov    0xe4(%eax),%ecx
  4fb148:	85 c9                	test   %ecx,%ecx
  4fb14a:	0f 84 18 01 00 00    	je     0x4fb268
  4fb150:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
  4fb154:	e8 97 4d 02 00       	call   0x51fef0
  4fb159:	83 f8 03             	cmp    $0x3,%eax
  4fb15c:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fb160:	0f 85 02 01 00 00    	jne    0x4fb268
  4fb166:	8b 90 f1 07 00 00    	mov    0x7f1(%eax),%edx
  4fb16c:	8b 88 d9 07 00 00    	mov    0x7d9(%eax),%ecx
  4fb172:	8b 82 10 01 00 00    	mov    0x110(%edx),%eax
  4fb178:	8b 49 04             	mov    0x4(%ecx),%ecx
  4fb17b:	8b 10                	mov    (%eax),%edx
  4fb17d:	3b 4a 0c             	cmp    0xc(%edx),%ecx
  4fb180:	76 02                	jbe    0x4fb184
  4fb182:	33 c9                	xor    %ecx,%ecx
  4fb184:	8b 50 08             	mov    0x8(%eax),%edx
  4fb187:	8b 0c 8a             	mov    (%edx,%ecx,4),%ecx
  4fb18a:	8b 50 0c             	mov    0xc(%eax),%edx
  4fb18d:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fb191:	03 ca                	add    %edx,%ecx
  4fb193:	89 8c 24 a0 00 00 00 	mov    %ecx,0xa0(%esp)
  4fb19a:	8b 80 d9 07 00 00    	mov    0x7d9(%eax),%eax
  4fb1a0:	8b 50 08             	mov    0x8(%eax),%edx
  4fb1a3:	8b 40 0c             	mov    0xc(%eax),%eax
  4fb1a6:	89 94 24 a4 00 00 00 	mov    %edx,0xa4(%esp)
  4fb1ad:	8b 54 24 30          	mov    0x30(%esp),%edx
  4fb1b1:	89 84 24 a8 00 00 00 	mov    %eax,0xa8(%esp)
  4fb1b8:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fb1bc:	89 94 24 8c 00 00 00 	mov    %edx,0x8c(%esp)
  4fb1c3:	8b 54 24 34          	mov    0x34(%esp),%edx
  4fb1c7:	8b 80 ac 00 00 00    	mov    0xac(%eax),%eax
  4fb1cd:	89 54 24 38          	mov    %edx,0x38(%esp)
  4fb1d1:	83 78 08 02          	cmpl   $0x2,0x8(%eax)
  4fb1d5:	75 1c                	jne    0x4fb1f3
  4fb1d7:	8d 4c 24 38          	lea    0x38(%esp),%ecx
  4fb1db:	8d 94 24 8c 00 00 00 	lea    0x8c(%esp),%edx
  4fb1e2:	51                   	push   %ecx
  4fb1e3:	8b 4c 24 20          	mov    0x20(%esp),%ecx
  4fb1e7:	52                   	push   %edx
  4fb1e8:	e8 d3 c0 00 00       	call   0x5072c0
  4fb1ed:	8b 44 24 38          	mov    0x38(%esp),%eax
  4fb1f1:	eb 0b                	jmp    0x4fb1fe
  4fb1f3:	8b 41 08             	mov    0x8(%ecx),%eax
  4fb1f6:	d1 e8                	shr    $1,%eax
  4fb1f8:	03 c2                	add    %edx,%eax
  4fb1fa:	89 44 24 38          	mov    %eax,0x38(%esp)
  4fb1fe:	8b 4c 24 3c          	mov    0x3c(%esp),%ecx
  4fb202:	8b 96 9d 00 00 00    	mov    0x9d(%esi),%edx
  4fb208:	6a 00                	push   $0x0
  4fb20a:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fb20f:	6a 00                	push   $0x0
  4fb211:	68 d0 8a ff ff       	push   $0xffff8ad0
  4fb216:	6a 1f                	push   $0x1f
  4fb218:	83 c1 20             	add    $0x20,%ecx
  4fb21b:	6a 00                	push   $0x0
  4fb21d:	51                   	push   %ecx
  4fb21e:	8b 4c 24 2c          	mov    0x2c(%esp),%ecx
  4fb222:	8d 54 0a 05          	lea    0x5(%edx,%ecx,1),%edx
  4fb226:	8b 8c 24 c4 00 00 00 	mov    0xc4(%esp),%ecx
  4fb22d:	52                   	push   %edx
  4fb22e:	8b 94 24 c4 00 00 00 	mov    0xc4(%esp),%edx
  4fb235:	03 c8                	add    %eax,%ecx
  4fb237:	8b 84 24 ac 00 00 00 	mov    0xac(%esp),%eax
  4fb23e:	03 d0                	add    %eax,%edx
  4fb240:	8b 44 24 38          	mov    0x38(%esp),%eax
  4fb244:	03 cd                	add    %ebp,%ecx
  4fb246:	03 d3                	add    %ebx,%edx
  4fb248:	51                   	push   %ecx
  4fb249:	8b 4c 24 44          	mov    0x44(%esp),%ecx
  4fb24d:	52                   	push   %edx
  4fb24e:	8b 54 24 3c          	mov    0x3c(%esp),%edx
  4fb252:	51                   	push   %ecx
  4fb253:	8b 8c 24 cc 00 00 00 	mov    0xcc(%esp),%ecx
  4fb25a:	52                   	push   %edx
  4fb25b:	50                   	push   %eax
  4fb25c:	51                   	push   %ecx
  4fb25d:	8b cf                	mov    %edi,%ecx
  4fb25f:	e8 cc 4a 00 00       	call   0x4ffd30
  4fb264:	8b 44 24 1c          	mov    0x1c(%esp),%eax
  4fb268:	8b 48 68             	mov    0x68(%eax),%ecx
  4fb26b:	85 c9                	test   %ecx,%ecx
  4fb26d:	75 21                	jne    0x4fb290
  4fb26f:	8b 48 70             	mov    0x70(%eax),%ecx
  4fb272:	85 c9                	test   %ecx,%ecx
  4fb274:	75 1a                	jne    0x4fb290
  4fb276:	8b 4c 24 1c          	mov    0x1c(%esp),%ecx
  4fb27a:	e8 a1 af 00 00       	call   0x506220
  4fb27f:	85                   	.byte 0x85
