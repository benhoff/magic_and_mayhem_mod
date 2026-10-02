/* Ghidra inferred pseudocode; not original or buildable source.
 * Entry VA: 004ebae0; see manifest.json for executable hash. */

/* WARNING: Removing unreachable block (ram,0x004ec6dd) */
/* WARNING: Removing unreachable block (ram,0x004ec6ea) */
/* WARNING: Removing unreachable block (ram,0x004ec6ee) */
/* WARNING: Removing unreachable block (ram,0x004ec6f7) */
/* WARNING: Removing unreachable block (ram,0x004ec6f9) */
/* WARNING: Removing unreachable block (ram,0x004ec700) */
/* WARNING: Removing unreachable block (ram,0x004ec70a) */
/* WARNING: Removing unreachable block (ram,0x004ec714) */
/* WARNING: Removing unreachable block (ram,0x004ec725) */
/* WARNING: Removing unreachable block (ram,0x004ec727) */
/* WARNING: Removing unreachable block (ram,0x004ec729) */
/* WARNING: Removing unreachable block (ram,0x004ec732) */
/* WARNING: Removing unreachable block (ram,0x004ec742) */
/* WARNING: Removing unreachable block (ram,0x004ec744) */
/* WARNING: Removing unreachable block (ram,0x004ec65c) */
/* WARNING: Removing unreachable block (ram,0x004ec65f) */
/* WARNING: Removing unreachable block (ram,0x004ec663) */
/* WARNING: Removing unreachable block (ram,0x004ec66c) */
/* WARNING: Removing unreachable block (ram,0x004ec66e) */
/* WARNING: Removing unreachable block (ram,0x004ec675) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004ebae0(uint *param_1,int param_2,int *param_3,int param_4,int *param_5)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  int extraout_ECX_02;
  int extraout_ECX_03;
  int extraout_ECX_04;
  int extraout_ECX_05;
  int extraout_ECX_06;
  int extraout_ECX_07;
  int extraout_ECX_08;
  undefined4 extraout_ECX_09;
  undefined4 extraout_ECX_10;
  undefined4 extraout_ECX_11;
  undefined4 extraout_ECX_12;
  undefined4 extraout_ECX_13;
  undefined4 extraout_ECX_14;
  undefined4 extraout_ECX_15;
  undefined4 extraout_ECX_16;
  undefined4 extraout_ECX_17;
  undefined4 extraout_ECX_18;
  undefined4 extraout_ECX_19;
  undefined4 extraout_ECX_20;
  undefined4 extraout_ECX_21;
  undefined4 extraout_ECX_22;
  undefined4 extraout_ECX_23;
  undefined4 extraout_ECX_24;
  undefined4 extraout_ECX_25;
  uint uVar8;
  uint *puVar9;
  int iVar10;
  uint *puVar11;
  int iVar12;
  uint *puVar13;
  uint *puVar14;
  uint *puVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  int *piVar18;
  undefined4 uVar19;
  int *piVar20;
  int *piVar21;
  undefined1 *puVar22;
  int local_8c;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  int local_74;
  uint *local_70;
  int local_6c;
  int local_68;
  int local_64;
  uint local_60;
  short *local_5c;
  int local_58;
  int local_54;
  uint *local_50;
  uint local_4c;
  float local_48;
  int local_44;
  undefined4 local_40;
  int local_3c;
  uint local_38;
  int local_34;
  int local_30;
  int local_2c;
  int *local_28;
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  uint local_14;
  undefined4 uStack_10;
  int local_c;
  undefined4 local_8;
  undefined1 local_4 [4];
  
  if (0 < *param_5) {
    iVar12 = ((int)param_1 - DAT_006c54dc) / 0xc;
    local_84 = 0;
    local_80 = 0;
    local_8c = 0;
    local_88 = 0;
    local_78 = iVar12 / DAT_006c54a0;
    local_58 = 5;
    local_4c = 0;
    local_48 = 0.0;
    local_44 = 0;
    local_40 = 0;
    iVar12 = iVar12 - (&DAT_006cb8c2)[local_78];
    local_70 = param_1;
    local_80 = FUN_004132e0(iVar12 / DAT_006c5494);
    local_84 = FUN_0040e650(iVar12 - (&DAT_006cb942)[local_80]);
    if (*(int *)((int)param_3 + 0x3a) == 0) {
      if (*param_3 == 0) {
        local_3c = (int)&DAT_006a5f80 + *(int *)(param_3[1] + 0xa8) * 0x5c9;
      }
      else {
        piVar18 = param_3 + 6;
        piVar21 = piVar18;
        FUN_0040eec0(&local_78);
        uVar16 = extraout_ECX_00;
        FUN_0040e8d0(&local_80);
        uVar7 = extraout_ECX_01;
        FUN_0040e2c0(&local_84);
        local_3c = FUN_004f4330(uVar7,uVar16,piVar18,piVar21);
      }
      if (*param_3 != 0) {
        local_7c = param_3[3];
        uVar3 = *(int *)((int)param_3 + 0x2e) - local_7c;
        uVar8 = (int)uVar3 >> 0x1f;
        iVar2 = DAT_006c5494 / 2;
        iVar10 = (uVar3 ^ uVar8) - uVar8;
        iVar1 = DAT_006c5498 / 2;
        iVar12 = DAT_006c5494;
        if (iVar2 < iVar10) {
          iVar12 = DAT_006c5494 - iVar10;
          iVar10 = iVar12;
        }
        uVar3 = *(int *)((int)param_3 + 0x32) - param_3[4];
        uVar8 = (int)uVar3 >> 0x1f;
        local_74 = (uVar3 ^ uVar8) - uVar8;
        if (iVar1 < local_74) {
          iVar12 = DAT_006c5498 - local_74;
          local_74 = iVar12;
        }
        uVar3 = *(int *)((int)param_3 + 0x36) - param_3[5];
        uVar8 = (int)uVar3 >> 0x1f;
        local_64 = (uVar3 ^ uVar8) - uVar8;
        local_2c = 0;
        local_1c = iVar2;
        local_18 = iVar1;
        FUN_0040e2c0(&local_84);
        uVar3 = FUN_00442090(iVar12);
        iVar4 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
        iVar12 = extraout_ECX_02;
        if (iVar2 < iVar4) {
          iVar4 = DAT_006c5494 - iVar4;
          iVar12 = iVar4;
        }
        if (iVar4 <= iVar10) {
          local_7c = *(int *)((int)param_3 + 0x2e);
          FUN_0040e2c0(&local_84);
          uVar3 = FUN_00442090(iVar12);
          iVar4 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
          iVar12 = extraout_ECX_03;
          if (iVar2 < iVar4) {
            iVar4 = DAT_006c5494 - iVar4;
            iVar12 = iVar4;
          }
          if (iVar4 <= iVar10) {
            local_2c = 1;
          }
        }
        local_30 = 0;
        FUN_0040e8d0(&local_80);
        uVar3 = FUN_004420a0(iVar12);
        iVar2 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
        iVar12 = extraout_ECX_04;
        if (iVar1 < iVar2) {
          iVar2 = DAT_006c5498 - iVar2;
          iVar12 = iVar2;
        }
        if (iVar2 <= local_74) {
          FUN_0040e8d0(&local_80);
          uVar3 = FUN_004420a0(iVar12);
          iVar12 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
          if (iVar1 < iVar12) {
            iVar12 = DAT_006c5498 - iVar12;
          }
          if (iVar12 <= local_74) {
            local_30 = 1;
          }
        }
        local_20 = 0;
        uVar3 = local_78 - param_3[5] >> 0x1f;
        if (((int)((local_78 - param_3[5] ^ uVar3) - uVar3) <= local_64) &&
           (uVar3 = local_78 - *(int *)((int)param_3 + 0x36), uVar8 = (int)uVar3 >> 0x1f,
           (int)((uVar3 ^ uVar8) - uVar8) <= local_64)) {
          local_20 = 1;
        }
        local_24 = iVar10 + 1;
        local_74 = local_74 + 1;
        local_64 = local_64 + 1;
      }
      local_28 = &DAT_005e178c;
      local_7c = 0;
      do {
        piVar18 = local_28;
        piVar21 = local_28 + 1;
        local_8c = FUN_0040e650(*local_28 + local_84);
        local_88 = FUN_004132e0(*piVar21 + local_80);
        iVar12 = piVar18[2] + local_78;
        local_28 = piVar18 + 3;
        if ((iVar12 < 0) || (DAT_006c549c <= iVar12)) goto LAB_004ec751;
        if (*param_3 == 0) {
          iVar10 = param_3[2];
          if ((param_3[2] == 0) &&
             ((((iVar10 = local_8c, local_8c != *(int *)((int)param_3 + 0x2e) ||
                (local_88 != *(int *)((int)param_3 + 0x32))) ||
               (iVar12 != *(int *)((int)param_3 + 0x36))) ||
              ((*(int *)(param_3[1] + 0xd03) != 0 ||
               ((local_84 != local_8c && (local_80 != local_88)))))))) {
            uVar7 = 0;
          }
          else {
            uVar7 = 1;
          }
          piVar18 = &local_6c;
          FUN_0040eeb0(iVar12);
          uVar19 = extraout_ECX_14;
          FUN_0040e8d0(&local_88);
          uVar17 = extraout_ECX_15;
          FUN_0040e2c0(&local_8c);
          uVar6 = extraout_ECX_16;
          FUN_0040eec0(&local_78);
          uVar5 = extraout_ECX_17;
          FUN_0040e8d0(&local_80);
          uVar16 = extraout_ECX_18;
          FUN_0040e2c0(&local_84);
          iVar10 = FUN_00514360(uVar16,uVar5,uVar6,uVar17,uVar19,iVar10,piVar18,uVar7);
LAB_004ec19f:
          if (iVar10 != 0) {
            local_5c = (short *)(DAT_006c54dc +
                                ((&DAT_006cb942)[local_88] + (&DAT_006cb8c2)[iVar12] + local_8c) *
                                0xc);
            iVar1 = iVar12 - local_78;
            iVar10 = local_78;
            FUN_0040e8d0(&local_80);
            FUN_0040eb70(iVar10);
            uVar7 = extraout_ECX_19;
            FUN_0040e2c0(&local_84);
            FUN_0040e4e0(uVar7);
            local_68 = FUN_004eac10(iVar1);
            if (*param_3 == 0) {
              if ((local_8c == local_84) && (local_88 == local_80)) {
                local_34 = *(int *)(param_4 + 4);
                local_70 = (uint *)(iVar12 - local_78);
              }
              else {
                iVar10 = FUN_0040eb70(local_80);
                uVar7 = extraout_ECX_20;
                FUN_0040e2c0(&local_84);
                iVar1 = FUN_0040e4e0(uVar7);
                local_70 = (uint *)0x0;
                local_34 = *(int *)(&DAT_005e41c4 + (iVar10 + iVar1 + iVar10 * 2) * 4);
              }
              if (local_70 == *(uint **)(param_4 + 8)) {
                uVar3 = (*(int *)(param_4 + 4) - local_34) + 8U & 0x80000007;
                if ((int)uVar3 < 0) {
                  uVar3 = (uVar3 - 1 | 0xfffffff8) + 1;
                }
                if (4 < (int)uVar3) {
                  uVar3 = uVar3 - 8;
                }
                if (1 < (int)((uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f)))
                goto LAB_004ec2f1;
                local_38 = *(uint *)(param_4 + 0xc);
              }
              else {
LAB_004ec2f1:
                local_38 = 0;
              }
              puVar22 = local_4;
              puVar11 = &local_38;
              iVar10 = iVar12 - local_78;
              iVar2 = iVar10;
              FUN_0040e8d0(&local_80);
              uVar5 = FUN_0040eb70(iVar10);
              uVar7 = extraout_ECX_21;
              FUN_0040e2c0(&local_84);
              uVar6 = FUN_0040e4e0(uVar7);
              iVar10 = local_6c;
              iVar1 = local_6c;
              FUN_0040eec0(&local_78);
              uVar16 = extraout_ECX_22;
              FUN_0040e8d0(&local_80);
              uVar7 = extraout_ECX_23;
              FUN_0040e2c0(&local_84);
              uVar7 = FUN_004eb030(uVar7,uVar16,iVar10);
              FUN_005205b0(uVar7,iVar1,uVar6,uVar5,iVar2,puVar11,puVar22);
              iVar10 = iVar12 - local_78;
              uVar7 = extraout_ECX_24;
              FUN_0040e8d0(&local_80);
              FUN_0040eb70(uVar7);
              uVar7 = extraout_ECX_25;
              FUN_0040e2c0(&local_84);
              FUN_0040e4e0(uVar7);
              iVar10 = FUN_004eac10(iVar10);
              local_60 = (uint)(iVar10 * *(int *)(local_3c + 0x589) * 4) / local_38;
              if ((local_6c == 2) || (local_6c == 3)) {
                local_60 = local_60 * 3;
              }
              else if (local_6c != 1) {
                local_60 = local_60 * 2;
              }
              if ((*(int *)(&DAT_006a652d + *(int *)(param_3[1] + 0xa8) * 0x5c9) != 0) &&
                 (((*(byte *)(DAT_006c54dc + 10 +
                             ((&DAT_006cb942)[local_88] + (&DAT_006cb8c2)[iVar12] + local_8c) * 0xc)
                   & 0x10) != 0 ||
                  ((1 < iVar12 &&
                   ((*(byte *)(DAT_006c54dc + 10 +
                              ((&DAT_006cb8be)[iVar12] + (&DAT_006cb942)[local_88] + local_8c) * 0xc
                              ) & 0x10) != 0)))))) {
                local_60 = local_60 * 3;
              }
              local_58 = local_6c;
              local_54 = local_34;
              local_4c = local_38;
              local_50 = local_70;
              local_14 = local_68 * 0x900;
              uStack_10 = 0;
              local_c = local_38 * 4;
              local_8 = 0;
              local_48 = (float)(uint)(local_68 * 0x900) / (float)(int)(local_38 * 4) +
                         *(float *)(param_4 + 0x10);
            }
            else {
              local_60 = local_68 * 8;
              local_58 = local_6c;
              local_54 = 0;
              local_50 = (uint *)0x0;
              local_4c = 0;
              local_48 = 0.0;
            }
            puVar11 = *(uint **)(param_2 + 8);
            iVar10 = *(int *)(param_2 + 0xc) - (int)puVar11;
            iVar12 = iVar10 >> 0x1f;
            local_58 = local_6c;
            if (iVar10 / 0x24 + iVar12 == iVar12) {
              iVar12 = *(int *)(param_2 + 4);
              if ((iVar12 == 0) || (uVar3 = ((int)puVar11 - iVar12) / 0x24, uVar3 < 2)) {
                uVar3 = 1;
              }
              if (iVar12 == 0) {
                local_68 = 0;
              }
              else {
                local_68 = ((int)puVar11 - iVar12) / 0x24;
              }
              local_68 = local_68 + uVar3;
              iVar12 = local_68;
              if (local_68 < 0) {
                iVar12 = 0;
              }
              local_70 = (uint *)FUN_00597850(iVar12 * 0x24);
              puVar14 = local_70;
              for (puVar9 = *(uint **)(param_2 + 4); puVar9 != puVar11; puVar9 = puVar9 + 9) {
                if (puVar14 != (uint *)0x0) {
                  puVar13 = puVar9;
                  puVar15 = puVar14;
                  for (iVar12 = 9; iVar12 != 0; iVar12 = iVar12 + -1) {
                    *puVar15 = *puVar13;
                    puVar13 = puVar13 + 1;
                    puVar15 = puVar15 + 1;
                  }
                }
                puVar14 = puVar14 + 9;
              }
              if (puVar14 != (uint *)0x0) {
                puVar9 = &local_60;
                puVar13 = puVar14;
                for (iVar12 = 9; iVar12 != 0; iVar12 = iVar12 + -1) {
                  *puVar13 = *puVar9;
                  puVar9 = puVar9 + 1;
                  puVar13 = puVar13 + 1;
                }
              }
              puVar13 = *(uint **)(param_2 + 8);
              puVar9 = puVar14 + 9;
              if (puVar11 != puVar13) {
                puVar11 = (uint *)((int)puVar9 + (-0x24 - (int)puVar14) + (int)puVar11);
                do {
                  if (puVar9 != (uint *)0x0) {
                    puVar14 = puVar11;
                    puVar15 = puVar9;
                    for (iVar12 = 9; iVar12 != 0; iVar12 = iVar12 + -1) {
                      *puVar15 = *puVar14;
                      puVar14 = puVar14 + 1;
                      puVar15 = puVar15 + 1;
                    }
                  }
                  puVar11 = puVar11 + 9;
                  puVar9 = puVar9 + 9;
                } while (puVar11 != puVar13);
              }
              local_34 = *(int *)(param_2 + 4);
              FUN_00597870(local_34);
              *(uint **)(param_2 + 0xc) = local_70 + local_68 * 9;
              iVar12 = *(int *)(param_2 + 4);
              if (iVar12 == 0) {
                *(uint **)(param_2 + 4) = local_70;
                *(uint **)(param_2 + 8) = local_70 + 9;
              }
              else {
                *(uint **)(param_2 + 4) = local_70;
                *(uint **)(param_2 + 8) =
                     local_70 + ((*(int *)(param_2 + 8) - iVar12) / 0x24) * 9 + 9;
              }
            }
            else {
              puVar9 = *(uint **)(param_2 + 8);
              for (iVar12 = 1 - ((int)puVar9 - (int)puVar11) / 0x24; iVar12 != 0;
                  iVar12 = iVar12 + -1) {
                if (puVar9 != (uint *)0x0) {
                  puVar14 = &local_60;
                  puVar13 = puVar9;
                  for (iVar10 = 9; iVar10 != 0; iVar10 = iVar10 + -1) {
                    *puVar13 = *puVar14;
                    puVar14 = puVar14 + 1;
                    puVar13 = puVar13 + 1;
                  }
                }
                puVar9 = puVar9 + 9;
              }
              puVar9 = *(uint **)(param_2 + 8);
              while (puVar11 != puVar9) {
                puVar15 = puVar11 + 9;
                puVar14 = &local_60;
                puVar13 = puVar11;
                for (iVar12 = 9; puVar11 = puVar15, iVar12 != 0; iVar12 = iVar12 + -1) {
                  *puVar13 = *puVar14;
                  puVar14 = puVar14 + 1;
                  puVar13 = puVar13 + 1;
                }
              }
              *(int *)(param_2 + 8) = *(int *)(param_2 + 8) + 0x24;
            }
          }
        }
        else {
          iVar10 = extraout_ECX_05;
          if (local_2c == 0) {
            FUN_0040e2c0(&local_8c);
            uVar3 = FUN_00442090(iVar10);
            iVar1 = local_1c;
            iVar2 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
            iVar10 = extraout_ECX_06;
            if (local_1c < iVar2) {
              iVar2 = DAT_006c5494 - iVar2;
              iVar10 = iVar2;
            }
            if (iVar2 <= local_24) {
              FUN_0040e2c0(&local_8c);
              uVar3 = FUN_00442090(iVar10);
              iVar2 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
              iVar10 = extraout_ECX_07;
              if (iVar1 < iVar2) {
                iVar2 = DAT_006c5494 - iVar2;
                iVar10 = iVar2;
              }
              if (iVar2 <= local_24) goto LAB_004ebffb;
            }
          }
          else {
LAB_004ebffb:
            if (local_30 == 0) {
              FUN_0040e8d0(&local_88);
              uVar3 = FUN_004420a0(iVar10);
              iVar10 = local_18;
              iVar2 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
              iVar1 = extraout_ECX_08;
              if (local_18 < iVar2) {
                iVar2 = DAT_006c5498 - iVar2;
                iVar1 = iVar2;
              }
              if (iVar2 <= local_74) {
                FUN_0040e8d0(&local_88);
                uVar3 = FUN_004420a0(iVar1);
                iVar1 = (uVar3 ^ (int)uVar3 >> 0x1f) - ((int)uVar3 >> 0x1f);
                if (iVar10 < iVar1) {
                  iVar1 = DAT_006c5498 - iVar1;
                }
                if (iVar1 <= local_74) goto LAB_004ec072;
              }
            }
            else {
LAB_004ec072:
              if ((local_20 != 0) ||
                 ((uVar3 = iVar12 - param_3[5] >> 0x1f,
                  (int)((iVar12 - param_3[5] ^ uVar3) - uVar3) <= local_64 &&
                  (uVar3 = iVar12 - *(int *)((int)param_3 + 0x36), uVar8 = (int)uVar3 >> 0x1f,
                  (int)((uVar3 ^ uVar8) - uVar8) <= local_64)))) {
                piVar21 = &local_6c;
                piVar18 = param_3 + 6;
                piVar20 = piVar18;
                iVar10 = local_3c;
                FUN_0040eeb0(iVar12);
                uVar17 = extraout_ECX_09;
                FUN_0040e8d0(&local_88);
                uVar6 = extraout_ECX_10;
                FUN_0040e2c0(&local_8c);
                uVar5 = extraout_ECX_11;
                FUN_0040eec0(&local_78);
                uVar16 = extraout_ECX_12;
                FUN_0040e8d0(&local_80);
                uVar7 = extraout_ECX_13;
                FUN_0040e2c0(&local_84);
                iVar10 = FUN_004f3990(uVar7,uVar16,uVar5,uVar6,uVar17,piVar18,piVar20,piVar21,iVar10
                                     );
                goto LAB_004ec19f;
              }
            }
          }
        }
LAB_004ec751:
        local_7c = local_7c + 1;
      } while (local_7c < 0x1a);
    }
    else {
      local_7c = *(int *)((int)param_3 + 0x42);
      if ((local_7c == -1) || (*(float *)(param_4 + 0x10) < (float)local_7c)) {
        iVar12 = 0;
        do {
          if (((uint)(ushort)param_1[2] & *(uint *)((int)&DAT_005c7108 + iVar12)) != 0) {
            local_8c = FUN_0040e650(*(int *)((int)&DAT_005c70c0 + iVar12) + local_84);
            local_88 = FUN_004132e0(*(int *)((int)&DAT_005c70d8 + iVar12) + local_80);
            iVar10 = *(int *)((int)&DAT_005c70f0 + iVar12) + local_78;
            if (*(int *)((int)param_3 + 0x3e) != -1) {
              iVar1 = local_78;
              FUN_0040e2c0(&local_8c);
              iVar1 = FUN_00442090(iVar1);
              local_7c = param_3[4];
              uVar7 = extraout_ECX;
              FUN_0040e8d0(&local_88);
              iVar2 = FUN_004420a0(uVar7);
              local_7c = iVar2 * iVar2 + iVar1 * iVar1;
              local_7c = __ftol();
            }
            if (((*(int *)((int)param_3 + 0x3e) == -1) || (local_7c < *(int *)((int)param_3 + 0x3e))
                ) && (local_5c = (short *)(DAT_006c54dc +
                                          ((&DAT_006cb942)[local_88] + (&DAT_006cb8c2)[iVar10] +
                                          local_8c) * 0xc), *local_5c != 0)) {
              local_60 = *(uint *)((int)&DAT_005c7120 + iVar12);
              local_44 = *(int *)(param_4 + 0x14) + local_60;
              local_48 = *(float *)(param_4 + 0x10) + _DAT_005c5388;
              FUN_005024e0(*(undefined4 *)(param_2 + 8),1,&local_60);
            }
          }
          iVar12 = iVar12 + 4;
          param_1 = local_70;
        } while (iVar12 < 0x18);
      }
    }
    *param_5 = *param_5 + -1;
  }
  return;
}

