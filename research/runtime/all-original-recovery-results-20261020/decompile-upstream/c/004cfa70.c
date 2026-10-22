
void __fastcall FUN_004cfa70(int param_1)

{
  int *piVar1;
  int iVar2;
  int local_4;
  
  local_4 = param_1;
  FUN_0058d370(param_1 + 0x4d,*(undefined4 *)(param_1 + 0x59),*(undefined4 *)(param_1 + 0x5d));
  if (*(int *)(param_1 + 0x21) != 0) {
    iVar2 = FUN_0058b660();
    if (iVar2 == 0) {
      return;
    }
    FUN_0057dc20(*(undefined4 *)(*(int *)(param_1 + 0x35) + 0x14));
    FUN_0057dc60(*(undefined4 *)(param_1 + 0x45),*(undefined4 *)(param_1 + 0x49));
    FUN_004a5a90(&local_4,8);
    FUN_004a5a90(&local_4,4);
    FUN_004a5a90(&local_4,0x10);
    FUN_004a5ce0(*(undefined4 *)(param_1 + 0x21),param_1 + 0x3d,0,0,0,local_4,0);
    piVar1 = *(int **)(*(int *)(param_1 + 0x35) + 4);
    (**(code **)(*piVar1 + 0x80))(piVar1,iVar2);
  }
  (**(code **)(**(int **)(param_1 + 0x25) + 0x10))();
  if (*(int **)(param_1 + 0x29) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x29) + 0x10))();
  }
  return;
}

