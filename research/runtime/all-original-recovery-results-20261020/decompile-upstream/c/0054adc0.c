
undefined4 __thiscall FUN_0054adc0(int *param_1,undefined4 param_2,int *param_3)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int local_4;
  
  uVar3 = 0;
  param_1[3] = param_3[2];
  iVar4 = param_3[3];
  param_1[4] = iVar4;
  iVar4 = param_1[3] * iVar4;
  local_4 = 0;
  if (*param_1 != 0) {
    FUN_005979b0(*param_1);
    *param_1 = 0;
    param_1[1] = 0;
  }
  iVar1 = FUN_00597880(iVar4);
  *param_1 = iVar1;
  if (iVar1 != 0) {
    param_1[1] = iVar4;
    iVar4 = *param_3;
    if (param_1[4] != 0) {
      do {
        iVar2 = FUN_0054b100(iVar4,iVar1);
        iVar1 = iVar1 + iVar2;
        iVar4 = iVar4 + param_1[3];
        local_4 = local_4 + iVar2;
        uVar3 = uVar3 + 1;
      } while (uVar3 < (uint)param_1[4]);
    }
    FUN_00533850(local_4);
    if (param_1[2] == 0) {
      iVar4 = FUN_00597850(0x100);
      param_1[2] = iVar4;
    }
    FUN_0058be90(param_1[2]);
    FUN_0054afe0(param_2);
    return 1;
  }
  return 0;
}

