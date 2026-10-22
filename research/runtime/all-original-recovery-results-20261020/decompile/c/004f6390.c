
undefined4 __thiscall
FUN_004f6390(int param_1,uint param_2,uint param_3,uint param_4,int param_5,int param_6,int param_7,
            int param_8,int *param_9,uint *param_10,uint *param_11,uint *param_12,
            undefined4 param_13)

{
  ushort *puVar1;
  ushort uVar2;
  uint *puVar3;
  uint *puVar4;
  int iVar5;
  uint extraout_ECX;
  undefined4 extraout_ECX_00;
  int iVar6;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 extraout_ECX_08;
  undefined4 extraout_ECX_09;
  undefined4 extraout_ECX_10;
  undefined4 extraout_ECX_11;
  undefined4 extraout_ECX_12;
  undefined4 extraout_ECX_13;
  undefined4 extraout_ECX_14;
  undefined4 extraout_ECX_15;
  uint uVar7;
  int iVar8;
  int *piVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined4 uVar14;
  undefined4 uVar15;
  undefined4 uVar16;
  undefined4 uVar17;
  uint local_50;
  uint local_4c;
  uint local_48;
  uint local_44;
  uint local_40;
  undefined1 local_3c [60];
  
  FUN_004df200();
  puVar3 = param_12;
  piVar9 = param_9;
  local_44 = 0;
  local_40 = 0;
  uVar7 = extraout_ECX;
  if (((*param_10 != param_2) || (uVar7 = param_3, *param_11 != param_3)) ||
     (uVar7 = param_4, *param_12 != param_4)) {
    FUN_0040e2c0(param_10);
    iVar5 = FUN_00442090(uVar7);
    iVar8 = ((piVar9[1] + iVar5 * 2) * 0x20 + -0x20) / 2;
    uVar10 = extraout_ECX_00;
    FUN_0040e8d0(param_11);
    iVar5 = FUN_004420a0(uVar10);
    puVar4 = param_12;
    iVar6 = ((piVar9[1] + iVar5 * 2) * 0x20 + -0x20) / 2;
    iVar5 = (int)((*piVar9 + (*puVar3 - param_4) * 2) * 0x10 + -0x10) / 2;
    uVar7 = iVar5 * iVar5 + iVar6 * iVar6 + iVar8 * iVar8;
    if (uVar7 >> 10 <= (uint)(param_8 * param_8)) {
      param_5 = param_5 * 0x20;
      param_6 = param_6 * 0x20;
      param_7 = param_7 * 0x10;
      iVar5 = iVar8 * param_5 + iVar6 * param_6 + iVar5 * param_7;
      if ((-1 < iVar5) &&
         ((int)((param_5 * param_5 + param_6 * param_6 + param_7 * param_7) * uVar7) <=
          iVar5 * iVar5 * 4)) {
        uVar7 = *param_12;
        while( true ) {
          iVar5 = *puVar4 + *param_9;
          if (DAT_005e1780 + -1 <= (int)(*puVar4 + *param_9)) {
            iVar5 = DAT_005e1780 + -1;
          }
          if (iVar5 < (int)uVar7) break;
          local_40 = *param_11;
          piVar9 = param_9;
          param_12 = (uint *)uVar7;
          while (iVar5 = FUN_0040eb70(*param_11), iVar5 < piVar9[1]) {
            local_44 = *param_10;
            while (iVar5 = FUN_0040e4e0(*param_10), iVar5 < piVar9[1]) {
              local_48 = param_4;
              local_50 = param_2;
              local_4c = param_3;
              FUN_004df6e0(param_2,param_3,param_4,local_44,local_40,uVar7,local_3c);
              do {
                uVar7 = (uint)param_12;
                if (((local_50 == local_44) && (local_4c == local_40)) &&
                   ((uint *)local_48 == param_12)) {
                  FUN_004df500(&local_50,&local_4c,&local_48,&param_8);
                  puVar3 = param_10;
                  iVar5 = FUN_0040e4e0(local_44);
                  local_50 = local_50 + iVar5;
                  iVar5 = FUN_0040eb70(local_40);
                  local_4c = local_4c + iVar5;
                  local_48 = local_48 + (*puVar4 - uVar7);
                  if ((DAT_006e1fa0 <= (int)local_48) && ((int)local_48 < DAT_005e1780)) {
                    uVar17 = 0xffffffff;
                    uVar15 = param_13;
                    piVar9 = param_9;
                    uVar16 = param_13;
                    FUN_0040eeb0(local_48);
                    uVar14 = extraout_ECX_01;
                    FUN_0040e8a0(local_4c);
                    uVar13 = extraout_ECX_02;
                    FUN_0040e290(local_50);
                    uVar12 = extraout_ECX_03;
                    FUN_0040eec0(puVar4);
                    uVar11 = extraout_ECX_04;
                    FUN_0040e8d0(param_11);
                    uVar10 = extraout_ECX_05;
                    FUN_0040e2c0(puVar3);
                    iVar5 = FUN_004f3990(uVar10,uVar11,uVar12,uVar13,uVar14,uVar15,piVar9,uVar16,
                                         uVar17);
                    if (iVar5 != 0) goto LAB_004f68d2;
                  }
                  iVar5 = *puVar4 - 1;
                  iVar6 = DAT_006e1fa0;
                  if (DAT_006e1fa0 <= iVar5) {
                    iVar6 = iVar5;
                  }
                  if ((iVar6 < (int)local_48) &&
                     (((local_50 != *puVar3 || (local_4c != *param_11)) || (local_48 - 1 != *puVar4)
                      ))) {
                    uVar17 = 0xffffffff;
                    uVar15 = param_13;
                    piVar9 = param_9;
                    uVar16 = param_13;
                    FUN_0040eeb0(local_48 - 1);
                    uVar14 = extraout_ECX_06;
                    FUN_0040e8a0(local_4c);
                    uVar13 = extraout_ECX_07;
                    FUN_0040e290(local_50);
                    uVar12 = extraout_ECX_08;
                    FUN_0040eec0(puVar4);
                    uVar11 = extraout_ECX_09;
                    FUN_0040e8d0(param_11);
                    uVar10 = extraout_ECX_10;
                    FUN_0040e2c0(puVar3);
                    iVar5 = FUN_004f3990(uVar10,uVar11,uVar12,uVar13,uVar14,uVar15,piVar9,uVar16,
                                         uVar17);
                    if (iVar5 != 0) {
                      local_48 = local_48 - 1;
                      goto LAB_004f68d2;
                    }
                  }
                  iVar5 = *puVar4 + 1;
                  if (DAT_005e1780 + -1 < iVar5) {
                    iVar5 = DAT_005e1780 + -1;
                  }
                  if (((int)local_48 < iVar5) &&
                     (((local_50 != *puVar3 || (local_4c != *param_11)) || (local_48 + 1 != *puVar4)
                      ))) {
                    uVar17 = 0xffffffff;
                    uVar15 = param_13;
                    piVar9 = param_9;
                    uVar16 = param_13;
                    FUN_0040eeb0(local_48 + 1);
                    uVar14 = extraout_ECX_11;
                    FUN_0040e8a0(local_4c);
                    uVar13 = extraout_ECX_12;
                    FUN_0040e290(local_50);
                    uVar12 = extraout_ECX_13;
                    FUN_0040eec0(puVar4);
                    uVar11 = extraout_ECX_14;
                    FUN_0040e8d0(param_11);
                    uVar10 = extraout_ECX_15;
                    FUN_0040e2c0(puVar3);
                    iVar5 = FUN_004f3990(uVar10,uVar11,uVar12,uVar13,uVar14,uVar15,piVar9,uVar16,
                                         uVar17);
                    if (iVar5 != 0) {
                      local_48 = local_48 + 1;
LAB_004f68d2:
                      if ((int)local_50 < 0) {
                        do {
                          local_50 = local_50 + DAT_006c5494;
                        } while ((int)local_50 < 0);
                      }
                      else {
                        for (; DAT_006c5494 <= local_50; local_50 = local_50 - DAT_006c5494) {
                        }
                      }
                      *puVar3 = local_50;
                      if ((int)local_4c < 0) {
                        do {
                          local_4c = local_4c + DAT_006c5498;
                        } while ((int)local_4c < 0);
                      }
                      else {
                        for (; DAT_006c5498 <= local_4c; local_4c = local_4c - DAT_006c5498) {
                        }
                      }
                      *param_11 = local_4c;
                      *puVar4 = local_48;
                      return 1;
                    }
                  }
                  break;
                }
                FUN_004df500(&local_50,&local_4c,&local_48,local_3c);
                local_50 = FUN_0040e650(local_50);
                local_4c = FUN_004132e0(local_4c);
                puVar1 = (ushort *)
                         (*(int *)(param_1 + 0x4c) +
                         (*(int *)(param_1 + 0x64b2 + local_4c * 4) +
                          *(int *)(param_1 + 0x6432 + local_48 * 4) + local_50) * 0xc);
                uVar2 = *puVar1;
              } while (((uVar2 == 0) || ((puVar1[5] & 0x80) != 0)) ||
                      (*(int *)(DAT_0065660c + 0x94 + (uint)uVar2 * 0x164) != 0x10));
              local_44 = FUN_0040e650(local_44 + 1);
              uVar7 = (uint)param_12;
              piVar9 = param_9;
            }
            local_40 = FUN_004132e0(local_40 + 1);
          }
          uVar7 = uVar7 + 1;
        }
      }
    }
  }
  return 0;
}

