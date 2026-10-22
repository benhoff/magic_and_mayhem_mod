
void __thiscall FUN_00546030(int param_1,int param_2,int param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int iVar3;
  
  *(int *)(param_1 + 0x24) = param_4;
  iVar3 = *(int *)(param_1 + 0x28);
  iVar1 = (param_4 * 100) / iVar3;
  *(int *)(param_1 + 0x20) = param_3;
  *(int *)(param_1 + 0x1c) = param_2;
  iVar2 = (param_3 * 100) / iVar3;
  iVar3 = (param_2 * 100) / iVar3;
  if (iVar3 < 0x100) {
    if (iVar3 < 0) {
      iVar3 = 0;
    }
  }
  else {
    iVar3 = 0xff;
  }
  *(int *)(param_1 + 0x3c) = (int)(iVar3 + (iVar3 >> 0x1f & 7U)) >> 3;
  if (iVar2 < 0x100) {
    if (iVar2 < 0) {
      iVar2 = 0;
    }
  }
  else {
    iVar2 = 0xff;
  }
  *(int *)(param_1 + 0x40) = iVar2 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
  if (0xff < iVar1) {
    *(undefined4 *)(param_1 + 0x44) = 0x1f;
    return;
  }
  if (-1 < iVar1) {
    *(int *)(param_1 + 0x44) = (int)(iVar1 + (iVar1 >> 0x1f & 7U)) >> 3;
    return;
  }
  *(undefined4 *)(param_1 + 0x44) = 0;
  return;
}

