
void __fastcall FUN_00490260(int param_1)

{
  undefined4 uVar1;
  void **ppvVar2;
  int iVar3;
  int iVar4;
  ushort *puVar5;
  undefined4 *puVar6;
  int *piVar7;
  uint local_13c;
  undefined4 local_138;
  int local_134;
  int local_130;
  undefined4 local_12c;
  int local_128;
  undefined4 local_118;
  undefined4 local_114;
  int local_10c;
  int local_108 [4];
  undefined4 local_f8;
  int local_f4;
  undefined4 local_f0;
  undefined4 local_ec;
  undefined4 local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c02be;
  local_c = ExceptionList;
  uVar1 = *(undefined4 *)(param_1 + 0x4c);
  puVar5 = *(ushort **)(param_1 + 0x194);
  piVar7 = local_108;
  for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
    *piVar7 = 0;
    piVar7 = piVar7 + 1;
  }
  iVar4 = 7;
  local_f4 = -1;
  local_108[2] = -1;
  local_e4 = 0xffffffff;
  local_e8 = 0xffffffff;
  local_ec = 0xffffffff;
  local_f0 = 0xffffffff;
  local_d8 = 7;
  local_e0 = 1;
  local_dc = 0xffffffff;
  local_138 = 0;
  local_12c = 0;
  local_130 = 0;
  switch(uVar1) {
  case 0x41:
    local_13c = 0x167;
    break;
  default:
    goto switchD_004902fd_caseD_42;
  case 0x45:
    iVar4 = 6;
    local_13c = 0x153;
    break;
  case 0x46:
    local_13c = 0x159;
    break;
  case 0x5b:
    iVar4 = 6;
    local_13c = 0x14d;
    break;
  case 0x5c:
    local_13c = 0x160;
  }
  local_108[0] = *(int *)(param_1 + 0x14);
  local_108[1] = *(undefined4 *)(param_1 + 0x18);
  local_134 = iVar4 + -1;
  iVar3 = DAT_006c54a0;
  ExceptionList = &local_c;
  ppvVar2 = &local_c;
  local_128 = param_1;
  local_108[3] = local_108[0];
  local_f8 = local_108[1];
  if (0 < local_134) {
    do {
      ExceptionList = ppvVar2;
      if ((local_13c <= *puVar5) && ((uint)*puVar5 < local_13c + iVar4)) {
        local_130 = (((int)puVar5 - DAT_006c54dc) / 0xc) / iVar3;
        local_12c = FUN_004132e0();
        local_138 = FUN_0040e650();
        FUN_0040eec0();
        FUN_0040e8d0();
        FUN_0040e2c0(&local_138);
        FUN_004f49e0();
        FUN_004eb710();
        if (DAT_006e1f3c != 0) {
          FUN_0040eec0();
          FUN_0040e8d0();
          FUN_0040e2c0(&local_138);
          FUN_004f49e0();
          iVar3 = FUN_00540430(local_118,local_114);
          if (iVar3 != -1) {
            FUN_0040eec0();
            FUN_0040e8d0();
            FUN_0040e2c0(&local_138);
            FUN_00540140();
          }
        }
        iVar3 = FUN_0049cdb0();
        if (iVar3 != 0) {
          local_108[2] = local_130 << 4;
          local_f4 = local_108[2];
          FUN_00493f20();
        }
        puVar5 = puVar5 + DAT_006c54a0 * -6;
        iVar3 = DAT_006c54a0;
      }
      local_134 = local_134 + -1;
      ppvVar2 = ExceptionList;
    } while (local_134 != 0);
  }
  local_134 = 200;
  do {
    local_10c = FUN_00597850();
    puVar6 = (undefined4 *)0x0;
    local_4 = 0;
    if (local_10c != 0) {
      FUN_0054e200();
      FUN_0054e200();
      FUN_0054e200();
      puVar6 = (undefined4 *)FUN_00545e90();
    }
    local_4 = 0xffffffff;
    if ((puVar6 != (undefined4 *)0x0) && (iVar4 = FUN_005457e0(), iVar4 == 0)) {
      (**(code **)*puVar6)();
    }
    iVar4 = local_128;
    local_134 = local_134 + -1;
  } while (local_134 != 0);
  FUN_00571310(0x402,*(undefined4 *)(local_128 + 8),*(undefined4 *)(local_128 + 0xc));
  for (piVar7 = (int *)*DAT_006898e4; piVar7 != DAT_006898e4; piVar7 = (int *)*piVar7) {
    if (piVar7[2] == iVar4) {
      *(int *)piVar7[1] = *piVar7;
      *(int *)(*piVar7 + 4) = piVar7[1];
      FUN_00597870();
      DAT_006898e8 = DAT_006898e8 + -1;
      break;
    }
  }
  FUN_004943f0();
switchD_004902fd_caseD_42:
  ExceptionList = local_c;
  return;
}

