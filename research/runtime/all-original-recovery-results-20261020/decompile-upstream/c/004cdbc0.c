
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004cdbc0(undefined4 *param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c1bf8;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LAB_005c6d10;
  piVar3 = (int *)((int)param_1 + 0x7a);
  local_4 = 0;
  iVar2 = 3;
  do {
    if ((int *)*piVar3 != (int *)0x0) {
      (**(code **)(*(int *)*piVar3 + 4))(1);
    }
    piVar3 = piVar3 + 1;
    iVar2 = iVar2 + -1;
  } while (iVar2 != 0);
  puVar1 = *(undefined4 **)((int)param_1 + 0x41);
  if (puVar1 != (undefined4 *)0x0) {
    *(undefined4 *)((int)puVar1 + 0x55) = 0;
    *puVar1 = &PTR_LAB_005c5f24;
    if (*(int *)((int)puVar1 + 0x45) != 0) {
      FUN_00597870(*(int *)((int)puVar1 + 0x45));
    }
    FUN_004ca910();
    FUN_00597870(puVar1);
    *(undefined4 *)((int)param_1 + 0x41) = 0;
  }
  puVar1 = *(undefined4 **)((int)param_1 + 0x45);
  if (puVar1 != (undefined4 *)0x0) {
    *(undefined4 *)((int)puVar1 + 0x55) = 0;
    *puVar1 = &PTR_LAB_005c5f24;
    if (*(int *)((int)puVar1 + 0x45) != 0) {
      FUN_00597870(*(int *)((int)puVar1 + 0x45));
    }
    *puVar1 = &PTR_LAB_005c5c20;
    FUN_004a4f70();
    puVar1[1] = 0xffffffff;
    FUN_00597870(puVar1);
    *(undefined4 *)((int)param_1 + 0x45) = 0;
  }
  puVar1 = *(undefined4 **)((int)param_1 + 0x49);
  if (puVar1 != (undefined4 *)0x0) {
    *(undefined4 *)((int)puVar1 + 0x55) = 0;
    *puVar1 = &PTR_LAB_005c5f24;
    if (*(int *)((int)puVar1 + 0x45) != 0) {
      FUN_00597870(*(int *)((int)puVar1 + 0x45));
    }
    *puVar1 = &PTR_LAB_005c5c20;
    FUN_004a4f70();
    puVar1[1] = 0xffffffff;
    FUN_00597870(puVar1);
    *(undefined4 *)((int)param_1 + 0x49) = 0;
  }
  DAT_006e2138 = DAT_006e2138 + -1;
  if (DAT_006e2138 == 0) {
    if (DAT_006e2068 != 0) {
      FUN_005979b0(DAT_006e2068);
      DAT_006e2068 = 0;
      _DAT_006e206c = 0;
      DAT_006e2070 = 0;
      DAT_006e2074 = 0;
    }
    iVar2 = 0x10;
    do {
      if (*(int *)((int)&DAT_006e2068 + iVar2) != 0) {
        FUN_0057d0d0(*(int *)((int)&DAT_006e2068 + iVar2));
        *(undefined4 *)((int)&DAT_006e2068 + iVar2) = 0;
      }
      iVar2 = iVar2 + 4;
    } while (iVar2 < 0x20);
    if (DAT_006e2088 != 0) {
      FUN_005979b0(DAT_006e2088);
      DAT_006e2088 = 0;
    }
  }
  local_4 = 0xffffffff;
  *param_1 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  param_1[1] = 0xffffffff;
  ExceptionList = pvStack_c;
  return;
}

