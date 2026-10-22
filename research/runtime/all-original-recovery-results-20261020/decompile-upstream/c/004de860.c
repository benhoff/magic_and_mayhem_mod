
void __fastcall FUN_004de860(int param_1)

{
  if (*(int *)(param_1 + 0x14) != 0) {
    if (DAT_006de6dd != 0) {
      if (DAT_006e1ee4 != 0) {
        FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(param_1 + 0x30),
                     *(undefined4 *)(param_1 + 0x34),1);
        return;
      }
      FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x30),*(undefined4 *)(param_1 + 0x34));
      return;
    }
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x30),*(undefined4 *)(param_1 + 0x34));
  }
  return;
}

