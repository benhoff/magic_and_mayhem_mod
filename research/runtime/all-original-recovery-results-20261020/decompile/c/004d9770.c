
void __fastcall FUN_004d9770(int param_1)

{
  if (*(int *)(param_1 + 0x5a4) != 0) {
    if (DAT_006de6dd != 0) {
      if (DAT_006e1ee4 != 0) {
        FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(param_1 + 0x5c0),
                     *(undefined4 *)(param_1 + 0x5c4),1);
        return;
      }
      FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x5c0),*(undefined4 *)(param_1 + 0x5c4));
      return;
    }
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x5c0),*(undefined4 *)(param_1 + 0x5c4));
  }
  return;
}

