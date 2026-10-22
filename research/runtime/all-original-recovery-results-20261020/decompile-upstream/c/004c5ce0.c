
void __fastcall FUN_004c5ce0(int param_1)

{
  int iVar1;
  int *piVar2;
  undefined4 *puVar3;
  int iVar4;
  int iStack_4;
  
  iVar1 = *(int *)(param_1 + 0x16);
  if (iVar1 != 0) {
    FUN_004c35d0();
    FUN_00597870(iVar1);
    *(undefined4 *)(param_1 + 0x16) = 0;
  }
  piVar2 = *(int **)(param_1 + 0x12);
  if (piVar2 != (int *)0x0) {
    if (*piVar2 != 0) {
      FUN_00597870(*piVar2);
    }
    FUN_00597870(piVar2);
    *(undefined4 *)(param_1 + 0x12) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x22);
  if (iVar1 != 0) {
    FUN_0058b3e0();
    FUN_00597870(iVar1);
    *(undefined4 *)(param_1 + 0x22) = 0;
  }
  if (*(int *)(param_1 + 0x1a) != 0) {
    FUN_00597870(*(int *)(param_1 + 0x1a));
    *(undefined4 *)(param_1 + 0x1a) = 0;
  }
  if (*(int **)(param_1 + 0xa00) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0xa00) + 4))(1);
    *(undefined4 *)(param_1 + 0xa00) = 0;
  }
  iVar1 = 0;
  if (0 < *(int *)(param_1 + 0x1a2a)) {
    puVar3 = (undefined4 *)(param_1 + 0xaee);
    do {
      FUN_004ca950();
      FUN_004a4f70();
      *puVar3 = 0xffffffff;
      iVar1 = iVar1 + 1;
      puVar3 = (undefined4 *)((int)puVar3 + 0x7a);
    } while (iVar1 < *(int *)(param_1 + 0x1a2a));
  }
  *(undefined4 *)(param_1 + 0x1a2a) = 0;
  puVar3 = (undefined4 *)(param_1 + 0x634);
  iVar1 = 8;
  do {
    FUN_004ca950();
    FUN_004ca950();
    FUN_004a4f70();
    puVar3[-0xf4] = 0xffffffff;
    FUN_004a4f70();
    *puVar3 = 0xffffffff;
    puVar3 = (undefined4 *)((int)puVar3 + 0x7a);
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  FUN_004ca950();
  FUN_004ca950();
  FUN_004ca950();
  FUN_004a4f70();
  *(undefined4 *)(param_1 + 0x7a) = 0xffffffff;
  FUN_004a4f70();
  *(undefined4 *)(param_1 + 0x12f) = 0xffffffff;
  FUN_004a4f70();
  *(undefined4 *)(param_1 + 0xa50) = 0xffffffff;
  if (*(int *)(param_1 + 0xac6) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xac6));
    *(undefined4 *)(param_1 + 0xac6) = 0;
    *(undefined4 *)(param_1 + 0xaca) = 0;
    *(undefined4 *)(param_1 + 0xace) = 0;
    *(undefined4 *)(param_1 + 0xad2) = 0;
  }
  piVar2 = (int *)(param_1 + 0xad6);
  iStack_4 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iStack_4 = iStack_4 + -1;
  } while (iStack_4 != 0);
  if (*(int *)(param_1 + 0xae6) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xae6));
    *(undefined4 *)(param_1 + 0xae6) = 0;
  }
  if (*(int *)(param_1 + 0xa04) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xa04));
    *(undefined4 *)(param_1 + 0xa04) = 0;
    *(undefined4 *)(param_1 + 0xa08) = 0;
    *(undefined4 *)(param_1 + 0xa0c) = 0;
    *(undefined4 *)(param_1 + 0xa10) = 0;
  }
  piVar2 = (int *)(param_1 + 0xa14);
  iStack_4 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iStack_4 = iStack_4 + -1;
  } while (iStack_4 != 0);
  if (*(int *)(param_1 + 0xa24) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xa24));
    *(undefined4 *)(param_1 + 0xa24) = 0;
  }
  if (*(int *)(param_1 + 0xa28) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xa28));
    *(undefined4 *)(param_1 + 0xa28) = 0;
    *(undefined4 *)(param_1 + 0xa2c) = 0;
    *(undefined4 *)(param_1 + 0xa30) = 0;
    *(undefined4 *)(param_1 + 0xa34) = 0;
  }
  piVar2 = (int *)(param_1 + 0xa38);
  iVar1 = 4;
  do {
    if (*piVar2 != 0) {
      FUN_0057d0d0(*piVar2);
      *piVar2 = 0;
    }
    piVar2 = piVar2 + 1;
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  if (*(int *)(param_1 + 0xa48) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0xa48));
    *(undefined4 *)(param_1 + 0xa48) = 0;
  }
  iVar1 = *(int *)(param_1 + 0x1a3a);
  if (iVar1 != 0) {
    FUN_0058b3e0();
    FUN_00597870(iVar1);
    *(undefined4 *)(param_1 + 0x1a3a) = 0;
  }
  piVar2 = (int *)(param_1 + 0x26);
  iVar1 = 2;
  do {
    iVar4 = 2;
    do {
      if (*piVar2 != 0) {
        FUN_004c5fb0(1);
        *piVar2 = 0;
      }
      piVar2 = piVar2 + 1;
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
    iVar1 = iVar1 + -1;
  } while (iVar1 != 0);
  return;
}

