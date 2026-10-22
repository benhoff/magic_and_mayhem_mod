
undefined4 __thiscall FUN_00523860(int param_1,int *param_2)

{
  uint uVar1;
  int iVar2;
  int iVar3;
  uint uVar4;
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
  uint uVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  undefined4 *puVar13;
  int local_44;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  undefined4 local_30 [4];
  undefined1 local_20;
  undefined4 local_1f;
  undefined1 local_1b;
  undefined4 local_18 [6];
  
  iVar3 = DAT_006eaf10;
  local_44 = 0;
  local_40 = 0;
  local_3c = 0;
  local_34 = DAT_006eaf10;
  uVar1 = FUN_0054e200();
  *param_2 = (int)((longlong)(ulonglong)(uVar1 & 0x7fffffff) % (longlong)(iVar3 + -1));
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x467d);
  }
  iVar2 = *param_2;
  if ((iVar2 < DAT_006eaf0c) && (-1 < iVar2)) {
    uVar1 = DAT_006eaf08 + iVar2 * 0x294;
  }
  else {
    uVar1 = 0;
  }
  local_38 = 0;
  if (0 < iVar3) {
    do {
      iVar3 = *(int *)(param_1 + 0xbdb);
      iVar2 = FUN_004f4b30(*(undefined4 *)(param_1 + 0xbd7),*(undefined4 *)(uVar1 + 4),
                           *(undefined4 *)(uVar1 + 8),*(undefined4 *)(uVar1 + 0xc));
      iVar3 = iVar3 * 2;
      if (iVar2 <= iVar3) {
        FUN_0040eeb0(*(undefined4 *)(uVar1 + 0xc));
        uVar12 = extraout_ECX;
        FUN_0040e8a0(*(undefined4 *)(uVar1 + 8));
        uVar11 = extraout_ECX_00;
        FUN_0040e290(*(undefined4 *)(uVar1 + 4));
        uVar10 = extraout_ECX_01;
        FUN_0040eeb0(*(undefined4 *)(param_1 + 0x10));
        uVar9 = extraout_ECX_02;
        FUN_0040e8a0(*(undefined4 *)(param_1 + 0xc));
        uVar8 = extraout_ECX_03;
        FUN_0040e290(*(undefined4 *)(param_1 + 8));
        iVar3 = FUN_004e1380(uVar8,uVar9,uVar10,uVar11,uVar12,iVar3);
        if (iVar3 == 0) {
          iVar3 = 0;
          if (0 < DAT_006c5c84) {
            do {
              FUN_004f3840(iVar3,&local_44,&local_40,&local_3c);
              if (((*(int *)(uVar1 + 4) == local_44) && (*(int *)(uVar1 + 8) == local_40)) &&
                 (uVar4 = local_3c - *(int *)(uVar1 + 0xc), uVar5 = (int)uVar4 >> 0x1f,
                 (int)((uVar4 ^ uVar5) - uVar5) < 2)) goto switchD_00523a94_default;
              iVar3 = iVar3 + 1;
            } while (iVar3 < DAT_006c5c84);
          }
          if ((*(char *)(param_1 + 0x723) == '\0') && (*(int *)(param_1 + 0x5ec) != 0xe)) {
            local_1b = 0;
          }
          else {
            local_1b = 1;
          }
          iVar3 = *(int *)(param_1 + 0xac);
          local_30[0] = *(undefined4 *)(iVar3 + 0xc);
          local_30[1] = *(undefined4 *)(iVar3 + 8);
          local_30[3] = *(undefined4 *)(iVar3 + 0x44);
          local_30[2] = *(undefined4 *)(param_1 + 0x104);
          local_20 = 0;
          local_1f = *(undefined4 *)(param_1 + 0xa8);
          puVar6 = local_30;
          puVar7 = local_18;
          for (iVar3 = 5; iVar3 != 0; iVar3 = iVar3 + -1) {
            *puVar7 = *puVar6;
            puVar6 = puVar6 + 1;
            puVar7 = puVar7 + 1;
          }
          uVar8 = *(undefined4 *)(uVar1 + 0xc);
          puVar13 = local_18;
          *(undefined2 *)puVar7 = *(undefined2 *)puVar6;
          FUN_0040eeb0(uVar8);
          uVar9 = extraout_ECX_04;
          FUN_0040e8a0(*(undefined4 *)(uVar1 + 8));
          uVar8 = extraout_ECX_05;
          FUN_0040e290(*(undefined4 *)(uVar1 + 4));
          iVar3 = FUN_004f4900(uVar8,uVar9,iVar3,puVar13);
          if (iVar3 != 0) {
            switch(*(undefined4 *)(param_1 + 0xbdf)) {
            case 0:
              return 1;
            case 1:
              uVar8 = *(undefined4 *)(uVar1 + 0xc);
              uVar10 = uVar8;
              FUN_0040e8a0(*(undefined4 *)(uVar1 + 8));
              uVar9 = extraout_ECX_07;
              FUN_0040e290(*(undefined4 *)(uVar1 + 4));
              iVar3 = FUN_00414890(uVar9,uVar8,uVar10);
              if (iVar3 != 0) {
                return 1;
              }
              break;
            case 2:
              if (*(int *)(param_1 + 0x104) != 0) {
                uVar8 = *(undefined4 *)(uVar1 + 0xc);
                uVar10 = uVar8;
                FUN_0040e8a0(*(undefined4 *)(uVar1 + 8));
                uVar9 = extraout_ECX_08;
                FUN_0040e290(*(undefined4 *)(uVar1 + 4));
                iVar3 = FUN_00414890(uVar9,uVar8,uVar10);
                if ((iVar3 == 0) && (DAT_006c5c80 <= *(int *)(uVar1 + 0xc))) {
                  return 1;
                }
              }
              break;
            case 3:
              uVar8 = *(undefined4 *)(uVar1 + 0xc);
              uVar10 = extraout_ECX_06;
              FUN_0040e8a0(*(undefined4 *)(uVar1 + 8));
              uVar9 = extraout_ECX_09;
              FUN_0040e290(*(undefined4 *)(uVar1 + 4));
              iVar3 = FUN_00414890(uVar9,uVar10,uVar8);
              if (((iVar3 != 0) &&
                  ((uint)(DAT_006c5c80 - *(int *)(*(int *)(param_1 + 0xac) + 0xc)) <=
                   *(uint *)(uVar1 + 0xc))) && ((int)*(uint *)(uVar1 + 0xc) < DAT_006c5c80)) {
                return 1;
              }
            }
          }
        }
      }
switchD_00523a94_default:
      uVar1 = uVar1 + 0x294;
      iVar3 = *param_2;
      *param_2 = iVar3 + 1;
      if (local_34 <= iVar3 + 1) {
        *param_2 = 0;
        uVar1 = (DAT_006eaf0c < 1) - 1 & DAT_006eaf08;
      }
      local_38 = local_38 + 1;
    } while (local_38 < local_34);
  }
  return 0;
}

