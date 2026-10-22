
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005782a0(int *param_1)

{
  int *piVar1;
  int iVar2;
  int *piVar3;
  int iStack_18;
  undefined4 uStack_10;
  undefined4 uStack_c;
  undefined4 uStack_8;
  byte bStack_4;
  undefined3 uStack_3;
  
  param_1[2] = 0;
  if (param_1[0x94] == 1) {
    if (DAT_006593e0 != 0) {
      FUN_005979b0(DAT_006593e0);
      DAT_006593e0 = 0;
      _DAT_006593e4 = 0;
      DAT_006593e8 = 0;
      DAT_006593ec = 0;
    }
    iVar2 = 0x10;
    do {
      if (*(int *)((int)&DAT_006593e0 + iVar2) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006593e0 + iVar2));
        *(undefined4 *)((int)&DAT_006593e0 + iVar2) = 0;
      }
      iVar2 = iVar2 + 4;
    } while (iVar2 < 0x20);
    if (DAT_00659400 != 0) {
      FUN_005979b0(DAT_00659400);
      DAT_00659400 = 0;
    }
    FUN_00534b60();
    FUN_004de230();
  }
  FUN_00575890();
  (**(code **)(*param_1 + 0x18))();
  *(undefined1 *)(param_1 + 0x9a) = 0;
  if ((int *)param_1[0x8e] != (int *)0x0) {
    (**(code **)(*(int *)param_1[0x8e] + 4))(1);
    param_1[0x8e] = 0;
  }
  if ((int *)param_1[0x8d] != (int *)0x0) {
    (**(code **)(*(int *)param_1[0x8d] + 4))(1);
    param_1[0x8d] = 0;
  }
  if ((int *)param_1[0x91] != (int *)0x0) {
    (**(code **)(*(int *)param_1[0x91] + 4))(1);
    param_1[0x91] = 0;
  }
  if ((int *)param_1[0x90] != (int *)0x0) {
    (**(code **)(*(int *)param_1[0x90] + 4))(1);
    param_1[0x90] = 0;
  }
  if ((int *)param_1[0x8f] != (int *)0x0) {
    (**(code **)(*(int *)param_1[0x8f] + 4))(1);
    param_1[0x8f] = 0;
  }
  piVar1 = (int *)param_1[0x1f];
  if (piVar1 != (int *)0x0) {
    if (*piVar1 != 0) {
      FUN_005979b0(*piVar1);
      *piVar1 = 0;
      piVar1[1] = 0;
      piVar1[2] = 0;
      piVar1[3] = 0;
    }
    piVar3 = piVar1 + 4;
    iStack_18 = 4;
    do {
      if (*piVar3 != 0) {
        FUN_0057d0d0(*piVar3);
        *piVar3 = 0;
      }
      piVar3 = piVar3 + 1;
      iStack_18 = iStack_18 + -1;
    } while (iStack_18 != 0);
    if (piVar1[8] != 0) {
      FUN_005979b0(piVar1[8]);
      piVar1[8] = 0;
    }
    FUN_00597870(piVar1);
    param_1[0x1f] = 0;
  }
  piVar1 = (int *)param_1[0x1e];
  if (piVar1 != (int *)0x0) {
    if (*piVar1 != 0) {
      FUN_005979b0(*piVar1);
      *piVar1 = 0;
      piVar1[1] = 0;
      piVar1[2] = 0;
      piVar1[3] = 0;
    }
    piVar3 = piVar1 + 4;
    iStack_18 = 4;
    do {
      if (*piVar3 != 0) {
        FUN_0057d0d0(*piVar3);
        *piVar3 = 0;
      }
      piVar3 = piVar3 + 1;
      iStack_18 = iStack_18 + -1;
    } while (iStack_18 != 0);
    if (piVar1[8] != 0) {
      FUN_005979b0(piVar1[8]);
      piVar1[8] = 0;
    }
    FUN_00597870(piVar1);
    param_1[0x1e] = 0;
  }
  piVar1 = (int *)param_1[0x20];
  if (piVar1 != (int *)0x0) {
    if (*piVar1 != 0) {
      FUN_005979b0(*piVar1);
      *piVar1 = 0;
      piVar1[1] = 0;
      piVar1[2] = 0;
      piVar1[3] = 0;
    }
    piVar3 = piVar1 + 4;
    iVar2 = 4;
    do {
      if (*piVar3 != 0) {
        FUN_0057d0d0(*piVar3);
        *piVar3 = 0;
      }
      piVar3 = piVar3 + 1;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    if (piVar1[8] != 0) {
      FUN_005979b0(piVar1[8]);
      piVar1[8] = 0;
    }
    FUN_00597870(piVar1);
    param_1[0x20] = 0;
  }
  if (param_1[0x25] != -1) {
    param_1[0x25] = -1;
  }
  FUN_0058b3e0();
  if (param_1[0x26] != 0) {
    FUN_0058a0a0();
    param_1[0x26] = 0;
  }
  FUN_0058a0a0();
  FUN_0058a0a0();
  FUN_0058a0a0();
  FUN_00575940();
  FUN_005759c0();
  FUN_00572e70();
  if ((param_1[0x94] == 2) && (*(char *)((int)param_1 + 0x269) == '\0')) {
    FUN_004726f0();
    FUN_004de5a0((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x1e0,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x158);
    uStack_10 = 0;
    _bStack_4 = CONCAT31(uStack_3,(byte)DAT_00656634);
    uStack_c = 0xe;
    uStack_8 = 0xd;
    FUN_00594050((int)&DAT_00658f10 + (uint)(byte)DAT_00656634 * 0x133);
    FUN_00537bd0(&uStack_10);
  }
  return;
}

