
/* WARNING: Removing unreachable block (ram,0x00516099) */
/* WARNING: Removing unreachable block (ram,0x00516068) */
/* WARNING: Removing unreachable block (ram,0x0051607f) */

void __thiscall FUN_00515f70(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  uint uVar6;
  undefined4 *puVar7;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c243b;
  local_c = ExceptionList;
  if (*(int *)(param_1 + 0x114) == 0) {
    ExceptionList = &local_c;
    *(undefined4 *)(param_1 + 0x114) = 1;
    iVar3 = DAT_006c4830;
    *(uint *)(param_1 + 0x11c) = param_2;
    param_2 = 0;
    *(int *)(param_1 + 0x118) = iVar3 + 0x32;
    iVar3 = *(int *)(*(int *)(param_1 + 0xac) + 8);
    if (iVar3 * *(int *)(*(int *)(param_1 + 0xac) + 0xc) + 1 + iVar3 != 0) {
      do {
        iVar3 = FUN_00597850(0x20);
        local_4 = 0;
        if (iVar3 == 0) {
          puVar7 = (undefined4 *)0x0;
        }
        else {
          iVar3 = *(int *)(*(int *)(param_1 + 0xac) + 0x19c);
          uVar4 = FUN_0054e200();
          iVar1 = *(int *)(*(int *)(param_1 + 0xac) + 0x198);
          uVar5 = FUN_0054e200();
          iVar2 = *(int *)(*(int *)(param_1 + 0xac) + 0x198);
          uVar6 = FUN_0054e200();
          puVar7 = (undefined4 *)
                   FUN_005491b0((uVar6 & 0x7f) + *(int *)(param_1 + 0x14) + -0x40 + iVar2,
                                (uVar5 & 0x7f) + *(int *)(param_1 + 0x18) + -0x40 + iVar1,
                                (uVar4 & 0x7f) + *(int *)(param_1 + 0x1c) + -0x40 + iVar3);
        }
        local_4 = 0xffffffff;
        if (puVar7 != (undefined4 *)0x0) {
          puVar7[7] = param_1;
          iVar3 = FUN_005457e0(puVar7);
          if (iVar3 == 0) {
            (**(code **)*puVar7)(1);
          }
        }
        param_2 = param_2 + 1;
      } while (param_2 < (*(int *)(*(int *)(param_1 + 0xac) + 0xc) + 1) *
                         *(int *)(*(int *)(param_1 + 0xac) + 8) + 1U);
    }
    if (*(int *)(param_1 + 0x120) == 0) {
      FUN_00571310(0x2c9,*(undefined4 *)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),
                   *(undefined4 *)(param_1 + 0x10),0,param_1 + 0x120,1);
    }
  }
  ExceptionList = local_c;
  return;
}

