
void __thiscall FUN_004d3a30(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  int local_4;
  
  if (*(int *)(param_1 + 0x1ef) == 0) {
    return;
  }
  iVar2 = -((param_2 * 0x50) / *(int *)(param_1 + 0xcb));
  iVar1 = iVar2 + 0x50;
  if (param_2 < *(int *)(param_1 + 199)) {
    local_10 = 0x51;
    local_c = 0;
    local_8 = 0xa1;
    local_4 = iVar1;
    if (iVar1 == 0) goto LAB_004d3af1;
    if (DAT_006e1ee4 == 0) {
      iVar2 = 0x1e;
LAB_004d3ae0:
      FUN_0058c8a0(&DAT_006a4642,&local_10,0x66,iVar2);
      goto LAB_004d3af1;
    }
    iVar2 = 0x1e;
  }
  else {
    local_10 = 0;
    local_8 = 0x50;
    local_4 = 0x50;
    local_c = iVar1;
    if (iVar1 == 0x50) goto LAB_004d3af1;
    iVar2 = iVar2 + 0x6e;
    if (DAT_006e1ee4 == 0) goto LAB_004d3ae0;
  }
  FUN_0058d5b0(0,&DAT_006a4642,&local_10,0x66,iVar2,1);
LAB_004d3af1:
  *(int *)(param_1 + 199) = param_2;
  DAT_005e16fc = 2;
  if (*(int *)(*(int *)(param_1 + 0x1ef) + 0x68) != 0) {
    FUN_004d3c90();
  }
  return;
}

