
void __fastcall FUN_005404a0(int *param_1)

{
  int iVar1;
  bool bVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 uVar6;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 extraout_ECX_08;
  undefined4 extraout_ECX_09;
  uint uVar7;
  int iVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 *puVar11;
  int local_6c;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined1 local_50;
  undefined4 local_4f;
  undefined1 local_4b;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined1 local_38;
  undefined4 local_37;
  undefined1 local_33;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined1 local_20;
  undefined4 local_1f;
  undefined1 local_1b;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined1 local_8;
  undefined4 local_7;
  undefined1 local_3;
  
  local_6c = 0;
  if (DAT_006e1f48 == 0) {
    if (*(int *)((int)param_1 + 0x69d5) == -1) {
      iVar4 = *(int *)((int)param_1 + 0x69cd);
      iVar8 = *(int *)((int)param_1 + iVar4 * 0xc + 0x1b1);
      if (iVar8 != -1) {
        *(int *)((int)param_1 + 0x69d5) = iVar8;
        *(undefined4 *)((int)param_1 + 0x6a05) = 0;
        *(undefined4 *)((int)param_1 + 0x6a09) = 0;
        iVar8 = *(int *)((int)param_1 + iVar4 * 0xc + 0x1b5);
        if (iVar8 == 1) {
          *(undefined4 *)((int)param_1 + 0x6a05) = 1;
        }
        else if (iVar8 == 2) {
          *(undefined4 *)((int)param_1 + 0x6a09) = 1;
        }
        *(undefined4 *)((int)param_1 + iVar4 * 0xc + 0x1b1) = 0xffffffff;
        iVar4 = *(int *)((int)param_1 + 0x69cd) + 1;
        *(int *)((int)param_1 + 0x69cd) = iVar4;
        if (0x540 < iVar4) {
          *(undefined4 *)((int)param_1 + 0x69cd) = 0;
        }
        *(undefined4 *)((int)param_1 + 0x69d9) = 0;
        *(undefined4 *)((int)param_1 + 0x69dd) = 0;
        *(undefined4 *)((int)param_1 + 0x69e1) = 0;
        local_6c = 1;
      }
    }
    if (*(int *)((int)param_1 + 0x69d5) != -1) {
      local_64 = 10;
      iVar4 = *param_1 + *(int *)((int)param_1 + 0x69d5) * 0x294;
      if (*(int *)((int)param_1 + 0x69d9) < param_1[2]) {
        do {
          iVar8 = *(int *)((int)param_1 + 0x69d9);
          if (iVar8 != *(int *)((int)param_1 + 0x69d5)) {
            iVar1 = *param_1 + iVar8 * 0x294;
            uVar3 = *(int *)(iVar4 + 4) - *(int *)(*param_1 + 4 + iVar8 * 0x294);
            uVar7 = (int)uVar3 >> 0x1f;
            iVar5 = (uVar3 ^ uVar7) - uVar7;
            uVar3 = *(int *)(iVar4 + 8) - *(int *)(iVar1 + 8);
            uVar7 = (int)uVar3 >> 0x1f;
            iVar8 = (uVar3 ^ uVar7) - uVar7;
            if (DAT_006c5494 / 2 < iVar5) {
              iVar5 = DAT_006c5494 - iVar5;
            }
            if (DAT_006c5498 / 2 < iVar8) {
              iVar8 = DAT_006c5498 - iVar8;
            }
            if (iVar8 < iVar5) {
              iVar8 = iVar8 + iVar5 * 2;
            }
            else {
              iVar8 = iVar5 + iVar8 * 2;
            }
            iVar8 = iVar8 / 2;
            if (iVar8 < 9) {
              bVar2 = true;
              if (local_6c != 0) {
                local_60 = 1;
                local_5c = 1;
                local_50 = 1;
                puVar11 = &local_60;
                local_58 = 0;
                local_54 = 0;
                local_4f = 0;
                local_4b = 0;
                iVar5 = local_6c;
                FUN_0040eeb0(*(undefined4 *)(iVar4 + 0xc));
                uVar9 = extraout_ECX;
                FUN_0040e8a0(*(undefined4 *)(iVar4 + 8));
                uVar6 = extraout_ECX_00;
                FUN_0040e290(*(undefined4 *)(iVar4 + 4));
                iVar5 = FUN_004f4900(uVar6,uVar9,iVar5,puVar11);
                uVar6 = extraout_ECX_01;
                if (iVar5 != 0) {
                  local_48 = 1;
                  local_44 = 1;
                  local_38 = 1;
                  puVar11 = &local_48;
                  local_40 = 0;
                  local_3c = 0;
                  local_37 = 0;
                  local_33 = 0;
                  uVar10 = extraout_ECX_01;
                  FUN_0040eeb0(*(undefined4 *)(iVar1 + 0xc));
                  uVar9 = extraout_ECX_02;
                  FUN_0040e8a0(*(undefined4 *)(iVar1 + 8));
                  uVar6 = extraout_ECX_03;
                  FUN_0040e290(*(undefined4 *)(iVar1 + 4));
                  iVar5 = FUN_004f4900(uVar6,uVar9,uVar10,puVar11);
                  uVar6 = extraout_ECX_04;
                  if ((iVar5 == 0) && (5 < iVar8)) {
                    bVar2 = false;
                  }
                }
                local_30 = 1;
                local_2c = 1;
                local_20 = 1;
                puVar11 = &local_30;
                local_28 = 0;
                local_24 = 0;
                local_1f = 0;
                local_1b = 0;
                FUN_0040eeb0(*(undefined4 *)(iVar4 + 0xc));
                uVar10 = extraout_ECX_05;
                FUN_0040e8a0(*(undefined4 *)(iVar4 + 8));
                uVar9 = extraout_ECX_06;
                FUN_0040e290(*(undefined4 *)(iVar4 + 4));
                iVar5 = FUN_004f4900(uVar9,uVar10,uVar6,puVar11);
                if (iVar5 == 0) {
                  local_18 = 1;
                  local_14 = 1;
                  local_8 = 1;
                  puVar11 = &local_18;
                  local_10 = 0;
                  local_c = 0;
                  local_7 = 0;
                  local_3 = 0;
                  uVar10 = extraout_ECX_07;
                  FUN_0040eeb0(*(undefined4 *)(iVar1 + 0xc));
                  uVar9 = extraout_ECX_08;
                  FUN_0040e8a0(*(undefined4 *)(iVar1 + 8));
                  uVar6 = extraout_ECX_09;
                  FUN_0040e290(*(undefined4 *)(iVar1 + 4));
                  iVar5 = FUN_004f4900(uVar6,uVar9,uVar10,puVar11);
                  if ((iVar5 != 0) && (5 < iVar8)) goto LAB_005407db;
                }
                if (!bVar2) goto LAB_005407db;
              }
              iVar8 = FUN_00540820(iVar4,iVar1,1,local_6c,&local_64);
              if (iVar8 == 0) break;
              local_6c = 1;
            }
          }
LAB_005407db:
          iVar8 = *(int *)((int)param_1 + 0x69d9) + 1;
          *(int *)((int)param_1 + 0x69d9) = iVar8;
        } while (iVar8 < param_1[2]);
      }
      if (param_1[2] <= *(int *)((int)param_1 + 0x69d9)) {
        *(undefined4 *)((int)param_1 + 0x69d5) = 0xffffffff;
      }
    }
  }
  return;
}

