
undefined4 __fastcall FUN_005456c0(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar1 = *(int *)(param_1 + 4) + *(int *)(param_1 + 0x10);
  *(int *)(param_1 + 4) = iVar1;
  if (iVar1 < 0) {
    do {
      iVar1 = *(int *)(param_1 + 4) + DAT_005eb480;
      *(int *)(param_1 + 4) = iVar1;
    } while (iVar1 < 0);
  }
  else if (DAT_005eb480 <= iVar1) {
    do {
      iVar1 = *(int *)(param_1 + 4) - DAT_005eb480;
      *(int *)(param_1 + 4) = iVar1;
    } while (DAT_005eb480 <= iVar1);
  }
  iVar1 = *(int *)(param_1 + 8) + *(int *)(param_1 + 0x14);
  *(int *)(param_1 + 8) = iVar1;
  if (iVar1 < 0) {
    do {
      iVar1 = *(int *)(param_1 + 8) + DAT_005eb484;
      *(int *)(param_1 + 8) = iVar1;
    } while (iVar1 < 0);
  }
  else if (DAT_005eb484 <= iVar1) {
    do {
      iVar1 = *(int *)(param_1 + 8) - DAT_005eb484;
      *(int *)(param_1 + 8) = iVar1;
    } while (DAT_005eb484 <= iVar1);
  }
  iVar1 = *(int *)(param_1 + 0x18);
  iVar2 = *(int *)(param_1 + 0xc) + iVar1;
  *(int *)(param_1 + 0xc) = iVar2;
  if (iVar2 < 0) {
    *(undefined4 *)(param_1 + 0xc) = 0;
  }
  if (((*(int *)(param_1 + 0xc) <= DAT_005eb488 * 2) || (iVar1 < 0)) &&
     ((*(int *)(param_1 + 0xc) != 0 || (-1 < iVar1)))) {
    return 1;
  }
  return 0;
}

