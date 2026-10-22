
void __fastcall FUN_00550370(int param_1)

{
  int iVar1;
  int *piVar2;
  int local_4;
  
  if (*(int *)(param_1 + 0x12) != 0) {
    iVar1 = *(int *)(param_1 + 0x16);
    *(undefined4 *)(param_1 + 0x12) = 0;
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x8c,*(undefined4 *)(iVar1 + -4),FUN_0054fce0);
      FUN_00597870(iVar1 + -4);
    }
    iVar1 = *(int *)(param_1 + 0x1a);
    *(undefined4 *)(param_1 + 0x16) = 0;
    if (iVar1 != 0) {
      FUN_0059bf07(iVar1,0x14,*(undefined4 *)(iVar1 + -4),FUN_004c1130);
      FUN_00597870(iVar1 + -4);
    }
    *(undefined4 *)(param_1 + 0x1a) = 0;
    FUN_00597870(*(undefined4 *)(param_1 + 0x1e));
    *(undefined4 *)(param_1 + 0x1e) = 0;
    if (*(int *)(param_1 + 0x59) != 0) {
      FUN_005979b0(*(int *)(param_1 + 0x59));
      *(undefined4 *)(param_1 + 0x59) = 0;
      *(undefined4 *)(param_1 + 0x5d) = 0;
    }
    FUN_0058b3e0();
    FUN_0058b3e0();
    FUN_00597870(*(undefined4 *)(param_1 + 0x1d7));
    *(undefined4 *)(param_1 + 0x1d7) = 0;
    FUN_00597870(*(undefined4 *)(param_1 + 0x1db));
    *(undefined4 *)(param_1 + 0x1db) = 0;
    FUN_00464ad0();
    if (*(int *)(param_1 + 0x9f) != 0) {
      FUN_005979b0(*(int *)(param_1 + 0x9f));
      *(undefined4 *)(param_1 + 0x9f) = 0;
      *(undefined4 *)(param_1 + 0xa3) = 0;
      *(undefined4 *)(param_1 + 0xa7) = 0;
      *(undefined4 *)(param_1 + 0xab) = 0;
    }
    piVar2 = (int *)(param_1 + 0xaf);
    local_4 = 4;
    do {
      if (*piVar2 != 0) {
        FUN_0057d0d0(*piVar2);
        *piVar2 = 0;
      }
      piVar2 = piVar2 + 1;
      local_4 = local_4 + -1;
    } while (local_4 != 0);
    if (*(int *)(param_1 + 0xbf) != 0) {
      FUN_005979b0(*(int *)(param_1 + 0xbf));
      *(undefined4 *)(param_1 + 0xbf) = 0;
    }
    FUN_0056d930();
    if (*(int **)(param_1 + 0xa23) != (int *)0x0) {
      (**(code **)(**(int **)(param_1 + 0xa23) + 4))(1);
    }
    *(undefined4 *)(param_1 + 0xa23) = 0;
    FUN_004ca950();
    FUN_004ca950();
    FUN_004ca950();
    FUN_004ca950();
    FUN_004a4f70();
    *(undefined4 *)(param_1 + 0x82b) = 0xffffffff;
    FUN_004a4f70();
    *(undefined4 *)(param_1 + 0x91f) = 0xffffffff;
    FUN_004a4f70();
    *(undefined4 *)(param_1 + 0x8a5) = 0xffffffff;
    FUN_004a4f70();
    *(undefined4 *)(param_1 + 0x999) = 0xffffffff;
    FUN_00597870(*(undefined4 *)(param_1 + 0xa3c));
    *(undefined4 *)(param_1 + 0xa3c) = 0;
    if (*(int *)(param_1 + 0xa40) != 0) {
      FUN_00597870(*(int *)(param_1 + 0xa40));
      *(undefined4 *)(param_1 + 0xa40) = 0;
    }
    iVar1 = *(int *)(param_1 + 0xa44);
    if (iVar1 != 0) {
      FUN_00572400();
      FUN_00597870(iVar1);
      *(undefined4 *)(param_1 + 0xa44) = 0;
    }
  }
  DAT_006f2d24 = FUN_004caf50();
  FUN_004cacb0();
  return;
}

