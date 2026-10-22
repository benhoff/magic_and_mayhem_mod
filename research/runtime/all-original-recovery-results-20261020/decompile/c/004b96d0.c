
void __fastcall FUN_004b96d0(int *param_1)

{
  int *piVar1;
  bool bVar2;
  undefined4 *puVar3;
  int iVar4;
  undefined4 uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c1021;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  FUN_004b7240();
  (**(code **)(*param_1 + 0x1c))(s_Interface_MultiplayerBattleSetup_005e0ff4);
  puVar3 = (undefined4 *)FUN_00597850(0x10);
  if (puVar3 == (undefined4 *)0x0) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3[1] = &LAB_004b9c30;
    puVar3[2] = 0;
    puVar3[3] = param_1 + -1;
    *puVar3 = &PTR_LAB_005c6938;
  }
  *(undefined4 **)param_1[0x29] = puVar3;
  puVar3 = (undefined4 *)FUN_00597850(0x10);
  if (puVar3 == (undefined4 *)0x0) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3[1] = &LAB_004b9dd0;
    puVar3[2] = 0;
    puVar3[3] = param_1 + -1;
    *puVar3 = &PTR_LAB_005c6938;
  }
  *(undefined4 **)(param_1[0x29] + 4) = puVar3;
  puVar3 = (undefined4 *)FUN_00597850(0x10);
  if (puVar3 == (undefined4 *)0x0) {
    puVar3 = (undefined4 *)0x0;
  }
  else {
    puVar3[1] = &LAB_004b9e50;
    puVar3[2] = 0;
    puVar3[3] = param_1 + -1;
    *puVar3 = &PTR_LAB_005c6938;
  }
  iVar8 = 0;
  iVar7 = 0;
  *(undefined4 **)(param_1[0x29] + 8) = puVar3;
  do {
    iVar4 = FUN_00597850(0x86);
    puStack_8 = (undefined1 *)0x0;
    if (iVar4 == 0) {
      uVar5 = 0;
    }
    else {
      uVar5 = FUN_004cdac0();
    }
    puStack_8 = (undefined1 *)0xffffffff;
    *(undefined4 *)(param_1[0x2a] + iVar8 * 4) = uVar5;
    FUN_004ca9b0(&DAT_006a49e0);
    if (DAT_006de6d5 == 0) {
      uVar5 = *(undefined4 *)param_1[0x29];
      iVar4 = iVar7 + 4 + param_1[0x2d];
    }
    else {
      uVar5 = *(undefined4 *)param_1[0x29];
      iVar4 = iVar7 + 0x14 + param_1[0x2d];
    }
    FUN_004ce050(iVar8,iVar4,uVar5,1);
    FUN_004cdff0(*(undefined4 *)(iVar7 + param_1[0x2d] + 0x134),
                 *(undefined4 *)(iVar7 + 0x130 + param_1[0x2d]));
    iVar6 = iVar7 + 0x13d;
    iVar4 = iVar8 * 4;
    iVar7 = iVar7 + 0x141;
    iVar8 = iVar8 + 1;
    *(undefined4 *)(*(int *)(param_1[0x2a] + iVar4) + 0x76) = *(undefined4 *)(iVar6 + param_1[0x2d])
    ;
  } while (iVar8 < 0xd);
  iVar8 = 0;
  iVar7 = 0x104d;
  do {
    iVar4 = FUN_00597850(0x59);
    puStack_8 = (undefined1 *)0x1;
    if (iVar4 == 0) {
      uVar5 = 0;
    }
    else {
      uVar5 = FUN_004cc720();
    }
    puStack_8 = (undefined1 *)0xffffffff;
    *(undefined4 *)(param_1[0x2b] + iVar8 * 4) = uVar5;
    FUN_004cb040(iVar7 + 0x24 + param_1[0x2d]);
    FUN_004cb030(*(undefined4 *)(iVar7 + param_1[0x2d]));
    FUN_004ca9b0(&DAT_006a49e0);
    *(undefined4 *)(*(int *)(param_1[0x2b] + iVar8 * 4) + 0x55) = 0x28;
    if (DAT_006de6d5 == 0) {
      uVar5 = *(undefined4 *)(param_1[0x29] + 4);
      iVar4 = iVar7 + 4 + param_1[0x2d];
    }
    else {
      uVar5 = *(undefined4 *)(param_1[0x29] + 4);
      iVar4 = iVar7 + 0x14 + param_1[0x2d];
    }
    FUN_004cb0b0(iVar8,iVar4,uVar5,0,1);
    iVar7 = iVar7 + 0x141;
    iVar8 = iVar8 + 1;
  } while (iVar8 < 2);
  iVar4 = 0;
  iVar8 = 0x12cf;
  iVar7 = 1;
  do {
    iVar6 = FUN_00597850(0x7a);
    puStack_8 = (undefined1 *)0x2;
    if (iVar6 == 0) {
      uVar5 = 0;
    }
    else {
      uVar5 = FUN_004cb4b0();
    }
    puStack_8 = (undefined1 *)0xffffffff;
    *(undefined4 *)(iVar4 + param_1[0x2c]) = uVar5;
    FUN_004ca9b0(&DAT_006a49e0);
    FUN_004cb810((int)param_1 + 0x67);
    FUN_004cb7d0(0x18,0x1a,0xffffffff);
    FUN_004cb7f0(0xffffffff,0x19,0xffffffff);
    if (DAT_006de6d5 == 0) {
      uVar5 = *(undefined4 *)(param_1[0x29] + 8);
      iVar6 = param_1[0x2d] + 4;
    }
    else {
      uVar5 = *(undefined4 *)(param_1[0x29] + 8);
      iVar6 = param_1[0x2d] + 0x14;
    }
    FUN_004cb0b0(iVar7,iVar6 + iVar8,uVar5,0,1);
    iVar8 = iVar8 + 0x141;
    iVar4 = iVar4 + 4;
    bVar2 = iVar7 < 3;
    iVar7 = iVar7 + 1;
  } while (bVar2);
  iVar7 = 0;
  do {
    FUN_004cad00(*(undefined4 *)(iVar7 + param_1[0x2a]),0);
    iVar7 = iVar7 + 4;
  } while (iVar7 < 0x34);
  iVar7 = 0;
  do {
    FUN_004cad00(*(undefined4 *)(iVar7 + param_1[0x2b]),0);
    iVar7 = iVar7 + 4;
  } while (iVar7 < 8);
  iVar7 = 0;
  do {
    FUN_004cad00(*(undefined4 *)(iVar7 + param_1[0x2c]),0);
    iVar7 = iVar7 + 4;
  } while (iVar7 < 0xc);
  FUN_004ba850();
  if (DAT_006e1f0c != 0) {
    DAT_006c375c = 200;
    DAT_006c3758 = DAT_00656620;
    DAT_006c3760 = 400;
    DAT_006c3778 = 0;
  }
  iVar7 = FUN_004b8090();
  uVar5 = DAT_006c3758;
  if (iVar7 == 0) {
    FUN_004d28c0(s_No_Map_005e07c8);
    DAT_006c3758 = 0;
    piVar1 = *(int **)((int)param_1 + 0x8b);
  }
  else {
    FUN_004d28c0(iVar7);
    piVar1 = *(int **)((int)param_1 + 0x8b);
    DAT_006c3758 = uVar5;
  }
  if (piVar1 != (int *)0x0) {
    (**(code **)(*piVar1 + 0x60))(&DAT_006c3758);
  }
  FUN_004ba340(&DAT_006c3758);
  FUN_004b7f30(&DAT_006c3758);
  ExceptionList = param_1;
  return;
}

