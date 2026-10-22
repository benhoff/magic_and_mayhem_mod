
bool __thiscall FUN_004cb100(int param_1,int param_2,int param_3)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  short sVar5;
  int iVar6;
  
  if (*(int *)(param_1 + 0x4d) == 0) {
    return false;
  }
  iVar2 = *(int *)(*(int *)(param_1 + 0x31) + 0x20);
  iVar3 = *(int *)(*(int *)(param_1 + 0x31) + 0x24);
  sVar5 = FUN_0058d4b0();
  iVar6 = FUN_0058b660();
  if (iVar6 == 0) {
    return false;
  }
  sVar1 = *(short *)(iVar6 + (((*(uint *)(*(int *)(param_1 + 0x31) + 0x18) >> 1 & 0xffff) *
                               (param_3 - iVar3) - iVar2) + param_2) * 2);
  piVar4 = *(int **)(*(int *)(param_1 + 0x31) + 4);
  (**(code **)(*piVar4 + 0x80))(piVar4,iVar6);
  return sVar1 == sVar5;
}

