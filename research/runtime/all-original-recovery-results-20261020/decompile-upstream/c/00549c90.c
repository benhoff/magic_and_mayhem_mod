
bool __fastcall FUN_00549c90(int param_1)

{
  int iVar1;
  int iVar2;
  
  iVar2 = *(int *)(param_1 + 0x10);
  if ((iVar2 != 0) && (*(int *)(iVar2 + 0xe4) != 0)) {
    iVar1 = *(int *)(param_1 + 0x18) +
            *(int *)(iVar2 + 0x14) + *(int *)(*(int *)(iVar2 + 0xac) + 0x198);
    *(int *)(param_1 + 4) = iVar1;
    while (iVar1 < 0) {
      iVar1 = *(int *)(param_1 + 4) + DAT_005eb480;
      *(int *)(param_1 + 4) = iVar1;
    }
    if (DAT_005eb480 <= *(int *)(param_1 + 4)) {
      do {
        iVar1 = *(int *)(param_1 + 4) - DAT_005eb480;
        *(int *)(param_1 + 4) = iVar1;
      } while (DAT_005eb480 <= iVar1);
    }
    iVar1 = *(int *)(param_1 + 0x1c) +
            *(int *)(iVar2 + 0x18) + *(int *)(*(int *)(iVar2 + 0xac) + 0x198);
    *(int *)(param_1 + 8) = iVar1;
    while (iVar1 < 0) {
      iVar1 = *(int *)(param_1 + 8) + DAT_005eb484;
      *(int *)(param_1 + 8) = iVar1;
    }
    if (DAT_005eb484 <= *(int *)(param_1 + 8)) {
      do {
        iVar1 = *(int *)(param_1 + 8) - DAT_005eb484;
        *(int *)(param_1 + 8) = iVar1;
      } while (DAT_005eb484 <= iVar1);
    }
    *(int *)(param_1 + 0xc) =
         *(int *)(iVar2 + 0x1c) + *(int *)(*(int *)(iVar2 + 0xac) + 0x19c) +
         *(int *)(param_1 + 0x20);
  }
  iVar2 = *(int *)(param_1 + 0x14) + -1;
  *(int *)(param_1 + 0x14) = iVar2;
  return 0 < iVar2;
}

