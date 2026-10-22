
void __fastcall FUN_00539b80(int param_1)

{
  undefined4 uVar1;
  
  if (*(char *)(param_1 + 0x1a) != '\0') {
    FUN_00557130();
  }
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
  if (*(int *)(param_1 + 0x1c) != 0) {
    FUN_00537100();
    FUN_00538780();
    return;
  }
  if ((*(char *)(param_1 + 0xc) == '\0') && (DAT_005e04c8 == 0x16)) {
    FUN_004a39a0();
  }
  if (*(char *)(param_1 + 0x1b) == '\0') {
    uVar1 = FUN_0058a6b0();
    FUN_004d28c0(uVar1);
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
    FUN_00470950();
    *(undefined1 *)(param_1 + 0x1b) = 1;
    uVar1 = FUN_0058a6b0();
    FUN_004d28c0(uVar1);
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
    FUN_00557040();
  }
  return;
}

