
void __fastcall FUN_004d3b40(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  int local_4;
  
  if (*(int *)(param_1 + 0x1ef) != 0) {
    local_10 = 0x51;
    local_c = 0;
    local_8 = 0xa1;
    iVar1 = -((*(int *)(param_1 + 199) * 0x50) / *(int *)(param_1 + 0xcb));
    iVar2 = iVar1 + 0x50;
    if (iVar2 != 0) {
      local_4 = iVar2;
      if (DAT_006e1ee4 == 0) {
        FUN_0058c8a0(&DAT_006a4642,&local_10,0x66,0x1e);
      }
      else {
        FUN_0058d5b0(0,&DAT_006a4642,&local_10,0x66,0x1e,1);
      }
    }
    local_10 = 0;
    local_8 = 0x50;
    local_4 = 0x50;
    local_c = iVar2;
    if (iVar2 != 0x50) {
      iVar1 = iVar1 + 0x6e;
      if (DAT_006e1ee4 == 0) {
        FUN_0058c8a0(&DAT_006a4642,&local_10,0x66,iVar1);
      }
      else {
        FUN_0058d5b0(0,&DAT_006a4642,&local_10,0x66,iVar1,1);
      }
    }
    *(undefined4 *)(param_1 + 0xdb) = 0;
    *(undefined4 *)(param_1 + 0xdf) = 0x80;
    FUN_004d2e50();
    if (*(int *)(*(int *)(param_1 + 0x1ef) + 0x68) != 0) {
      FUN_004d3c90();
    }
  }
  return;
}

