
void __fastcall FUN_00539e60(int param_1)

{
  char cVar1;
  undefined4 *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  int *piVar7;
  char *pcVar8;
  char *pcVar9;
  int local_26c [26];
  undefined4 local_204;
  char local_200 [256];
  char local_100 [256];
  
  if (*(int *)(param_1 + 0x16) == 0) {
    piVar7 = local_26c;
    for (iVar3 = 0x1b; iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar7 = 0;
      piVar7 = piVar7 + 1;
    }
    local_26c[0] = 0x6c;
    local_26c[1] = 7;
    local_204 = 0x840;
    local_26c[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    local_26c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    iVar3 = FUN_00597850(0x32);
    if (iVar3 == 0) {
      iVar3 = 0;
    }
    else {
      *(undefined4 *)(iVar3 + 4) = 0;
      *(undefined4 *)(iVar3 + 8) = 0;
      *(undefined4 *)(iVar3 + 0xc) = 0;
      *(undefined4 *)(iVar3 + 0x18) = 0;
      *(undefined4 *)(iVar3 + 0x14) = 0;
      *(undefined4 *)(iVar3 + 0x10) = 0;
      *(undefined4 *)(iVar3 + 0x20) = 0;
      *(undefined4 *)(iVar3 + 0x24) = 0;
      *(undefined4 *)(iVar3 + 0x28) = 0;
      *(undefined2 *)(iVar3 + 0x2c) = 0;
      DAT_006f694c = 0;
    }
    *(int *)(param_1 + 0x12) = iVar3;
    FUN_0058ad90(local_26c,s_c__Projects_Chaos_NetLoad_cpp_005eb08c,0x11b);
    if (*(int *)(param_1 + 0x16) == 0) {
      puVar2 = (undefined4 *)FUN_00597850(0xc);
      if (puVar2 == (undefined4 *)0x0) {
        puVar2 = (undefined4 *)0x0;
      }
      else {
        puVar2[1] = &LAB_0053a070;
        puVar2[2] = param_1;
        *puVar2 = &PTR_LAB_005c7318;
      }
      *(undefined4 **)(param_1 + 0x16) = puVar2;
      *(undefined4 **)(*(int *)(param_1 + 0x12) + 0x28) = puVar2;
    }
    FUN_0059c1fd(local_200,s__s_ix_i__005da418,s_Interface_MultiplayerBattleLoad__005eb068,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    FUN_0059c1fd(local_100,s__s__i__i_s_005da5a8,s_Multiplayer_Battle_Load_005eb050,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,&DAT_005da5b4);
    uVar4 = 0xffffffff;
    pcVar6 = local_100;
    do {
      pcVar9 = pcVar6;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar9 = pcVar6 + 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar9;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    iVar3 = -1;
    pcVar6 = local_200;
    do {
      pcVar8 = pcVar6;
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      pcVar8 = pcVar6 + 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar8;
    } while (cVar1 != '\0');
    pcVar6 = pcVar9 + -uVar4;
    pcVar9 = pcVar8 + -1;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar6;
      pcVar6 = pcVar6 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar9 = *pcVar6;
      pcVar6 = pcVar6 + 1;
      pcVar9 = pcVar9 + 1;
    }
    if (*(int *)(param_1 + 0x12) != 0) {
      FUN_0058d320(local_200);
    }
  }
  return;
}

