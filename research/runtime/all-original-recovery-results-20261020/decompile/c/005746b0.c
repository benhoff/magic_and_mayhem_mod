
void __fastcall FUN_005746b0(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(param_1 + 0x6d);
  if ((iVar1 != 0) && (iVar2 = *(int *)(iVar1 + 8), iVar2 != 0)) {
    if (DAT_006de6dd != 0) {
      if (DAT_006e1ee4 != 0) {
        FUN_0058d5b0(1,iVar2,0,*(undefined4 *)(iVar1 + 0x2c),*(undefined4 *)(iVar1 + 0x30),1);
        DAT_005e16fc = 1;
        return;
      }
      FUN_0058c140(iVar2,*(undefined4 *)(iVar1 + 0x2c),*(undefined4 *)(iVar1 + 0x30));
      DAT_005e16fc = 1;
      return;
    }
    FUN_0058bf40(iVar2,*(undefined4 *)(iVar1 + 0x2c),*(undefined4 *)(iVar1 + 0x30));
    DAT_005e16fc = 1;
  }
  return;
}

