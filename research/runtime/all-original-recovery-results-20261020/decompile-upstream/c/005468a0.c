
/* WARNING: Removing unreachable block (ram,0x00546979) */
/* WARNING: Removing unreachable block (ram,0x0054699e) */

undefined4 __fastcall FUN_005468a0(int param_1)

{
  int iVar1;
  uint uVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  undefined4 uStack_10;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c283b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  iVar1 = FUN_005456c0();
  iVar6 = *(int *)(param_1 + 0x18) + -1;
  *(int *)(param_1 + 0x18) = iVar6;
  if ((iVar1 != 0) && (0 < *(int *)(param_1 + 0xc))) {
    iVar1 = *(int *)(param_1 + 0x28) - DAT_005e1470;
    uStack_10 = 1;
    *(int *)(param_1 + 0x28) = iVar1;
    if ((iVar6 <= -DAT_005e146c) || (iVar1 < 0xb)) {
      iVar1 = *(int *)(param_1 + 0x2c) + -1;
      *(int *)(param_1 + 0x2c) = iVar1;
      if (iVar1 < 1) {
        uStack_10 = 0;
      }
      else {
        *(int *)(param_1 + 0x28) = *(int *)(param_1 + 0x28) + DAT_005e1474;
        iVar1 = DAT_005e1468;
        uVar2 = FUN_0054e200();
        iVar1 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar1);
        *(int *)(param_1 + 0x10) = iVar1;
        iVar1 = DAT_005e1468 - iVar1;
        uVar2 = FUN_0054e200();
        *(undefined4 *)(param_1 + 0x18) = 0;
        *(int *)(param_1 + 0x14) =
             (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar1);
        uVar2 = FUN_0054e200();
        if ((uVar2 & 1) != 0) {
          *(int *)(param_1 + 0x10) = -*(int *)(param_1 + 0x10);
        }
        uVar2 = FUN_0054e200();
        if ((uVar2 & 1) != 0) {
          *(int *)(param_1 + 0x14) = -*(int *)(param_1 + 0x14);
        }
        if (*(char *)(param_1 + 0x44) == '\x01') {
          *(undefined1 *)(param_1 + 0x44) = 0;
          iVar1 = FUN_00597850(0x45);
          uStack_4 = 0;
          if (iVar1 == 0) {
            puVar3 = (undefined4 *)0x0;
          }
          else {
            puVar3 = (undefined4 *)
                     FUN_00546660(*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 8),
                                  *(undefined4 *)(param_1 + 0xc));
          }
          uStack_4 = 0xffffffff;
          if (puVar3 != (undefined4 *)0x0) {
            puVar3[10] = *(undefined4 *)(param_1 + 0x28);
            puVar3[0xb] = *(undefined4 *)(param_1 + 0x2c);
            puVar3[4] = -*(int *)(param_1 + 0x10);
            puVar3[5] = -*(int *)(param_1 + 0x14);
            iVar6 = *(int *)(param_1 + 0x1c);
            iVar4 = *(int *)(param_1 + 0x24);
            iVar5 = *(int *)(param_1 + 0x20);
            uVar2 = FUN_0054e200();
            puVar3[9] = iVar4;
            puVar3[8] = iVar5;
            iVar1 = ((uVar2 & 0x7fffffff) % 0x15 - 10) + puVar3[10];
            puVar3[7] = iVar6;
            iVar4 = (iVar4 * 100) / iVar1;
            iVar5 = (iVar5 * 100) / iVar1;
            iVar1 = (iVar6 * 100) / iVar1;
            if (iVar1 < 0x100) {
              if (iVar1 < 0) {
                iVar1 = 0;
              }
            }
            else {
              iVar1 = 0xff;
            }
            puVar3[0xe] = (int)(iVar1 + (iVar1 >> 0x1f & 7U)) >> 3;
            if (iVar5 < 0x100) {
              if (iVar5 < 0) {
                iVar5 = 0;
              }
            }
            else {
              iVar5 = 0xff;
            }
            puVar3[0xf] = iVar5 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
            if (iVar4 < 0x100) {
              if (iVar4 < 0) {
                iVar4 = 0;
              }
            }
            else {
              iVar4 = 0xff;
            }
            puVar3[0x10] = (int)(iVar4 + (iVar4 >> 0x1f & 7U)) >> 3;
            iVar1 = FUN_005457e0(puVar3);
            if (iVar1 == 0) {
              (**(code **)*puVar3)(1);
            }
          }
        }
      }
    }
    if (10 < *(int *)(param_1 + 0x28)) {
      uVar2 = FUN_0054e200();
      iVar1 = ((uVar2 & 0x7fffffff) % 0x15 - 10) + *(int *)(param_1 + 0x28);
      iVar6 = (*(int *)(param_1 + 0x24) * iVar1) / 100;
      iVar4 = (*(int *)(param_1 + 0x20) * iVar1) / 100;
      iVar1 = (*(int *)(param_1 + 0x1c) * iVar1) / 100;
      if (iVar1 < 0x100) {
        if (iVar1 < 0) {
          iVar1 = 0;
        }
      }
      else {
        iVar1 = 0xff;
      }
      *(int *)(param_1 + 0x38) = (int)(iVar1 + (iVar1 >> 0x1f & 7U)) >> 3;
      if (iVar4 < 0x100) {
        if (iVar4 < 0) {
          iVar4 = 0;
        }
      }
      else {
        iVar4 = 0xff;
      }
      *(int *)(param_1 + 0x3c) = iVar4 / (int)((-(uint)(DAT_006e1f88 != 0) & 4) + 4);
      if (iVar6 < 0x100) {
        if (iVar6 < 0) {
          iVar6 = 0;
        }
      }
      else {
        iVar6 = 0xff;
      }
      *(int *)(param_1 + 0x40) = (int)(iVar6 + (iVar6 >> 0x1f & 7U)) >> 3;
      ExceptionList = pvStack_c;
      return uStack_10;
    }
  }
  ExceptionList = pvStack_c;
  return 0;
}

