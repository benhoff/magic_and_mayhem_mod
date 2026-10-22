
void __fastcall FUN_004b7240(int *param_1)

{
  char cVar1;
  undefined4 *puVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  char *pcVar8;
  int *piVar9;
  char *pcVar10;
  char *pcVar11;
  int iVar12;
  int aiStack_47c [26];
  undefined4 uStack_414;
  char acStack_410 [256];
  char acStack_310 [256];
  undefined1 auStack_210 [256];
  undefined1 auStack_110 [256];
  undefined4 uStack_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c0e74;
  local_c = ExceptionList;
  if (param_1[2] == 0) {
    ExceptionList = &local_c;
    (**(code **)(*param_1 + 0x1c))(s_Interface_MultiplayerBattleSetup_005e0ff4);
    piVar9 = aiStack_47c;
    for (iVar5 = 0x1b; iVar5 != 0; iVar5 = iVar5 + -1) {
      *piVar9 = 0;
      piVar9 = piVar9 + 1;
    }
    aiStack_47c[0] = 0x6c;
    aiStack_47c[1] = 7;
    uStack_414 = 0x840;
    aiStack_47c[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    aiStack_47c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    iVar5 = FUN_00597850(0x32);
    if (iVar5 == 0) {
      iVar5 = 0;
    }
    else {
      *(undefined4 *)(iVar5 + 4) = 0;
      *(undefined4 *)(iVar5 + 8) = 0;
      *(undefined4 *)(iVar5 + 0xc) = 0;
      *(undefined4 *)(iVar5 + 0x18) = 0;
      *(undefined4 *)(iVar5 + 0x14) = 0;
      *(undefined4 *)(iVar5 + 0x10) = 0;
      *(undefined4 *)(iVar5 + 0x20) = 0;
      *(undefined4 *)(iVar5 + 0x24) = 0;
      *(undefined4 *)(iVar5 + 0x28) = 0;
      *(undefined2 *)(iVar5 + 0x2c) = 0;
      DAT_006f694c = 0;
    }
    *(int *)((int)param_1 + 0x13) = iVar5;
    FUN_0058ad90(aiStack_47c,s_c__Projects_Chaos_game_cpp_005e05c0,0x2205);
    if (*(int *)((int)param_1 + 0x47) == 0) {
      puVar2 = (undefined4 *)FUN_00597850(0x10);
      if (puVar2 == (undefined4 *)0x0) {
        puVar2 = (undefined4 *)0x0;
      }
      else {
        puVar2[1] = &LAB_004b7df0;
        puVar2[2] = 0;
        puVar2[3] = param_1 + -1;
        *puVar2 = &PTR_LAB_005c67f0;
      }
      *(undefined4 **)((int)param_1 + 0x47) = puVar2;
      *(undefined4 **)(*(int *)((int)param_1 + 0x13) + 0x28) = puVar2;
    }
    FUN_0059c1fd(acStack_410,s__s_ix_i__005da418,s_Interface_MultiplayerBattleSetup_005e0fd0,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    FUN_00559230(auStack_210);
    FUN_0059c1fd(acStack_310,s__s__i__i_s_005da5a8,auStack_210,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,&DAT_005da5b4);
    uVar6 = 0xffffffff;
    pcVar8 = acStack_310;
    do {
      pcVar11 = pcVar8;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar11 = pcVar8 + 1;
      cVar1 = *pcVar8;
      pcVar8 = pcVar11;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    iVar5 = -1;
    pcVar8 = acStack_410;
    do {
      pcVar10 = pcVar8;
      if (iVar5 == 0) break;
      iVar5 = iVar5 + -1;
      pcVar10 = pcVar8 + 1;
      cVar1 = *pcVar8;
      pcVar8 = pcVar10;
    } while (cVar1 != '\0');
    pcVar8 = pcVar11 + -uVar6;
    pcVar11 = pcVar10 + -1;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar11 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar11 = pcVar11 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar11 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar11 = pcVar11 + 1;
    }
    iVar5 = FUN_0058d320(acStack_410);
    if (iVar5 == 0) {
      FUN_0049e4e0();
    }
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[1] = &LAB_004b7bf0;
      puVar2[2] = 0;
      puVar2[3] = param_1 + -1;
      *puVar2 = &PTR_LAB_005c67f0;
    }
    **(undefined4 **)((int)param_1 + 0x4b) = puVar2;
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[1] = &LAB_004b7c20;
      puVar2[2] = 0;
      puVar2[3] = param_1 + -1;
      *puVar2 = &PTR_LAB_005c67f0;
    }
    *(undefined4 **)(*(int *)((int)param_1 + 0x4b) + 4) = puVar2;
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[1] = &LAB_004b7db0;
      puVar2[2] = 0;
      puVar2[3] = param_1 + -1;
      *puVar2 = &PTR_LAB_005c67f0;
    }
    *(undefined4 **)(*(int *)((int)param_1 + 0x4b) + 8) = puVar2;
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[1] = FUN_004b7bc0;
      puVar2[2] = 0;
      puVar2[3] = param_1 + -1;
      *puVar2 = &PTR_LAB_005c67f0;
    }
    *(undefined4 **)(*(int *)((int)param_1 + 0x4b) + 0xc) = puVar2;
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[1] = FUN_004b7ef0;
      puVar2[2] = 0;
      puVar2[3] = param_1 + -1;
      *puVar2 = &PTR_LAB_005c67f0;
    }
    *(undefined4 **)(*(int *)((int)param_1 + 0x4b) + 0x10) = puVar2;
    FUN_0059c1fd(auStack_110,s_Interface_MultiplayerBattleSetup_005da560,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    iVar5 = FUN_0057d310(auStack_110,2);
    if (iVar5 == 0) {
      MessageBoxA((HWND)0x0,s_ERROR_005d96d8,s_Unable_to_load_sprite_in_mutliPb_005da530,0x42030);
      DestroyWindow(DAT_00656614);
    }
    iVar5 = 0;
    iVar12 = 0;
    do {
      iVar3 = FUN_00597850(0x29);
      puStack_8 = (undefined1 *)0x0;
      if (iVar3 == 0) {
        uVar4 = 0;
      }
      else {
        uVar4 = FUN_004d27b0();
      }
      puStack_8 = (undefined1 *)0xffffffff;
      *(undefined4 *)(iVar5 + *(int *)((int)param_1 + 0x5b)) = uVar4;
      FUN_004d28b0(*(undefined4 *)(*(int *)((int)param_1 + 0x3f) + iVar12));
      FUN_004d2960(&DAT_006a49e0);
      FUN_004d28c0(*(int *)((int)param_1 + 0x3f) + 0x24 + iVar12);
      FUN_004d2970(*(undefined1 *)(*(int *)((int)param_1 + 0x3f) + 0x138 + iVar12));
      iVar3 = *(int *)((int)param_1 + 0x3f) + 4;
      if (DAT_006de6d5 != 0) {
        iVar3 = *(int *)((int)param_1 + 0x3f) + 0x14;
      }
      FUN_004d2930(iVar3 + iVar12);
      iVar5 = iVar5 + 4;
      iVar12 = iVar12 + 0x141;
    } while (iVar5 < 0xa0);
    iVar5 = 0x10;
    do {
      FUN_004d28c0(&DAT_005da8a0);
      iVar5 = iVar5 + 4;
    } while (iVar5 < 0x20);
    iVar5 = 0x6c;
    do {
      FUN_004d29a0(0);
      iVar5 = iVar5 + 4;
    } while (iVar5 < 0x9d);
    iVar5 = FUN_00597850(0x59);
    puStack_8 = (undefined1 *)0x1;
    if (iVar5 == 0) {
      uVar4 = 0;
    }
    else {
      uVar4 = FUN_004cc720();
    }
    puStack_8 = (undefined1 *)0xffffffff;
    **(undefined4 **)((int)param_1 + 0x57) = uVar4;
    FUN_004cb040(*(int *)((int)param_1 + 0x3f) + 0x324c);
    FUN_004cb030(*(undefined4 *)(*(int *)((int)param_1 + 0x3f) + 0x3228));
    FUN_004ca9b0(&DAT_006a49e0);
    *(undefined4 *)(**(int **)((int)param_1 + 0x57) + 0x55) = 0x28;
    if (DAT_006de6d5 == 0) {
      iVar5 = *(int *)((int)param_1 + 0x3f) + 0x322c;
      uVar4 = **(undefined4 **)((int)param_1 + 0x4b);
    }
    else {
      iVar5 = *(int *)((int)param_1 + 0x3f) + 0x323c;
      uVar4 = **(undefined4 **)((int)param_1 + 0x4b);
    }
    FUN_004cb0b0(0,iVar5,uVar4,0,1);
    iVar5 = FUN_00597850(0x6d);
    puStack_8 = (undefined1 *)0x2;
    if (iVar5 == 0) {
      uVar4 = 0;
    }
    else {
      uVar4 = FUN_004d1f60();
    }
    *(undefined4 *)((int)param_1 + 0x4f) = uVar4;
    puStack_8 = (undefined1 *)0xffffffff;
    FUN_004d2260(*(undefined4 *)(*(int *)((int)param_1 + 0x3f) + 0x3369));
    FUN_004ca9b0(&DAT_006a49e0);
    *(undefined4 *)(*(int *)((int)param_1 + 0x4f) + 0x61) = 0x28;
    if (DAT_006de6d5 == 0) {
      iVar5 = *(int *)((int)param_1 + 0x3f) + 0x336d;
    }
    else {
      iVar5 = *(int *)((int)param_1 + 0x3f) + 0x337d;
    }
    (**(code **)(**(int **)((int)param_1 + 0x4f) + 8))(iVar5,1);
    iVar5 = FUN_00597850(100);
    uStack_10 = 3;
    if (iVar5 == 0) {
      uVar4 = 0;
    }
    else {
      uVar4 = FUN_004cccc0();
    }
    *(undefined4 *)((int)param_1 + 0x53) = uVar4;
    uStack_10 = 0xffffffff;
    FUN_004cb030(*(undefined4 *)(*(int *)((int)param_1 + 0x3f) + 0x34aa));
    FUN_004ca9b0(&DAT_006a49e0);
    *(undefined4 *)(*(int *)((int)param_1 + 0x53) + 0x59) = 0x28;
    if (DAT_006de6d5 == 0) {
      iVar5 = *(int *)((int)param_1 + 0x3f) + 0x34ae;
    }
    else {
      iVar5 = *(int *)((int)param_1 + 0x3f) + 0x34be;
    }
    FUN_004cb0b0(0,iVar5,*(undefined4 *)(*(int *)((int)param_1 + 0x4b) + 0xc),0,1);
    FUN_004ca930();
    iVar12 = 0x35eb;
    puVar2 = &DAT_005c6300;
    iVar5 = 0;
    do {
      iVar3 = iVar5;
      aiStack_47c[0] = FUN_00597850(0x7a);
      uStack_4 = 4;
      if (aiStack_47c[0] == 0) {
        uVar4 = 0;
      }
      else {
        uVar4 = FUN_004cb4b0();
      }
      uStack_4 = 0xffffffff;
      *(undefined4 *)(*(int *)((int)param_1 + 0x5f) + iVar3 * 4) = uVar4;
      FUN_004ca9b0(&DAT_006a49e0);
      FUN_004cb810((int)param_1 + 0x67);
      FUN_004cb7d0(*puVar2,puVar2[2],*puVar2);
      FUN_004cb7f0(0xffffffff,puVar2[1],0xffffffff);
      if (DAT_006de6d5 == 0) {
        uVar4 = *(undefined4 *)(*(int *)((int)param_1 + 0x4b) + 4);
        iVar5 = *(int *)((int)param_1 + 0x3f) + 4;
      }
      else {
        uVar4 = *(undefined4 *)(*(int *)((int)param_1 + 0x4b) + 4);
        iVar5 = *(int *)((int)param_1 + 0x3f) + 0x14;
      }
      FUN_004cb0b0(iVar3,iVar5 + iVar12,uVar4,0,1);
      (**(code **)(**(int **)(*(int *)((int)param_1 + 0x5f) + iVar3 * 4) + 0x18))();
      iVar12 = iVar12 + 0x141;
      puVar2 = puVar2 + 3;
      iVar5 = iVar3 + 1;
    } while ((int)puVar2 < 0x5c6330);
    if (iVar5 < 8) {
      iVar12 = 0x3aef;
      puVar2 = &DAT_005c62a0 + iVar3 + -0xb + iVar5 * 2;
      do {
        aiStack_47c[0] = FUN_00597850(0x7a);
        uStack_4 = 5;
        if (aiStack_47c[0] == 0) {
          uVar4 = 0;
        }
        else {
          uVar4 = FUN_004cb4b0();
        }
        uStack_4 = 0xffffffff;
        *(undefined4 *)(*(int *)((int)param_1 + 0x5f) + iVar5 * 4) = uVar4;
        FUN_004ca9b0(&DAT_006a49e0);
        FUN_004cb810((int)param_1 + 0x67);
        FUN_004cb7d0(*puVar2,puVar2[2],*puVar2);
        FUN_004cb7f0(0xffffffff,puVar2[1],0xffffffff);
        if (DAT_006de6d5 == 0) {
          uVar4 = *(undefined4 *)(*(int *)((int)param_1 + 0x4b) + 4);
          iVar3 = iVar12 + 4 + *(int *)((int)param_1 + 0x3f);
        }
        else {
          uVar4 = *(undefined4 *)(*(int *)((int)param_1 + 0x4b) + 4);
          iVar3 = iVar12 + 0x14 + *(int *)((int)param_1 + 0x3f);
        }
        FUN_004cb0b0(iVar5,iVar3,uVar4,0,1);
        (**(code **)(**(int **)(*(int *)((int)param_1 + 0x5f) + iVar5 * 4) + 0x18))();
        iVar12 = iVar12 + 0x141;
        puVar2 = puVar2 + 3;
        iVar5 = iVar5 + 1;
      } while ((int)puVar2 < 0x5c62d0);
    }
    FUN_004cabd0(10);
    if (**(int **)((int)param_1 + 0x57) != 0) {
      FUN_004cad00(**(int **)((int)param_1 + 0x57),0);
    }
    iVar5 = 0;
    do {
      FUN_004cad00(*(undefined4 *)(*(int *)((int)param_1 + 0x5f) + iVar5),0);
      iVar5 = iVar5 + 4;
    } while (iVar5 < 0x20);
    FUN_004cad00(*(undefined4 *)((int)param_1 + 0x53),1);
    param_1[2] = 1;
  }
  ExceptionList = local_c;
  return;
}

