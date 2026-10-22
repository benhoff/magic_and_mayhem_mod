
void __thiscall FUN_0051fba0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  undefined4 *puVar13;
  int iVar14;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c245b;
  local_c = ExceptionList;
  if (((-1 < DAT_00644520) && (DAT_00644520 < 8)) &&
     (*(int *)(param_1 + 0x24 + DAT_00644520 * 4) != 0)) {
    iVar14 = *(int *)(param_1 + 0x14);
    iVar1 = *(int *)(param_1 + 0x18);
    iVar2 = *(int *)(param_1 + 0x1c);
    iVar3 = *(int *)(*(int *)(param_1 + 0xac) + 0x198);
    iVar4 = *(int *)(*(int *)(param_1 + 0xac) + 0x19c);
    ExceptionList = &local_c;
    uVar9 = FUN_0054e200();
    uVar10 = FUN_0054e200();
    uVar11 = FUN_0054e200();
    iVar8 = ((param_2 / 2 + 4) * DAT_005e1420) / 100;
    iVar12 = FUN_00597850(0x41);
    local_4 = 0;
    if (iVar12 == 0) {
      puVar13 = (undefined4 *)0x0;
    }
    else {
      puVar13 = (undefined4 *)
                FUN_00548ac0(iVar14 + iVar3,iVar1 + iVar3,iVar2 + iVar4,(uVar9 & 0x7fffffff) % 5 - 2
                             ,(uVar10 & 0x7fffffff) % 5 - 2,(uVar11 & 0x7fffffff) % 3 + 4);
    }
    local_4 = 0xffffffff;
    if (puVar13 != (undefined4 *)0x0) {
      iVar14 = *(int *)(param_1 + 0xac);
      uVar5 = *(undefined4 *)(iVar14 + 0x5a5);
      uVar6 = *(undefined4 *)(iVar14 + 0x5a1);
      uVar7 = *(undefined4 *)(iVar14 + 0x59d);
      puVar13[0xb] = *(undefined4 *)(iVar14 + 0x599);
      puVar13[0xc] = uVar7;
      puVar13[0xd] = uVar6;
      puVar13[0xe] = uVar5;
      puVar13[0xf] = param_1;
      if (*(int *)(param_1 + 0xe4) == 0) {
        puVar13[4] = 5;
        puVar13[5] = iVar8;
        puVar13[6] = 0xfffffffe;
      }
      else {
        puVar13[4] = 10;
        puVar13[5] = iVar8;
        puVar13[6] = 0xffffffff;
      }
      iVar14 = FUN_005457e0(puVar13);
      if (iVar14 == 0) {
        (**(code **)*puVar13)(1);
      }
    }
  }
  ExceptionList = local_c;
  return;
}

