
void __thiscall FUN_004711d0(int param_1,char param_2)

{
  int iVar1;
  int *piVar2;
  
  if (DAT_006e1f0c == 0) {
    FUN_004d6c60();
    FUN_004d6320();
  }
  FUN_004d39b0();
  FUN_004d3800();
  FUN_004d9840();
  FUN_004d94f0();
  FUN_00555eb0();
  FUN_00554840();
  if (DAT_006e1f0c == 0) {
    FUN_004da210();
  }
  FUN_004dad60();
  if (DAT_006e1f0c == 0) {
    FUN_004dd1e0();
    FUN_004dc870();
    DAT_006905ac = 0;
    FUN_004de230();
    DAT_006903f4 = 0;
    FUN_004de8d0();
  }
  if (param_2 != '\0') {
    FUN_00463cb0();
    FUN_00463e20();
  }
  if (*(int *)(param_1 + 0x100bb) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x100bb));
    *(undefined4 *)(param_1 + 0x100bb) = 0;
    *(undefined4 *)(param_1 + 0x100bf) = 0;
    *(undefined4 *)(param_1 + 0x100c3) = 0;
    *(undefined4 *)(param_1 + 0x100c7) = 0;
  }
  piVar2 = (int *)(param_1 + 0x100cb);
  iVar1 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (*(int *)(param_1 + 0x100db) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x100db));
    *(undefined4 *)(param_1 + 0x100db) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x100b3);
  if (iVar1 != 0) {
    FUN_004cb560();
    FUN_00597870(iVar1);
    *(undefined4 *)(param_1 + 0x100b3) = 0;
  }
  if (*(int **)(param_1 + 0x100b7) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x100b7) + 4))(1);
    *(undefined4 *)(param_1 + 0x100b7) = 0;
  }
  return;
}

