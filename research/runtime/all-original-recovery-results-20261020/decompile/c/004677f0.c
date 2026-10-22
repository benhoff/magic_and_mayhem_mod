
/* WARNING: Removing unreachable block (ram,0x00467ac1) */
/* WARNING: Removing unreachable block (ram,0x00467ad7) */

undefined4 __thiscall FUN_004677f0(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  uint uVar6;
  undefined4 *puVar7;
  int local_34;
  int local_30;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005bf3fb;
  local_c = ExceptionList;
  uVar6 = -(uint)(DAT_006def5c != 0) & DAT_006def58;
  ExceptionList = &local_c;
  iVar1 = FUN_00594260();
  uVar5 = 0;
  if (iVar1 != 0) {
    iVar1 = *(int *)(iVar1 + 0x44);
    iVar2 = FUN_00466870();
    uVar5 = 0;
    if (iVar2 != 0) {
      FUN_00571310(0x2e8);
      local_30 = 0;
      if (0 < DAT_006df180) {
        do {
          if ((((*(int *)(uVar6 + 4) != 0) && (iVar3 = **(int **)(uVar6 + 0xac), iVar3 != 0)) &&
              (iVar3 != 0x18)) &&
             (((iVar3 != 0x19 && (iVar3 != 0x1a)) &&
              ((*(int *)(uVar6 + 0xe4) != 0 && (iVar3 = FUN_0051e4a0(), iVar3 != 0)))))) {
            FUN_0040eeb0();
            FUN_0040e8a0();
            FUN_0040e290();
            FUN_0040eeb0();
            FUN_0040e8a0();
            FUN_0040e290(*(undefined4 *)(param_1 + 8));
            iVar3 = FUN_004e1380();
            if (iVar3 != 0) {
              uVar4 = FUN_0054e200();
              if (DAT_006e1f30 != 0) {
                FUN_00485b10();
              }
              if (((uVar4 & 0x7fffffff) % 100 <=
                   (uint)(param_2 * 100) / (uint)(*(int *)(param_1 + 0x84) + param_2)) &&
                 (*(int *)(iVar2 + 0x8f5) < iVar1)) {
                FUN_0056c650();
                FUN_0051ec80();
                FUN_005074f0();
                if ((-1 < DAT_00644520) &&
                   (((DAT_00644520 < 8 && (*(int *)(uVar6 + 0x24 + DAT_00644520 * 4) != 0)) &&
                    (local_34 = ((*(int *)(*(int *)(uVar6 + 0xac) + 0xc) + 1) *
                                 *(int *)(*(int *)(uVar6 + 0xac) + 8) + 1) * 6, 0 < local_34)))) {
                  do {
                    iVar3 = FUN_00597850();
                    puVar7 = (undefined4 *)0x0;
                    local_4 = 0;
                    if (iVar3 != 0) {
                      FUN_0054e200();
                      FUN_0054e200();
                      FUN_0054e200();
                      puVar7 = (undefined4 *)FUN_00549a00();
                    }
                    local_4 = 0xffffffff;
                    if ((puVar7 != (undefined4 *)0x0) && (iVar3 = FUN_005457e0(), iVar3 == 0)) {
                      (**(code **)*puVar7)();
                    }
                    local_34 = local_34 + -1;
                  } while (local_34 != 0);
                }
              }
            }
          }
          local_30 = local_30 + 1;
          uVar6 = uVar6 + 0xe4b;
        } while (local_30 < DAT_006df180);
      }
      uVar5 = 1;
    }
  }
  ExceptionList = local_c;
  return uVar5;
}

