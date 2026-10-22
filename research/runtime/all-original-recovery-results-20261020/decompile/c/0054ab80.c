
void __thiscall FUN_0054ab80(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x46) + 0x7f;
  *(int *)(param_1 + 0x52) = param_4;
  *(int *)(param_1 + 0x4a) = param_2;
  *(int *)(param_1 + 0x4e) = param_3;
  FUN_00547890((iVar1 * param_2) / 0x7f,(iVar1 * param_3) / 0x7f,(iVar1 * param_4) / 0x7f);
  return;
}

