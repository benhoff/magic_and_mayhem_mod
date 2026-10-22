
undefined4 FUN_004b25e0(void)

{
  char cVar1;
  undefined4 uVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  char *pcVar7;
  char *pcVar8;
  char *pcVar9;
  char local_300 [256];
  undefined1 local_200 [256];
  char local_100 [256];
  
  FUN_0059c1fd(local_300,s__s_ix_i__005da418,s_Interface_MiniMenu__005e0af4,
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  FUN_00559230(local_200);
  FUN_0059c1fd(local_100,s__s__i__i_s_005da5a8,local_200,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280
               ,(-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,&DAT_005e0aec);
  uVar3 = 0xffffffff;
  pcVar7 = local_100;
  do {
    pcVar9 = pcVar7;
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    pcVar9 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar9;
  } while (cVar1 != '\0');
  uVar3 = ~uVar3;
  iVar4 = -1;
  pcVar7 = local_300;
  do {
    pcVar8 = pcVar7;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  pcVar7 = pcVar9 + -uVar3;
  pcVar9 = pcVar8 + -1;
  for (uVar5 = uVar3 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar9 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar9 = pcVar9 + 4;
  }
  for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
    *pcVar9 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar9 = pcVar9 + 1;
  }
  if (DAT_006de6d5 == 0) {
    uVar2 = 0x50;
    uVar6 = 0x50;
  }
  else {
    uVar2 = 100;
    uVar6 = 100;
  }
  iVar4 = FUN_0058d280(local_300,uVar2,uVar6);
  if (iVar4 == 0) {
    FUN_0049e4e0();
  }
  return 0;
}

