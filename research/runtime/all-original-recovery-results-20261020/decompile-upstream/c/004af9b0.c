
undefined4 __fastcall FUN_004af9b0(int param_1)

{
  byte bVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  byte *pbVar5;
  byte *pbVar6;
  bool bVar7;
  
  iVar4 = 0;
  if ((0 < *(int *)(param_1 + 0x67)) &&
     (iVar2 = *(int *)(*(int *)(param_1 + 0x5f) + *(int *)(param_1 + 99) * 4), iVar2 != 0)) {
    iVar4 = *(int *)(iVar2 + 0x2d);
  }
  if (iVar4 == 0) {
    DAT_006de6b0 = 1;
    DAT_006ead94 = DAT_005eab08;
  }
  else if (iVar4 == 1) {
    DAT_006de6b0 = 2;
    DAT_006ead94 = DAT_005eab0c;
  }
  else if (iVar4 == 2) {
    DAT_006de6b0 = 3;
    DAT_006ead94 = DAT_005eab10;
  }
  pbVar6 = &DAT_005faf28;
  pbVar5 = *(byte **)(*(int *)(param_1 + 0x5b) + 0x45);
  do {
    bVar1 = *pbVar5;
    bVar7 = bVar1 < *pbVar6;
    if (bVar1 != *pbVar6) {
LAB_004afa46:
      iVar4 = (1 - (uint)bVar7) - (uint)(bVar7 != 0);
      goto LAB_004afa4b;
    }
    if (bVar1 == 0) break;
    bVar1 = pbVar5[1];
    bVar7 = bVar1 < pbVar6[1];
    if (bVar1 != pbVar6[1]) goto LAB_004afa46;
    pbVar5 = pbVar5 + 2;
    pbVar6 = pbVar6 + 2;
  } while (bVar1 != 0);
  iVar4 = 0;
LAB_004afa4b:
  if (iVar4 == 0) {
    return 0;
  }
  cVar3 = FUN_005358a0();
  if (cVar3 != '\0') {
    DAT_006de6ac = 0;
    FUN_005365b0();
    FUN_00537b80();
    return 1;
  }
  FUN_0058a680(8);
  FUN_0049e680();
  FUN_00535970();
  return 0;
}

