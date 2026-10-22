
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0058b3e0(int param_1)

{
  int *piVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  iVar4 = 0;
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 != (int *)0x0) {
    if (DAT_006e1f1c != 0) {
      (**(code **)(*piVar1 + 0x98))(piVar1,0);
    }
    do {
      iVar4 = iVar4 + 1;
      iVar2 = (**(code **)(**(int **)(param_1 + 4) + 8))(*(int **)(param_1 + 4));
      if (iVar2 == 0) break;
    } while (iVar4 < 1000);
    do {
      iVar4 = iVar4 + 1;
      iVar2 = (**(code **)(**(int **)(param_1 + 8) + 8))(*(int **)(param_1 + 8));
      if (iVar2 == 0) break;
    } while (iVar4 < 1000);
    piVar1 = *(int **)(param_1 + 0xc);
    if (piVar1 != (int *)0x0) {
      (**(code **)(*piVar1 + 8))(piVar1);
      *(undefined4 *)(param_1 + 0xc) = 0;
    }
    if (param_1 == DAT_006f98c0) {
      DAT_006f98c0 = 0;
    }
    uVar3 = *(uint *)(param_1 + 0x1c);
    while (uVar3 < 0x7f) {
      (&DAT_006f6950)[*(int *)(param_1 + 0x1c)] =
           *(undefined4 *)(&DAT_006f6954 + *(int *)(param_1 + 0x1c) * 4);
      if ((&DAT_006f6950)[*(int *)(param_1 + 0x1c)] == 0) break;
      *(int *)((&DAT_006f6950)[*(int *)(param_1 + 0x1c)] + 0x1c) = *(int *)(param_1 + 0x1c);
      uVar3 = *(int *)(param_1 + 0x1c) + 1;
      *(uint *)(param_1 + 0x1c) = uVar3;
    }
    _DAT_006f6b4c = 0;
    DAT_006f8154 = DAT_006f8154 + -1;
    (&DAT_006f6950)[*(int *)(param_1 + 0x1c)] = 0;
    *(undefined4 *)(param_1 + 4) = 0;
    *(undefined4 *)(param_1 + 8) = 0;
    *(undefined4 *)(param_1 + 0x18) = 0;
    *(undefined4 *)(param_1 + 0x14) = 0;
    *(undefined4 *)(param_1 + 0x10) = 0;
    *(undefined4 *)(param_1 + 0x28) = 0;
  }
  return;
}

