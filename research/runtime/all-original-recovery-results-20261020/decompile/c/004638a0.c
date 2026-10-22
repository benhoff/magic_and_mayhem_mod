
void __fastcall FUN_004638a0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  int local_4;
  
  iVar1 = 0;
  iVar2 = 10;
  if ((*(int *)(param_1 + 0x25) != 0) && (*(int *)(param_1 + 0x57) != 0)) {
    if (*(int *)(param_1 + 0x89) != 0) {
      iVar1 = *(int *)(*(int *)(param_1 + 0x89) + 0xe4);
      iVar2 = FUN_005075f0();
    }
    local_10 = 0x31;
    local_c = 0;
    local_8 = 0x60;
    iVar2 = 0x30 - (iVar1 * 0x30) / iVar2;
    if (iVar2 != 0) {
      local_4 = iVar2;
      if (DAT_006e1ee4 == 0) {
        FUN_0058c8a0(param_1 + 0x53,&local_10,0,0);
      }
      else {
        FUN_0058d5b0(0,param_1 + 0x53,&local_10,0,0,1);
      }
    }
    local_10 = 0;
    local_8 = 0x30;
    local_4 = 0x30;
    if (iVar2 != 0x30) {
      local_c = iVar2;
      if (DAT_006e1ee4 == 0) {
        FUN_0058c8a0(param_1 + 0x53,&local_10,0,iVar2);
      }
      else {
        FUN_0058d5b0(0,param_1 + 0x53,&local_10,0,iVar2,1);
      }
    }
    *(int *)(param_1 + 0x8e) = iVar1;
    if (iVar1 == 0) {
      *(undefined4 *)(param_1 + 0x89) = 0;
    }
  }
  return;
}

