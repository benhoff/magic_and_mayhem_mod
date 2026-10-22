
bool __thiscall FUN_004dd160(int param_1,int param_2,int param_3)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  if (*(int *)(param_1 + -4) != 0) {
    iVar2 = *(int *)(param_1 + 0x9d);
    iVar3 = *(int *)(param_1 + 0x99);
    iVar4 = FUN_0058b660();
    if (iVar4 != 0) {
      sVar1 = *(short *)(iVar4 + (((uint)(*(int *)(param_1 + 0x91) * (param_3 - iVar2)) >> 1) +
                                 (param_2 - iVar3)) * 2);
      (**(code **)(**(int **)(param_1 + 0x7d) + 0x80))(*(int **)(param_1 + 0x7d),iVar4);
      return sVar1 == 0x1f;
    }
  }
  return true;
}

