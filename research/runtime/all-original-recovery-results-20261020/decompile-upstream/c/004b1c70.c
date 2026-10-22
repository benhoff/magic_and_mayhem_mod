
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004b1c70(int param_1)

{
  char cVar1;
  int iVar2;
  
  if (*(char *)(param_1 + 0x6f) != '\0') {
    *(undefined1 *)(param_1 + 0x6f) = 0;
    cVar1 = FUN_004b1a30();
    if (cVar1 == '\0') {
      iVar2 = 0;
      do {
        if (*(int *)(*(int *)(param_1 + 0x53) + iVar2) != 0) {
          FUN_004ca930();
        }
        iVar2 = iVar2 + 4;
      } while (iVar2 < 8);
      iVar2 = 0;
      do {
        if (*(int *)(*(int *)(param_1 + 0x57) + iVar2) != 0) {
          FUN_004ca930();
        }
        iVar2 = iVar2 + 4;
      } while (iVar2 < 0xc);
      iVar2 = 0;
      do {
        if (*(int *)(*(int *)(param_1 + 0x4b) + iVar2) != 0) {
          FUN_004ca930();
        }
        iVar2 = iVar2 + 4;
      } while (iVar2 < 8);
      FUN_0058bc10(0);
      return;
    }
    FUN_00556fb0();
    DAT_006a4ba7 = &DAT_006903e0;
    _DAT_006903e4 = &DAT_006a4b18;
    _DAT_006a4bb7 = *(undefined4 *)(param_1 + 4);
    *(undefined **)(param_1 + 0x33) = &DAT_006a4b1c;
  }
  return;
}

