
void __fastcall FUN_005120c0(undefined4 *param_1)

{
  int iVar1;
  int *piVar2;
  int local_fc [4];
  int local_ec;
  int local_e8;
  undefined4 local_e4;
  undefined4 local_e0;
  undefined4 local_dc;
  undefined4 local_d8;
  undefined4 local_d4;
  undefined4 local_d0;
  undefined4 local_cc;
  
  if ((((param_1[0x5d] == DAT_00644520) || (DAT_006e1f0c != 0)) &&
      ((param_1[0x17b] == 2 || (param_1[0x17c] == 2)))) && (param_1[0x1ba] == 0)) {
    iVar1 = FUN_0049cec0();
    param_1[0x1ba] = iVar1;
    if (iVar1 == 0) {
      return;
    }
    piVar2 = local_fc;
    for (iVar1 = 0x3f; iVar1 != 0; iVar1 = iVar1 + -1) {
      *piVar2 = 0;
      piVar2 = piVar2 + 1;
    }
    local_d8 = 0xffffffff;
    local_d0 = 0xffffffff;
    local_dc = 0xffffffff;
    local_e0 = 0xffffffff;
    local_d4 = 1;
    local_fc[0] = *(int *)((int)param_1 + 0x96b) * 0x20 + 0x10;
    local_cc = 1;
    local_e4 = *param_1;
    local_fc[1] = *(int *)((int)param_1 + 0x96f) * 0x20 + 0x10;
    local_fc[2] = *(int *)((int)param_1 + 0x973) << 4;
    local_fc[3] = local_fc[0];
    local_ec = local_fc[1];
    local_e8 = local_fc[2];
    FUN_00493f20(0x2b,local_fc);
  }
  FUN_00512220();
  return;
}

