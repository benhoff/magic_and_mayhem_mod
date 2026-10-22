
int __fastcall FUN_00546350(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  
  iVar1 = *(int *)(param_1 + 0x60);
  if ((iVar1 != 0) && (*(int *)(iVar1 + 0xe4) != 0)) {
    if (*(int *)(param_1 + 0x54) != 0) {
      iVar3 = *(int *)(param_1 + 0x48) +
              *(int *)(*(int *)(iVar1 + 0xac) + 0x198) + *(int *)(iVar1 + 0x14);
      *(int *)(param_1 + 4) = iVar3;
      while (iVar3 < 0) {
        iVar3 = *(int *)(param_1 + 4) + DAT_005eb480;
        *(int *)(param_1 + 4) = iVar3;
      }
      if (DAT_005eb480 <= *(int *)(param_1 + 4)) {
        do {
          iVar3 = *(int *)(param_1 + 4) - DAT_005eb480;
          *(int *)(param_1 + 4) = iVar3;
        } while (DAT_005eb480 <= iVar3);
      }
    }
    if (*(int *)(param_1 + 0x58) != 0) {
      iVar3 = *(int *)(param_1 + 0x4c) +
              *(int *)(*(int *)(iVar1 + 0xac) + 0x198) + *(int *)(iVar1 + 0x18);
      *(int *)(param_1 + 8) = iVar3;
      while (iVar3 < 0) {
        iVar3 = *(int *)(param_1 + 8) + DAT_005eb484;
        *(int *)(param_1 + 8) = iVar3;
      }
      if (DAT_005eb484 <= *(int *)(param_1 + 8)) {
        do {
          iVar3 = *(int *)(param_1 + 8) - DAT_005eb484;
          *(int *)(param_1 + 8) = iVar3;
        } while (DAT_005eb484 <= iVar3);
      }
    }
    if (*(int *)(param_1 + 0x5c) != 0) {
      *(int *)(param_1 + 0xc) =
           *(int *)(*(int *)(iVar1 + 0xac) + 0x19c) + *(int *)(iVar1 + 0x1c) +
           *(int *)(param_1 + 0x50);
    }
  }
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

