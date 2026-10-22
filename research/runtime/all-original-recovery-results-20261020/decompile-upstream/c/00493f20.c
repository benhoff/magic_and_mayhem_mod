
void __thiscall FUN_00493f20(uint *param_1,int param_2,uint *param_3)

{
  uint *puVar1;
  ushort uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  longlong lVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  uint *puVar14;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c02db;
  local_c = ExceptionList;
  if (param_2 < 0) {
    return;
  }
  if (0x58 < param_2) {
    return;
  }
  ExceptionList = &local_c;
  if ((((DAT_005fed7c != 0) && (uVar7 = *param_1, ExceptionList = &local_c, 0x7c < uVar7)) &&
      (ExceptionList = &local_c, DAT_006c5040 != '\0')) &&
     (ExceptionList = &local_c, iVar6 = FUN_0059cae2(&DAT_006c5040,&DAT_005dc850),
     iVar9 = DAT_006c5140, iVar6 != 0)) {
    DAT_006c5140 = DAT_006c5140 + 1;
    FUN_0059d50b(iVar6,s_count___d_type___s_index___d_fra_005e00f8,iVar9,
                 s_DIRECTMISSILE_005dc8d8 + param_2 * 0x32,uVar7,DAT_006c5144,s_create_005e0128);
    FUN_0059c4c3(iVar6);
  }
  *(undefined4 *)((int)param_1 + 0x1b2) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1ae) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1aa) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1be) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1ba) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1b6) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1ca) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1c6) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x1c2) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x212) = 0xffffffff;
  puVar1 = param_1 + 0xb;
  *(undefined4 *)((int)param_1 + 0x216) = 0xffffffff;
  *(undefined4 *)((int)param_1 + 0x21a) = 0xffffffff;
  puVar14 = puVar1;
  for (iVar9 = 0x3f; iVar9 != 0; iVar9 = iVar9 + -1) {
    *puVar14 = *param_3;
    param_3 = param_3 + 1;
    puVar14 = puVar14 + 1;
  }
  uVar7 = DAT_006c549c * 0x10 - 1;
  uVar11 = param_1[0xd];
  if ((int)uVar7 <= (int)param_1[0xd]) {
    uVar11 = uVar7;
  }
  param_1[0xd] = uVar11;
  if ((int)param_1[0x10] < (int)uVar7) {
    uVar7 = param_1[0x10];
  }
  param_1[0x10] = uVar7;
  iVar6 = DAT_006c5498;
  iVar9 = DAT_006c5494;
  puVar13 = (undefined4 *)0x0;
  iVar8 = DAT_006c5494 * 0x20;
  iVar10 = DAT_006c5498 * 0x20;
  if ((int)*puVar1 < 0) {
    *puVar1 = *puVar1 + iVar8;
  }
  if (iVar8 <= (int)*puVar1) {
    *puVar1 = *puVar1 + iVar9 * -0x20;
  }
  if ((int)param_1[0xc] < 0) {
    param_1[0xc] = param_1[0xc] + iVar10;
  }
  if (iVar10 <= (int)param_1[0xc]) {
    param_1[0xc] = param_1[0xc] + iVar6 * -0x20;
  }
  if ((int)param_1[0xe] < 0) {
    param_1[0xe] = param_1[0xe] + iVar8;
  }
  if (iVar8 <= (int)param_1[0xe]) {
    param_1[0xe] = param_1[0xe] + iVar9 * -0x20;
  }
  if ((int)param_1[0xf] < 0) {
    param_1[0xf] = param_1[0xf] + iVar10;
  }
  if (iVar10 <= (int)param_1[0xf]) {
    param_1[0xf] = param_1[0xf] + iVar6 * -0x20;
  }
  uVar7 = param_1[0x11];
  if (uVar7 != 0xffffffff) {
    if (uVar7 < DAT_006def5c) {
      iVar9 = uVar7 * 0xe4b + DAT_006def58;
    }
    else {
      iVar9 = 0;
    }
    *(undefined4 *)((int)param_1 + 0x212) = *(undefined4 *)(iVar9 + 0x174);
    FUN_00522720((undefined2 *)((int)param_1 + 0x216));
  }
  switch(param_2) {
  case 9:
  case 0xb:
  case 0x1f:
  case 0x20:
    break;
  default:
    uVar7 = *puVar1;
    param_1[2] = uVar7 >> 5;
    uVar11 = param_1[0xc];
    param_1[3] = uVar11 >> 5;
    uVar12 = param_1[0xd];
    goto LAB_00494261;
  case 0x15:
    iVar9 = FUN_0048a270();
    if (iVar9 == 0) {
      ExceptionList = local_c;
      return;
    }
    break;
  case 0x1e:
    if (0 < DAT_005e1478) {
      iVar9 = FUN_00597850(100);
      local_4 = 0;
      if (iVar9 != 0) {
        puVar13 = (undefined4 *)FUN_00546c20(param_1[5],param_1[6],DAT_006c549c << 4);
      }
      local_4 = 0xffffffff;
      if (puVar13 != (undefined4 *)0x0) {
        if (param_1[0x12] != 0xffffffff) {
          iVar9 = *(int *)(&DAT_006b13cd + param_1[0x13] * 0x2d1);
          iVar6 = *(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1);
          lVar5 = (longlong)DAT_005e1478;
          puVar13[5] = 0;
          puVar13[6] = (int)((iVar6 * lVar5 & 0xffffffffU) / 100);
          puVar13[7] = 0;
          uVar3 = *(undefined4 *)(&DAT_005e1444 + iVar9 * 0xc);
          uVar4 = *(undefined4 *)(&DAT_005e1440 + iVar9 * 0xc);
          puVar13[10] = (&DAT_005e143c)[iVar9 * 3];
          puVar13[0xb] = uVar4;
          puVar13[0xc] = uVar3;
          puVar13[0xd] = 0x40;
          if (param_1[0x12] < DAT_006def5c) {
            iVar9 = param_1[0x12] * 0xe4b + DAT_006def58;
          }
          else {
            iVar9 = 0;
          }
          puVar13[0x15] = iVar9;
          puVar13[0x16] = 1;
          puVar13[0x17] = 1;
        }
        iVar9 = FUN_005457e0(puVar13);
        if (iVar9 == 0) {
          (**(code **)*puVar13)(1);
          goto LAB_00494272;
        }
      }
    }
  }
  uVar7 = param_1[0xe];
  param_1[2] = uVar7 >> 5;
  uVar11 = param_1[0xf];
  param_1[3] = uVar11 >> 5;
  uVar12 = param_1[0x10];
LAB_00494261:
  param_1[5] = uVar7;
  param_1[4] = uVar12 >> 4;
  param_1[6] = uVar11;
  param_1[7] = uVar12;
LAB_00494272:
  param_1[0x66] = 0;
  uVar7 = DAT_006c54dc +
          ((&DAT_006cb942)[param_1[3]] + (&DAT_006cb8c2)[param_1[4]] + param_1[2]) * 0xc;
  param_1[0x65] = uVar7;
  if ((*(byte *)(uVar7 + 0xb) & 0x40) == 0) {
    uVar2 = *(ushort *)(uVar7 + 2);
    if (uVar2 == 0xffff) {
      *(short *)(uVar7 + 2) = (short)*param_1;
    }
    else {
      do {
        if ((uint)uVar2 < (uint)((int *)param_1[9])[1]) {
          iVar9 = *(int *)param_1[9] + (uint)uVar2 * 0x22e;
        }
        else {
          iVar9 = 0;
        }
        uVar2 = *(ushort *)(iVar9 + 0x1a0);
      } while (uVar2 != 0xffff);
      *(short *)(iVar9 + 0x1a0) = (short)*param_1;
      *(undefined2 *)(param_1 + 0x68) = 0xffff;
    }
    *(ushort *)(uVar7 + 8) = *(ushort *)(uVar7 + 8) & 0xff7f;
  }
  param_1[100] = DAT_0065660c + (uint)*(ushort *)param_1[0x65] * 0x164;
  param_1[1] = 1;
  if (DAT_006898dc < (int)(*param_1 + 1)) {
    DAT_006898dc = *param_1 + 1;
  }
  *(undefined4 *)((int)param_1 + 0x22a) = 1;
  *(undefined4 *)((int)param_1 + 0x20e) = 0;
  *(undefined4 *)((int)param_1 + 0x21e) = 0;
  param_1[0x67] = 0;
  *(undefined4 *)((int)param_1 + 0x222) = 0;
  FUN_00494ab0(param_2);
  *(uint *)((int)param_1 + 0x202) = param_1[2];
  *(uint *)((int)param_1 + 0x1f6) = param_1[2];
  *(uint *)((int)param_1 + 0x206) = param_1[3];
  *(uint *)((int)param_1 + 0x1fa) = param_1[3];
  *(uint *)((int)param_1 + 0x20a) = param_1[4];
  *(uint *)((int)param_1 + 0x1fe) = param_1[4];
  ExceptionList = local_c;
  return;
}

