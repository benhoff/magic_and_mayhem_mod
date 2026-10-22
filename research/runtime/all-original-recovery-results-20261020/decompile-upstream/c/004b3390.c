
void __fastcall FUN_004b3390(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 8) != 0) {
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)(param_1 + 0x13) + 0x20),
                 *(undefined4 *)(*(int *)(param_1 + 0x13) + 0x24));
    iVar1 = 0;
    do {
      (**(code **)(*(int *)(*(int *)(param_1 + 0x4b) + iVar1) + 0x10))();
      iVar1 = iVar1 + 0x59;
    } while (iVar1 < 0xb2);
    if (*(int *)(param_1 + 0x7c) != 0x16) {
      iVar1 = 0;
      do {
        (**(code **)(*(int *)(iVar1 + *(int *)(param_1 + 0x4f)) + 0x10))();
        iVar1 = iVar1 + 0x7a;
      } while (iVar1 < 0x16e);
      iVar1 = 0;
      do {
        (**(code **)(*(int *)(iVar1 + *(int *)(param_1 + 0x53)) + 0x10))();
        iVar1 = iVar1 + 0x7e;
      } while (iVar1 < 0x1f8);
    }
    FUN_004d2800();
    FUN_004db7e0();
  }
  return;
}

