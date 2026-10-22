
void __fastcall FUN_00572e90(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 8);
  if (iVar1 != 0) {
    if (DAT_006de6dd != 0) {
      if (DAT_006e1ee4 != 0) {
        FUN_0058d5b0(1,iVar1,0,*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30),1);
        DAT_005e16fc = 1;
        return;
      }
      FUN_0058c140(iVar1,*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30));
      DAT_005e16fc = 1;
      return;
    }
    FUN_0058bf40(iVar1,*(undefined4 *)(param_1 + 0x2c),*(undefined4 *)(param_1 + 0x30));
    DAT_005e16fc = 1;
  }
  return;
}

