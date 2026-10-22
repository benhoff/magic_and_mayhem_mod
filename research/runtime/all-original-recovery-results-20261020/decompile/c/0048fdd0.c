
undefined4 __fastcall FUN_0048fdd0(uint *param_1)

{
  ushort uVar1;
  int iVar2;
  short *psVar3;
  int iVar4;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  uint uVar5;
  int iVar6;
  int iVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 *puVar10;
  undefined4 uVar11;
  undefined4 *puVar12;
  int local_28;
  undefined4 local_24;
  int local_20;
  uint local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if (*(short *)(param_1[0x65] + 6) != -1) {
    return 0;
  }
  if (*(int *)((int)param_1 + 0x1da) == 1) {
    FUN_00571310(0x2d7,param_1[2],param_1[3],param_1[4],0,0,0);
  }
  psVar3 = (short *)param_1[0x65];
  if (*psVar3 == 0) {
    psVar3 = psVar3 + DAT_006c54a0 * -6;
  }
  local_1c = *(byte *)(psVar3 + 4) & 3;
  if (((*(int *)((int)param_1 + 0x212) == -1) && (param_1[0x15] == 4)) &&
     (iVar4 = FUN_00466900(s_Wandering_005e0180), iVar4 != 0)) {
    *(int *)((int)param_1 + 0x212) = (iVar4 + -0x643a80) / 0x154;
  }
  switch(param_1[0x13]) {
  case 0x41:
    iVar4 = *(int *)((int)param_1 + 0x1da) + 0x167;
    break;
  default:
    goto switchD_0048fe9b_caseD_42;
  case 0x45:
    iVar6 = 6;
    iVar4 = *(int *)((int)param_1 + 0x1da) + 0x153;
    goto LAB_0048fef5;
  case 0x46:
    iVar4 = *(int *)((int)param_1 + 0x1da) + 0x159;
    break;
  case 0x5b:
    iVar6 = 6;
    iVar4 = *(int *)((int)param_1 + 0x1da) + 0x14d;
    goto LAB_0048fef5;
  case 0x5c:
    iVar4 = *(int *)((int)param_1 + 0x1da) + 0x160;
  }
  iVar6 = 7;
LAB_0048fef5:
  if (*(short *)(param_1[0x65] + 4) == -1) {
    local_24 = 0;
    local_28 = 0;
    iVar7 = (int)(param_1[0x65] - DAT_006c54dc) / 0xc;
    local_20 = iVar7 / DAT_006c54a0;
    iVar7 = iVar7 - (&DAT_006cb8c2)[local_20];
    local_28 = FUN_004132e0(iVar7 / DAT_006c5494);
    local_24 = FUN_0040e650(iVar7 - (&DAT_006cb942)[local_28]);
    puVar10 = &local_c;
    puVar12 = puVar10;
    FUN_0040eec0(&local_20);
    uVar9 = extraout_ECX;
    FUN_0040e8d0(&local_28);
    uVar8 = extraout_ECX_00;
    FUN_0040e2c0(&local_24);
    FUN_004f49e0(uVar8,uVar9,puVar10,puVar12);
    if ((*(byte *)(param_1[0x65] + 10) & 0x30) != 0) {
      uVar5 = (uint)*(ushort *)(param_1[0x65] + 2);
      if ((uVar5 < (uint)((int *)param_1[9])[1]) &&
         (iVar7 = *(int *)param_1[9] + uVar5 * 0x22e, iVar7 != 0)) {
        while( true ) {
          do {
            iVar2 = *(int *)(iVar7 + 0x28);
            if ((iVar2 == 0xe) || ((iVar2 == 0x1a || (iVar2 == 0x19)))) {
              FUN_004943f0();
            }
            uVar5 = (uint)*(ushort *)(iVar7 + 0x1a0);
          } while (uVar5 == *param_1);
          if (uVar5 == 0xffff) break;
          if (uVar5 < (uint)((int *)param_1[9])[1]) {
            iVar7 = *(int *)param_1[9] + uVar5 * 0x22e;
          }
          else {
            iVar7 = 0;
          }
        }
      }
    }
    FUN_004eb710(iVar4);
    *(ushort *)(param_1[0x65] + 8) = *(ushort *)(param_1[0x65] + 8) & 0xfffe;
    *(ushort *)(param_1[0x65] + 8) = *(ushort *)(param_1[0x65] + 8) & 0xfffd;
    *(byte *)(param_1[0x65] + 9) = *(byte *)(param_1[0x65] + 9) | 2;
    FUN_004eb010(local_1c);
    if (DAT_006e1f3c != 0) {
      puVar10 = &local_18;
      uVar11 = extraout_ECX_01;
      FUN_0040eec0(&local_20);
      uVar9 = extraout_ECX_02;
      FUN_0040e8d0(&local_28);
      uVar8 = extraout_ECX_03;
      FUN_0040e2c0(&local_24);
      FUN_004f49e0(uVar8,uVar9,uVar11,puVar10);
      iVar4 = FUN_00540430(local_c,local_8,local_4,local_18,local_14,local_10);
      if (iVar4 != -1) {
        uVar11 = extraout_ECX_04;
        FUN_0040eec0(&local_20);
        uVar9 = extraout_ECX_05;
        FUN_0040e8d0(&local_28);
        uVar8 = extraout_ECX_06;
        FUN_0040e2c0(&local_24);
        FUN_00540140(uVar8,uVar9,uVar11,iVar4);
      }
    }
    iVar4 = *(int *)((int)param_1 + 0x1da) + 1;
    *(int *)((int)param_1 + 0x1da) = iVar4;
    if (iVar4 == iVar6) {
      FUN_00494ab0(0x21);
      return 1;
    }
    FUN_00488330(param_1[0x65]);
    uVar5 = param_1[4] + 1;
    param_1[4] = uVar5;
    param_1[7] = uVar5 * 0x10;
    uVar5 = DAT_006c54dc + ((&DAT_006cb942)[param_1[3]] + (&DAT_006cb8c2)[uVar5] + param_1[2]) * 0xc
    ;
    param_1[0x65] = uVar5;
    if ((*(byte *)(uVar5 + 0xb) & 0x40) == 0) {
      uVar1 = *(ushort *)(uVar5 + 2);
      if (uVar1 == 0xffff) {
        *(short *)(uVar5 + 2) = (short)*param_1;
      }
      else {
        do {
          if ((uint)uVar1 < (uint)((int *)param_1[9])[1]) {
            iVar4 = *(int *)param_1[9] + (uint)uVar1 * 0x22e;
          }
          else {
            iVar4 = 0;
          }
          uVar1 = *(ushort *)(iVar4 + 0x1a0);
        } while (uVar1 != 0xffff);
        *(short *)(iVar4 + 0x1a0) = (short)*param_1;
        *(undefined2 *)(param_1 + 0x68) = 0xffff;
      }
      *(ushort *)(uVar5 + 8) = *(ushort *)(uVar5 + 8) & 0xff7f;
    }
    param_1[100] = DAT_0065660c + (uint)*(ushort *)param_1[0x65] * 0x164;
    return 1;
  }
switchD_0048fe9b_caseD_42:
  return 0;
}

