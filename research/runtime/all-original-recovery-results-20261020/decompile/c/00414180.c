
undefined1 __thiscall FUN_00414180(int param_1)

{
  int *piVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  uint extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 extraout_ECX_08;
  uint extraout_ECX_09;
  undefined4 extraout_ECX_10;
  undefined4 extraout_ECX_11;
  uint extraout_ECX_12;
  undefined4 extraout_ECX_13;
  undefined4 extraout_ECX_14;
  undefined4 extraout_ECX_15;
  undefined4 extraout_ECX_16;
  undefined4 extraout_ECX_17;
  undefined4 extraout_ECX_18;
  undefined4 extraout_ECX_19;
  uint extraout_ECX_20;
  undefined4 extraout_ECX_21;
  uint extraout_ECX_22;
  undefined4 extraout_ECX_23;
  uint extraout_ECX_24;
  undefined4 extraout_ECX_25;
  undefined4 extraout_ECX_26;
  undefined4 extraout_ECX_27;
  undefined4 extraout_ECX_28;
  undefined4 extraout_ECX_29;
  undefined4 extraout_ECX_30;
  uint *puVar5;
  int iVar6;
  int iVar7;
  int in_stack_00000058;
  uint *in_stack_0000005c;
  uint *in_stack_00000060;
  uint *in_stack_00000064;
  uint *in_stack_00000068;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  undefined1 *puVar14;
  uint local_50;
  undefined1 local_39;
  uint local_38;
  uint local_34;
  uint local_30;
  uint local_2c;
  uint local_28;
  uint local_24;
  uint local_20;
  uint local_1c;
  uint local_18;
  int local_14;
  int *local_10;
  undefined1 local_c [4];
  uint local_8;
  undefined1 local_4 [4];
  
  local_28 = 0;
  local_1c = 0;
  local_34 = 0;
  local_20 = 0;
  local_24 = 0;
  local_30 = 0;
  local_38 = 0;
  if (*(int *)(param_1 + 0xe4) != *(int *)(param_1 + 0xd8)) {
    piVar1 = (int *)(*(int *)(param_1 + 0xe4) + -0x20);
    local_50 = 0;
    local_14 = param_1;
    FUN_0042fd90(&local_8);
    if (piVar1 != (int *)0x0) {
      iVar6 = (*piVar1 - DAT_006c54dc) / 0xc;
      uVar2 = iVar6 / DAT_006c54a0;
      iVar6 = iVar6 - (&DAT_006cb8c2)[uVar2];
      local_50 = iVar6 / (int)DAT_006c5494;
      local_18 = uVar2;
      local_20 = FUN_004132e0();
      local_50 = iVar6 - (&DAT_006cb942)[local_20];
      local_28 = FUN_0040e650();
      local_39 = 0;
      *in_stack_00000068 = DAT_006c5494 + DAT_006c5498 >> 1;
      puVar5 = in_stack_00000068;
      do {
        if (*(int *)(local_14 + 0xe4) == *(int *)(local_14 + 0xd8)) {
          return local_39;
        }
        local_50 = 0;
        piVar1 = (int *)(*(int *)(local_14 + 0xe4) + -0x20);
        FUN_0042fd90(local_4);
        if (piVar1 == (int *)0x0) {
          return local_39;
        }
        uVar12 = extraout_ECX;
        local_50 = uVar2;
        FUN_0040e8d0(&local_20);
        uVar11 = extraout_ECX_00;
        FUN_0040e2c0(&local_28);
        uVar10 = extraout_ECX_01;
        FUN_0040eec0(&stack0x0000003a);
        uVar9 = extraout_ECX_02;
        FUN_0040e8d0(&stack0x00000036);
        uVar8 = extraout_ECX_03;
        FUN_0040e2c0(&stack0x00000032);
        iVar6 = FUN_0040a6b0(uVar8,uVar9,uVar10,uVar11,uVar12);
        if (iVar6 < in_stack_00000058 * in_stack_00000058) {
          return local_39;
        }
        iVar6 = (*piVar1 - DAT_006c54dc) / 0xc;
        local_8 = iVar6 / DAT_006c54a0;
        iVar6 = iVar6 - (&DAT_006cb8c2)[local_8];
        local_50 = iVar6 / (int)DAT_006c5494;
        local_24 = FUN_004132e0();
        local_50 = iVar6 - (&DAT_006cb942)[local_24];
        local_1c = FUN_0040e650();
        if ((local_1c != local_28) || (local_24 != local_20)) {
          local_34 = local_28;
          local_30 = local_20;
          local_2c = 1;
          local_50 = local_20;
          local_38 = uVar2;
          iVar6 = FUN_0040eb70();
          iVar6 = -iVar6;
          local_50 = local_28;
          iVar3 = FUN_0040e4e0();
          if (1 < (int)*in_stack_00000068) {
            do {
              iVar7 = (DAT_005e1780 - local_38) + -1;
              local_50 = local_38;
              if (1 < iVar7) {
                iVar7 = 1;
              }
              while( true ) {
                iVar4 = DAT_006e1fa0 - local_38;
                if (iVar4 < -1) {
                  iVar4 = -1;
                }
                if (iVar7 <= iVar4) break;
                local_10 = (int *)&local_50;
                iVar4 = FUN_00414880();
                *local_10 = iVar4 + iVar7;
                iVar4 = FUN_00414870();
                uVar9 = extraout_ECX_04;
                FUN_0040e8a0(iVar4 + iVar3);
                iVar4 = FUN_00414860(uVar9);
                uVar8 = extraout_ECX_05;
                FUN_0040e290(iVar4 + iVar6);
                iVar4 = FUN_00414890(uVar8,uVar9);
                local_50 = extraout_ECX_06;
                if (iVar4 != 0) {
                  local_10 = (int *)&local_50;
                  iVar4 = FUN_00414880();
                  *local_10 = iVar4 + iVar7;
                  iVar4 = FUN_00414870();
                  uVar9 = extraout_ECX_07;
                  FUN_0040e8a0(iVar4 + iVar3);
                  iVar4 = FUN_00414860(uVar9);
                  uVar8 = extraout_ECX_08;
                  FUN_0040e290(iVar4 + iVar6);
                  iVar4 = FUN_00414940(uVar8,uVar9);
                  local_50 = extraout_ECX_09;
                  if (iVar4 != 0) {
                    local_10 = (int *)&local_50;
                    iVar4 = FUN_00414880();
                    *local_10 = iVar4 + iVar7;
                    iVar4 = FUN_00414870();
                    uVar9 = extraout_ECX_10;
                    FUN_0040e8a0(iVar4 + iVar3);
                    iVar4 = FUN_00414860(uVar9);
                    uVar8 = extraout_ECX_11;
                    FUN_0040e290(iVar4 + iVar6);
                    iVar4 = FUN_004149f0(uVar8,uVar9);
                    local_50 = extraout_ECX_12;
                    if (iVar4 == 0) break;
                  }
                }
                iVar7 = iVar7 + -1;
              }
              puVar14 = local_c;
              local_50 = 0;
              iVar4 = FUN_00414880(puVar14);
              uVar13 = extraout_ECX_13;
              FUN_0040eeb0(iVar4 + iVar7);
              iVar4 = FUN_00414870(uVar13);
              uVar12 = extraout_ECX_14;
              FUN_0040e8a0(iVar4 + iVar3);
              iVar4 = FUN_00414860(uVar12);
              uVar11 = extraout_ECX_15;
              FUN_0040e290(iVar4 + iVar6);
              uVar10 = extraout_ECX_16;
              FUN_0040eec0(&local_38);
              uVar9 = extraout_ECX_17;
              FUN_0040e8d0(&local_30);
              uVar8 = extraout_ECX_18;
              FUN_0040e2c0(&local_34);
              iVar4 = FUN_00514360(uVar8,uVar9,uVar10,uVar11,uVar12,uVar13,puVar14);
              if (iVar4 == 0) break;
              local_2c = local_2c + 1;
              local_50 = iVar6 + local_34;
              local_34 = FUN_0040e650();
              local_50 = local_30 + iVar3;
              local_30 = FUN_004132e0();
              local_38 = local_38 + iVar7;
            } while ((int)local_2c < (int)*in_stack_00000068);
          }
          local_34 = local_28;
          local_30 = local_20;
          local_38 = local_18;
          local_50 = local_20;
          iVar6 = FUN_0040eb70();
          local_50 = local_28;
          iVar3 = FUN_0040e4e0();
          local_18 = -iVar3;
          if ((int)local_2c < (int)*in_stack_00000068) {
            do {
              local_50 = DAT_005e1780 - local_38;
              iVar3 = local_50 - 1;
              if (1 < iVar3) {
                iVar3 = 1;
              }
              while( true ) {
                iVar7 = DAT_006e1fa0 - local_38;
                if (iVar7 < -1) {
                  iVar7 = -1;
                }
                if (iVar3 <= iVar7) break;
                iVar7 = FUN_00414880();
                local_50 = iVar7 + iVar3;
                iVar7 = FUN_00414870();
                uVar2 = local_18;
                FUN_0040e8a0(iVar7 + local_18);
                iVar7 = FUN_00414860(uVar2);
                uVar8 = extraout_ECX_19;
                FUN_0040e290(iVar7 + iVar6);
                iVar7 = FUN_00414890(uVar8,uVar2);
                local_50 = extraout_ECX_20;
                if (iVar7 != 0) {
                  iVar7 = FUN_00414880();
                  local_50 = iVar7 + iVar3;
                  iVar7 = FUN_00414870();
                  uVar2 = local_18;
                  FUN_0040e8a0(iVar7 + local_18);
                  iVar7 = FUN_00414860(uVar2);
                  uVar8 = extraout_ECX_21;
                  FUN_0040e290(iVar7 + iVar6);
                  iVar7 = FUN_00414940(uVar8,uVar2);
                  local_50 = extraout_ECX_22;
                  if (iVar7 != 0) {
                    iVar7 = FUN_00414880();
                    local_50 = iVar7 + iVar3;
                    iVar7 = FUN_00414870();
                    uVar2 = local_18;
                    FUN_0040e8a0(iVar7 + local_18);
                    iVar7 = FUN_00414860(uVar2);
                    uVar8 = extraout_ECX_23;
                    FUN_0040e290(iVar7 + iVar6);
                    iVar7 = FUN_004149f0(uVar8,uVar2);
                    local_50 = extraout_ECX_24;
                    if (iVar7 == 0) break;
                  }
                }
                iVar3 = iVar3 + -1;
              }
              puVar14 = local_c;
              local_50 = 0;
              iVar7 = FUN_00414880(puVar14);
              uVar13 = extraout_ECX_25;
              FUN_0040eeb0(iVar7 + iVar3);
              iVar7 = FUN_00414870(uVar13);
              uVar2 = local_18;
              uVar12 = extraout_ECX_26;
              FUN_0040e8a0(iVar7 + local_18);
              iVar7 = FUN_00414860(uVar12);
              uVar11 = extraout_ECX_27;
              FUN_0040e290(iVar7 + iVar6);
              uVar10 = extraout_ECX_28;
              FUN_0040eec0(&local_38);
              uVar9 = extraout_ECX_29;
              FUN_0040e8d0(&local_30);
              uVar8 = extraout_ECX_30;
              FUN_0040e2c0(&local_34);
              iVar7 = FUN_00514360(uVar8,uVar9,uVar10,uVar11,uVar12,uVar13,puVar14);
              if (iVar7 == 0) break;
              local_2c = local_2c + 1;
              local_34 = local_34 + iVar6;
              if ((int)local_34 < 0) {
                do {
                  local_34 = local_34 + DAT_006c5494;
                } while ((int)local_34 < 0);
              }
              else {
                for (; DAT_006c5494 <= local_34; local_34 = local_34 - DAT_006c5494) {
                }
              }
              local_30 = uVar2 + local_30;
              if ((int)local_30 < 0) {
                do {
                  local_30 = local_30 + DAT_006c5498;
                } while ((int)local_30 < 0);
              }
              else {
                for (; DAT_006c5498 <= local_30; local_30 = local_30 - DAT_006c5498) {
                }
              }
              local_38 = local_38 + iVar3;
            } while ((int)local_2c < (int)*in_stack_00000068);
          }
          local_28 = local_1c;
          local_20 = local_24;
          local_18 = local_8;
          puVar5 = in_stack_00000068;
          uVar2 = local_8;
        }
        if ((int)local_2c < (int)*puVar5) {
          local_39 = 1;
          *in_stack_0000005c = local_28;
          *in_stack_00000060 = local_20;
          *in_stack_00000064 = uVar2;
          *puVar5 = local_2c;
        }
      } while( true );
    }
  }
  return 0;
}

