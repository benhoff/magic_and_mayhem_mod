
void __fastcall FUN_00554840(int param_1)

{
  int iVar1;
  int *piVar2;
  
  FUN_0058b3e0();
  if (*(int *)(param_1 + 0x61be) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x61be));
    *(undefined4 *)(param_1 + 0x61be) = 0;
    *(undefined4 *)(param_1 + 0x61c2) = 0;
    *(undefined4 *)(param_1 + 0x61c6) = 0;
    *(undefined4 *)(param_1 + 0x61ca) = 0;
  }
  piVar2 = (int *)(param_1 + 0x61ce);
  iVar1 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (*(int *)(param_1 + 0x61de) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x61de));
    *(undefined4 *)(param_1 + 0x61de) = 0;
  }
  if (*(int **)(param_1 + 0x65a2) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x65a2) + 4))(1);
    *(undefined4 *)(param_1 + 0x65a2) = 0;
  }
  return;
}

