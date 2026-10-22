
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00586660(int param_1,int *param_2,char *param_3)

{
  char cVar1;
  char cVar2;
  char cVar3;
  DWORD DVar4;
  ushort *puVar5;
  uint uVar6;
  ushort uVar7;
  uint uVar8;
  uint uVar9;
  int iVar10;
  undefined4 uVar11;
  uint uVar12;
  uint uVar13;
  char *pcVar14;
  double *pdVar15;
  char *pcVar16;
  char *pcVar17;
  float10 fVar18;
  uint local_c4c;
  int local_c48;
  int local_c44;
  ushort *local_c30;
  int local_c2c;
  char *local_c24;
  double local_814 [256];
  void *local_14;
  undefined1 *puStack_10;
  undefined4 local_c;
  
  local_c = 0xffffffff;
  puStack_10 = &LAB_005c2e0b;
  local_14 = ExceptionList;
  ExceptionList = &local_14;
  GetCurrentDirectoryA(0x100,&DAT_006f56ac);
  uVar8 = 0xffffffff;
  pcVar14 = &DAT_006f56ac;
  do {
    pcVar17 = pcVar14;
    if (uVar8 == 0) break;
    uVar8 = uVar8 - 1;
    pcVar17 = pcVar14 + 1;
    cVar1 = *pcVar14;
    pcVar14 = pcVar17;
  } while (cVar1 != '\0');
  uVar8 = ~uVar8;
  pcVar14 = pcVar17 + -uVar8;
  pcVar17 = (char *)&DAT_006f58ac;
  for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
    *(undefined4 *)pcVar17 = *(undefined4 *)pcVar14;
    pcVar14 = pcVar14 + 4;
    pcVar17 = pcVar17 + 4;
  }
  for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
    *pcVar17 = *pcVar14;
    pcVar14 = pcVar14 + 1;
    pcVar17 = pcVar17 + 1;
  }
  uVar8 = 0xffffffff;
  pcVar14 = (char *)&DAT_006f58ac;
  do {
    if (uVar8 == 0) break;
    uVar8 = uVar8 - 1;
    cVar1 = *pcVar14;
    pcVar14 = pcVar14 + 1;
  } while (cVar1 != '\0');
  if (((~uVar8 != 1) && ((&DAT_006f58aa)[~uVar8] != '\\')) && ((&DAT_006f58aa)[~uVar8] != '/')) {
    uVar8 = 0xffffffff;
    pcVar14 = &DAT_005d9058;
    do {
      pcVar17 = pcVar14;
      if (uVar8 == 0) break;
      uVar8 = uVar8 - 1;
      pcVar17 = pcVar14 + 1;
      cVar1 = *pcVar14;
      pcVar14 = pcVar17;
    } while (cVar1 != '\0');
    uVar8 = ~uVar8;
    iVar10 = -1;
    pcVar14 = (char *)&DAT_006f58ac;
    do {
      pcVar16 = pcVar14;
      if (iVar10 == 0) break;
      iVar10 = iVar10 + -1;
      pcVar16 = pcVar14 + 1;
      cVar1 = *pcVar14;
      pcVar14 = pcVar16;
    } while (cVar1 != '\0');
    pcVar14 = pcVar17 + -uVar8;
    pcVar17 = pcVar16 + -1;
    for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
      *(undefined4 *)pcVar17 = *(undefined4 *)pcVar14;
      pcVar14 = pcVar14 + 4;
      pcVar17 = pcVar17 + 4;
    }
    for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
      *pcVar17 = *pcVar14;
      pcVar14 = pcVar14 + 1;
      pcVar17 = pcVar17 + 1;
    }
  }
  uVar8 = 0xffffffff;
  pcVar14 = s_cfg_tables_cfg_005f194c;
  do {
    pcVar17 = pcVar14;
    if (uVar8 == 0) break;
    uVar8 = uVar8 - 1;
    pcVar17 = pcVar14 + 1;
    cVar1 = *pcVar14;
    pcVar14 = pcVar17;
  } while (cVar1 != '\0');
  uVar8 = ~uVar8;
  iVar10 = -1;
  pcVar14 = (char *)&DAT_006f58ac;
  do {
    pcVar16 = pcVar14;
    if (iVar10 == 0) break;
    iVar10 = iVar10 + -1;
    pcVar16 = pcVar14 + 1;
    cVar1 = *pcVar14;
    pcVar14 = pcVar16;
  } while (cVar1 != '\0');
  pcVar14 = pcVar17 + -uVar8;
  pcVar17 = pcVar16 + -1;
  for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
    *(undefined4 *)pcVar17 = *(undefined4 *)pcVar14;
    pcVar14 = pcVar14 + 4;
    pcVar17 = pcVar17 + 4;
  }
  for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
    *pcVar17 = *pcVar14;
    pcVar14 = pcVar14 + 1;
    pcVar17 = pcVar17 + 1;
  }
  DVar4 = GetPrivateProfileStringA
                    (s_STONE_EFFECT_005f1934,s_Reducer_005f1944,&DAT_005faf28,&DAT_006f5bac,0x100,
                     (LPCSTR)&DAT_006f58ac);
  if (DVar4 != 0) {
    FUN_0059d4be(&DAT_006f5bac);
  }
  DVar4 = GetPrivateProfileStringA
                    (s_STONE_EFFECT_005f1934,s_SpecalValue_005f1928,&DAT_005faf28,&DAT_006f5bac,
                     0x100,(LPCSTR)&DAT_006f58ac);
  if (DVar4 != 0) {
    FUN_0059d4be(&DAT_006f5bac);
  }
  DVar4 = GetPrivateProfileStringA
                    (s_STONE_EFFECT_005f1934,s__Darkest_005f191f + 1,&DAT_005faf28,&DAT_006f5bac,
                     0x100,(LPCSTR)&DAT_006f58ac);
  if (DVar4 != 0) {
    FUN_0059d4be(&DAT_006f5bac);
  }
  FUN_0054e170(0x499602d2);
  local_c = 0;
  FUN_0054e170(2000);
  iVar10 = __ftol();
  local_c4c = 0x100;
  pdVar15 = local_814;
  do {
    if (iVar10 == 1) {
      local_c44 = 1;
    }
    else {
      uVar8 = FUN_0054e200();
      local_c44 = (int)((longlong)(ulonglong)(uVar8 & 0x7fffffff) % (longlong)(iVar10 + -1)) + 1;
    }
    local_c4c = local_c4c + -1;
    *pdVar15 = (double)local_c44;
    pdVar15 = pdVar15 + 1;
  } while (local_c4c != 0);
  if (((((param_1 != 2) && (param_1 != 4)) &&
       ((param_1 != 8 && ((param_1 != 0x10 && (param_1 != 0x20)))))) && (param_1 != 0x40)) &&
     ((param_1 != 0x80 && (param_1 != 0x100)))) {
    ExceptionList = local_14;
    return 0;
  }
  uVar8 = param_1 - 1U >> 1;
  iVar10 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar10;
  if (iVar10 == 0) {
    ExceptionList = local_14;
    return 0;
  }
  switch(param_1) {
  case 2:
    uVar11 = 7;
    break;
  default:
    uVar11 = 0;
    break;
  case 4:
    uVar11 = 6;
    break;
  case 8:
    uVar11 = 5;
    break;
  case 0x10:
    uVar11 = 4;
    break;
  case 0x20:
    uVar11 = 3;
    break;
  case 0x40:
    uVar11 = 2;
    break;
  case 0x80:
    uVar11 = 1;
  }
  *(undefined4 *)(iVar10 + 4) = uVar11;
  *(uint *)(*param_2 + 8) = uVar8;
  *(undefined4 *)*param_2 = 0;
  fVar18 = (float10)FUN_0059bcc0();
  _DAT_005f18e8 = (double)fVar18;
  fVar18 = (float10)FUN_0059bcc0();
  _DAT_005f18f0 = (double)fVar18;
  puVar5 = (ushort *)(*param_2 + 0xc);
  uVar9 = uVar8;
  if (uVar8 != 0) {
    for (; uVar9 != 0; uVar9 = uVar9 - 1) {
      FUN_0059bcc0();
      local_c24 = param_3;
      local_c2c = 0x100;
      local_c30 = puVar5;
      do {
        cVar1 = *local_c24;
        cVar2 = local_c24[1];
        cVar3 = local_c24[2];
        local_c24 = local_c24 + 3;
        if (cVar1 != '\0') {
          __ftol();
        }
        if (cVar2 != '\0') {
          __ftol();
        }
        if (cVar3 != '\0') {
          __ftol();
        }
        uVar6 = __ftol();
        if ((int)uVar6 < 0xff) {
          uVar12 = uVar6;
          if ((int)uVar6 < 1) {
            uVar12 = 0;
            goto LAB_00586b7b;
          }
LAB_00586b82:
          if ((int)uVar6 < 1) {
            uVar13 = 0;
            goto LAB_00586b96;
          }
          uVar13 = uVar6;
          if (0xfe < (int)uVar6) goto LAB_00586b8d;
LAB_00586b9d:
          if ((int)uVar6 < 1) {
            uVar6 = 0;
          }
          else if (0xfe < (int)uVar6) goto LAB_00586ba8;
        }
        else {
          uVar12 = 0xff;
LAB_00586b7b:
          if ((int)uVar6 < 0xff) goto LAB_00586b82;
LAB_00586b8d:
          uVar13 = 0xff;
LAB_00586b96:
          if ((int)uVar6 < 0xff) goto LAB_00586b9d;
LAB_00586ba8:
          uVar6 = 0xff;
        }
        if (0xff < (ushort)uVar12) {
          uVar12 = 0xff;
        }
        if (0xff < (ushort)uVar13) {
          uVar13 = 0xff;
        }
        if (0xff < (ushort)uVar6) {
          uVar6 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar7 = (ushort)(((uVar12 & 0xfff8) << 5 | uVar13 & 0xfc) << 3);
        }
        else {
          uVar7 = (ushort)(((uVar12 & 0xf8) << 5 | uVar13 & 0xf8) << 2);
        }
        puVar5 = local_c30 + 1;
        *local_c30 = uVar7 | (ushort)(uVar6 >> 3) & 0x1f;
        local_c2c = local_c2c + -1;
        local_c30 = puVar5;
      } while (local_c2c != 0);
    }
  }
  local_c48 = 0x100;
  do {
    uVar9 = __ftol();
    if ((int)uVar9 < 0xff) {
      uVar6 = uVar9;
      if ((int)uVar9 < 1) {
        uVar6 = 0;
        goto LAB_00586cf8;
      }
LAB_00586cff:
      if ((int)uVar9 < 1) {
        uVar12 = 0;
        goto LAB_00586d13;
      }
      uVar12 = uVar9;
      if (0xfe < (int)uVar9) goto LAB_00586d0a;
LAB_00586d1a:
      if ((int)uVar9 < 1) {
        uVar9 = 0;
      }
      else if (0xfe < (int)uVar9) goto LAB_00586d25;
    }
    else {
      uVar6 = 0xff;
LAB_00586cf8:
      if ((int)uVar9 < 0xff) goto LAB_00586cff;
LAB_00586d0a:
      uVar12 = 0xff;
LAB_00586d13:
      if ((int)uVar9 < 0xff) goto LAB_00586d1a;
LAB_00586d25:
      uVar9 = 0xff;
    }
    if (0xff < (ushort)uVar6) {
      uVar6 = 0xff;
    }
    if (0xff < (ushort)uVar12) {
      uVar12 = 0xff;
    }
    if (0xff < (ushort)uVar9) {
      uVar9 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar7 = (ushort)(((uVar6 & 0xfff8) << 5 | uVar12 & 0xfc) << 3);
    }
    else {
      uVar7 = (ushort)(((uVar6 & 0xf8) << 5 | uVar12 & 0xf8) << 2);
    }
    local_c30 = puVar5 + 1;
    *puVar5 = uVar7 | (ushort)(uVar9 >> 3) & 0x1f;
    local_c48 = local_c48 + -1;
    puVar5 = local_c30;
    if (local_c48 == 0) {
      if ((uVar8 != param_1 - 2U) && (local_c4c = 1, uVar8 != 0)) {
        do {
          local_c48 = 0x100;
          do {
            uVar9 = __ftol();
            if ((int)uVar9 < 0xff) {
              uVar6 = uVar9;
              if ((int)uVar9 < 1) {
                uVar6 = 0;
                goto LAB_00586eb8;
              }
LAB_00586ebf:
              if ((int)uVar9 < 1) {
                uVar12 = 0;
                goto LAB_00586ed3;
              }
              uVar12 = uVar9;
              if (0xfe < (int)uVar9) goto LAB_00586eca;
LAB_00586eda:
              if ((int)uVar9 < 1) {
                uVar9 = 0;
              }
              else if (0xfe < (int)uVar9) goto LAB_00586ee5;
            }
            else {
              uVar6 = 0xff;
LAB_00586eb8:
              if ((int)uVar9 < 0xff) goto LAB_00586ebf;
LAB_00586eca:
              uVar12 = 0xff;
LAB_00586ed3:
              if ((int)uVar9 < 0xff) goto LAB_00586eda;
LAB_00586ee5:
              uVar9 = 0xff;
            }
            if (0xff < (ushort)uVar6) {
              uVar6 = 0xff;
            }
            if (0xff < (ushort)uVar12) {
              uVar12 = 0xff;
            }
            if (0xff < (ushort)uVar9) {
              uVar9 = 0xff;
            }
            if (DAT_006e1f88 == 0) {
              uVar7 = (ushort)(((uVar6 & 0xfff8) << 5 | uVar12 & 0xfc) << 3);
            }
            else {
              uVar7 = (ushort)(((uVar6 & 0xf8) << 5 | uVar12 & 0xf8) << 2);
            }
            *local_c30 = uVar7 | (ushort)(uVar9 >> 3) & 0x1f;
            local_c30 = local_c30 + 1;
            local_c48 = local_c48 + -1;
          } while (local_c48 != 0);
          local_c4c = local_c4c + 1;
        } while (local_c4c <= uVar8);
      }
      ExceptionList = local_14;
      return 1;
    }
  } while( true );
}

