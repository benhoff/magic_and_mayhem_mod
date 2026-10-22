
int __thiscall FUN_004c1ea0(int *param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  short *psVar4;
  int iVar5;
  short *psVar6;
  short *psVar7;
  int iVar8;
  int iVar9;
  int *piVar10;
  short sVar11;
  short *local_20;
  int local_10;
  
  sVar11 = (-(ushort)(DAT_006e1f88 != 0) & 0x8400) - 0x7e1;
  param_1[1] = 0;
  param_1[2] = 0;
  FUN_0058db80(0xff,0,0xff,0xf);
  psVar7 = (short *)FUN_0058b660();
  if (psVar7 == (short *)0x0) {
    return 0;
  }
  iVar1 = *(int *)(param_2 + 0x14);
  iVar2 = *(int *)(param_2 + 0x10);
  local_10 = 0;
  local_20 = psVar7;
  if (0 < iVar1) {
    do {
      iVar8 = 0;
      if (0 < iVar2) {
        do {
          if (*local_20 == sVar11) {
            iVar3 = param_1[1];
            iVar9 = 0;
            psVar4 = local_20;
            iVar5 = local_10;
            if (0 < iVar3) {
              piVar10 = (int *)*param_1;
              do {
                if ((((*piVar10 <= iVar8) && (iVar8 < piVar10[2])) && (piVar10[1] <= local_10)) &&
                   (local_10 < piVar10[3])) goto LAB_004c2012;
                iVar9 = iVar9 + 1;
                piVar10 = piVar10 + 4;
              } while (iVar9 < iVar3);
            }
            for (; (iVar9 = iVar8, psVar6 = local_20, iVar5 < iVar1 && (*psVar4 == sVar11));
                psVar4 = (short *)((int)psVar4 + (*(uint *)(param_2 + 0x18) & 0xfffffffe))) {
              iVar5 = iVar5 + 1;
            }
            for (; (iVar9 < iVar2 && (*psVar6 == sVar11)); psVar6 = psVar6 + 1) {
              iVar9 = iVar9 + 1;
            }
            if (((0x20 < (iVar9 + 2) - (iVar8 + -2)) && (0x10 < (iVar5 + 2) - (local_10 + -2))) &&
               (iVar3 < 0x20)) {
              piVar10 = (int *)(iVar3 * 0x10 + *param_1);
              *piVar10 = iVar8 + -2;
              piVar10[1] = local_10 + -2;
              piVar10[2] = iVar9 + 2;
              piVar10[3] = iVar5 + 2;
              param_1[1] = param_1[1] + 1;
            }
          }
LAB_004c2012:
          local_20 = local_20 + 1;
          iVar8 = iVar8 + 1;
        } while (iVar8 < iVar2);
      }
      local_10 = local_10 + 1;
      local_20 = local_20 + ((*(uint *)(param_2 + 0x18) >> 1) - iVar2);
    } while (local_10 < iVar1);
  }
  (**(code **)(**(int **)(param_2 + 4) + 0x80))(*(int **)(param_2 + 4),psVar7);
  return iVar2;
}

