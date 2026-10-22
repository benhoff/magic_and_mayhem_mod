
int __fastcall FUN_00546120(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = FUN_005456c0();
  if (iVar1 != 0) {
    if (*(int *)(param_1 + 0x30) < 1) {
      *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) - *(int *)(param_1 + 0x2c);
    }
    else {
      *(int *)(param_1 + 0x30) = *(int *)(param_1 + 0x30) + -1;
    }
    iVar3 = *(int *)(param_1 + 0x28);
    if (0 < iVar3) {
      iVar2 = (*(int *)(param_1 + 0x24) * iVar3) / 100;
      iVar4 = (*(int *)(param_1 + 0x20) * iVar3) / 100;
      iVar3 = (*(int *)(param_1 + 0x1c) * iVar3) / 100;
      if (iVar3 < 0x100) {
        if (iVar3 < 0) {
          iVar3 = 0;
        }
      }
      else {
        iVar3 = 0xff;
      }
      *(int *)(param_1 + 0x3c) = (int)(iVar3 + (iVar3 >> 0x1f & 7U)) >> 3;
      if (iVar4 < 0x100) {
        if (iVar4 < 0) {
          iVar4 = 0;
        }
      }
      else {
        iVar4 = 0xff;
      }
      *(int *)(param_1 + 0x40) = iVar4 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
      if (iVar2 < 0x100) {
        if (iVar2 < 0) {
          iVar2 = 0;
        }
      }
      else {
        iVar2 = 0xff;
      }
      *(int *)(param_1 + 0x44) = (int)(iVar2 + (iVar2 >> 0x1f & 7U)) >> 3;
      return iVar1;
    }
  }
  return 0;
}

