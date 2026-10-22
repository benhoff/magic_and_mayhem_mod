
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005496b0(void)

{
  int iVar1;
  
  if (DAT_006f2268 != 0) {
    FUN_005979b0(DAT_006f2268);
    DAT_006f2268 = 0;
    _DAT_006f226c = 0;
    DAT_006f2270 = 0;
    DAT_006f2274 = 0;
    iVar1 = 0x10;
    do {
      if (*(int *)((int)&DAT_006f2268 + iVar1) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006f2268 + iVar1));
        *(undefined4 *)((int)&DAT_006f2268 + iVar1) = 0;
      }
      iVar1 = iVar1 + 4;
    } while (iVar1 < 0x20);
    if (DAT_006f2288 != 0) {
      FUN_005979b0(DAT_006f2288);
      DAT_006f2288 = 0;
    }
  }
  return;
}

