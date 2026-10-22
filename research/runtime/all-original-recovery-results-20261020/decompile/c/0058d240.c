
void __fastcall FUN_0058d240(int param_1)

{
  int *piVar1;
  undefined1 *puStack_c;
  int local_4;
  
  puStack_c = (undefined1 *)&local_4;
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 != (int *)0x0) {
    local_4 = param_1;
    (**(code **)(*piVar1 + 0x44))(piVar1);
    FUN_00486a80(&puStack_c,0,0);
    (**(code **)(**(int **)(param_1 + 8) + 0x68))(*(int **)(param_1 + 8),puStack_c);
  }
  return;
}

