
void __fastcall FUN_004908a0(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined4 *puVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  int iVar9;
  uint uVar10;
  uint uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  
  uVar7 = *(uint *)(param_1 + 0x60);
  if (uVar7 < DAT_006def5c) {
    iVar9 = uVar7 * 0xe4b + DAT_006def58;
  }
  else {
    iVar9 = 0;
  }
  uVar3 = uVar7 + (uint)(*(int *)(&DAT_006b13e9 + *(int *)(param_1 + 0x4c) * 0x2d1) * 0x7d) / 100;
  if (0x7c < uVar3) {
    uVar3 = 0x7d;
  }
  do {
    if ((int)uVar3 <= (int)uVar7) {
      uVar7 = *(int *)(param_1 + 0x60) + *(int *)(&DAT_006b13e9 + *(int *)(param_1 + 0x4c) * 0x2d1);
      *(uint *)(param_1 + 0x60) = uVar7;
      if (0x7c < uVar7) {
        *(undefined4 *)(param_1 + 0x60) = 0;
      }
      return;
    }
    if ((*(int *)(iVar9 + 4) != 0) && (*(int *)(iVar9 + 0xe4) != 0)) {
      iVar8 = *(int *)(param_1 + 0x4c) * 0x2d0;
      FUN_0040eeb0(*(undefined4 *)(iVar9 + 0x10));
      uVar16 = extraout_ECX;
      FUN_0040e8a0(*(undefined4 *)(iVar9 + 0xc));
      uVar15 = extraout_ECX_00;
      FUN_0040e290(*(undefined4 *)(iVar9 + 8));
      uVar14 = extraout_ECX_01;
      FUN_0040eeb0(*(undefined4 *)(param_1 + 0x10));
      uVar13 = extraout_ECX_02;
      FUN_0040e8a0(*(undefined4 *)(param_1 + 0xc));
      uVar12 = extraout_ECX_03;
      FUN_0040e290(*(undefined4 *)(param_1 + 8));
      iVar4 = FUN_004e1380(uVar12,uVar13,uVar14,uVar15,uVar16,iVar8);
      iVar2 = DAT_006c1590;
      iVar1 = DAT_006c12bf;
      iVar8 = DAT_006bc985;
      if (iVar4 != 0) {
        iVar4 = *(int *)(param_1 + 0x4c);
        if (iVar4 == 0x41) {
          iVar1 = *(int *)(iVar9 + 0x88);
          if (((DAT_006bcae6 != 0) || (DAT_006bcaea != 0)) || (DAT_006bcaee != 0)) {
            uVar6 = FUN_0054e200();
            if (DAT_006e1f30 != 0) {
              FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0xf2a);
            }
            if ((uint)(iVar8 * 100) / (uint)(iVar8 + iVar1) <= (uVar6 & 0x7fffffff) % 100)
            goto LAB_00490e46;
          }
          if (*(int *)(iVar9 + 0x114) == 0) {
            FUN_00515f70(*(undefined4 *)(param_1 + 0x212));
LAB_00490dab:
            if (((-1 < DAT_00644520) && (DAT_00644520 < 8)) &&
               (*(int *)(iVar9 + 0x24 + DAT_00644520 * 4) != 0)) {
              iVar8 = FUN_005908e0();
              if (((iVar8 * 0x93a != -0x65b250) &&
                  (iVar8 = *(int *)(&DAT_0065bb86 + iVar8 * 0x93a), iVar8 != 0)) &&
                 ((iVar1 = *(int *)(param_1 + 0x4c), -1 < iVar1 &&
                  ((iVar1 < 0x7c && (*(char *)(iVar1 + 0x26c + iVar8) == '\0')))))) {
                *(undefined1 *)(iVar1 + 0x26c + iVar8) = 1;
                if (*(int *)(iVar8 + 0x578) < 0x14) {
                  *(undefined4 *)(iVar8 + 0x4d8 + *(int *)(iVar8 + 0x578) * 8) = 5;
                  *(int *)(iVar8 + 0x4dc + *(int *)(iVar8 + 0x578) * 8) = iVar1;
                  *(int *)(iVar8 + 0x578) = *(int *)(iVar8 + 0x578) + 1;
                }
              }
            }
          }
        }
        else if (iVar4 == 0x5b) {
          if ((*(int *)(iVar9 + 0xde3) == 0) && (*(int *)(iVar9 + 0x798) == 0)) {
            if (*(int *)(param_1 + 0x212) == -1) {
              puVar5 = (undefined4 *)0x0;
            }
            else {
              puVar5 = &DAT_00643a80 + *(int *)(param_1 + 0x212) * 0x55;
            }
            if (*(char *)((int)puVar5 + *(int *)(iVar9 + 0x174) + 0x18) == '\0') {
              iVar8 = *(int *)(&DAT_006a6545 + *(int *)(iVar9 + 0xa8) * 0x5c9);
              if (((DAT_006c1420 != 0) || (DAT_006c1424 != 0)) || (DAT_006c1428 != 0)) {
                uVar6 = FUN_0054e200();
                if (DAT_006e1f30 != 0) {
                  FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0xf2a);
                }
                if ((uint)(iVar1 * 100) / (uint)(iVar1 + iVar8) <= (uVar6 & 0x7fffffff) % 100)
                goto LAB_00490e46;
              }
              uVar6 = *(uint *)(iVar9 + 8);
              uVar10 = *(int *)(iVar9 + 0xc) - *(int *)(param_1 + 0xc);
              uVar11 = uVar6 - *(int *)(param_1 + 8);
              if ((int)DAT_006c5494 / 2 < (int)uVar11) {
                uVar11 = uVar11 - DAT_006c5494;
              }
              if ((int)uVar11 < -((int)DAT_006c5494 / 2)) {
                uVar11 = uVar11 + DAT_006c5494;
              }
              if ((int)DAT_006c5498 / 2 < (int)uVar10) {
                uVar10 = uVar10 - DAT_006c5498;
              }
              if ((int)uVar10 < -((int)DAT_006c5498 / 2)) {
                uVar10 = uVar10 + DAT_006c5498;
              }
              if ((uVar11 & 0x80000000) == 0) {
                if (uVar11 == 0) {
                  if ((int)uVar6 < 0) {
                    do {
                      uVar6 = uVar6 + DAT_006c5494;
                    } while ((int)uVar6 < 0);
                  }
                  else {
                    for (; DAT_006c5494 <= uVar6; uVar6 = uVar6 - DAT_006c5494) {
                    }
                  }
                }
                else {
                  uVar6 = (*(uint *)(&DAT_006b13c9 + *(int *)(param_1 + 0x4c) * 0x2d1) >> 1) + uVar6
                  ;
                  if ((int)uVar6 < 0) {
                    do {
                      uVar6 = uVar6 + DAT_006c5494;
                    } while ((int)uVar6 < 0);
                  }
                  else {
                    for (; DAT_006c5494 <= uVar6; uVar6 = uVar6 - DAT_006c5494) {
                    }
                  }
                }
              }
              else {
                uVar6 = FUN_0040e650(uVar6 - (*(uint *)(&DAT_006b13c9 +
                                                       *(int *)(param_1 + 0x4c) * 0x2d1) >> 1));
              }
              if ((uVar10 & 0x80000000) == 0) {
                if (uVar10 == 0) {
                  uVar10 = *(uint *)(iVar9 + 0xc);
                  if ((int)uVar10 < 0) {
                    do {
                      uVar10 = uVar10 + DAT_006c5498;
                    } while ((int)uVar10 < 0);
                  }
                  else {
                    for (; DAT_006c5498 <= uVar10; uVar10 = uVar10 - DAT_006c5498) {
                    }
                  }
                }
                else {
                  uVar10 = (*(uint *)(&DAT_006b13c9 + *(int *)(param_1 + 0x4c) * 0x2d1) >> 1) +
                           *(int *)(iVar9 + 0xc);
                  if ((int)uVar10 < 0) {
                    do {
                      uVar10 = uVar10 + DAT_006c5498;
                    } while ((int)uVar10 < 0);
                  }
                  else {
                    for (; DAT_006c5498 <= uVar10; uVar10 = uVar10 - DAT_006c5498) {
                    }
                  }
                }
              }
              else {
                uVar10 = *(int *)(iVar9 + 0xc) -
                         (*(uint *)(&DAT_006b13c9 + *(int *)(param_1 + 0x4c) * 0x2d1) >> 1);
                if ((int)uVar10 < 0) {
                  do {
                    uVar10 = uVar10 + DAT_006c5498;
                  } while ((int)uVar10 < 0);
                }
                else {
                  for (; DAT_006c5498 <= uVar10; uVar10 = uVar10 - DAT_006c5498) {
                  }
                }
              }
              FUN_00522970(*(uint *)(&DAT_006b13c9 + *(int *)(param_1 + 0x4c) * 0x2d1) >> 1,uVar6,
                           uVar10,*(undefined4 *)(iVar9 + 0x10));
              goto LAB_00490dab;
            }
          }
        }
        else if (iVar4 == 0x5c) {
          if (*(int *)(param_1 + 0x212) == -1) {
            puVar5 = (undefined4 *)0x0;
          }
          else {
            puVar5 = &DAT_00643a80 + *(int *)(param_1 + 0x212) * 0x55;
          }
          if (*(char *)((int)puVar5 + *(int *)(iVar9 + 0x174) + 0x18) == '\0') {
            iVar8 = *(int *)(iVar9 + 0x88);
            if (((DAT_006c16f1 != 0) || (DAT_006c16f5 != 0)) || (DAT_006c16f9 != 0)) {
              uVar6 = FUN_0054e200();
              if (DAT_006e1f30 != 0) {
                FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0xf2a);
              }
              if ((uint)(iVar2 * 100) / (uint)(iVar2 + iVar8) <= (uVar6 & 0x7fffffff) % 100)
              goto LAB_00490e46;
            }
            FUN_005229d0(*(undefined4 *)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),
                         *(undefined4 *)(param_1 + 0x10));
            goto LAB_00490dab;
          }
        }
      }
    }
LAB_00490e46:
    uVar7 = uVar7 + 1;
    iVar9 = iVar9 + 0xe4b;
  } while( true );
}

