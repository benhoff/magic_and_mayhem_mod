
void __fastcall FUN_004de1c0(int param_1)

{
  if (*(int *)(param_1 + 0xc) != 0) {
    if (DAT_006de6dd != 0) {
      if (DAT_006e1ee4 != 0) {
        FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(param_1 + 0x28),
                     *(undefined4 *)(param_1 + 0x2c),1);
        return;
      }
      FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
      return;
    }
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x28),*(undefined4 *)(param_1 + 0x2c));
  }
  return;
}

