
void __thiscall FUN_005159c0(int *param_1,uint param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  int *piVar7;
  int *piVar8;
  undefined4 uVar9;
  int local_18;
  undefined4 local_8 [2];
  
  if (param_1[0x1e1] == 0) {
    local_18 = param_1[0x5d];
  }
  else {
    local_18 = param_1[0x1e2];
  }
  if (param_2 < DAT_006def5c) {
    piVar8 = (int *)(param_2 * 0xe4b + DAT_006def58);
  }
  else {
    piVar8 = (int *)0x0;
  }
  if (param_1[0x1f] == 0) {
LAB_00515b5f:
    if (piVar8[0x1f] != 0) goto LAB_00515b6a;
  }
  else {
    if (piVar8[0x1f] == 0) {
      uVar2 = FUN_0054e200();
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x297e);
      }
      iVar3 = param_1[4] - piVar8[4];
      uVar4 = FUN_0040e8a0(piVar8[3]);
      uVar9 = extraout_ECX;
      FUN_0040e8d0(uVar4);
      FUN_0040e8a0(param_1[3]);
      FUN_0040eb70(uVar9);
      uVar4 = FUN_0040e290(piVar8[2]);
      uVar9 = extraout_ECX_00;
      FUN_0040e2c0(uVar4);
      FUN_0040e290(param_1[2]);
      FUN_0040e4e0(uVar9);
      iVar3 = FUN_004eaca0(iVar3);
      if (((int)(iVar3 + (iVar3 >> 0x1f & 3U)) >> 2) + (uVar2 & 0x7fffffff) % 100 <
          (uint)(piVar8[0x22] * 100) / (uint)(piVar8[0x22] + DAT_006c04aa)) {
        iVar3 = 0;
        puVar6 = local_8;
        do {
          iVar5 = FUN_00522770(iVar3);
          if (iVar5 == 0) {
            *puVar6 = 0xffffffff;
          }
          else {
            *puVar6 = *(undefined4 *)(iVar5 + 0x1f);
          }
          iVar3 = iVar3 + 1;
          puVar6 = puVar6 + 1;
        } while (iVar3 < 2);
        if (param_1[0x5d] == -1) {
          puVar6 = (undefined4 *)0x0;
        }
        else {
          puVar6 = &DAT_00643a80 + param_1[0x5d] * 0x55;
        }
        if (puVar6[1] != 0) {
          DAT_006e1fa8 = DAT_006e1fa8 + 1;
        }
        iVar5 = param_1[0x39];
        iVar3 = piVar8[0x5d];
        goto LAB_00515ccf;
      }
      goto LAB_00515b5f;
    }
LAB_00515b6a:
    if (param_1[0x1f] == 0) {
      uVar2 = FUN_0054e200();
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x298e);
      }
      iVar3 = param_1[4] - piVar8[4];
      uVar4 = FUN_0040e8a0(piVar8[3]);
      uVar9 = extraout_ECX_01;
      FUN_0040e8d0(uVar4);
      FUN_0040e8a0(param_1[3]);
      FUN_0040eb70(uVar9);
      uVar4 = FUN_0040e290(piVar8[2]);
      uVar9 = extraout_ECX_02;
      FUN_0040e2c0(uVar4);
      FUN_0040e290(param_1[2]);
      FUN_0040e4e0(uVar9);
      iVar3 = FUN_004eaca0(iVar3);
      if (((int)(iVar3 + (iVar3 >> 0x1f & 3U)) >> 2) + (uVar2 & 0x7fffffff) % 100 <
          (uint)(param_1[0x22] * 100) / (uint)(param_1[0x22] + DAT_006c04aa)) {
        puVar6 = local_8;
        piVar7 = param_1 + 0x19e;
        iVar3 = 2;
        do {
          iVar5 = *piVar7;
          if (iVar5 == 0) {
LAB_00515c7b:
            *puVar6 = 0xffffffff;
          }
          else {
            for (iVar1 = *(int *)(iVar5 + 0x39); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x39)) {
              iVar5 = iVar1;
            }
            if (*(int *)(iVar5 + 5) == 0) goto LAB_00515c7b;
            *puVar6 = *(undefined4 *)(*(int *)(iVar5 + 5) + 0x1f);
          }
          piVar7 = piVar7 + 1;
          puVar6 = puVar6 + 1;
          iVar3 = iVar3 + -1;
        } while (iVar3 != 0);
        if (piVar8[0x5d] == -1) {
          puVar6 = (undefined4 *)0x0;
        }
        else {
          puVar6 = &DAT_00643a80 + piVar8[0x5d] * 0x55;
        }
        if (puVar6[1] != 0) {
          DAT_006e1fa8 = DAT_006e1fa8 + 1;
        }
        iVar5 = piVar8[0x39];
        iVar3 = param_1[0x5d];
LAB_00515ccf:
        FUN_005076f0(-iVar5,iVar3,local_8,1);
        FUN_00571310(0x2dd,param_1[2],param_1[3],param_1[4],0,0,0);
        return;
      }
    }
  }
  if ((piVar8[0x3e] == 0) || (param_1[0x3e] != 0)) {
    iVar3 = *(int *)(param_1[0x2b] + 0x280 + piVar8[0x2a] * 4);
  }
  else {
    iVar3 = *(int *)(param_1[0x2b] + 0x280 + piVar8[0x2a] * 4) / 3;
  }
  uVar2 = FUN_0054e200();
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x299e);
  }
  param_2 = iVar3 * ((uVar2 & 0x7fffffff) % 0x101) >> 7;
  if (param_1[0x49] == 1) {
    uVar2 = (uint)((ulonglong)(DAT_005e1910 * param_2) * 0x51eb851f >> 0x20);
  }
  else {
    if (param_1[0x49] != 2) goto LAB_00515dc0;
    uVar2 = (uint)((ulonglong)(DAT_005e1914 * param_2) * 0x51eb851f >> 0x20);
  }
  param_2 = param_2 + (uVar2 >> 5);
LAB_00515dc0:
  if (piVar8[0x39] != 0) {
    iVar3 = *param_1;
    *(undefined4 *)((int)piVar8 + 0x811) = 100;
    piVar7 = param_1 + 0x19e;
    *(int *)((int)piVar8 + 0x80d) = iVar3 + 1;
    puVar6 = local_8;
    iVar3 = 2;
    do {
      iVar5 = *piVar7;
      if (iVar5 == 0) {
LAB_00515e15:
        *puVar6 = 0xffffffff;
      }
      else {
        for (iVar1 = *(int *)(iVar5 + 0x39); iVar1 != 0; iVar1 = *(int *)(iVar1 + 0x39)) {
          iVar5 = iVar1;
        }
        if (*(int *)(iVar5 + 5) == 0) goto LAB_00515e15;
        *puVar6 = *(undefined4 *)(*(int *)(iVar5 + 5) + 0x1f);
      }
      piVar7 = piVar7 + 1;
      puVar6 = puVar6 + 1;
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    iVar3 = param_1[0x2a];
    if (iVar3 == 8) {
      FUN_0050d060(param_2 / (uint)piVar8[0x21],0,0);
    }
    else if (iVar3 == 0x14) {
      FUN_0050d060(param_2 / (uint)piVar8[0x21],2,0);
    }
    else if (iVar3 == 0x17) {
      *(uint *)((int)piVar8 + 0x8d5) = *(int *)((int)piVar8 + 0x8d5) + param_2;
      *(int *)((int)piVar8 + 0x8dd) = local_18;
    }
    else {
      FUN_00514860(param_2,local_18,local_8);
    }
  }
  if (*param_1 != *piVar8) {
    if (param_1[0x5d] == -1) {
      puVar6 = (undefined4 *)0x0;
    }
    else {
      puVar6 = &DAT_00643a80 + param_1[0x5d] * 0x55;
    }
    if (((*(char *)(piVar8[0x5d] + 0x18 + (int)puVar6) == '\0') && (param_1[0x2a] != 8)) &&
       (param_1[0x2a] != 0x14)) {
      FUN_00514e40(param_2);
    }
  }
  return;
}

