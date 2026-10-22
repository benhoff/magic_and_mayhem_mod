
undefined4 __fastcall FUN_004b1a30(int param_1)

{
  byte bVar1;
  int iVar2;
  char cVar3;
  int iVar4;
  byte *pbVar5;
  char *pcVar6;
  uint uVar7;
  uint uVar8;
  byte *pbVar9;
  byte *pbVar10;
  char *pcVar11;
  bool bVar12;
  undefined4 uVar13;
  undefined4 uStack_4;
  
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
  pbVar9 = &DAT_005faf28;
  pbVar10 = *(byte **)(**(int **)(param_1 + 0x53) + 0x45);
  pbVar5 = pbVar10;
  do {
    bVar1 = *pbVar5;
    bVar12 = bVar1 < *pbVar9;
    if (bVar1 != *pbVar9) {
LAB_004b1ad3:
      iVar4 = (1 - (uint)bVar12) - (uint)(bVar12 != 0);
      goto LAB_004b1ad8;
    }
    if (bVar1 == 0) break;
    bVar1 = pbVar5[1];
    bVar12 = bVar1 < pbVar9[1];
    if (bVar1 != pbVar9[1]) goto LAB_004b1ad3;
    pbVar5 = pbVar5 + 2;
    pbVar9 = pbVar9 + 2;
  } while (bVar1 != 0);
  iVar4 = 0;
LAB_004b1ad8:
  if (iVar4 == 0) {
    return 0;
  }
  uVar7 = 0xffffffff;
  do {
    pbVar5 = pbVar10;
    if (uVar7 == 0) break;
    uVar7 = uVar7 - 1;
    pbVar5 = pbVar10 + 1;
    bVar1 = *pbVar10;
    pbVar10 = pbVar5;
  } while (bVar1 != 0);
  uVar7 = ~uVar7;
  pbVar10 = pbVar5 + -uVar7;
  pbVar5 = (byte *)&DAT_006de59c;
  for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
    *(undefined4 *)pbVar5 = *(undefined4 *)pbVar10;
    pbVar10 = pbVar10 + 4;
    pbVar5 = pbVar5 + 4;
  }
  for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
    *pbVar5 = *pbVar10;
    pbVar10 = pbVar10 + 1;
    pbVar5 = pbVar5 + 1;
  }
  uStack_4 = param_1;
  cVar3 = FUN_005358a0();
  if (cVar3 == '\0') {
    uVar13 = 8;
  }
  else {
    DAT_006de6ac = 1;
    FUN_005365b0();
    FUN_00537b80();
    if (DAT_006de6b0 == 3) {
      FUN_0058bc10(0);
      FUN_0058bc10(0);
      ShowCursor(1);
      *(undefined1 *)(param_1 + 0x11) = 0;
    }
    iVar4 = FUN_00535cb0((int)&uStack_4 + 3);
    if (iVar4 == 0) {
      if (DAT_006de6b0 == 3) {
        ShowCursor(0);
      }
      if (uStack_4._3_1_ != '\0') goto LAB_004b1c34;
      uVar13 = 0xc;
    }
    else {
      if (DAT_006de6b0 == 3) {
        ShowCursor(0);
      }
      iVar4 = FUN_00536270();
      if (iVar4 == 0) {
        uVar13 = 0xb;
      }
      else {
        uVar7 = 0xffffffff;
        pcVar6 = *(char **)(*(int *)(*(int *)(param_1 + 0x53) + 4) + 0x45);
        pcVar11 = pcVar6;
        do {
          if (uVar7 == 0) break;
          uVar7 = uVar7 - 1;
          cVar3 = *pcVar11;
          pcVar11 = pcVar11 + 1;
        } while (cVar3 != '\0');
        uVar7 = ~uVar7 - 1;
        cVar3 = FUN_0047d920(uVar7,1);
        if (cVar3 != '\0') {
          pcVar11 = DAT_006de715;
          for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
            *(undefined4 *)pcVar11 = *(undefined4 *)pcVar6;
            pcVar6 = pcVar6 + 4;
            pcVar11 = pcVar11 + 4;
          }
          for (uVar8 = uVar7 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
            *pcVar11 = *pcVar6;
            pcVar6 = pcVar6 + 1;
            pcVar11 = pcVar11 + 1;
          }
          DAT_006de719 = uVar7;
          DAT_006de715[uVar7] = '\0';
        }
        FUN_0054c890();
        pcVar6 = DAT_006de715;
        if (DAT_006de715 == (char *)0x0) {
          pcVar6 = &DAT_005c6160;
        }
        iVar4 = FUN_00537da0(pcVar6);
        if (iVar4 != 0) {
          DAT_006de6b4 = 1;
          return 1;
        }
        uVar13 = 10;
      }
    }
  }
  FUN_0058a680(uVar13);
  FUN_0049e680();
LAB_004b1c34:
  FUN_00535970();
  return 0;
}

