
void __fastcall FUN_004a9670(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 8) != 0) {
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)(param_1 + 0x13) + 0x20),
                 *(undefined4 *)(*(int *)(param_1 + 0x13) + 0x24));
    iVar1 = 0;
    do {
      FUN_004d2800();
      iVar1 = iVar1 + 0x29;
    } while (iVar1 < 0x1c3);
    iVar1 = 0;
    do {
      (**(code **)(*(int *)(*(int *)(param_1 + 0x53) + iVar1) + 0x10))();
      iVar1 = iVar1 + 0x86;
    } while (iVar1 < 0x10c);
    iVar1 = 0;
    do {
      (**(code **)(*(int *)(*(int *)(param_1 + 0x57) + iVar1) + 0x10))();
      iVar1 = iVar1 + 0x7e;
    } while (iVar1 < 0x5e8);
    iVar1 = 0;
    do {
      (**(code **)(*(int *)(*(int *)(param_1 + 0x5b) + iVar1) + 0x10))();
      iVar1 = iVar1 + 0x59;
    } while (iVar1 < 0xb2);
  }
  return;
}

