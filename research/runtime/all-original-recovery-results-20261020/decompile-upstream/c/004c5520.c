
void __thiscall FUN_004c5520(int param_1,int param_2,int param_3,int param_4,undefined4 param_5)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 *puVar4;
  int *piVar5;
  char *pcVar6;
  int iVar7;
  uint uVar8;
  undefined4 *puVar9;
  uint local_234;
  int local_22c;
  int local_228;
  int local_224;
  int local_220;
  uint local_21c;
  undefined1 local_218 [256];
  undefined1 local_118 [20];
  undefined1 local_104 [260];
  
  local_21c = (uint)(param_4 / 2 == (param_4 + 1) / 2);
  iVar1 = FUN_005908e0();
  if (*(int *)(&DAT_0065bb86 + iVar1 * 0x93a) == 0) {
    local_234 = 0;
  }
  else {
    iVar1 = FUN_005908e0();
    local_234 = (uint)*(byte *)(*(int *)(&DAT_0065bb86 + iVar1 * 0x93a) + param_2 * 0x7c + param_3);
  }
  if (DAT_006e1f20 == 1) {
    local_234 = 4;
  }
  pcVar6 = s__s_ix_i_C_cS_02i_P_c____005e1fb0;
  if (99 < param_3) {
    pcVar6 = s__s_ix_i_C_cS_03i_P_c____005e1f98;
  }
  FUN_0059c1fd(local_218,pcVar6,"Interface\\Grimoire\\",(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,param_2 + 0x30,param_3,param_4 + 0x30);
  iVar1 = FUN_0059dc1d(local_218,local_118);
  if (iVar1 == -1) {
    FUN_0059c1fd(local_218,s__s_ix_i_Generic_c_02i___005e1f74,"Interface\\Grimoire\\",
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,
                 (int)(char)((-(local_21c != 0) & 6U) + 0x4c),
                 (param_2 + param_3 + param_4) % *(int *)(param_1 + 0x56 + local_21c * 4) + 1);
    iVar1 = FUN_0059dc1d(local_218,local_118);
    if (iVar1 != -1) {
      FUN_0059c1fd(local_218,s__s_ix_i__s_005e1f8c,"Interface\\Grimoire\\",
                   (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                   (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,local_104);
      FUN_0059ddac(iVar1);
    }
  }
  else {
    FUN_0059c1fd(local_218,s__s_ix_i__s_005e1f8c,"Interface\\Grimoire\\",
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,local_104);
    FUN_0059ddac(iVar1);
  }
  iVar1 = FUN_0058d320(local_218);
  if (iVar1 == 0) {
    FUN_0058d1a0(local_218);
  }
  FUN_004c1ea0(param_5);
  FUN_0058db80(0,0,0xff,10);
  piVar5 = *(int **)(param_1 + 0x12);
  if (0 < piVar5[1]) {
    piVar3 = (int *)*piVar5;
    local_22c = *piVar3;
    local_228 = piVar3[1];
    local_224 = piVar3[2];
    local_220 = piVar3[3];
    piVar5[2] = 1;
    while( true ) {
      iVar1 = local_22c;
      if (local_21c == 1) {
        iVar7 = 0;
        iVar2 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
        if (iVar2 == 0x280) {
          iVar7 = 0x140;
        }
        else if (iVar2 == 800) {
          iVar7 = 400;
        }
        local_224 = local_224 + (iVar2 - iVar7);
        iVar7 = 0;
        if (iVar2 == 0x280) {
          iVar7 = 0x140;
        }
        else if (iVar2 == 800) {
          iVar7 = 400;
        }
        local_22c = local_22c + (iVar2 - iVar7);
      }
      if (DAT_006e1ee4 == 0) {
        FUN_0058c4a0(param_5,&local_22c,iVar1,local_228);
      }
      else {
        FUN_0058d5b0(2,param_5,&local_22c,iVar1,local_228,1);
      }
      piVar5 = *(int **)(param_1 + 0x12);
      if (piVar5[1] <= piVar5[2]) break;
      piVar3 = (int *)(piVar5[2] * 0x10 + *piVar5);
      local_22c = *piVar3;
      local_228 = piVar3[1];
      local_224 = piVar3[2];
      local_220 = piVar3[3];
      piVar5[2] = piVar5[2] + 1;
    }
    puVar9 = (undefined4 *)0x0;
    iVar1 = *(int *)(param_1 + 0x16) + param_2 * 0x1fc;
    iVar2 = *(int *)(iVar1 + 0xc + param_3 * 4);
    if ((iVar2 == 0) || (iVar7 = param_3, *(int *)(iVar2 + 4) != param_3)) {
      iVar7 = 0;
      piVar5 = (int *)(iVar1 + 0xc);
      do {
        if ((*piVar5 != 0) && (*(int *)(*piVar5 + 4) == param_3)) break;
        iVar7 = iVar7 + 1;
        piVar5 = piVar5 + 1;
      } while (iVar7 < 0x7c);
    }
    if (iVar7 != 0x7c) {
      puVar9 = *(undefined4 **)(iVar1 + 0xc + iVar7 * 4);
    }
    if (param_4 == 1) {
      puVar4 = puVar9 + local_234 + 3;
      uVar8 = local_234;
      do {
        pcVar6 = (char *)*puVar4;
        if (pcVar6 != (char *)0x0) break;
        uVar8 = uVar8 - 1;
        puVar4 = puVar4 + -1;
      } while (-1 < (int)uVar8);
      iVar1 = *(int *)(param_1 + 0x1a);
      FUN_004c7960();
      if (pcVar6 != (char *)0x0) {
        *(char **)(iVar1 + 0x808) = pcVar6;
        *(undefined4 *)(iVar1 + 0x818) = 1;
        if ((**(char **)(iVar1 + 0x804) == '\0') && (*pcVar6 != '\0')) {
          FUN_004c9bd0();
        }
      }
    }
    if ((*(int *)(param_1 + 0x52) != 0) || (param_4 == 1)) {
      iVar1 = *(int *)(param_1 + 0x1a);
      piVar5 = *(int **)(param_1 + 0x12);
      *(int **)(iVar1 + 0x834) = piVar5;
      iVar2 = piVar5[1];
      if (0 < iVar2) {
        puVar4 = (undefined4 *)*piVar5;
        *(undefined4 *)(iVar1 + 0x838) = *puVar4;
        *(undefined4 *)(iVar1 + 0x83c) = puVar4[1];
        *(undefined4 *)(iVar1 + 0x840) = puVar4[2];
        *(undefined4 *)(iVar1 + 0x844) = puVar4[3];
        piVar5[2] = 1;
      }
      uVar8 = (uint)(0 < iVar2);
      *(uint *)(iVar1 + 0x848) = uVar8;
      if (uVar8 != 0) {
        *(undefined4 *)(iVar1 + 0x80c) = *(undefined4 *)(iVar1 + 0x838);
      }
      *(undefined4 *)(*(int *)(param_1 + 0x1a) + 0x84c) = param_5;
      if (param_4 == 1) {
        FUN_004ca0c0(*puVar9);
      }
      iVar1 = FUN_004c9fd0();
      if (iVar1 != 0) {
        piVar5 = puVar9 + local_234 + 3;
        do {
          iVar1 = *piVar5;
          if (iVar1 != 0) break;
          local_234 = local_234 - 1;
          piVar5 = piVar5 + -1;
        } while (-1 < (int)local_234);
        *(int *)(param_1 + 0x52) = iVar1;
        return;
      }
      *(undefined4 *)(param_1 + 0x52) = 0;
    }
  }
  return;
}

