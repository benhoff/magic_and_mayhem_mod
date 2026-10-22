
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00553a40(int param_1)

{
  undefined2 uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined2 *puVar6;
  undefined2 *puVar7;
  int iVar8;
  int iVar9;
  int local_20;
  undefined2 *local_1c;
  uint local_18;
  int local_14;
  uint local_10;
  undefined2 *local_c;
  
  *(undefined4 *)(param_1 + 0xff) = 0xfffffffe;
  iVar2 = FUN_0058b660();
  uVar3 = *(uint *)(param_1 + 0x61c1) >> 1;
  if (*(int *)(param_1 + 0x29) != 0) {
    if (DAT_005e1404 == 0) {
      switch(*(undefined4 *)(param_1 + 0xc5)) {
      case 0:
        iVar5 = *(int *)(param_1 + 0x6199);
        iVar8 = *(int *)(param_1 + 0x619d);
        local_c = *(undefined2 **)(param_1 + 0xe7);
        iVar4 = *(int *)(param_1 + 0x61a1) - iVar5 / 2;
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (iVar4 < 0) {
          iVar4 = iVar4 + iVar5;
        }
        if (iVar5 <= iVar4) {
          iVar4 = iVar4 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        local_10 = 0;
        iVar8 = (iVar5 * local_20 + iVar4) * 0x10 + *(int *)(param_1 + 0x29);
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            uVar3 = 0;
            if (0 < iVar5) {
              do {
                puVar6 = (undefined2 *)(iVar8 + 0xc);
                iVar8 = iVar8 + 0x10;
                *local_c = *puVar6;
                iVar5 = *(int *)(param_1 + 0x6199);
                iVar4 = iVar4 + 1;
                if (iVar5 <= iVar4) {
                  iVar4 = iVar4 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((uVar3 & 1) == 0) {
                  local_c = local_c + 1;
                }
                else {
                  local_c = local_c + *(int *)(param_1 + 0xeb) + 1;
                }
                uVar3 = uVar3 + 1;
              } while ((int)uVar3 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar9 = *(int *)(param_1 + 0x619d);
            if (iVar9 <= local_20) {
              local_20 = local_20 - iVar9;
              iVar8 = iVar8 + iVar9 * iVar5 * -0x10;
            }
            iVar9 = -(((*(int *)(param_1 + 0xeb) + 1) * iVar5) / 2) - iVar5 / 2;
            if ((local_10 & 1) == 0) {
              iVar9 = iVar9 + *(int *)(param_1 + 0xeb);
            }
            else {
              iVar9 = iVar9 + -2;
            }
            local_c = local_c + iVar9;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
        break;
      case 1:
        iVar5 = *(int *)(param_1 + 0x6199);
        iVar8 = iVar5 / 2;
        iVar4 = (*(int *)(param_1 + 0x61a1) - iVar8) + 1;
        local_c = (undefined2 *)
                  (*(int *)(param_1 + 0xe7) + 4 +
                  ((iVar8 + -1) * *(int *)(param_1 + 0xeb) + iVar8 * -2) * 2);
        iVar8 = *(int *)(param_1 + 0x619d);
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (iVar4 < 0) {
          iVar4 = iVar4 + iVar5;
        }
        if (iVar5 <= iVar4) {
          iVar4 = iVar4 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        local_10 = 0;
        iVar8 = (iVar5 * local_20 + iVar4) * 0x10 + *(int *)(param_1 + 0x29);
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            local_18 = 0;
            if (0 < iVar5) {
              do {
                puVar6 = (undefined2 *)(iVar8 + 0xc);
                iVar8 = iVar8 + 0x10;
                *local_c = *puVar6;
                iVar5 = *(int *)(param_1 + 0x6199);
                iVar4 = iVar4 + 1;
                if (iVar5 <= iVar4) {
                  iVar4 = iVar4 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((local_18 & 1) == 0) {
                  local_c = local_c + 1;
                }
                else {
                  local_c = local_c + (1 - *(int *)(param_1 + 0xeb));
                }
                local_18 = local_18 + 1;
              } while ((int)local_18 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar9 = *(int *)(param_1 + 0x619d);
            if (iVar9 <= local_20) {
              local_20 = local_20 - iVar9;
              iVar8 = iVar8 + iVar9 * iVar5 * -0x10;
            }
            iVar9 = ((*(int *)(param_1 + 0xeb) + -1) * iVar5) / 2 - iVar5 / 2;
            if ((local_10 & 1) == 0) {
              iVar9 = iVar9 + *(int *)(param_1 + 0xeb);
            }
            else {
              iVar9 = iVar9 + 2;
            }
            local_c = local_c + iVar9;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
        break;
      case 2:
        iVar5 = *(int *)(param_1 + 0x6199);
        puVar6 = (undefined2 *)
                 (*(int *)(param_1 + 0xe7) + 2 + (iVar5 + -1) * *(int *)(param_1 + 0xeb) * 2);
        iVar8 = *(int *)(param_1 + 0x619d);
        local_14 = *(int *)(param_1 + 0x61a1) - iVar5 / 2;
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (local_14 < 0) {
          local_14 = local_14 + iVar5;
        }
        if (iVar5 <= local_14) {
          local_14 = local_14 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        local_10 = 0;
        iVar8 = (iVar5 * local_20 + local_14) * 0x10 + *(int *)(param_1 + 0x29);
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            uVar3 = 0;
            if (0 < iVar5) {
              do {
                puVar7 = (undefined2 *)(iVar8 + 0xc);
                iVar8 = iVar8 + 0x10;
                *puVar6 = *puVar7;
                iVar5 = *(int *)(param_1 + 0x6199);
                local_14 = local_14 + 1;
                if (iVar5 <= local_14) {
                  local_14 = local_14 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((uVar3 & 1) == 0) {
                  puVar6 = puVar6 + -1;
                }
                else {
                  puVar6 = puVar6 + (-1 - *(int *)(param_1 + 0xeb));
                }
                uVar3 = uVar3 + 1;
              } while ((int)uVar3 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar4 = *(int *)(param_1 + 0x619d);
            if (iVar4 <= local_20) {
              local_20 = local_20 - iVar4;
              iVar8 = iVar8 + iVar4 * iVar5 * -0x10;
            }
            iVar4 = ((*(int *)(param_1 + 0xeb) + 1) * iVar5) / 2 + iVar5 / 2;
            if ((local_10 & 1) == 0) {
              iVar4 = iVar4 - *(int *)(param_1 + 0xeb);
            }
            else {
              iVar4 = iVar4 + 2;
            }
            puVar6 = puVar6 + iVar4;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
        break;
      case 3:
        iVar5 = *(int *)(param_1 + 0x6199);
        local_c = (undefined2 *)
                  (*(int *)(param_1 + 0xe7) + -2 + (*(int *)(param_1 + 0xeb) + 2) * (iVar5 / 2) * 2)
        ;
        iVar8 = *(int *)(param_1 + 0x619d);
        iVar4 = (*(int *)(param_1 + 0x61a1) - iVar5 / 2) + 1;
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (iVar4 < 0) {
          iVar4 = iVar4 + iVar5;
        }
        if (iVar5 <= iVar4) {
          iVar4 = iVar4 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        local_10 = 0;
        iVar8 = (iVar5 * local_20 + iVar4) * 0x10 + *(int *)(param_1 + 0x29);
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            uVar3 = 0;
            if (0 < iVar5) {
              do {
                puVar6 = (undefined2 *)(iVar8 + 0xc);
                iVar8 = iVar8 + 0x10;
                *local_c = *puVar6;
                iVar5 = *(int *)(param_1 + 0x6199);
                iVar4 = iVar4 + 1;
                if (iVar5 <= iVar4) {
                  iVar4 = iVar4 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((uVar3 & 1) == 0) {
                  local_c = local_c + -1;
                }
                else {
                  local_c = local_c + *(int *)(param_1 + 0xeb) + -1;
                }
                uVar3 = uVar3 + 1;
              } while ((int)uVar3 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar9 = *(int *)(param_1 + 0x619d);
            if (iVar9 <= local_20) {
              local_20 = local_20 - iVar9;
              iVar8 = iVar8 + iVar9 * iVar5 * -0x10;
            }
            iVar9 = iVar5 / 2 - ((*(int *)(param_1 + 0xeb) + -1) * iVar5) / 2;
            if ((local_10 & 1) == 0) {
              iVar9 = iVar9 - *(int *)(param_1 + 0xeb);
            }
            else {
              iVar9 = iVar9 + -2;
            }
            local_c = local_c + iVar9;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
      }
    }
    else {
      switch(*(undefined4 *)(param_1 + 0xc5)) {
      case 0:
        iVar5 = *(int *)(param_1 + 0x6199);
        iVar8 = *(int *)(param_1 + 0x619d);
        puVar6 = *(undefined2 **)(param_1 + 0xe7);
        local_1c = (undefined2 *)(iVar2 + -2 + iVar5 * 2);
        local_14 = *(int *)(param_1 + 0x61a1) - iVar5 / 2;
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (local_14 < 0) {
          local_14 = local_14 + iVar5;
        }
        if (iVar5 <= local_14) {
          local_14 = local_14 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        local_10 = 0;
        iVar8 = (iVar5 * local_20 + local_14) * 0x10 + *(int *)(param_1 + 0x29);
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            local_18 = 0;
            if (0 < iVar5) {
              do {
                if (**(char **)(iVar8 + 8) == _DAT_005e18f0) {
                  uVar1 = *local_1c;
                }
                else {
                  uVar1 = *(undefined2 *)(iVar8 + 0xc);
                }
                *puVar6 = uVar1;
                iVar5 = *(int *)(param_1 + 0x6199);
                iVar8 = iVar8 + 0x10;
                local_14 = local_14 + 1;
                if (iVar5 <= local_14) {
                  local_14 = local_14 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((local_18 & 1) == 0) {
                  puVar6 = puVar6 + 1;
                  local_1c = local_1c + 1;
                }
                else {
                  puVar6 = puVar6 + *(int *)(param_1 + 0xeb) + 1;
                  local_1c = local_1c + uVar3 + 1;
                }
                local_18 = local_18 + 1;
              } while ((int)local_18 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar4 = *(int *)(param_1 + 0x619d);
            if (iVar4 <= local_20) {
              local_20 = local_20 - iVar4;
              iVar8 = iVar8 + iVar4 * iVar5 * -0x10;
            }
            iVar4 = -(((*(int *)(param_1 + 0xeb) + 1) * iVar5) / 2) - iVar5 / 2;
            iVar9 = -((int)((uVar3 + 1) * iVar5) / 2) - iVar5 / 2;
            if ((local_10 & 1) == 0) {
              iVar4 = iVar4 + *(int *)(param_1 + 0xeb);
              iVar9 = iVar9 + uVar3;
            }
            else {
              iVar4 = iVar4 + -2;
              iVar9 = iVar9 + -2;
            }
            puVar6 = puVar6 + iVar4;
            local_1c = local_1c + iVar9;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
        break;
      case 1:
        iVar5 = *(int *)(param_1 + 0x6199);
        iVar4 = iVar5 / 2;
        puVar6 = (undefined2 *)
                 (*(int *)(param_1 + 0xe7) + 4 +
                 (*(int *)(param_1 + 0xeb) * (iVar4 + -1) + iVar4 * -2) * 2);
        iVar8 = *(int *)(param_1 + 0x619d);
        local_1c = (undefined2 *)(iVar2 + 2 + ((iVar4 + -1) * uVar3 + iVar4 * -2 + iVar5) * 2);
        local_14 = (*(int *)(param_1 + 0x61a1) - iVar4) + 1;
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (local_14 < 0) {
          local_14 = local_14 + iVar5;
        }
        if (iVar5 <= local_14) {
          local_14 = local_14 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        local_10 = 0;
        iVar8 = (iVar5 * local_20 + local_14) * 0x10 + *(int *)(param_1 + 0x29);
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            local_18 = 0;
            if (0 < iVar5) {
              do {
                if (**(char **)(iVar8 + 8) == _DAT_005e18f0) {
                  uVar1 = *local_1c;
                }
                else {
                  uVar1 = *(undefined2 *)(iVar8 + 0xc);
                }
                *puVar6 = uVar1;
                iVar5 = *(int *)(param_1 + 0x6199);
                iVar8 = iVar8 + 0x10;
                local_14 = local_14 + 1;
                if (iVar5 <= local_14) {
                  local_14 = local_14 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((local_18 & 1) == 0) {
                  puVar6 = puVar6 + 1;
                  local_1c = local_1c + 1;
                }
                else {
                  puVar6 = puVar6 + (1 - *(int *)(param_1 + 0xeb));
                  local_1c = local_1c + (1 - uVar3);
                }
                local_18 = local_18 + 1;
              } while ((int)local_18 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar4 = *(int *)(param_1 + 0x619d);
            if (iVar4 <= local_20) {
              local_20 = local_20 - iVar4;
              iVar8 = iVar8 + iVar4 * iVar5 * -0x10;
            }
            iVar4 = ((*(int *)(param_1 + 0xeb) + -1) * iVar5) / 2 - iVar5 / 2;
            iVar9 = (int)((uVar3 - 1) * iVar5) / 2 - iVar5 / 2;
            if ((local_10 & 1) == 0) {
              iVar4 = iVar4 + *(int *)(param_1 + 0xeb);
              iVar9 = iVar9 + uVar3;
            }
            else {
              iVar4 = iVar4 + 2;
              iVar9 = iVar9 + 2;
            }
            puVar6 = puVar6 + iVar4;
            local_1c = local_1c + iVar9;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
        break;
      case 2:
        iVar5 = *(int *)(param_1 + 0x6199);
        puVar6 = (undefined2 *)
                 (*(int *)(param_1 + 0xe7) + 2 + *(int *)(param_1 + 0xeb) * (iVar5 + -1) * 2);
        puVar7 = (undefined2 *)(iVar2 + ((iVar5 + -1) * uVar3 + iVar5) * 2);
        local_14 = *(int *)(param_1 + 0x61a1) - iVar5 / 2;
        local_20 = *(int *)(param_1 + 0x61a5) - *(int *)(param_1 + 0x619d) / 2;
        if (local_14 < 0) {
          local_14 = local_14 + iVar5;
        }
        if (iVar5 <= local_14) {
          local_14 = local_14 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + *(int *)(param_1 + 0x619d);
        }
        if (*(int *)(param_1 + 0x619d) <= local_20) {
          local_20 = local_20 - *(int *)(param_1 + 0x619d);
        }
        local_10 = 0;
        local_1c = (undefined2 *)((iVar5 * local_20 + local_14) * 0x10 + *(int *)(param_1 + 0x29));
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            local_18 = 0;
            if (0 < iVar5) {
              do {
                if (**(char **)((int)local_1c + 8) == _DAT_005e18f0) {
                  *puVar6 = *puVar7;
                }
                else {
                  *puVar6 = *(undefined2 *)((int)local_1c + 0xc);
                }
                iVar5 = *(int *)(param_1 + 0x6199);
                local_1c = (undefined2 *)((int)local_1c + 0x10);
                local_14 = local_14 + 1;
                if (iVar5 <= local_14) {
                  local_14 = local_14 - iVar5;
                  local_1c = (undefined2 *)((int)local_1c + iVar5 * -0x10);
                }
                if ((local_18 & 1) == 0) {
                  puVar6 = puVar6 + -1;
                  puVar7 = puVar7 + -1;
                }
                else {
                  puVar6 = puVar6 + (-1 - *(int *)(param_1 + 0xeb));
                  puVar7 = puVar7 + (-1 - uVar3);
                }
                local_18 = local_18 + 1;
              } while ((int)local_18 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            local_1c = (undefined2 *)((int)local_1c + iVar5 * 0x10);
            iVar8 = *(int *)(param_1 + 0x619d);
            if (iVar8 <= local_20) {
              local_20 = local_20 - iVar8;
              local_1c = (undefined2 *)((int)local_1c + iVar8 * iVar5 * -0x10);
            }
            iVar8 = ((*(int *)(param_1 + 0xeb) + 1) * iVar5) / 2 + iVar5 / 2;
            iVar4 = (int)((uVar3 + 1) * iVar5) / 2 + iVar5 / 2;
            if ((local_10 & 1) == 0) {
              iVar8 = iVar8 - *(int *)(param_1 + 0xeb);
              iVar4 = iVar4 - uVar3;
            }
            else {
              iVar8 = iVar8 + 2;
              iVar4 = iVar4 + 2;
            }
            puVar6 = puVar6 + iVar8;
            puVar7 = puVar7 + iVar4;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
        break;
      case 3:
        iVar5 = *(int *)(param_1 + 0x6199);
        iVar8 = *(int *)(param_1 + 0x619d);
        iVar4 = iVar5 / 2;
        puVar6 = (undefined2 *)
                 (*(int *)(param_1 + 0xe7) + -2 + (*(int *)(param_1 + 0xeb) + 2) * iVar4 * 2);
        local_1c = (undefined2 *)(iVar2 + -4 + ((uVar3 + 2) * iVar4 + iVar5) * 2);
        local_14 = (*(int *)(param_1 + 0x61a1) - iVar4) + 1;
        local_20 = *(int *)(param_1 + 0x61a5) - iVar8 / 2;
        if (local_14 < 0) {
          local_14 = local_14 + iVar5;
        }
        if (iVar5 <= local_14) {
          local_14 = local_14 - iVar5;
        }
        if (local_20 < 0) {
          local_20 = local_20 + iVar8;
        }
        if (iVar8 <= local_20) {
          local_20 = local_20 - iVar8;
        }
        iVar8 = (iVar5 * local_20 + local_14) * 0x10 + *(int *)(param_1 + 0x29);
        local_10 = 0;
        if (0 < *(int *)(param_1 + 0x619d)) {
          do {
            local_18 = 0;
            if (0 < iVar5) {
              do {
                if (**(char **)(iVar8 + 8) == _DAT_005e18f0) {
                  *puVar6 = *local_1c;
                }
                else {
                  *puVar6 = *(undefined2 *)(iVar8 + 0xc);
                }
                iVar5 = *(int *)(param_1 + 0x6199);
                iVar8 = iVar8 + 0x10;
                local_14 = local_14 + 1;
                if (iVar5 <= local_14) {
                  local_14 = local_14 - iVar5;
                  iVar8 = iVar8 + iVar5 * -0x10;
                }
                if ((local_18 & 1) == 0) {
                  puVar6 = puVar6 + -1;
                  local_1c = local_1c + -1;
                }
                else {
                  puVar6 = puVar6 + *(int *)(param_1 + 0xeb) + -1;
                  local_1c = local_1c + (uVar3 - 1);
                }
                local_18 = local_18 + 1;
              } while ((int)local_18 < iVar5);
            }
            iVar5 = *(int *)(param_1 + 0x6199);
            local_20 = local_20 + 1;
            iVar8 = iVar8 + iVar5 * 0x10;
            iVar4 = *(int *)(param_1 + 0x619d);
            if (iVar4 <= local_20) {
              local_20 = local_20 - iVar4;
              iVar8 = iVar8 + iVar4 * iVar5 * -0x10;
            }
            iVar4 = iVar5 / 2 - ((*(int *)(param_1 + 0xeb) + -1) * iVar5) / 2;
            iVar9 = iVar5 / 2 - (int)((uVar3 - 1) * iVar5) / 2;
            if ((local_10 & 1) == 0) {
              iVar4 = iVar4 - *(int *)(param_1 + 0xeb);
              iVar9 = iVar9 - uVar3;
            }
            else {
              iVar4 = iVar4 + -2;
              iVar9 = iVar9 + -2;
            }
            puVar6 = puVar6 + iVar4;
            local_1c = local_1c + iVar9;
            local_10 = local_10 + 1;
          } while ((int)local_10 < *(int *)(param_1 + 0x619d));
        }
      }
    }
    (**(code **)(**(int **)(param_1 + 0x61ad) + 0x80))(*(int **)(param_1 + 0x61ad),iVar2);
  }
  return;
}

