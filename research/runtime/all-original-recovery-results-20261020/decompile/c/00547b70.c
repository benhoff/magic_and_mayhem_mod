
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00547b70(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  int iVar3;
  undefined2 *puVar4;
  
  iVar2 = *(int *)(param_1 + 4) + -1;
  iVar3 = *(int *)(param_1 + 8) + -1;
  if (_DAT_005c5388 < SQRT((float)(iVar2 * iVar2 + iVar3 * iVar3))) {
    if (*(int *)(param_1 + 0x12) == 0) {
      iVar2 = FUN_00597850(*(int *)(param_1 + 4) * *(int *)(param_1 + 8) * 6);
      *(int *)(param_1 + 0x12) = iVar2;
      if (iVar2 == 0) {
        return;
      }
    }
    puVar4 = *(undefined2 **)(param_1 + 0x12);
    iVar2 = 0;
    if (0 < *(int *)(param_1 + 8)) {
      do {
        iVar3 = 0;
        if (0 < *(int *)(param_1 + 4)) {
          do {
            uVar1 = __ftol();
            *puVar4 = uVar1;
            uVar1 = __ftol();
            puVar4[1] = uVar1;
            uVar1 = __ftol();
            puVar4[2] = uVar1;
            puVar4 = puVar4 + 3;
            iVar3 = iVar3 + 1;
          } while (iVar3 < *(int *)(param_1 + 4));
        }
        iVar2 = iVar2 + 1;
      } while (iVar2 < *(int *)(param_1 + 8));
    }
  }
  return;
}

