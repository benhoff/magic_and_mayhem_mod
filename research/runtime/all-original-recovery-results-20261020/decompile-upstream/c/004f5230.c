
undefined4 __thiscall
FUN_004f5230(int param_1,undefined4 param_2,int param_3,int *param_4,int *param_5,int *param_6)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
  undefined4 uVar5;
  int iVar6;
  int *piVar7;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 extraout_ECX_08;
  undefined4 extraout_ECX_09;
  int extraout_ECX_10;
  undefined4 extraout_ECX_11;
  undefined4 extraout_ECX_12;
  undefined4 extraout_ECX_13;
  undefined4 extraout_ECX_14;
  undefined4 extraout_ECX_15;
  undefined4 extraout_ECX_16;
  int extraout_ECX_17;
  undefined4 extraout_ECX_18;
  undefined4 extraout_ECX_19;
  int extraout_ECX_20;
  undefined4 extraout_ECX_21;
  undefined4 extraout_ECX_22;
  undefined4 extraout_ECX_23;
  int extraout_ECX_24;
  undefined4 extraout_ECX_25;
  undefined4 extraout_ECX_26;
  undefined4 extraout_ECX_27;
  int extraout_ECX_28;
  undefined4 extraout_ECX_29;
  undefined4 extraout_ECX_30;
  undefined4 extraout_ECX_31;
  undefined4 extraout_ECX_32;
  undefined4 extraout_ECX_33;
  undefined4 extraout_ECX_34;
  undefined4 extraout_ECX_35;
  undefined4 extraout_ECX_36;
  undefined4 extraout_ECX_37;
  undefined4 extraout_ECX_38;
  undefined4 extraout_ECX_39;
  undefined4 extraout_ECX_40;
  undefined4 extraout_ECX_41;
  undefined4 extraout_ECX_42;
  undefined4 extraout_ECX_43;
  undefined4 extraout_ECX_44;
  undefined4 extraout_ECX_45;
  undefined4 extraout_ECX_46;
  undefined4 extraout_ECX_47;
  undefined4 extraout_ECX_48;
  undefined4 extraout_ECX_49;
  undefined4 extraout_ECX_50;
  undefined4 extraout_ECX_51;
  undefined4 extraout_ECX_52;
  undefined4 extraout_ECX_53;
  undefined4 extraout_ECX_54;
  undefined4 extraout_ECX_55;
  undefined4 extraout_ECX_56;
  undefined4 extraout_ECX_57;
  undefined4 extraout_ECX_58;
  int iVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  int local_50;
  int local_4c;
  int local_48;
  uint local_44;
  int local_40;
  int local_3c;
  int local_38;
  uint local_34;
  int local_30;
  int local_2c;
  int *local_28;
  int *local_24;
  int local_20 [8];
  
  iVar2 = param_3;
  if ((DAT_006e895c & 1) == 0) {
    DAT_006e895c = DAT_006e895c | 1;
    FUN_0054e170(0x499602d2);
    FUN_0059b7e2(&DAT_004f5af0);
  }
  local_40 = 0;
  local_3c = 0;
  local_38 = 0;
  local_50 = 0;
  local_4c = 0;
  local_48 = 0;
  FUN_0054e170(((*(int *)(param_1 + 0x64b2 + *param_5 * 4) +
                 *(int *)(param_1 + 0x6432 + *param_6 * 4) + *param_4) * 0xc) / 0xc);
  bVar1 = false;
  local_30 = 0;
  do {
    param_3 = 0;
    iVar8 = local_30;
    if (-1 < local_30) {
      do {
        local_34 = 2;
        local_44 = 2;
        if (iVar8 != 0) {
          local_44 = 4;
          local_34 = 4;
        }
        if (param_3 != 0) {
          local_34 = local_34 * 2;
          local_44 = local_34;
        }
        if (iVar8 == param_3) {
          local_34 = local_34 / 2;
          local_44 = local_34;
        }
        iVar3 = 0;
        if (local_34 != 0) {
          piVar7 = local_20;
          do {
            *piVar7 = iVar3;
            iVar3 = iVar3 + 1;
            piVar7 = piVar7 + 1;
          } while (iVar3 < (int)local_34);
        }
        local_2c = 0;
        if (local_34 != 0) {
          local_28 = local_20 + (local_34 - 1);
LAB_004f534b:
          uVar4 = FUN_0054e200();
          iVar3 = (int)((longlong)(ulonglong)(uVar4 & 0x7fffffff) % (longlong)(int)local_34);
          local_24 = local_20 + iVar3;
          switch(local_20[iVar3]) {
          case 0:
            local_50 = FUN_0040e650(*param_4 + iVar8);
            iVar8 = *param_5 + param_3;
            break;
          case 1:
            uVar5 = extraout_ECX;
            FUN_0040e2c0(param_4);
            iVar3 = FUN_00442090(uVar5);
            goto LAB_004f5423;
          case 2:
            uVar5 = extraout_ECX;
            FUN_0040e2c0(param_4);
            uVar5 = FUN_00442090(uVar5);
            local_50 = FUN_0040e650(uVar5);
            uVar5 = extraout_ECX_00;
            goto LAB_004f54c1;
          case 3:
            local_50 = FUN_0040e650(*param_4 + param_3);
            uVar5 = extraout_ECX_01;
            FUN_0040e8d0(param_5);
            goto LAB_004f54ca;
          case 4:
            iVar3 = *param_4 + param_3;
LAB_004f5423:
            local_50 = FUN_0040e650(iVar3);
            iVar8 = *param_5 + iVar8;
            break;
          case 5:
            uVar5 = extraout_ECX;
            FUN_0040e2c0(param_4);
            uVar5 = FUN_00442090(uVar5);
            local_50 = FUN_0040e650(uVar5);
            iVar8 = *param_5 + param_3;
            break;
          case 6:
            uVar5 = extraout_ECX;
            FUN_0040e2c0(param_4);
            uVar5 = FUN_00442090(uVar5);
            local_50 = FUN_0040e650(uVar5);
            uVar5 = extraout_ECX_02;
            FUN_0040e8d0(param_5);
            goto LAB_004f54ca;
          case 7:
            local_50 = FUN_0040e650(*param_4 + iVar8);
            uVar5 = extraout_ECX_03;
LAB_004f54c1:
            FUN_0040e8d0(param_5);
LAB_004f54ca:
            iVar8 = FUN_004420a0(uVar5);
            break;
          default:
            goto switchD_004f5374_default;
          }
          local_4c = FUN_004132e0(iVar8);
switchD_004f5374_default:
          iVar8 = *param_6;
          iVar3 = 0;
          local_48 = iVar8;
          do {
            iVar6 = *param_6 + iVar3;
            if (((DAT_005e1780 <= iVar6) && (*param_6 - iVar3 < DAT_006e1fa0)) || (3 < iVar3))
            goto LAB_004f5617;
            if (iVar6 < DAT_005e1780) {
              iVar8 = iVar2;
              iVar6 = FUN_00414880(iVar2);
              uVar10 = extraout_ECX_04;
              FUN_0040eeb0(iVar6 + iVar3);
              uVar9 = extraout_ECX_05;
              FUN_0040e8d0(&local_4c);
              uVar5 = extraout_ECX_06;
              FUN_0040e2c0(&local_50);
              iVar6 = FUN_004f46b0(uVar5,uVar9,uVar10,iVar8);
              iVar8 = local_48;
              if (iVar6 != 0) {
                iVar8 = iVar2;
                iVar6 = FUN_00414880(iVar2);
                uVar10 = extraout_ECX_07;
                FUN_0040eeb0(iVar6 + iVar3);
                uVar9 = extraout_ECX_08;
                FUN_0040e8d0(&local_4c);
                uVar5 = extraout_ECX_09;
                FUN_0040e2c0(&local_50);
                iVar6 = FUN_004f41a0(uVar5,uVar9,uVar10,iVar8);
                iVar8 = local_48;
                if (iVar6 == 0) {
                  iVar8 = *param_6 + iVar3;
                  iVar6 = extraout_ECX_10;
                  local_48 = iVar8;
                  goto LAB_004f5617;
                }
              }
            }
            if (-1 < *param_6 - iVar3) {
              iVar8 = iVar2;
              iVar6 = FUN_00414880(iVar2);
              uVar10 = extraout_ECX_11;
              FUN_0040eeb0(iVar6 - iVar3);
              uVar9 = extraout_ECX_12;
              FUN_0040e8d0(&local_4c);
              uVar5 = extraout_ECX_13;
              FUN_0040e2c0(&local_50);
              iVar6 = FUN_004f46b0(uVar5,uVar9,uVar10,iVar8);
              iVar8 = local_48;
              if (iVar6 != 0) {
                iVar8 = iVar2;
                iVar6 = FUN_00414880(iVar2);
                uVar10 = extraout_ECX_14;
                FUN_0040eeb0(iVar6 - iVar3);
                uVar9 = extraout_ECX_15;
                FUN_0040e8d0(&local_4c);
                uVar5 = extraout_ECX_16;
                FUN_0040e2c0(&local_50);
                iVar6 = FUN_004f41a0(uVar5,uVar9,uVar10,iVar8);
                iVar8 = local_48;
                if (iVar6 == 0) goto LAB_004f560f;
              }
            }
            iVar3 = iVar3 + 1;
          } while( true );
        }
LAB_004f59c3:
        param_3 = param_3 + 1;
      } while (param_3 <= iVar8);
    }
    local_30 = iVar8 + 1;
    if (5 < local_30) {
      return 0;
    }
  } while( true );
LAB_004f560f:
  iVar8 = *param_6 - iVar3;
  iVar6 = extraout_ECX_17;
  local_48 = iVar8;
LAB_004f5617:
  if ((*(int *)(iVar2 + 8) == 0) &&
     ((*(int *)(iVar2 + 0xc) != 2 || (*(int *)(param_1 + 0x7f0) <= iVar8)))) {
    while( true ) {
      iVar8 = iVar2;
      FUN_0040eec0(&local_48);
      uVar9 = extraout_ECX_18;
      FUN_0040e8d0(&local_4c);
      uVar5 = extraout_ECX_19;
      FUN_0040e2c0(&local_50);
      iVar3 = FUN_004f4330(uVar5,uVar9,iVar6,iVar8);
      iVar8 = extraout_ECX_20;
      if ((iVar3 != 0) || (local_48 < 1)) break;
      iVar8 = iVar2;
      iVar3 = FUN_00414880(iVar2);
      uVar10 = extraout_ECX_21;
      FUN_0040eeb0(iVar3 + -1);
      uVar9 = extraout_ECX_22;
      FUN_0040e8d0(&local_4c);
      uVar5 = extraout_ECX_23;
      FUN_0040e2c0(&local_50);
      iVar3 = FUN_004f46b0(uVar5,uVar9,uVar10,iVar8);
      iVar8 = extraout_ECX_24;
      if (iVar3 == 0) break;
      iVar8 = iVar2;
      iVar3 = FUN_00414880(iVar2);
      uVar10 = extraout_ECX_25;
      FUN_0040eeb0(iVar3 + -1);
      uVar9 = extraout_ECX_26;
      FUN_0040e8d0(&local_4c);
      uVar5 = extraout_ECX_27;
      FUN_0040e2c0(&local_50);
      iVar3 = FUN_004f41a0(uVar5,uVar9,uVar10,iVar8);
      iVar8 = extraout_ECX_28;
      if (iVar3 != 0) break;
      local_48 = local_48 + -1;
      iVar6 = extraout_ECX_28;
    }
    iVar3 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_29;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_30;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f4330(uVar5,uVar9,iVar8,iVar3);
    if (iVar8 == 0) goto LAB_004f598a;
    uVar10 = extraout_ECX_31;
    iVar8 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_32;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_33;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f46b0(uVar5,uVar9,uVar10,iVar8);
    if (iVar8 == 0) goto LAB_004f598a;
    uVar10 = extraout_ECX_34;
    iVar8 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_35;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_36;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f41a0(uVar5,uVar9,uVar10,iVar8);
    if (iVar8 != 0) goto LAB_004f598a;
    uVar10 = param_2;
    uVar11 = param_2;
    iVar8 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_37;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_38;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f47d0(uVar5,uVar9,uVar10,uVar11,iVar8);
    if (iVar8 != 0) {
      if (!bVar1) goto LAB_004f59ea;
      uVar13 = extraout_ECX_39;
      iVar8 = iVar2;
      FUN_0040eec0(&local_48);
      uVar12 = extraout_ECX_40;
      FUN_0040e8d0(&local_4c);
      uVar11 = extraout_ECX_41;
      FUN_0040e2c0(&local_50);
      uVar10 = extraout_ECX_42;
      FUN_0040eec0(&local_38);
      uVar9 = extraout_ECX_43;
      FUN_0040e8d0(&local_3c);
      uVar5 = extraout_ECX_44;
      FUN_0040e2c0(&local_40);
      iVar8 = FUN_004f4ba0(uVar5,uVar9,uVar10,uVar11,uVar12,uVar13,iVar8);
      goto joined_r0x004f595b;
    }
    if (bVar1) goto LAB_004f598a;
    iVar8 = *(int *)(iVar2 + 0x11);
  }
  else {
    iVar8 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_45;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_46;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f46b0(uVar5,uVar9,iVar6,iVar8);
    if (iVar8 == 0) goto LAB_004f598a;
    uVar10 = extraout_ECX_47;
    iVar8 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_48;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_49;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f41a0(uVar5,uVar9,uVar10,iVar8);
    if (iVar8 != 0) goto LAB_004f598a;
    uVar10 = extraout_ECX_50;
    uVar11 = param_2;
    iVar8 = iVar2;
    FUN_0040eec0(&local_48);
    uVar9 = extraout_ECX_51;
    FUN_0040e8d0(&local_4c);
    uVar5 = extraout_ECX_52;
    FUN_0040e2c0(&local_50);
    iVar8 = FUN_004f47d0(uVar5,uVar9,uVar10,uVar11,iVar8);
    if (iVar8 != 0) {
      if (!bVar1) goto LAB_004f59ea;
      uVar13 = extraout_ECX_53;
      iVar8 = iVar2;
      FUN_0040eec0(&local_48);
      uVar12 = extraout_ECX_54;
      FUN_0040e8d0(&local_4c);
      uVar11 = extraout_ECX_55;
      FUN_0040e2c0(&local_50);
      uVar10 = extraout_ECX_56;
      FUN_0040eec0(&local_38);
      uVar9 = extraout_ECX_57;
      FUN_0040e8d0(&local_3c);
      uVar5 = extraout_ECX_58;
      FUN_0040e2c0(&local_40);
      iVar8 = FUN_004f4ba0(uVar5,uVar9,uVar10,uVar11,uVar12,uVar13,iVar8);
joined_r0x004f595b:
      if (iVar8 != 0) {
LAB_004f59ea:
        *param_4 = local_50;
        *param_5 = local_4c;
        *param_6 = local_48;
        return 1;
      }
      goto LAB_004f598a;
    }
    if (bVar1) goto LAB_004f598a;
    iVar8 = *(int *)(iVar2 + 0x11);
  }
  if (iVar8 != 0x16) {
    bVar1 = true;
    local_40 = local_50;
    local_3c = local_4c;
    local_38 = local_48;
  }
LAB_004f598a:
  iVar8 = *local_28;
  local_28 = local_28 + -1;
  *local_24 = iVar8;
  local_2c = local_2c + 1;
  local_34 = local_34 - 1;
  iVar8 = local_30;
  if ((int)local_44 <= local_2c) goto LAB_004f59c3;
  goto LAB_004f534b;
}

