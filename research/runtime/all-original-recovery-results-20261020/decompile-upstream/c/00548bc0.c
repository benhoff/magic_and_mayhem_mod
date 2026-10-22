
undefined4 __fastcall FUN_00548bc0(int param_1)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iStack_38;
  int *piStack_34;
  int iStack_28;
  int iStack_24;
  int iStack_20;
  int iStack_1c;
  int iStack_18;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c2921;
  pvStack_c = ExceptionList;
  iVar4 = *(int *)(param_1 + 0x3c);
  ExceptionList = &pvStack_c;
  if ((iVar4 != 0) && (ExceptionList = &pvStack_c, *(int *)(iVar4 + 0xe4) != 0)) {
    iVar5 = *(int *)(*(int *)(iVar4 + 0xac) + 0x198) + *(int *)(iVar4 + 0x14);
    ExceptionList = &pvStack_c;
    *(int *)(param_1 + 4) = iVar5;
    while (iVar5 < 0) {
      iVar5 = *(int *)(param_1 + 4) + DAT_005eb480;
      *(int *)(param_1 + 4) = iVar5;
    }
    if (DAT_005eb480 <= *(int *)(param_1 + 4)) {
      do {
        iVar5 = *(int *)(param_1 + 4) - DAT_005eb480;
        *(int *)(param_1 + 4) = iVar5;
      } while (DAT_005eb480 <= iVar5);
    }
    iVar5 = *(int *)(*(int *)(iVar4 + 0xac) + 0x198) + *(int *)(iVar4 + 0x18);
    *(int *)(param_1 + 8) = iVar5;
    while (iVar5 < 0) {
      iVar5 = *(int *)(param_1 + 8) + DAT_005eb484;
      *(int *)(param_1 + 8) = iVar5;
    }
    if (DAT_005eb484 <= *(int *)(param_1 + 8)) {
      do {
        iVar5 = *(int *)(param_1 + 8) - DAT_005eb484;
        *(int *)(param_1 + 8) = iVar5;
      } while (DAT_005eb484 <= iVar5);
    }
    *(int *)(param_1 + 0xc) = *(int *)(iVar4 + 0x1c) + *(int *)(*(int *)(iVar4 + 0xac) + 0x19c);
  }
  iStack_28 = *(int *)(param_1 + 0x34);
  iStack_20 = *(int *)(param_1 + 0x2c);
  iStack_24 = *(int *)(param_1 + 0x30);
  iStack_18 = 0;
  if (0 < *(int *)(param_1 + 0x14)) {
    do {
      iVar4 = *(int *)(param_1 + 0x28);
      uVar2 = FUN_0054e200();
      iVar5 = *(int *)(param_1 + 0x28);
      iVar8 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar4) - iVar5 / 2) +
              *(int *)(param_1 + 4);
      uVar2 = FUN_0054e200();
      iVar4 = *(int *)(param_1 + 0x28);
      iVar9 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar5) - iVar4 / 2) +
              *(int *)(param_1 + 8);
      uVar2 = FUN_0054e200();
      iVar5 = *(int *)(param_1 + 0x28);
      iStack_38 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar4) - iVar5 / 2) +
                  *(int *)(param_1 + 0xc);
      uVar2 = FUN_0054e200();
      iVar4 = *(int *)(param_1 + 0x28);
      iVar6 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar5) - iVar4 / 2) +
              *(int *)(param_1 + 0x1c);
      uVar2 = FUN_0054e200();
      iVar5 = *(int *)(param_1 + 0x28);
      iVar7 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar4) - iVar5 / 2) +
              *(int *)(param_1 + 0x20);
      uVar2 = FUN_0054e200();
      iVar4 = *(int *)(param_1 + 0x38);
      iVar5 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar5) -
              *(int *)(param_1 + 0x28) / 2) + *(int *)(param_1 + 0x24);
      if (iVar4 != 0) {
        uVar2 = FUN_0054e200();
        iVar1 = *(int *)(param_1 + 0x38);
        iStack_20 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar4) -
                    *(int *)(param_1 + 0x38) / 2) + *(int *)(param_1 + 0x2c);
        uVar2 = FUN_0054e200();
        iVar4 = *(int *)(param_1 + 0x38);
        iStack_24 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar1) - iVar4 / 2)
                    + *(int *)(param_1 + 0x30);
        uVar2 = FUN_0054e200();
        iStack_28 = ((int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar4) -
                    *(int *)(param_1 + 0x38) / 2) + *(int *)(param_1 + 0x34);
      }
      uVar2 = FUN_0054e200();
      if (DAT_005e1424 < (int)((uVar2 & 0x7fffffff) % 100)) {
        if (*(char *)(param_1 + 0x40) != '\0') {
          piVar3 = (int *)FUN_00597850(0x32);
          uStack_4 = 0;
          if (piVar3 == (int *)0x0) {
            piVar3 = (int *)0x0;
          }
          else {
            FUN_00545590(iVar8,iVar9,iStack_38);
            piVar3[4] = iVar6;
            piVar3[5] = iVar7;
            piVar3[6] = iVar5;
            *piVar3 = (int)&PTR_FUN_005c73d8;
            uStack_4 = CONCAT31(uStack_4._1_3_,1);
            piVar3[7] = 2;
            piVar3[8] = 2;
            piVar3[9] = 2;
            *(undefined4 *)((int)piVar3 + 0x2e) = 0;
            FUN_00547890(0xc0,0,0);
            *piVar3 = (int)&PTR_FUN_005c7494;
            piVar3[7] = 2;
          }
          uStack_4 = 0xffffffff;
          if (piVar3 != (int *)0x0) {
            (**(code **)(*piVar3 + 0x18))(iStack_20,iStack_24,iStack_28);
            iVar4 = FUN_005457e0(piVar3);
            if (iVar4 == 0) {
              if (piVar3 != (int *)0x0) {
                (**(code **)*piVar3)(1);
              }
              break;
            }
          }
        }
      }
      else {
        iVar4 = FUN_00597850(0x5a);
        uStack_4 = 2;
        if (iVar4 == 0) {
          piStack_34 = (int *)0x0;
        }
        else {
          piStack_34 = (int *)FUN_00548650(iVar8,iVar9,iStack_38,iVar6,iVar7,iVar5);
        }
        uStack_4 = 0xffffffff;
        if (piStack_34 != (int *)0x0) {
          (**(code **)(*piStack_34 + 0x18))(iStack_20,iStack_24,iStack_28);
          iVar4 = FUN_005457e0(piStack_34);
          if (iVar4 == 0) {
            if (piStack_34 != (int *)0x0) {
              (**(code **)*piStack_34)(1);
            }
            break;
          }
          if (((0 < DAT_006e1f5c) && (*(char *)(param_1 + 0x40) != '\0')) &&
             (iStack_1c = 0, 0 < DAT_006e1f5c)) {
            do {
              iVar9 = iVar9 - iVar7;
              iVar5 = iVar5 + 1;
              iVar8 = iVar8 - iVar6;
              iStack_38 = iStack_38 - iVar5;
              piVar3 = (int *)FUN_00597850(0x32);
              uStack_4 = 3;
              if (piVar3 == (int *)0x0) {
                piVar3 = (int *)0x0;
              }
              else {
                FUN_00545590(iVar8,iVar9,iStack_38);
                piVar3[4] = iVar6;
                piVar3[5] = iVar7;
                piVar3[6] = iVar5;
                *piVar3 = (int)&PTR_FUN_005c73d8;
                uStack_4 = CONCAT31(uStack_4._1_3_,4);
                piVar3[7] = 2;
                piVar3[8] = 2;
                piVar3[9] = 2;
                *(undefined4 *)((int)piVar3 + 0x2e) = 0;
                FUN_00547890(0xc0,0,0);
                piVar3[7] = 2;
                *piVar3 = (int)&PTR_FUN_005c7494;
              }
              uStack_4 = 0xffffffff;
              if (piVar3 != (int *)0x0) {
                if ((piVar3[8] != 1) || (piVar3[9] != 1)) {
                  piVar3[8] = 1;
                  piVar3[9] = 1;
                  if (*(int *)((int)piVar3 + 0x2e) != 0) {
                    FUN_00597870(*(int *)((int)piVar3 + 0x2e));
                    *(undefined4 *)((int)piVar3 + 0x2e) = 0;
                    FUN_00547b70();
                  }
                }
                piVar3[7] = 0;
                (**(code **)(*piVar3 + 0x18))(iStack_20,iStack_24,iStack_28);
                iVar4 = FUN_005457e0(piVar3);
                if (iVar4 == 0) {
                  if (piVar3 != (int *)0x0) {
                    (**(code **)*piVar3)(1);
                  }
                  break;
                }
              }
              iStack_1c = iStack_1c + 1;
            } while (iStack_1c < DAT_006e1f5c);
          }
        }
      }
      iStack_18 = iStack_18 + 1;
    } while (iStack_18 < *(int *)(param_1 + 0x14));
  }
  iVar5 = *(int *)(param_1 + 0x10) + -1;
  iVar4 = *(int *)(param_1 + 0x14) + *(int *)(param_1 + 0x18);
  *(int *)(param_1 + 0x10) = iVar5;
  *(undefined1 *)(param_1 + 0x40) = 0;
  *(int *)(param_1 + 0x14) = iVar4;
  if ((0 < iVar4) && (0 < iVar5)) {
    ExceptionList = pvStack_c;
    return 1;
  }
  ExceptionList = pvStack_c;
  return 0;
}

