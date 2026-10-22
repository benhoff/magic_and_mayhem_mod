
/* WARNING: Removing unreachable block (ram,0x0052df8a) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_0052d530(undefined4 param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5,
                 undefined4 param_6,undefined4 param_7,undefined2 param_8,undefined4 param_9,
                 undefined4 param_10,undefined2 param_11,undefined4 param_12,undefined1 param_13,
                 undefined4 param_14,uint param_15,uint param_16,undefined4 param_17,int param_18,
                 undefined4 param_19,undefined2 param_20)

{
  char cVar1;
  int iVar2;
  undefined4 *puVar3;
  int iVar4;
  int iVar5;
  int extraout_ECX;
  uint uVar6;
  uint uVar7;
  int iVar8;
  char *pcVar9;
  char *pcVar10;
  char *pcVar11;
  char *pcVar12;
  int *piVar13;
  undefined1 uStack00000022;
  undefined4 uStack00000023;
  undefined1 uStack0000002e;
  undefined4 uStack0000002f;
  undefined1 uStack00000035;
  undefined2 uStack00000036;
  undefined1 uStack00000052;
  undefined2 in_stack_00000150;
  undefined1 uStack00000152;
  int in_stack_00001398;
  int in_stack_0000139c;
  int in_stack_000013ac;
  int in_stack_000013b0;
  char *in_stack_0000248c;
  
  FUN_0059c000();
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005faf28;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x000008c8;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005faf28;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x000009c8;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005ea270;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x000006a0;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = s_Check_agains_me__005ea25c;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x00000550;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  puVar3 = (undefined4 *)&stack0x000013a0;
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  iVar8 = 0x32;
  do {
    puVar3[-1] = 1;
    *puVar3 = 1;
    puVar3[1] = 1;
    puVar3 = puVar3 + 0x13;
    iVar8 = iVar8 + -1;
  } while (iVar8 != 0);
  uStack0000002f = 0;
  param_12._3_1_ = 0;
  param_13 = 0;
  in_stack_00000150 = DAT_005ea258;
  uStack00000035 = 0;
  uStack00000152 = DAT_005ea25a;
  param_20 = DAT_005ea258;
  puVar3 = (undefined4 *)&stack0x00000153;
  for (iVar8 = 0x3f; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  *(undefined1 *)puVar3 = 0;
  uStack00000023 = 0;
  stack0x00000027 = 0;
  uStack00000052 = DAT_005ea25a;
  param_10._1_1_ = 0;
  puVar3 = (undefined4 *)&stack0x00000053;
  for (iVar8 = 0x3f; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  *(undefined1 *)puVar3 = 0;
  param_11 = DAT_005ea258;
  puVar3 = (undefined4 *)&stack0x00000acb;
  for (iVar8 = 0x3f; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  iVar8 = 0x32;
  uStack0000002e = DAT_005ea25a;
  *(undefined1 *)puVar3 = 0;
  param_8 = DAT_005ea258;
  uStack00000022 = DAT_005ea25a;
  *(undefined4 *)(extraout_ECX + 0x20) = 0;
  puVar3 = (undefined4 *)&stack0x00001398;
  do {
    *puVar3 = 0xffffffff;
    puVar3 = puVar3 + 0x13;
    iVar8 = iVar8 + -1;
  } while (iVar8 != 0);
  FUN_0059de2f();
  uVar6 = 0xffffffff;
  pcVar9 = s_Region_005da08c;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = (char *)&param_11;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = (char *)&param_8;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  iVar8 = -1;
  pcVar9 = (char *)&param_11;
  do {
    pcVar12 = pcVar9;
    if (iVar8 == 0) break;
    iVar8 = iVar8 + -1;
    pcVar12 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar12;
  } while (cVar1 != '\0');
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = pcVar12 + -1;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  iVar8 = FUN_0059dc1d();
  if (iVar8 == -1) {
    uVar6 = 0xffffffff;
    pcVar9 = s_Cannot_locate_Configuration_file_005ea234;
    do {
      pcVar10 = pcVar9;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar10 = pcVar9 + 1;
      cVar1 = *pcVar9;
      pcVar9 = pcVar10;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar9 = pcVar10 + -uVar6;
    pcVar10 = (char *)&stack0x00000150;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
      pcVar9 = pcVar9 + 4;
      pcVar10 = pcVar10 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar10 = *pcVar9;
      pcVar9 = pcVar9 + 1;
      pcVar10 = pcVar10 + 1;
    }
    uVar6 = 0xffffffff;
    do {
      pcVar9 = in_stack_0000248c;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar9 = in_stack_0000248c + 1;
      cVar1 = *in_stack_0000248c;
      in_stack_0000248c = pcVar9;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    iVar8 = -1;
    pcVar10 = (char *)&stack0x00000150;
    do {
      pcVar12 = pcVar10;
      if (iVar8 == 0) break;
      iVar8 = iVar8 + -1;
      pcVar12 = pcVar10 + 1;
      cVar1 = *pcVar10;
      pcVar10 = pcVar12;
    } while (cVar1 != '\0');
    pcVar9 = pcVar9 + -uVar6;
    pcVar10 = pcVar12 + -1;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
      pcVar9 = pcVar9 + 4;
      pcVar10 = pcVar10 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar10 = *pcVar9;
      pcVar9 = pcVar9 + 1;
      pcVar10 = pcVar10 + 1;
    }
    MessageBoxA((HWND)0x0,(LPCSTR)&stack0x00000150,s_Configuration_file_error_005ea218,0x40010);
    FUN_0059ddac();
    DestroyWindow(DAT_00656614);
  }
  FUN_0059ddac();
  FUN_00482d60();
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005ea20c;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x000006a0;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  FUN_00482e00();
  FUN_00482e40();
  FUN_00482e90();
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005ea1f8;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  iVar8 = -1;
  pcVar9 = (char *)(extraout_ECX + 0x3ee5);
  do {
    pcVar12 = pcVar9;
    if (iVar8 == 0) break;
    iVar8 = iVar8 + -1;
    pcVar12 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar12;
  } while (cVar1 != '\0');
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = pcVar12 + -1;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  FUN_00482e40();
  iVar8 = FUN_00482fb0();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_Sizes_specified_for_map_005ea1c4,s_Map_cfg_Error_005ea1e0,0x40010);
    FUN_0059d8f8();
  }
  iVar8 = FUN_00482fb0();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_Y_Size_specified_for_map_005ea1a8,s_Map_cfg_Error_005ea1e0,0x40010);
    DestroyWindow(DAT_00656614);
  }
  FUN_00482e40();
  param_17 = 5;
  iVar8 = FUN_00482fb0();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_max_water_level_specified_for_005ea170,s_Map_cfg_Error_005ea1e0,
                0x40010);
    FUN_0059d8f8();
  }
  FUN_00482e40();
  param_14 = 0;
  iVar8 = FUN_00482fb0();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_min_water_level_specified_for_005ea138,s_Map_cfg_Error_005ea1e0,
                0x40010);
    FUN_0059d8f8();
  }
  FUN_004f3210();
  FUN_00482e40();
  iVar8 = FUN_00482fb0();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_water_level_specified_for_map_005ea108,s_Map_cfg_Error_005ea1e0,
                0x40010);
    FUN_0059d8f8();
  }
  FUN_00482e40();
  iVar8 = FUN_00483210();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_RedFactor_specified_for_map_005ea0dc,s_Map_cfg_Error_005ea1e0,0x40010
               );
    FUN_0059d8f8();
  }
  FUN_00482e40();
  iVar8 = FUN_00483210();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_GreenFactor_specified_for_map_005ea0ac,s_Map_cfg_Error_005ea1e0,
                0x40010);
    FUN_0059d8f8();
  }
  FUN_00482e40();
  iVar8 = FUN_00483210();
  if (iVar8 == 0) {
    MessageBoxA((HWND)0x0,s_No_BlueFactor_specified_for_map_005ea080,s_Map_cfg_Error_005ea1e0,
                0x40010);
    FUN_0059d8f8();
  }
  if ((DAT_00689920 == 2) || (DAT_00689920 == 1)) {
    DAT_006c5c84 = DAT_006c3780;
    DAT_006a2890 = DAT_006c3784;
  }
  else {
    FUN_00482e40();
    iVar8 = FUN_00482fb0();
    if (iVar8 == 0) {
      MessageBoxA((HWND)0x0,s_No_MaxRegionofPower_specified_fo_005ea044,s_Map_cfg_Error_005ea1e0,
                  0x40010);
    }
    DAT_006c5c84 = 0;
    FUN_00482e40();
    FUN_00482fb0();
  }
  FUN_00482e40();
  FUN_00482fb0();
  if (DAT_00657cdc == 0) {
    DAT_00657cdc = 4000;
  }
  param_16 = 0;
  FUN_00482e40();
  FUN_00482fb0();
  if ((int)param_16 < 2) {
    DAT_00689999 = 3;
  }
  else {
    DAT_00689999 = param_16;
    if (2 < (int)param_16) {
      DAT_00689999 = 3;
    }
  }
  param_15 = 0;
  FUN_00482e40();
  FUN_00482fb0();
  if ((int)param_15 < 1) {
    if (DAT_0068999d == 0) {
LAB_0052dc57:
      DAT_00689991 = 1;
      goto LAB_0052dc61;
    }
    if (DAT_0068999d == 1) {
      DAT_00689995 = 1;
      goto LAB_0052dc61;
    }
  }
  else {
    if (DAT_0068999d == 0) {
      uVar6 = *(uint *)(DAT_00689955 + 0xc);
      uVar7 = param_15;
      if (uVar6 <= param_15) {
        uVar7 = uVar6;
      }
      if (uVar7 < 2) goto LAB_0052dc57;
      DAT_00689991 = param_15;
      if (uVar6 <= param_15) {
        DAT_00689991 = uVar6;
      }
    }
    else {
      if (DAT_0068999d != 1) goto LAB_0052dc67;
      uVar6 = param_15;
      if (DAT_00689999 <= param_15) {
        uVar6 = DAT_00689999;
      }
      if (uVar6 < 2) {
        DAT_00689995 = 1;
      }
      else {
        DAT_00689995 = param_15;
        if (DAT_00689999 <= param_15) {
          DAT_00689995 = DAT_00689999;
        }
      }
    }
LAB_0052dc61:
    _DAT_00689965 = 0;
  }
LAB_0052dc67:
  param_13 = 0x32;
  uStack00000035 = 0;
  uStack00000036 = 0;
  FUN_00482e40();
  FUN_00482fb0();
  iVar8 = CONCAT22(uStack00000036,CONCAT11(uStack00000035,param_13));
  if (0 < iVar8) {
    _DAT_006c5c94 = iVar8;
  }
  FUN_00482e40();
  FUN_00482e90();
  iVar8 = __strcmpi(&stack0x0000024c,s_NORTH_005e9fd4);
  if (iVar8 == 0) {
    DAT_006a291c = 0;
  }
  else {
    iVar8 = __strcmpi(&stack0x0000024c,s_NORTHEAST_005e9fc8);
    if (iVar8 == 0) {
      DAT_006a291c = 1;
    }
    else {
      iVar8 = __strcmpi(&stack0x0000024c,&DAT_005e9fc0);
      if (iVar8 == 0) {
        DAT_006a291c = 2;
      }
      else {
        iVar8 = __strcmpi(&stack0x0000024c,s_SOUTHEAST_005e9fb4);
        if (iVar8 == 0) {
          DAT_006a291c = 3;
        }
        else {
          iVar8 = __strcmpi(&stack0x0000024c,s_SOUTH_005e9fac);
          if (iVar8 == 0) {
            DAT_006a291c = 4;
          }
          else {
            iVar8 = __strcmpi(&stack0x0000024c,s_SOUTHWEST_005e9fa0);
            if (iVar8 == 0) {
              DAT_006a291c = 5;
            }
            else {
              iVar8 = __strcmpi(&stack0x0000024c,&DAT_005e9f98);
              if (iVar8 == 0) {
                DAT_006a291c = 6;
              }
              else {
                iVar8 = __strcmpi(&stack0x0000024c,s_NORTHWEST_005e9f8c);
                if (iVar8 == 0) {
                  DAT_006a291c = 7;
                }
              }
            }
          }
        }
      }
    }
  }
  FUN_00482e40();
  FUN_004831c0();
  FUN_00482e40();
  iVar8 = FUN_00482fb0();
  param_3 = 0;
  while (iVar8 != 0) {
    *(undefined4 *)(&stack0x00000bc4 + param_3 * 0x14) = 0x32;
    iVar8 = FUN_00482fb0();
    if (iVar8 == 0) {
      MessageBoxA((HWND)0x0,s_Rotation_value_missing_in_specif_005e9f2c,s_Map_cfg_Error_005ea1e0,
                  0x40010);
      FUN_0059d8f8();
LAB_0052dfbc:
      MessageBoxA((HWND)0x0,s_X_placement_value_missing_005e9f10,s_Map_cfg_Error_005ea1e0,0x40010);
      FUN_0059d8f8();
LAB_0052dfd5:
      MessageBoxA((HWND)0x0,s_Y_placement_value_missing_005e9ef4,s_Map_cfg_Error_005ea1e0,0x40010);
      FUN_0059d8f8();
      iVar8 = extraout_ECX;
      param_1 = param_3;
      goto LAB_0052dfee;
    }
    *(undefined4 *)(&stack0x00000bc8 + param_3 * 0x14) = 0x32;
    iVar8 = FUN_00482fb0();
    if (iVar8 == 0) goto LAB_0052dfbc;
    *(undefined4 *)(&stack0x00000bcc + param_3 * 0x14) = 0x32;
    iVar8 = FUN_00482fb0();
    if (iVar8 == 0) goto LAB_0052dfd5;
    *(undefined4 *)(&stack0x00000bd0 + param_3 * 0x14) = 0x32;
    iVar8 = FUN_00482fb0();
    param_3 = param_3 + 1;
  }
  FUN_00482e40();
  iVar8 = FUN_00482fb0();
  param_1 = 0;
  while (iVar8 != 0) {
    *(undefined4 *)(&stack0x00000fac + param_1 * 0x14) = 0x32;
    iVar4 = FUN_00482fb0();
    iVar8 = param_3;
    if (iVar4 == 0) goto LAB_0052dfee;
    *(undefined4 *)(&stack0x00000fbc + param_1 * 0x14) = 0x32;
    iVar8 = FUN_00482fb0();
    param_1 = param_1 + 1;
  }
LAB_0052e007:
  FUN_00482e40();
  pcVar9 = (char *)(param_2 + 0x3de5);
  FUN_00482e90();
  if (DAT_006e1ed0 == 0) {
    uVar6 = 0xffffffff;
    pcVar10 = pcVar9;
    do {
      pcVar12 = pcVar10;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar12 = pcVar10 + 1;
      cVar1 = *pcVar10;
      pcVar10 = pcVar12;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar10 = pcVar12 + -uVar6;
    pcVar12 = (char *)&param_19;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar12 = *(undefined4 *)pcVar10;
      pcVar10 = pcVar10 + 4;
      pcVar12 = pcVar12 + 4;
    }
  }
  else {
    uVar6 = 0xffffffff;
    pcVar10 = &DAT_005e12e4;
    do {
      pcVar12 = pcVar10;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar12 = pcVar10 + 1;
      cVar1 = *pcVar10;
      pcVar10 = pcVar12;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar10 = pcVar12 + -uVar6;
    pcVar12 = (char *)&param_19;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar12 = *(undefined4 *)pcVar10;
      pcVar10 = pcVar10 + 4;
      pcVar12 = pcVar12 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar12 = *pcVar10;
      pcVar10 = pcVar10 + 1;
      pcVar12 = pcVar12 + 1;
    }
    uVar6 = 0xffffffff;
    pcVar10 = pcVar9;
    do {
      pcVar12 = pcVar10;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar12 = pcVar10 + 1;
      cVar1 = *pcVar10;
      pcVar10 = pcVar12;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    iVar8 = -1;
    pcVar10 = (char *)&param_19;
    do {
      pcVar11 = pcVar10;
      if (iVar8 == 0) break;
      iVar8 = iVar8 + -1;
      pcVar11 = pcVar10 + 1;
      cVar1 = *pcVar10;
      pcVar10 = pcVar11;
    } while (cVar1 != '\0');
    pcVar10 = pcVar12 + -uVar6;
    pcVar12 = pcVar11 + -1;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar12 = *(undefined4 *)pcVar10;
      pcVar10 = pcVar10 + 4;
      pcVar12 = pcVar12 + 4;
    }
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar12 = *pcVar10;
    pcVar10 = pcVar10 + 1;
    pcVar12 = pcVar12 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar10 = (char *)&param_19;
  do {
    pcVar12 = pcVar10;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar12 = pcVar10 + 1;
    cVar1 = *pcVar10;
    pcVar10 = pcVar12;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar10 = pcVar12 + -uVar6;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar9 = *(undefined4 *)pcVar10;
    pcVar10 = pcVar10 + 4;
    pcVar9 = pcVar9 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar9 = *pcVar10;
    pcVar10 = pcVar10 + 1;
    pcVar9 = pcVar9 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005d9058;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  iVar8 = -1;
  pcVar9 = (char *)&param_19;
  do {
    pcVar12 = pcVar9;
    if (iVar8 == 0) break;
    iVar8 = iVar8 + -1;
    pcVar12 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar12;
  } while (cVar1 != '\0');
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = pcVar12 + -1;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = (char *)&param_19;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x00002384;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  iVar8 = 0;
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  if (0 < param_3) {
    do {
      iVar4 = FUN_0052e9d0();
      if (iVar4 != 0) {
        FUN_0052ed20();
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 < param_3);
  }
  iVar8 = 0;
  if (0 < param_1) {
    do {
      iVar4 = FUN_0052e9d0();
      if (iVar4 != 0) {
        FUN_0052ed20();
      }
      iVar8 = iVar8 + 1;
    } while (iVar8 < param_1);
  }
  iVar4 = 0;
  *(int *)(param_2 + 0x24) = in_stack_00001398 / in_stack_000013ac;
  *(int *)(param_2 + 0x28) = in_stack_0000139c / in_stack_000013b0;
  iVar8 = param_1 + param_3;
  if (0 < iVar8) {
    do {
      iVar5 = iVar4 * 0x4c;
      if (*(int *)(&stack0x00001394 + iVar5) != -1) {
        if ((((*(int *)(&stack0x00001398 + iVar5) == 0) || (*(int *)(&stack0x0000139c + iVar5) == 0)
             ) || (*(int *)(&stack0x000013ac + iVar5) == 0)) ||
           (*(int *)(&stack0x000013b0 + iVar5) == 0)) {
          FUN_0059de2f();
          uVar6 = 0xffffffff;
          pcVar9 = s_Map_Section__005e9ec8;
          do {
            pcVar10 = pcVar9;
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            pcVar10 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar10;
          } while (cVar1 != '\0');
          uVar6 = ~uVar6;
          pcVar9 = pcVar10 + -uVar6;
          pcVar10 = &stack0x0000014c;
          for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
            *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
            pcVar9 = pcVar9 + 4;
            pcVar10 = pcVar10 + 4;
          }
          for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
            *pcVar10 = *pcVar9;
            pcVar9 = pcVar9 + 1;
            pcVar10 = pcVar10 + 1;
          }
          uVar6 = 0xffffffff;
          pcVar9 = (char *)&param_7;
          do {
            pcVar10 = pcVar9;
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            pcVar10 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar10;
          } while (cVar1 != '\0');
          uVar6 = ~uVar6;
          iVar5 = -1;
          pcVar9 = &stack0x0000014c;
          do {
            pcVar12 = pcVar9;
            if (iVar5 == 0) break;
            iVar5 = iVar5 + -1;
            pcVar12 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar12;
          } while (cVar1 != '\0');
          pcVar9 = pcVar10 + -uVar6;
          pcVar10 = pcVar12 + -1;
          for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
            *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
            pcVar9 = pcVar9 + 4;
            pcVar10 = pcVar10 + 4;
          }
          for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
            *pcVar10 = *pcVar9;
            pcVar9 = pcVar9 + 1;
            pcVar10 = pcVar10 + 1;
          }
          uVar6 = 0xffffffff;
          pcVar9 = s_has_no_size_set__Shall_I_continu_005e9e8c;
          do {
            pcVar10 = pcVar9;
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            pcVar10 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar10;
          } while (cVar1 != '\0');
          uVar6 = ~uVar6;
          iVar5 = -1;
          pcVar9 = &stack0x0000014c;
          do {
            pcVar12 = pcVar9;
            if (iVar5 == 0) break;
            iVar5 = iVar5 + -1;
            pcVar12 = pcVar9 + 1;
            cVar1 = *pcVar9;
            pcVar9 = pcVar12;
          } while (cVar1 != '\0');
          pcVar9 = pcVar10 + -uVar6;
          pcVar10 = pcVar12 + -1;
          for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
            *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
            pcVar9 = pcVar9 + 4;
            pcVar10 = pcVar10 + 4;
          }
          for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
            *pcVar10 = *pcVar9;
            pcVar9 = pcVar9 + 1;
            pcVar10 = pcVar10 + 1;
          }
          iVar5 = MessageBoxA((HWND)0x0,&stack0x0000014c,s_Mapping_Error_005e9eb8,0x40014);
          if (iVar5 == 6) {
            *(undefined4 *)(&stack0x00001394 + iVar4 * 0x4c) = 0xffffffff;
          }
          else {
            DestroyWindow(DAT_00656614);
          }
        }
        iVar5 = iVar4 * 0x4c;
        if ((*(int *)(&stack0x00001398 + iVar5) / *(int *)(&stack0x000013ac + iVar5) !=
             *(int *)(param_2 + 0x24)) ||
           (*(int *)(&stack0x0000139c + iVar5) / *(int *)(&stack0x000013b0 + iVar5) !=
            *(int *)(param_2 + 0x28))) {
          MessageBoxA((HWND)0x0,s_CAN_NOT_MAKE_MAP_Map_size_found_i_005e9e4c,
                      s_Mapping_Error_005e9eb8,0x40010);
        }
      }
      iVar4 = iVar4 + 1;
    } while (iVar4 < iVar8);
  }
  iVar4 = 0;
  if (0 < iVar8) {
    piVar13 = (int *)&stack0x000013bc;
    do {
      if (piVar13[-10] != -1) {
        iVar5 = piVar13[-2];
        if (iVar4 < iVar5) {
          iVar4 = iVar5;
        }
        iVar2 = piVar13[-1];
        if (iVar4 < iVar2) {
          iVar4 = iVar2;
        }
        if (iVar4 < *piVar13) {
          iVar4 = *piVar13;
        }
        if (iVar4 < piVar13[1]) {
          iVar4 = piVar13[1];
        }
        if (iVar4 < piVar13[2]) {
          iVar4 = iVar5;
        }
        if (iVar4 < piVar13[3]) {
          iVar4 = iVar2;
        }
        if (iVar4 < piVar13[4]) {
          iVar4 = piVar13[4];
        }
        if (iVar4 < piVar13[5]) {
          iVar4 = piVar13[5];
        }
      }
      piVar13 = piVar13 + 0x13;
      iVar8 = iVar8 + -1;
    } while (iVar8 != 0);
  }
  *(int *)(param_2 + 0xc) = iVar4 + 1;
  *(int *)(param_2 + 0x10) = iVar4 + 1;
  *(int *)(param_2 + 0x14) = iVar4 + 2;
  *(int *)(param_2 + 0x1c) = iVar4 + 4;
  iVar8 = 0;
  *(int *)(param_2 + 0x18) = iVar4 + 3;
  if (0 < param_3) {
    piVar13 = (int *)&stack0x000013b0;
    param_18 = param_3;
    iVar8 = param_3;
    do {
      iVar4 = 0;
      if (0 < *piVar13) {
        do {
          iVar8 = 0;
          if (0 < piVar13[-1]) {
            do {
              FUN_0052edf0();
              iVar8 = iVar8 + 1;
            } while (iVar8 < piVar13[-1]);
          }
          iVar4 = iVar4 + 1;
          iVar8 = param_18;
        } while (iVar4 < *piVar13);
      }
      piVar13 = piVar13 + 0x13;
      param_3 = param_3 + -1;
    } while (param_3 != 0);
  }
  if (0 < param_1) {
    param_4 = (int *)(&stack0x000013b0 + iVar8 * 0x4c);
    do {
      iVar8 = 0;
      if (0 < *param_4) {
        do {
          iVar4 = 0;
          if (0 < param_4[-1]) {
            do {
              FUN_0052edf0();
              iVar4 = iVar4 + 1;
            } while (iVar4 < param_4[-1]);
          }
          iVar8 = iVar8 + 1;
        } while (iVar8 < *param_4);
      }
      param_4 = param_4 + 0x13;
      param_1 = param_1 + -1;
    } while (param_1 != 0);
  }
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005e9e40;
  do {
    pcVar10 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar10 + -uVar6;
  pcVar10 = &stack0x0000069c;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar10 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar10 = pcVar10 + 1;
  }
  iVar8 = 0x570;
  do {
    FUN_0059c1fd();
    FUN_00482e40();
    FUN_004831c0();
    if (param_5 < -99) {
      param_5 = -100;
    }
    else if (99 < param_5) {
      param_5 = 100;
    }
    *(int *)(&DAT_006c5490 + iVar8) = param_5;
    iVar4 = DAT_006b1254;
    iVar8 = iVar8 + 4;
  } while (iVar8 < 0x5e0);
  iVar8 = 0;
  if (0 < DAT_006b1254) {
    piVar13 = &DAT_006c5a70;
    do {
      FUN_0059c1fd();
      FUN_00482e40();
      FUN_004831c0();
      if (param_5 < -99) {
        param_5 = -100;
      }
      else if (99 < param_5) {
        param_5 = 100;
      }
      *piVar13 = param_5;
      iVar8 = iVar8 + 1;
      piVar13 = piVar13 + 1;
    } while (iVar8 < iVar4);
  }
  return;
LAB_0052dfee:
  param_3 = iVar8;
  MessageBoxA((HWND)0x0,s_Max_placement_value_missing_005e9ed8,s_Map_cfg_Error_005ea1e0,0x40010);
  FUN_0059d8f8();
  goto LAB_0052e007;
}

