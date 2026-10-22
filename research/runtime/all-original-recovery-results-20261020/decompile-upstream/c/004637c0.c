
void __fastcall FUN_004637c0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  int local_4;
  
  if (*(int *)(param_1 + 0x89) == 0) {
    return;
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x89) + 0xe4);
  iVar2 = FUN_005075f0();
  iVar2 = 0x30 - (iVar1 * 0x30) / iVar2;
  if (iVar1 < *(int *)(param_1 + 0x8e)) {
    local_10 = 0x31;
    local_c = 0;
    local_8 = 0x60;
    local_4 = iVar2;
    if (iVar2 == 0) goto LAB_00463884;
  }
  else {
    local_10 = 0;
    local_8 = 0x30;
    local_4 = 0x30;
    local_c = iVar2;
    if (iVar2 == 0x30) goto LAB_00463884;
  }
  if (DAT_006e1ee4 == 0) {
    FUN_0058c8a0(param_1 + 0x53,&local_10,0,local_c);
  }
  else {
    FUN_0058d5b0(0,param_1 + 0x53,&local_10,0,local_c,1);
  }
LAB_00463884:
  *(int *)(param_1 + 0x8e) = iVar1;
  if (iVar1 == 0) {
    *(undefined4 *)(param_1 + 0x89) = 0;
  }
  return;
}

