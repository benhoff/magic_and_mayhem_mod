
void __thiscall FUN_0051fd60(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  undefined4 *puVar10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c247b;
  local_c = ExceptionList;
  if (((-1 < DAT_00644520) && (DAT_00644520 < 8)) &&
     (*(int *)(param_1 + 0x24 + DAT_00644520 * 4) != 0)) {
    iVar1 = *(int *)(param_1 + 0x14);
    iVar2 = *(int *)(param_1 + 0x18);
    iVar3 = *(int *)(*(int *)(param_1 + 0xac) + 0x198);
    iVar4 = *(int *)(*(int *)(param_1 + 0xac) + 0x19c);
    iVar5 = *(int *)(param_1 + 0x1c);
    param_2 = (DAT_005e1420 * param_2 * 2) / 100;
    ExceptionList = &local_c;
    if (0 < param_2) {
      do {
        iVar6 = FUN_00597850(0x48);
        puVar10 = (undefined4 *)0x0;
        local_4 = 0;
        if (iVar6 != 0) {
          uVar7 = FUN_0054e200();
          uVar8 = FUN_0054e200();
          uVar9 = FUN_0054e200();
          puVar10 = (undefined4 *)
                    FUN_00545e90(((uVar9 & 0x7fffffff) % 5 - 2) + iVar1 + iVar3,
                                 ((uVar8 & 0x7fffffff) % 5 - 2) + iVar2 + iVar3,
                                 ((uVar7 & 0x7fffffff) % 5 - 2) + iVar5 + iVar4);
        }
        local_4 = 0xffffffff;
        if (puVar10 != (undefined4 *)0x0) {
          if (*(int *)(param_1 + 0xa0) == 2) {
            FUN_00546030(0,0,0xff);
          }
          iVar6 = FUN_005457e0(puVar10);
          if (iVar6 == 0) {
            (**(code **)*puVar10)(1);
          }
        }
        param_2 = param_2 + -1;
      } while (param_2 != 0);
    }
  }
  ExceptionList = local_c;
  return;
}

