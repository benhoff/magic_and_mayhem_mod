
void __fastcall FUN_004d4970(int param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  undefined4 *puVar7;
  uint local_8;
  
  FUN_004d5640();
  uVar5 = 0;
  if (*(int *)(param_1 + 0x18ec) != 0) {
    uVar2 = *(uint *)(*(int *)(param_1 + 0x18ec) + 0x56c);
    if (uVar2 < 0x1a) {
      *(undefined4 *)(param_1 + 0x1518) = 0;
      *(uint *)(param_1 + 0x1921) = uVar2;
      if (uVar2 != 0) {
        if (uVar2 != 0) {
          iVar6 = 0x570;
          local_8 = uVar2;
          do {
            *(undefined4 *)(param_1 + 0x1b1 + *(int *)(param_1 + 0x1518) * 0xc6) =
                 *(undefined4 *)(iVar6 + *(int *)(param_1 + 0x18ec));
            *(undefined4 *)(param_1 + 0x1b5 + *(int *)(param_1 + 0x1518) * 0xc6) = 0;
            *(undefined4 *)(param_1 + 0x1b9 + *(int *)(param_1 + 0x1518) * 0xc6) = 0xffffffff;
            iVar1 = param_1 + *(int *)(param_1 + 0x1518) * 0xc6;
            *(int *)(iVar1 + 0x155) =
                 *(int *)(param_1 + 0x1b1 + *(int *)(param_1 + 0x1518) * 0xc6) + 2;
            *(undefined4 *)(iVar1 + 0x159) = 0xffffffff;
            *(undefined4 *)(iVar1 + 0x15d) = 0xffffffff;
            *(int *)(param_1 + *(int *)(param_1 + 0x1518) * 0xc6 + 0x131) =
                 *(int *)(param_1 + 0x1b1 + *(int *)(param_1 + 0x1518) * 0xc6) + 5;
            if (*(int *)(*(int *)(param_1 + 0x18ec) + 0x8f5) ==
                *(int *)(*(int *)(param_1 + 0x18ec) + 0x8f1)) {
              iVar1 = *(int *)(param_1 + 0x1518);
              iVar3 = *(int *)(param_1 + 0x1b1 + iVar1 * 0xc6);
              if (*(int *)(param_1 + 0x1b5 + iVar1 * 0xc6) == 0) {
                if (((iVar3 < 0x1b) || (iVar3 == 0x34)) || ((iVar3 == 0x36 || (iVar3 == 0x56)))) {
LAB_004d4aee:
                  *(undefined4 *)(param_1 + iVar1 * 0xc6 + 0x139) = 2;
                  FUN_004a5000();
                }
              }
              else if ((((0x1b < iVar3) && (iVar3 < 0x32)) || (iVar3 == 0x3a)) || (iVar3 == 0xc))
              goto LAB_004d4aee;
            }
            iVar6 = iVar6 + 4;
            *(int *)(param_1 + 0x1518) = *(int *)(param_1 + 0x1518) + 1;
            local_8 = local_8 - 1;
          } while (local_8 != 0);
        }
        puVar7 = (undefined4 *)(param_1 + 0x15f3);
        while( true ) {
          uVar4 = *(uint *)(param_1 + 0x17e8);
          if (uVar2 <= *(uint *)(param_1 + 0x17e8)) {
            uVar4 = uVar2;
          }
          if (uVar4 <= uVar5) break;
          puVar7[0x16] = *(undefined4 *)((int)puVar7 + -0x1442);
          *puVar7 = 0xffffffff;
          puVar7[-1] = *(int *)((int)puVar7 + -0x1442) + 2;
          puVar7[1] = 0xffffffff;
          puVar7[2] = 0x7f;
          puVar7[-10] = *(int *)((int)puVar7 + -0x1442) + 5;
          puVar7[0x19] = uVar5;
          puVar7[-9] = 0;
          if (puVar7[-8] != 2) {
            *(undefined4 *)((int)puVar7 + -0x55) = 1;
          }
          (**(code **)(*(int *)((int)puVar7 + -0x5d) + 0x10))();
          if (*(int *)(*(int *)(param_1 + 0x18ec) + 0x8f5) ==
              *(int *)(*(int *)(param_1 + 0x18ec) + 0x8f1)) {
            iVar6 = puVar7[0x16];
            if (puVar7[0x17] == 0) {
              if ((((iVar6 < 0x1b) || (iVar6 == 0x34)) || (iVar6 == 0x36)) || (iVar6 == 0x56)) {
LAB_004d4bdd:
                puVar7[-8] = 2;
                FUN_004a5000();
              }
            }
            else if (((0x1b < iVar6) && (iVar6 < 0x32)) || ((iVar6 == 0x3a || (iVar6 == 0xc))))
            goto LAB_004d4bdd;
          }
          uVar5 = uVar5 + 1;
          puVar7 = (undefined4 *)((int)puVar7 + 0xc6);
        }
      }
      uVar5 = 0;
      if (*(int *)(*(int *)(param_1 + 0x18ec) + 0x178) != 0) {
        do {
          iVar6 = FUN_00509790(uVar5);
          if (*(int *)(iVar6 + 0x14) != 0) {
            FUN_004d6620(iVar6,uVar5);
          }
          uVar5 = uVar5 + 1;
        } while (uVar5 < *(uint *)(*(int *)(param_1 + 0x18ec) + 0x178));
      }
    }
    else if (DAT_006e2032 != '\0') {
      MessageBoxA((HWND)0x0,s_Too_many_spells_to_fit_on_popup_b_005e2e78,
                  s_SELECT_BAR_MESSAGE_ERROR_005e2ea0,0x42030);
      return;
    }
  }
  return;
}

