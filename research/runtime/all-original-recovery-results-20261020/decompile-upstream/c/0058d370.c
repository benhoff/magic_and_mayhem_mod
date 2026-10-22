
void __thiscall FUN_0058d370(int param_1,undefined4 param_2)

{
  undefined4 unaff_retaddr;
  undefined1 *puStack_c;
  int local_4;
  
  puStack_c = (undefined1 *)&local_4;
  local_4 = param_1;
  (**(code **)(**(int **)(param_1 + 8) + 0x44))(*(int **)(param_1 + 8));
  FUN_00486a80(&puStack_c,unaff_retaddr,param_2);
  (**(code **)(**(int **)(param_1 + 8) + 0x68))(*(int **)(param_1 + 8),puStack_c);
  return;
}

