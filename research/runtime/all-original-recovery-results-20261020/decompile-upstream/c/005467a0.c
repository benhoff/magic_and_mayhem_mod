
void __thiscall FUN_005467a0(int param_1,int param_2,int param_3,int param_4)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  uVar1 = FUN_0054e200();
  *(int *)(param_1 + 0x20) = param_3;
  *(int *)(param_1 + 0x1c) = param_2;
  iVar4 = ((uVar1 & 0x7fffffff) % 0x15 - 10) + *(int *)(param_1 + 0x28);
  *(int *)(param_1 + 0x24) = param_4;
  iVar2 = (param_4 * 100) / iVar4;
  iVar3 = (param_3 * 100) / iVar4;
  iVar4 = (param_2 * 100) / iVar4;
  if (iVar4 < 0x100) {
    if (iVar4 < 0) {
      iVar4 = 0;
    }
  }
  else {
    iVar4 = 0xff;
  }
  *(int *)(param_1 + 0x38) = (int)(iVar4 + (iVar4 >> 0x1f & 7U)) >> 3;
  if (iVar3 < 0x100) {
    if (iVar3 < 0) {
      iVar3 = 0;
    }
  }
  else {
    iVar3 = 0xff;
  }
  *(int *)(param_1 + 0x3c) = iVar3 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
  if (0xff < iVar2) {
    *(undefined4 *)(param_1 + 0x40) = 0x1f;
    return;
  }
  if (-1 < iVar2) {
    *(int *)(param_1 + 0x40) = (int)(iVar2 + (iVar2 >> 0x1f & 7U)) >> 3;
    return;
  }
  *(undefined4 *)(param_1 + 0x40) = 0;
  return;
}

