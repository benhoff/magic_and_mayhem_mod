
undefined4 * __thiscall
FUN_00548650(undefined4 *param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  undefined4 *puVar1;
  ushort *puVar2;
  uint uVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c28c0;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00545650(param_2,param_3,param_4,param_5,param_6,param_7);
  *param_1 = &PTR_FUN_005c73d8;
  puVar1 = param_1 + 7;
  local_4 = 0;
  *puVar1 = 2;
  param_1[8] = 2;
  param_1[9] = 2;
  *(undefined4 *)((int)param_1 + 0x2e) = 0;
  FUN_00547890(0xc0,0,0);
  *puVar1 = 2;
  uVar3 = param_1[3];
  uVar7 = (uint)param_1[1] >> 5;
  uVar8 = (uint)param_1[2] >> 5;
  *param_1 = &PTR_FUN_005c74b0;
  *(uint *)((int)param_1 + 0x32) = uVar7;
  *(uint *)((int)param_1 + 0x36) = uVar8;
  *(uint *)((int)param_1 + 0x3a) = uVar3 >> 4;
  local_4 = 1;
  if ((int)uVar3 < DAT_005eb488) {
    puVar2 = (ushort *)
             (DAT_006c54dc + ((&DAT_006cb8c2)[uVar3 >> 4] + uVar7 + (&DAT_006cb942)[uVar8]) * 0xc);
    *(ushort **)((int)param_1 + 0x3e) = puVar2;
    *(uint *)((int)param_1 + 0x42) = DAT_0065660c + (uint)*puVar2 * 0x164;
    *(int *)((int)param_1 + 0x46) =
         (int)*(char *)((&DAT_006cb8c2)[uVar3 >> 5] + uVar7 + DAT_006c5c5c + (&DAT_006cb942)[uVar8])
    ;
  }
  uVar4 = DAT_005e1428;
  *puVar1 = 0;
  *(undefined4 *)((int)param_1 + 0x56) = uVar4;
  iVar6 = DAT_005e142c;
  if ((DAT_005e142c != param_1[8]) || (DAT_005e142c != param_1[9])) {
    if (DAT_005e142c < 1) {
      iVar5 = 1;
    }
    else {
      iVar5 = DAT_005e142c;
      if (0x10 < DAT_005e142c) {
        iVar5 = 0x10;
      }
    }
    param_1[8] = iVar5;
    if (iVar6 < 1) {
      iVar6 = 1;
    }
    else if (0x10 < iVar6) {
      iVar6 = 0x10;
    }
    param_1[9] = iVar6;
    if (*(int *)((int)param_1 + 0x2e) != 0) {
      FUN_00597870(*(int *)((int)param_1 + 0x2e));
      *(undefined4 *)((int)param_1 + 0x2e) = 0;
      FUN_00547b70();
    }
  }
  if (DAT_005e1430 != 0) {
    FUN_00547b70();
  }
  ExceptionList = local_c;
  return param_1;
}

