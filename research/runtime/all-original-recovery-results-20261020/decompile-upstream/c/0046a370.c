
void __fastcall FUN_0046a370(int param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  char *pcVar4;
  char *pcVar5;
  uint uVar6;
  uint uVar7;
  char *pcVar8;
  
  iVar2 = FUN_00473900();
  iVar3 = __strcmpi((char *)&DAT_006dde58,s_Multiplayer_005d9e58);
  if (iVar3 == 0) {
    pcVar4 = (char *)FUN_00597850(0x100);
    uVar6 = 0xffffffff;
    pcVar5 = (char *)(param_1 + 0xfc61);
    do {
      pcVar8 = pcVar5;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar8 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar8;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar5 = pcVar8 + -uVar6;
    pcVar8 = pcVar4;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar8 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar8 = pcVar8 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar8 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar8 = pcVar8 + 1;
    }
    cVar1 = *pcVar4;
    pcVar5 = pcVar4;
    while ((cVar1 != '\0' && (cVar1 != '\\'))) {
      pcVar8 = pcVar5 + 1;
      pcVar5 = pcVar5 + 1;
      cVar1 = *pcVar8;
    }
    *pcVar5 = '\0';
    FUN_0058f3c0(pcVar4);
    FUN_00597870(pcVar4);
  }
  else {
    FUN_0058f3c0(&DAT_006dde58);
  }
  FUN_00482010(1);
  FUN_00481890();
  if (*(int *)(iVar2 + 4) != 0) {
    FUN_0058f6c0();
    iVar2 = FUN_0058f6d0();
    while (iVar2 != -1) {
      FUN_00481860(iVar2);
      iVar2 = FUN_0058f6d0();
    }
  }
  FUN_00481a20(1);
  return;
}

