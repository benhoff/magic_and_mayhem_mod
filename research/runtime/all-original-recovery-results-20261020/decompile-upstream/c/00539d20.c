
void __fastcall FUN_00539d20(int param_1)

{
  if (*(int *)(param_1 + 0x12) != 0) {
    FUN_0058bf40(&DAT_006a49e0,0,0);
  }
  if (*(int *)(param_1 + 0x20) != 0) {
    FUN_004d2800();
  }
  DAT_00642c60 = 0;
  FUN_004a45b0();
  FUN_0058bf40(&DAT_006a4fe0,0,0);
  DAT_00642c60 = 1;
  return;
}

