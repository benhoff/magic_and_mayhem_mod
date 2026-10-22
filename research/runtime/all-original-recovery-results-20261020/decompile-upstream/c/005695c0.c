
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_005695c0(int param_1)

{
  byte bVar1;
  bool bVar2;
  int iVar3;
  int *piVar4;
  byte *pbVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  byte *pbVar9;
  byte *pbVar10;
  bool bVar11;
  undefined4 uVar12;
  uint local_128;
  uint local_124;
  uint local_114;
  int local_110;
  int local_10c;
  int local_108;
  uint local_104 [2];
  int local_fc [63];
  
  piVar4 = local_fc;
  for (iVar7 = 0x3f; iVar7 != 0; iVar7 = iVar7 + -1) {
    *piVar4 = 0;
    piVar4 = piVar4 + 1;
  }
  iVar8 = 0;
  local_110 = 0;
  bVar2 = false;
  local_10c = 0;
  local_fc[5] = -1;
  local_fc[4] = -1;
  local_fc[3] = -1;
  local_fc[2] = -1;
  local_fc[1] = -1;
  local_fc[0] = -1;
  local_fc[8] = 0xffffffff;
  local_fc[7] = 0xffffffff;
  local_fc[6] = 0xffffffff;
  local_fc[0xc] = 7;
  local_fc[10] = 1;
  local_fc[0xb] = 0xffffffff;
  local_fc[9] = 0;
  local_128 = FUN_004132e0(0);
  local_124 = FUN_0040e650(local_128);
  iVar7 = *(int *)(param_1 + 0x2cc);
  if (iVar7 == 1) {
    iVar7 = FUN_0056ba90(param_1 + 0x2f4,&local_114,local_104,&local_108);
    if (iVar7 != 0) {
      local_124 = local_114;
      if ((int)local_114 < 0) {
        do {
          local_124 = local_124 + DAT_006c5494;
        } while ((int)local_124 < 0);
      }
      else {
        for (; DAT_006c5494 <= local_124; local_124 = local_124 - DAT_006c5494) {
        }
      }
      local_128 = local_104[0];
      iVar8 = local_108;
      if ((int)local_104[0] < 0) {
        do {
          local_128 = local_128 + DAT_006c5498;
        } while ((int)local_128 < 0);
        bVar2 = true;
      }
      else {
        for (; DAT_006c5498 <= local_128; local_128 = local_128 - DAT_006c5498) {
        }
        bVar2 = true;
      }
    }
  }
  else if (iVar7 == 2) {
    iVar7 = 0;
    if (0 < DAT_006f37fc) {
      pbVar9 = (byte *)(DAT_006f37f8 + 1);
      piVar4 = DAT_006f37f8;
      do {
        if (*piVar4 != 0) {
          pbVar10 = (byte *)(param_1 + 0x2d0);
          pbVar5 = pbVar9;
          do {
            bVar1 = *pbVar5;
            bVar11 = bVar1 < *pbVar10;
            if (bVar1 != *pbVar10) {
LAB_00569782:
              iVar8 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
              goto LAB_00569787;
            }
            if (bVar1 == 0) break;
            bVar1 = pbVar5[1];
            bVar11 = bVar1 < pbVar10[1];
            if (bVar1 != pbVar10[1]) goto LAB_00569782;
            pbVar5 = pbVar5 + 2;
            pbVar10 = pbVar10 + 2;
          } while (bVar1 != 0);
          iVar8 = 0;
LAB_00569787:
          if (iVar8 == 0) {
            if ((iVar7 < DAT_006f37fc) && (piVar4 != (int *)0x0)) {
              uVar6 = piVar4[10];
              if (uVar6 != 0xffffffff) {
                bVar2 = true;
                if (uVar6 < DAT_006def5c) {
                  iVar7 = uVar6 * 0xe4b + DAT_006def58;
                }
                else {
                  iVar7 = 0;
                }
                iVar8 = *(int *)(param_1 + 0x2f0);
                if (iVar8 == 0) {
                  local_110 = 1;
                  if (*(int *)(iVar7 + 0x618) == 0) {
                    local_124 = FUN_0040e650(*(undefined4 *)(iVar7 + 8));
                    local_128 = FUN_004132e0(*(undefined4 *)(iVar7 + 0xc));
                    iVar8 = *(int *)(iVar7 + 0x10);
                  }
                  else {
                    iVar3 = *(int *)(iVar7 + 0x977) * 0x1c;
                    iVar8 = iVar3 + 0x97f + iVar7;
                    local_124 = FUN_0040e650(*(undefined4 *)(iVar3 + 0x97f + iVar7));
                    local_128 = FUN_004132e0(*(undefined4 *)(iVar8 + 4));
                    iVar8 = *(int *)(iVar8 + 8);
                  }
                  if (piVar4[9] == 2) {
                    pbVar9 = (byte *)(piVar4 + 1);
                    pbVar10 = (byte *)(param_1 + 0x88);
                    goto LAB_00569949;
                  }
                }
                else {
                  uVar6 = FUN_0054e200();
                  if (DAT_006e1f30 != 0) {
                    FUN_00485b10(s_c__Projects_Chaos_script_cpp_005ed8f4,0x1587);
                  }
                  local_124 = FUN_0040e650((*(int *)(iVar7 + 8) + *(int *)(param_1 + 0x2f0) / 2) -
                                           (int)((longlong)(ulonglong)(uVar6 & 0x7fffffff) %
                                                (longlong)iVar8));
                  iVar8 = *(int *)(param_1 + 0x2f0);
                  uVar6 = FUN_0054e200();
                  if (DAT_006e1f30 != 0) {
                    FUN_00485b10(s_c__Projects_Chaos_script_cpp_005ed8f4,0x1589);
                  }
                  local_128 = FUN_004132e0((*(int *)(iVar7 + 0xc) + *(int *)(param_1 + 0x2f0) / 2) -
                                           (int)((longlong)(ulonglong)(uVar6 & 0x7fffffff) %
                                                (longlong)iVar8));
                  local_fc[9] = 1;
                  iVar8 = *(int *)(iVar7 + 0x10);
                }
                break;
              }
              if ((piVar4[9] == 4) || (piVar4[9] == 2)) {
                bVar2 = true;
                iVar7 = *(int *)(param_1 + 0x2f0);
                uVar6 = FUN_0054e200();
                if (DAT_006e1f30 != 0) {
                  FUN_00485b10(s_c__Projects_Chaos_script_cpp_005ed8f4,0x15b4);
                }
                iVar8 = *(int *)(param_1 + 0x2f0);
                local_124 = (iVar8 / 2 + piVar4[0xb]) -
                            (int)((longlong)(ulonglong)(uVar6 & 0x7fffffff) % (longlong)iVar7);
                if ((int)local_124 < 0) {
                  do {
                    local_124 = local_124 + DAT_006c5494;
                  } while ((int)local_124 < 0);
                }
                else {
                  for (; DAT_006c5494 <= local_124; local_124 = local_124 - DAT_006c5494) {
                  }
                }
                uVar6 = FUN_0054e200();
                if (DAT_006e1f30 != 0) {
                  FUN_00485b10(s_c__Projects_Chaos_script_cpp_005ed8f4,0x15b6);
                }
                local_128 = (piVar4[0xc] + *(int *)(param_1 + 0x2f0) / 2) -
                            (int)((longlong)(ulonglong)(uVar6 & 0x7fffffff) % (longlong)iVar8);
                if ((int)local_128 < 0) {
                  do {
                    local_128 = local_128 + DAT_006c5498;
                  } while ((int)local_128 < 0);
                  local_fc[9] = 1;
                  iVar8 = piVar4[0xd];
                }
                else {
                  for (; DAT_006c5498 <= local_128; local_128 = local_128 - DAT_006c5498) {
                  }
                  local_fc[9] = 1;
                  iVar8 = piVar4[0xd];
                }
                break;
              }
            }
            iVar8 = 0;
            break;
          }
        }
        iVar8 = 0;
        iVar7 = iVar7 + 1;
        piVar4 = piVar4 + 0x10;
        pbVar9 = pbVar9 + 0x40;
      } while (iVar7 < DAT_006f37fc);
    }
  }
  else if (iVar7 == 3) {
    iVar7 = *(int *)(param_1 + 0x314);
    _DAT_006b125c = DAT_006b1248;
    _DAT_006b1258 = 0;
    iVar3 = FUN_00544d60();
    while (iVar3 != 0) {
      if (iVar7 == *(int *)(iVar3 + 0x2a)) {
        local_114 = *(uint *)(iVar3 + 8);
        local_104[0] = *(uint *)(iVar3 + 0xc);
        local_108 = *(int *)(iVar3 + 0x10);
        bVar2 = true;
        local_124 = local_114;
        if ((int)local_114 < 0) {
          do {
            local_124 = local_124 + DAT_006c5494;
          } while ((int)local_124 < 0);
        }
        else {
          for (; DAT_006c5494 <= local_124; local_124 = local_124 - DAT_006c5494) {
          }
        }
        local_128 = local_104[0];
        iVar8 = local_108;
        if ((int)local_104[0] < 0) {
          do {
            local_128 = local_128 + DAT_006c5498;
          } while ((int)local_128 < 0);
          local_fc[9] = 1;
        }
        else {
          for (; DAT_006c5498 <= local_128; local_128 = local_128 - DAT_006c5498) {
          }
          local_fc[9] = 1;
        }
        break;
      }
      iVar3 = FUN_00544d60();
    }
  }
LAB_00569b76:
  iVar7 = FUN_0049cdb0();
  if ((iVar7 == 0) && (*(int *)((-(uint)(DAT_0068949c != 0) & DAT_00689498) + 4) != 0)) {
    FUN_004943f0();
  }
  if (bVar2) {
    local_fc[0] = local_124 * 0x20 + 0x10;
    local_fc[0xb] = *(undefined4 *)(param_1 + 0x318);
    local_fc[1] = local_128 * 0x20 + 0x10;
    local_fc[6] = 0xffffffff;
    local_fc[7] = 0xffffffff;
    local_fc[2] = iVar8 << 4;
    local_fc[10] = 4;
    if ((local_110 == 0) || (local_10c != 0)) {
      uVar12 = 0x3e;
    }
    else {
      uVar12 = 0x41;
    }
    local_fc[3] = local_fc[0];
    local_fc[4] = local_fc[1];
    local_fc[5] = local_fc[2];
    FUN_00493f20(uVar12,local_fc);
    if ((*(int *)(param_1 + 0x2c) != -1) && (*(int *)(param_1 + 0x30) == 0)) {
      FUN_00571310(*(int *)(param_1 + 0x2c),local_124,local_128,iVar8,0,0,0);
    }
  }
  FUN_00564d60(param_1);
  return;
  while( true ) {
    bVar1 = pbVar10[1];
    bVar11 = bVar1 < pbVar9[1];
    if (bVar1 != pbVar9[1]) goto LAB_0056996d;
    pbVar10 = pbVar10 + 2;
    pbVar9 = pbVar9 + 2;
    if (bVar1 == 0) break;
LAB_00569949:
    bVar1 = *pbVar10;
    bVar11 = bVar1 < *pbVar9;
    if (bVar1 != *pbVar9) {
LAB_0056996d:
      iVar7 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      goto LAB_00569972;
    }
    if (bVar1 == 0) break;
  }
  iVar7 = 0;
LAB_00569972:
  if (((iVar7 == 0) && (*(int *)(param_1 + 0xa8) == 2)) && (uVar6 = piVar4[10], uVar6 != 0xffffffff)
     ) {
    if (uVar6 < DAT_006def5c) {
      iVar7 = uVar6 * 0xe4b + DAT_006def58;
    }
    else {
      iVar7 = 0;
    }
    piVar4[9] = 4;
    FUN_0050d060(0xffffffff,3,0);
    *(undefined4 *)(iVar7 + 0x8cd) = 0x37;
    if (*(int *)(iVar7 + 0x108) != 0) {
      local_10c = 1;
      local_fc[9] = 1;
    }
  }
  goto LAB_00569b76;
}

