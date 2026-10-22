
void __fastcall FUN_0050c0d0(int *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  int iVar6;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  int *piVar7;
  int *piVar8;
  bool bVar9;
  undefined4 uVar10;
  uint local_11c;
  int local_118;
  int local_114;
  undefined4 local_110 [5];
  int local_fc [7];
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  
  local_118 = 0;
  if (param_1[0x1e1] == 0) {
    local_114 = param_1[0x5d];
  }
  else {
    local_114 = param_1[0x1e2];
  }
  if ((uint)param_1[0x193] < DAT_006def5c) {
    piVar7 = (int *)(param_1[0x193] * 0xe4b + DAT_006def58);
  }
  else {
    piVar7 = (int *)0x0;
  }
  if (param_1[0x1f] == 0) {
LAB_0050c298:
    if (piVar7[0x1f] == 0) goto LAB_0050c439;
  }
  else if (piVar7[0x1f] == 0) {
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x1607);
    }
    iVar3 = param_1[4] - piVar7[4];
    uVar4 = FUN_0040e8a0(piVar7[3]);
    uVar10 = extraout_ECX;
    FUN_0040e8d0(uVar4);
    FUN_0040e8a0(param_1[3]);
    FUN_0040eb70(uVar10);
    uVar4 = FUN_0040e290(piVar7[2]);
    uVar10 = extraout_ECX_00;
    FUN_0040e2c0(uVar4);
    FUN_0040e290(param_1[2]);
    FUN_0040e4e0(uVar10);
    iVar3 = FUN_004eaca0(iVar3);
    if (((int)(iVar3 + (iVar3 >> 0x1f & 3U)) >> 2) + (uVar2 & 0x7fffffff) % 100 <
        (uint)(piVar7[0x22] * 100) / (uint)(piVar7[0x22] + DAT_006c04aa)) {
      puVar5 = local_110;
      piVar8 = piVar7 + 0x19e;
      iVar3 = 2;
      do {
        iVar6 = *piVar8;
        if (iVar6 == 0) {
LAB_0050c23f:
          *puVar5 = 0xffffffff;
        }
        else {
          for (iVar1 = *(int *)(iVar6 + 0x39); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x39)) {
            iVar6 = iVar1;
          }
          if (*(int *)(iVar6 + 5) == 0) goto LAB_0050c23f;
          *puVar5 = *(undefined4 *)(*(int *)(iVar6 + 5) + 0x1f);
        }
        piVar8 = piVar8 + 1;
        puVar5 = puVar5 + 1;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
      if (param_1[0x5d] == -1) {
        puVar5 = (undefined4 *)0x0;
      }
      else {
        puVar5 = &DAT_00643a80 + param_1[0x5d] * 0x55;
      }
      if (puVar5[1] != 0) {
        DAT_006e1fa8 = DAT_006e1fa8 + 1;
      }
      iVar6 = param_1[0x39];
      iVar3 = piVar7[0x5d];
      goto LAB_0050c408;
    }
    goto LAB_0050c298;
  }
  if (param_1[0x1f] == 0) {
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x1617);
    }
    iVar3 = param_1[4] - piVar7[4];
    uVar4 = FUN_0040e8a0(piVar7[3]);
    uVar10 = extraout_ECX_01;
    FUN_0040e8d0(uVar4);
    FUN_0040e8a0(param_1[3]);
    FUN_0040eb70(uVar10);
    uVar4 = FUN_0040e290(piVar7[2]);
    uVar10 = extraout_ECX_02;
    FUN_0040e2c0(uVar4);
    FUN_0040e290(param_1[2]);
    FUN_0040e4e0(uVar10);
    iVar3 = FUN_004eaca0(iVar3);
    if (((int)(iVar3 + (iVar3 >> 0x1f & 3U)) >> 2) + (uVar2 & 0x7fffffff) % 100 <
        (uint)(param_1[0x22] * 100) / (uint)(param_1[0x22] + DAT_006c04aa)) {
      puVar5 = local_110;
      piVar8 = param_1 + 0x19e;
      iVar3 = 2;
      do {
        iVar6 = *piVar8;
        if (iVar6 == 0) {
LAB_0050c3b4:
          *puVar5 = 0xffffffff;
        }
        else {
          for (iVar1 = *(int *)(iVar6 + 0x39); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x39)) {
            iVar6 = iVar1;
          }
          if (*(int *)(iVar6 + 5) == 0) goto LAB_0050c3b4;
          *puVar5 = *(undefined4 *)(*(int *)(iVar6 + 5) + 0x1f);
        }
        piVar8 = piVar8 + 1;
        puVar5 = puVar5 + 1;
        iVar3 = iVar3 + -1;
      } while (iVar3 != 0);
      if (piVar7[0x5d] == -1) {
        puVar5 = (undefined4 *)0x0;
      }
      else {
        puVar5 = &DAT_00643a80 + piVar7[0x5d] * 0x55;
      }
      if (puVar5[1] != 0) {
        DAT_006e1fa8 = DAT_006e1fa8 + 1;
      }
      iVar6 = piVar7[0x39];
      iVar3 = param_1[0x5d];
LAB_0050c408:
      FUN_005076f0(-iVar6,iVar3,local_110,1);
      FUN_00571310(0x2dd,param_1[2],param_1[3],param_1[4],0,0,0);
      return;
    }
  }
LAB_0050c439:
  if ((piVar7[0x3e] == 0) || (param_1[0x3e] != 0)) {
    local_11c = *(uint *)(param_1[0x2b] + 0x1a0 + piVar7[0x2a] * 4);
  }
  else {
    local_11c = *(int *)(param_1[0x2b] + 0x1a0 + piVar7[0x2a] * 4) / 3;
  }
  if (*(int *)((int)param_1 + 0x8ed) != 0) {
    local_11c = (int)(*(int *)((int)param_1 + 0x8ed) * local_11c) / 100 + local_11c;
  }
  if (param_1[0x25] != 0) {
    local_11c = local_11c * 2;
  }
  bVar9 = param_1[4] - piVar7[4] < 0;
  iVar3 = DAT_005e1778;
  if ((param_1[4] == piVar7[4] || bVar9) && (iVar3 = 0, bVar9)) {
    iVar3 = -DAT_005e1778;
  }
  iVar6 = FUN_0050c930(piVar7);
  iVar3 = iVar3 + (&DAT_005e1758)[iVar6];
  if (param_1[0x5b] != 0) {
    iVar6 = param_1[0x5c];
    param_1[0x5b] = 0;
    if (iVar6 < 0x3e9) {
      if (iVar6 < 0x2ef) {
        if (iVar6 < 0x1f5) {
          if (200 < iVar6) {
            local_118 = ((iVar6 + -200) * DAT_005e177c) / 300 + DAT_005e177c;
          }
        }
        else {
          local_118 = ((iVar6 + -500) * DAT_005e177c) / 0xfa + DAT_005e177c * 2;
        }
      }
      else {
        local_118 = ((iVar6 + -0x2ee) * DAT_005e177c) / 0xfa + DAT_005e177c * 3;
      }
    }
    else {
      local_118 = DAT_005e177c * 4;
    }
    iVar3 = iVar3 + local_118;
  }
  if ((piVar7[0x1ab] != 0) || (piVar7[0x1ac] != 0)) {
    iVar3 = iVar3 + 0x32;
  }
  if (iVar3 != 0) {
    local_11c = local_11c +
                ((int)(iVar3 * local_11c + ((int)(iVar3 * local_11c) >> 0x1f & 0xffU)) >> 8);
  }
  uVar2 = FUN_0054e200();
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x1682);
  }
  iVar3 = ((uVar2 & 0x7fffffff) % 0x101) * (local_11c & ((int)local_11c < 0) - 1);
  local_11c = (int)(iVar3 + (iVar3 >> 0x1f & 0x7fU)) >> 7;
  if (0 < (int)local_11c) {
    iVar3 = DAT_005e1910;
    if ((param_1[0x49] == 1) || (iVar3 = DAT_005e1914, param_1[0x49] == 2)) {
      local_11c = (int)(iVar3 * local_11c) / 100 + local_11c;
    }
    if (*(int *)((int)param_1 + 0x81d) != 0) {
      local_11c = local_11c * 4;
    }
    puVar5 = local_110;
    piVar8 = param_1 + 0x19e;
    iVar3 = 2;
    do {
      iVar6 = *piVar8;
      if (iVar6 == 0) {
LAB_0050c6c6:
        *puVar5 = 0xffffffff;
      }
      else {
        for (iVar1 = *(int *)(iVar6 + 0x39); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x39)) {
          iVar6 = iVar1;
        }
        if (*(int *)(iVar6 + 5) == 0) goto LAB_0050c6c6;
        *puVar5 = *(undefined4 *)(*(int *)(iVar6 + 5) + 0x1f);
      }
      piVar8 = piVar8 + 1;
      puVar5 = puVar5 + 1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    if (param_1[0x2a] == 0x10) {
      *(uint *)((int)piVar7 + 0x8d5) = *(int *)((int)piVar7 + 0x8d5) + local_11c;
      *(int *)((int)piVar7 + 0x8dd) = local_114;
    }
    else {
      FUN_00514860(local_11c,local_114,local_110);
    }
    if (*param_1 != *piVar7) {
      if (param_1[0x5d] == -1) {
        puVar5 = (undefined4 *)0x0;
      }
      else {
        puVar5 = &DAT_00643a80 + param_1[0x5d] * 0x55;
      }
      if (*(char *)(piVar7[0x5d] + 0x18 + (int)puVar5) == '\0') {
        FUN_00514e40(local_11c);
      }
    }
    if ((param_1[0x2a] == 0xc) && (piVar7[0x3e] == 0)) {
      FUN_005076f0(2,0xffffffff,0,1);
    }
    uVar2 = FUN_0054e200();
    uVar2 = (uVar2 & 0x7fffffff) % 100;
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x16a9);
    }
    iVar3 = *(int *)(param_1[0x2b] + 0x210 + piVar7[0x2a] * 4);
    if (((int)uVar2 <= iVar3) && (piVar7[0x17b] != 0xe)) {
      iVar3 = FUN_00521160(uVar2 - iVar3,*param_1);
      if (iVar3 == 0) {
        uVar2 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x16b3);
        }
        FUN_0050d060((uVar2 & 0x7fffffff) % 0x14 + 0x14,0,0);
      }
      else {
        FUN_00521420();
      }
    }
    FUN_00570510(param_1,piVar7);
    if (((((200 < local_118) || (param_1[0x25] != 0)) || (*(int *)((int)param_1 + 0x81d) != 0)) ||
        ((piVar7[0x1ab] != 0 || (piVar7[0x1ac] != 0)))) && (iVar3 = FUN_0049cdb0(), iVar3 != 0)) {
      iVar3 = piVar7[5];
      piVar8 = local_fc;
      for (iVar6 = 0x3f; iVar6 != 0; iVar6 = iVar6 + -1) {
        *piVar8 = 0;
        piVar8 = piVar8 + 1;
      }
      iVar6 = piVar7[0x2b];
      local_fc[5] = 0xffffffff;
      local_fc[4] = 0xffffffff;
      local_fc[3] = 0xffffffff;
      local_d8 = 0xffffffff;
      local_d4 = 1;
      local_d0 = 0xffffffff;
      local_fc[0] = iVar3 + *(int *)(iVar6 + 0x198);
      local_fc[1] = piVar7[6] + *(int *)(iVar6 + 0x198);
      local_dc = 0xffffffff;
      local_fc[2] = *(int *)(iVar6 + 0x19c) + piVar7[7];
      local_e0 = 0xffffffff;
      local_fc[6] = *param_1;
      local_cc = 1;
      FUN_00493f20(0x3b,local_fc);
    }
  }
  return;
}

