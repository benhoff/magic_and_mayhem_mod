
/* WARNING: Removing unreachable block (ram,0x0054a8e5) */

void FUN_0054a8d0(void)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  
  piVar2 = &DAT_006f1d58;
  do {
    if (*piVar2 == 0) {
      piVar3 = piVar2 + 4;
      iVar1 = 4;
      do {
        if (*piVar3 != 0) {
          FUN_0057d0d0(*piVar3);
          *piVar3 = 0;
        }
        piVar3 = piVar3 + 1;
        iVar1 = iVar1 + -1;
      } while (iVar1 != 0);
      if (piVar2[8] != 0) {
        FUN_005979b0(piVar2[8]);
        piVar2[8] = 0;
      }
    }
    piVar2 = piVar2 + 9;
  } while ((int)piVar2 < 0x6f2148);
  if (DAT_006f2218 == 0) {
    iVar1 = 0x10;
    do {
      if (*(int *)((int)&DAT_006f2218 + iVar1) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006f2218 + iVar1));
        *(undefined4 *)((int)&DAT_006f2218 + iVar1) = 0;
      }
      iVar1 = iVar1 + 4;
    } while (iVar1 < 0x20);
    if (DAT_006f2238 != 0) {
      FUN_005979b0(DAT_006f2238);
      DAT_006f2238 = 0;
    }
  }
  if (DAT_006f2240 == 0) {
    iVar1 = 0x10;
    do {
      if (*(int *)((int)&DAT_006f2240 + iVar1) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006f2240 + iVar1));
        *(undefined4 *)((int)&DAT_006f2240 + iVar1) = 0;
      }
      iVar1 = iVar1 + 4;
    } while (iVar1 < 0x20);
    if (DAT_006f2260 != 0) {
      FUN_005979b0(DAT_006f2260);
      DAT_006f2260 = 0;
    }
  }
  return;
}

