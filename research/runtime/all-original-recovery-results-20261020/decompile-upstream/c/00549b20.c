
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_00549b20(void)

{
  int iVar1;
  
  if (DAT_006f21e8 != 0) {
    FUN_005979b0(DAT_006f21e8);
    DAT_006f21e8 = 0;
    _DAT_006f21ec = 0;
    DAT_006f21f0 = 0;
    DAT_006f21f4 = 0;
    iVar1 = 0x10;
    do {
      if (*(int *)((int)&DAT_006f21e8 + iVar1) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006f21e8 + iVar1));
        *(undefined4 *)((int)&DAT_006f21e8 + iVar1) = 0;
      }
      iVar1 = iVar1 + 4;
    } while (iVar1 < 0x20);
    if (DAT_006f2208 != 0) {
      FUN_005979b0(DAT_006f2208);
      DAT_006f2208 = 0;
    }
  }
  return;
}

