
void __fastcall FUN_00486630(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x13);
  if (iVar1 != 0) {
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(iVar1 + 0x20),*(undefined4 *)(iVar1 + 0x24));
    if ((int *)**(int **)(param_1 + 0x4b) != (int *)0x0) {
      (**(code **)(*(int *)**(int **)(param_1 + 0x4b) + 0x10))();
    }
  }
  return;
}

