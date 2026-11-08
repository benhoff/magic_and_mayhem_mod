
/home/hoff/swdev/magic_and_mayhem_mod/working/worktrees/scene-attachment-combat-20261107/original/Arcane_Nocd/Chaos.exe:     file format pei-i386


Disassembly of section .text:

00507190 <.text+0x106190>:
  507190:	56                   	push   %esi
  507191:	8b f1                	mov    %ecx,%esi
  507193:	57                   	push   %edi
  507194:	8b 8e b4 00 00 00    	mov    0xb4(%esi),%ecx
  50719a:	85 c9                	test   %ecx,%ecx
  50719c:	75 1e                	jne    0x5071bc
  50719e:	8b 86 0c 06 00 00    	mov    0x60c(%esi),%eax
  5071a4:	51                   	push   %ecx
  5071a5:	05 20 01 00 00       	add    $0x120,%eax
  5071aa:	8d 8e b0 00 00 00    	lea    0xb0(%esi),%ecx
  5071b0:	50                   	push   %eax
  5071b1:	e8 fa da f5 ff       	call   0x464cb0
  5071b6:	8b 8e b4 00 00 00    	mov    0xb4(%esi),%ecx
  5071bc:	8b 86 cc 00 00 00    	mov    0xcc(%esi),%eax
  5071c2:	8b 96 b4 00 00 00    	mov    0xb4(%esi),%edx
  5071c8:	53                   	push   %ebx
  5071c9:	8b 80 10 01 00 00    	mov    0x110(%eax),%eax
  5071cf:	8b 52 04             	mov    0x4(%edx),%edx
  5071d2:	8b 38                	mov    (%eax),%edi
  5071d4:	8b 5f 0c             	mov    0xc(%edi),%ebx
  5071d7:	3b d3                	cmp    %ebx,%edx
  5071d9:	5b                   	pop    %ebx
  5071da:	76 02                	jbe    0x5071de
  5071dc:	33 d2                	xor    %edx,%edx
  5071de:	8b 78 08             	mov    0x8(%eax),%edi
  5071e1:	8b 14 97             	mov    (%edi,%edx,4),%edx
  5071e4:	8b 78 0c             	mov    0xc(%eax),%edi
  5071e7:	8b 44 24 0c          	mov    0xc(%esp),%eax
  5071eb:	03 d7                	add    %edi,%edx
  5071ed:	5f                   	pop    %edi
  5071ee:	89 10                	mov    %edx,(%eax)
  5071f0:	8b 51 08             	mov    0x8(%ecx),%edx
  5071f3:	89 50 04             	mov    %edx,0x4(%eax)
  5071f6:	8b 49 0c             	mov    0xc(%ecx),%ecx
  5071f9:	89 48 08             	mov    %ecx,0x8(%eax)
  5071fc:	8b 96 ac 00 00 00    	mov    0xac(%esi),%edx
  507202:	8b 72 08             	mov    0x8(%edx),%esi
  507205:	83 fe 02             	cmp    $0x2,%esi
  507208:	5e                   	pop    %esi
  507209:	75 3c                	jne    0x507247
  50720b:	8b 15 50 98 6e 00    	mov    0x6e9850,%edx
  507211:	4a                   	dec    %edx
  507212:	74 21                	je     0x507235
  507214:	4a                   	dec    %edx
  507215:	74 15                	je     0x50722c
  507217:	4a                   	dec    %edx
  507218:	75 2d                	jne    0x507247
  50721a:	8b 50 04             	mov    0x4(%eax),%edx
  50721d:	83 c2 e0             	add    $0xffffffe0,%edx
  507220:	83 c1 f0             	add    $0xfffffff0,%ecx
  507223:	89 50 04             	mov    %edx,0x4(%eax)
  507226:	89 48 08             	mov    %ecx,0x8(%eax)
  507229:	c2 04 00             	ret    $0x4
  50722c:	83 c1 e0             	add    $0xffffffe0,%ecx
  50722f:	89 48 08             	mov    %ecx,0x8(%eax)
  507232:	c2 04 00             	ret    $0x4
  507235:	8b 50 04             	mov    0x4(%eax),%edx
  507238:	8b 48 08             	mov    0x8(%eax),%ecx
  50723b:	83 c2 20             	add    $0x20,%edx
  50723e:	83 c1 f0             	add    $0xfffffff0,%ecx
  507241:	89 50 04             	mov    %edx,0x4(%eax)
  507244:	89 48 08             	mov    %ecx,0x8(%eax)
  507247:	c2 04 00             	ret    $0x4
  50724a:	90                   	nop
  50724b:	90                   	nop
  50724c:	90                   	nop
  50724d:	90                   	nop
  50724e:	90                   	nop
  50724f:	90                   	nop
  507250:	8b 81 b4 00 00 00    	mov    0xb4(%ecx),%eax
  507256:	56                   	push   %esi
  507257:	8b 74 24 08          	mov    0x8(%esp),%esi
  50725b:	8b 50 1c             	mov    0x1c(%eax),%edx
  50725e:	89 16                	mov    %edx,(%esi)
  507260:	8b 54 24 0c          	mov    0xc(%esp),%edx
  507264:	8b 40 20             	mov    0x20(%eax),%eax
  507267:	89 02                	mov    %eax,(%edx)
  507269:	8b 89 ac 00 00 00    	mov    0xac(%ecx),%ecx
  50726f:	83 79 08 02          	cmpl   $0x2,0x8(%ecx)
  507273:	75 38                	jne    0x5072ad
  507275:	8b 0d 50 98 6e 00    	mov    0x6e9850,%ecx
  50727b:	49                   	dec    %ecx
  50727c:	74 21                	je     0x50729f
  50727e:	49                   	dec    %ecx
  50727f:	74 15                	je     0x507296
  507281:	49                   	dec    %ecx
  507282:	75 29                	jne    0x5072ad
  507284:	8b 0e                	mov    (%esi),%ecx
  507286:	83 c1 e0             	add    $0xffffffe0,%ecx
  507289:	89 0e                	mov    %ecx,(%esi)
  50728b:	8b 02                	mov    (%edx),%eax
  50728d:	83 c0 f0             	add    $0xfffffff0,%eax
  507290:	5e                   	pop    %esi
  507291:	89 02                	mov    %eax,(%edx)
  507293:	c2 08 00             	ret    $0x8
  507296:	83 c0 e0             	add    $0xffffffe0,%eax
  507299:	89 02                	mov    %eax,(%edx)
  50729b:	5e                   	pop    %esi
  50729c:	c2 08 00             	ret    $0x8
  50729f:	8b 0e                	mov    (%esi),%ecx
  5072a1:	83 c1 20             	add    $0x20,%ecx
  5072a4:	89 0e                	mov    %ecx,(%esi)
  5072a6:	8b 02                	mov    (%edx),%eax
  5072a8:	83 c0 f0             	add    $0xfffffff0,%eax
  5072ab:	89 02                	mov    %eax,(%edx)
  5072ad:	5e                   	pop    %esi
  5072ae:	c2 08 00             	ret    $0x8
  5072b1:	90                   	nop
  5072b2:	90                   	nop
  5072b3:	90                   	nop
  5072b4:	90                   	nop
  5072b5:	90                   	nop
  5072b6:	90                   	nop
  5072b7:	90                   	nop
  5072b8:	90                   	nop
  5072b9:	90                   	nop
  5072ba:	90                   	nop
  5072bb:	90                   	nop
  5072bc:	90                   	nop
  5072bd:	90                   	nop
  5072be:	90                   	nop
  5072bf:	90                   	nop
  5072c0:	8b 81 b4 00 00 00    	mov    0xb4(%ecx),%eax
  5072c6:	56                   	push   %esi
  5072c7:	8b 74 24 08          	mov    0x8(%esp),%esi
  5072cb:	8b 50 24             	mov    0x24(%eax),%edx
  5072ce:	89 16                	mov    %edx,(%esi)
  5072d0:	8b 54 24 0c          	mov    0xc(%esp),%edx
  5072d4:	8b 40 28             	mov    0x28(%eax),%eax
  5072d7:	89 02                	mov    %eax,(%edx)
  5072d9:	8b 89 ac 00 00 00    	mov    0xac(%ecx),%ecx
  5072df:	83 79 08 02          	cmpl   $0x2,0x8(%ecx)
  5072e3:	75 38                	jne    0x50731d
  5072e5:	8b 0d 50 98 6e 00    	mov    0x6e9850,%ecx
  5072eb:	49                   	dec    %ecx
  5072ec:	74 21                	je     0x50730f
  5072ee:	49                   	dec    %ecx
  5072ef:	74 15                	je     0x507306
  5072f1:	49                   	dec    %ecx
  5072f2:	75 29                	jne    0x50731d
  5072f4:	8b 0e                	mov    (%esi),%ecx
  5072f6:	83 c1 e0             	add    $0xffffffe0,%ecx
  5072f9:	89 0e                	mov    %ecx,(%esi)
  5072fb:	8b 02                	mov    (%edx),%eax
  5072fd:	83 c0 f0             	add    $0xfffffff0,%eax
  507300:	5e                   	pop    %esi
  507301:	89 02                	mov    %eax,(%edx)
  507303:	c2 08 00             	ret    $0x8
  507306:	83 c0 e0             	add    $0xffffffe0,%eax
  507309:	89 02                	mov    %eax,(%edx)
  50730b:	5e                   	pop    %esi
  50730c:	c2 08 00             	ret    $0x8
  50730f:	8b 0e                	mov    (%esi),%ecx
  507311:	83 c1 20             	add    $0x20,%ecx
  507314:	89 0e                	mov    %ecx,(%esi)
  507316:	8b 02                	mov    (%edx),%eax
  507318:	83 c0 f0             	add    $0xfffffff0,%eax
  50731b:	89 02                	mov    %eax,(%edx)
  50731d:	5e                   	pop    %esi
  50731e:	c2 08 00             	ret    $0x8
  507321:	90                   	nop
  507322:	90                   	nop
  507323:	90                   	nop
  507324:	90                   	nop
  507325:	90                   	nop
  507326:	90                   	nop
  507327:	90                   	nop
  507328:	90                   	nop
  507329:	90                   	nop
  50732a:	90                   	nop
  50732b:	90                   	nop
  50732c:	90                   	nop
  50732d:	90                   	nop
  50732e:	90                   	nop
  50732f:	90                   	nop
  507330:	8b 91 29 08 00 00    	mov    0x829(%ecx),%edx
  507336:	8b c2                	mov    %edx,%eax
  507338:	56                   	push   %esi
  507339:	57                   	push   %edi
  50733a:	8b 70 04             	mov    0x4(%eax),%esi
  50733d:	8b 81 41 08 00 00    	mov    0x841(%ecx),%eax
  507343:	8b 80 10 01 00 00    	mov    0x110(%eax),%eax
  507349:	8b 38                	mov    (%eax),%edi
  50734b:	3b 77 0c             	cmp    0xc(%edi),%esi
  50734e:	76 02                	jbe    0x507352
  507350:	33 f6                	xor    %esi,%esi
  507352:	8b 78 08             	mov    0x8(%eax),%edi
  507355:	8b 34 b7             	mov    (%edi,%esi,4),%esi
  507358:	8b 78 0c             	mov    0xc(%eax),%edi
  50735b:	8b 44 24 0c          	mov    0xc(%esp),%eax
  50735f:	03 f7                	add    %edi,%esi
  507361:	5f                   	pop    %edi
  507362:	89 30                	mov    %esi,(%eax)
  507364:	8b 72 08             	mov    0x8(%edx),%esi
  507367:	89 70 04             	mov    %esi,0x4(%eax)
  50736a:	8b 52 0c             	mov    0xc(%edx),%edx
  50736d:	89 50 08             	mov    %edx,0x8(%eax)
  507370:	8b 89 ac 00 00 00    	mov    0xac(%ecx),%ecx
  507376:	8b 71 08             	mov    0x8(%ecx),%esi
  507379:	83 fe 02             	cmp    $0x2,%esi
  50737c:	5e                   	pop    %esi
  50737d:	75 3f                	jne    0x5073be
  50737f:	8b 0d 50 98 6e 00    	mov    0x6e9850,%ecx
  507385:	49                   	dec    %ecx
  507386:	74 24                	je     0x5073ac
  507388:	49                   	dec    %ecx
  507389:	74 18                	je     0x5073a3
  50738b:	49                   	dec    %ecx
  50738c:	75 30                	jne    0x5073be
  50738e:	8b 50 04             	mov    0x4(%eax),%edx
  507391:	8b 48 08             	mov    0x8(%eax),%ecx
  507394:	83 c2 e0             	add    $0xffffffe0,%edx
  507397:	83 c1 f0             	add    $0xfffffff0,%ecx
  50739a:	89 50 04             	mov    %edx,0x4(%eax)
  50739d:	89 48 08             	mov    %ecx,0x8(%eax)
  5073a0:	c2 04 00             	ret    $0x4
  5073a3:	83 c2 e0             	add    $0xffffffe0,%edx
  5073a6:	89 50 08             	mov    %edx,0x8(%eax)
  5073a9:	c2 04 00             	ret    $0x4
  5073ac:	8b 50 04             	mov    0x4(%eax),%edx
  5073af:	8b 48 08             	mov    0x8(%eax),%ecx
  5073b2:	83 c2 20             	add    $0x20,%edx
  5073b5:	83 c1 f0             	add    $0xfffffff0,%ecx
  5073b8:	89 50 04             	mov    %edx,0x4(%eax)
  5073bb:	89 48 08             	mov    %ecx,0x8(%eax)
  5073be:	c2 04 00             	ret    $0x4
  5073c1:	90                   	nop
