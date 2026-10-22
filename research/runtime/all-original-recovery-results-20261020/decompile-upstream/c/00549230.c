
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00549230(void)

{
  int iVar1;
  
  if (DAT_006f2150 != 0) {
    FUN_005979b0(DAT_006f2150);
    DAT_006f2150 = 0;
    _DAT_006f2154 = 0;
    DAT_006f2158 = 0;
    DAT_006f215c = 0;
    iVar1 = 0x10;
    do {
      if (*(int *)((int)&DAT_006f2150 + iVar1) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006f2150 + iVar1));
        *(undefined4 *)((int)&DAT_006f2150 + iVar1) = 0;
      }
      iVar1 = iVar1 + 4;
    } while (iVar1 < 0x20);
    if (DAT_006f2170 != 0) {
      FUN_005979b0(DAT_006f2170);
      DAT_006f2170 = 0;
    }
  }
  return;
}

