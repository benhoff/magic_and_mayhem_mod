
void __fastcall FUN_004dc5e0(int param_1)

{
  int iVar1;
  int iVar2;
  
  if ((*(int *)(param_1 + 0x52) != 0) && (*(int *)(param_1 + 0x20) != 0)) {
    iVar1 = *(int *)(param_1 + 0x18);
    iVar2 = iVar1 + 1;
    *(int *)(param_1 + 0x18) = iVar2;
    if (iVar1 <= *(int *)(param_1 + 0x14)) {
      if (iVar2 == *(int *)(param_1 + 0x14)) {
        *(undefined4 *)(param_1 + 8) = 0x256;
      }
      else {
        iVar2 = *(int *)(param_1 + 0x10) + *(int *)(param_1 + 8);
        if (0x255 < iVar2) {
          iVar2 = 0x256;
        }
        *(int *)(param_1 + 8) = iVar2;
      }
      if (DAT_006e1ee4 == 0) {
        FUN_0058c4a0(param_1 + 0x1c,param_1,*(undefined4 *)(param_1 + 0x6e),
                     *(undefined4 *)(param_1 + 0x72));
      }
      else {
        FUN_0058d5b0(2,param_1 + 0x1c,param_1,*(undefined4 *)(param_1 + 0x6e),
                     *(undefined4 *)(param_1 + 0x72),1);
      }
      FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x3c),*(undefined4 *)(param_1 + 0x40));
      DAT_00642c60 = 0;
      FUN_004a45b0();
      FUN_0058bf40(&DAT_006a4fe0,DAT_006a4a00,DAT_006a4a04);
      FUN_004a4380();
      DAT_00642c60 = 1;
    }
  }
  return;
}

