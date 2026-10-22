
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00587e70(int param_1,int *param_2,char *param_3)

{
  char cVar1;
  char cVar2;
  char cVar3;
  byte bVar4;
  DWORD DVar5;
  char *pcVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  ushort uVar10;
  uint uVar11;
  uint uVar12;
  int iVar13;
  undefined4 uVar14;
  char *pcVar15;
  char *pcVar16;
  float10 fVar17;
  ushort local_40;
  ushort *local_30;
  uint local_2c;
  int local_28;
  int local_24;
  
  GetCurrentDirectoryA(0x100,&DAT_006f5eb0);
  uVar11 = 0xffffffff;
  pcVar6 = &DAT_006f5eb0;
  do {
    pcVar16 = pcVar6;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar16 = pcVar6 + 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar16;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  pcVar6 = pcVar16 + -uVar11;
  pcVar16 = (char *)&DAT_006f59ac;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar16 = *(undefined4 *)pcVar6;
    pcVar6 = pcVar6 + 4;
    pcVar16 = pcVar16 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar16 = *pcVar6;
    pcVar6 = pcVar6 + 1;
    pcVar16 = pcVar16 + 1;
  }
  uVar11 = 0xffffffff;
  pcVar6 = (char *)&DAT_006f59ac;
  do {
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar6 + 1;
  } while (cVar1 != '\0');
  if (((~uVar11 != 1) && ((&DAT_006f59aa)[~uVar11] != '\\')) && ((&DAT_006f59aa)[~uVar11] != '/')) {
    uVar11 = 0xffffffff;
    pcVar6 = &DAT_005d9058;
    do {
      pcVar16 = pcVar6;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar16 = pcVar6 + 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar16;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    iVar13 = -1;
    pcVar6 = (char *)&DAT_006f59ac;
    do {
      pcVar15 = pcVar6;
      if (iVar13 == 0) break;
      iVar13 = iVar13 + -1;
      pcVar15 = pcVar6 + 1;
      cVar1 = *pcVar6;
      pcVar6 = pcVar15;
    } while (cVar1 != '\0');
    pcVar6 = pcVar16 + -uVar11;
    pcVar16 = pcVar15 + -1;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar16 = *(undefined4 *)pcVar6;
      pcVar6 = pcVar6 + 4;
      pcVar16 = pcVar16 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar16 = *pcVar6;
      pcVar6 = pcVar6 + 1;
      pcVar16 = pcVar16 + 1;
    }
  }
  uVar11 = 0xffffffff;
  pcVar6 = s_cfg_tables_cfg_005f194c;
  do {
    pcVar16 = pcVar6;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar16 = pcVar6 + 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar16;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  iVar13 = -1;
  pcVar6 = (char *)&DAT_006f59ac;
  do {
    pcVar15 = pcVar6;
    if (iVar13 == 0) break;
    iVar13 = iVar13 + -1;
    pcVar15 = pcVar6 + 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar15;
  } while (cVar1 != '\0');
  pcVar6 = pcVar16 + -uVar11;
  pcVar16 = pcVar15 + -1;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar16 = *(undefined4 *)pcVar6;
    pcVar6 = pcVar6 + 4;
    pcVar16 = pcVar16 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar16 = *pcVar6;
    pcVar6 = pcVar6 + 1;
    pcVar16 = pcVar16 + 1;
  }
  DVar5 = GetPrivateProfileStringA
                    (s_REDTINT_EFFECT_005f1984,s_Reducer_005f1944,&DAT_005faf28,&DAT_006f5fb0,0x100,
                     (LPCSTR)&DAT_006f59ac);
  if (DVar5 != 0) {
    FUN_0059d4be(&DAT_006f5fb0);
  }
  if (((((param_1 != 2) && (param_1 != 4)) &&
       ((param_1 != 8 && ((param_1 != 0x10 && (param_1 != 0x20)))))) && (param_1 != 0x40)) &&
     ((param_1 != 0x80 && (param_1 != 0x100)))) {
    return 0;
  }
  uVar11 = param_1 - 1U >> 1;
  iVar13 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar13;
  if (iVar13 != 0) {
    switch(param_1) {
    case 2:
      uVar14 = 7;
      break;
    default:
      uVar14 = 0;
      break;
    case 4:
      uVar14 = 6;
      break;
    case 8:
      uVar14 = 5;
      break;
    case 0x10:
      uVar14 = 4;
      break;
    case 0x20:
      uVar14 = 3;
      break;
    case 0x40:
      uVar14 = 2;
      break;
    case 0x80:
      uVar14 = 1;
    }
    *(undefined4 *)(iVar13 + 4) = uVar14;
    *(uint *)(*param_2 + 8) = uVar11;
    *(undefined4 *)*param_2 = 0;
    fVar17 = (float10)FUN_0059bcc0();
    _DAT_005f18e8 = (double)fVar17;
    fVar17 = (float10)FUN_0059bcc0();
    _DAT_005f18f0 = (double)fVar17;
    local_30 = (ushort *)(*param_2 + 0xc);
    uVar12 = uVar11;
    if (uVar11 != 0) {
      for (; uVar12 != 0; uVar12 = uVar12 - 1) {
        FUN_0059bcc0();
        local_2c = 0x100;
        pcVar6 = param_3;
        do {
          cVar1 = *pcVar6;
          cVar2 = pcVar6[1];
          cVar3 = pcVar6[2];
          pcVar6 = pcVar6 + 3;
          if (cVar1 != '\0') {
            __ftol();
          }
          if (cVar2 != '\0') {
            __ftol();
          }
          if (cVar3 != '\0') {
            __ftol();
          }
          uVar7 = __ftol();
          uVar8 = __ftol();
          uVar9 = __ftol();
          if ((int)uVar7 < 0xff) {
            if ((int)uVar7 < 1) {
              uVar7 = 0;
            }
          }
          else {
            uVar7 = 0xff;
          }
          if ((int)uVar8 < 0xff) {
            if ((int)uVar8 < 1) {
              uVar8 = 0;
            }
          }
          else {
            uVar8 = 0xff;
          }
          if ((int)uVar9 < 0xff) {
            if ((int)uVar9 < 1) {
              uVar9 = 0;
            }
          }
          else {
            uVar9 = 0xff;
          }
          if (0xff < (ushort)uVar7) {
            uVar7 = 0xff;
          }
          if (0xff < (ushort)uVar8) {
            uVar8 = 0xff;
          }
          if (0xff < (ushort)uVar9) {
            uVar9 = 0xff;
          }
          if (DAT_006e1f88 == 0) {
            uVar10 = (ushort)(((uVar7 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
          }
          else {
            uVar10 = (ushort)(((uVar7 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
          }
          *local_30 = uVar10 | (ushort)(uVar9 >> 3) & 0x1f;
          local_30 = local_30 + 1;
          local_2c = local_2c + -1;
        } while (local_2c != 0);
      }
    }
    local_28 = 0x100;
    do {
      uVar12 = __ftol();
      uVar7 = __ftol();
      iVar13 = __ftol();
      if ((int)uVar12 < 0xff) {
        if ((int)uVar12 < 1) {
          uVar12 = 0;
        }
      }
      else {
        uVar12 = 0xff;
      }
      if ((int)uVar7 < 0xff) {
        if ((int)uVar7 < 1) {
          uVar7 = 0;
        }
      }
      else {
        uVar7 = 0xff;
      }
      if (iVar13 < 0xff) {
        if (iVar13 < 1) {
          iVar13 = 0;
        }
      }
      else {
        iVar13 = 0xff;
      }
      bVar4 = (byte)iVar13;
      if (0xff < (ushort)uVar12) {
        uVar12 = 0xff;
      }
      if (0xff < (ushort)uVar7) {
        uVar7 = 0xff;
      }
      local_40 = (ushort)iVar13;
      if (0xff < local_40) {
        bVar4 = 0xff;
      }
      if (DAT_006e1f88 == 0) {
        uVar10 = (ushort)(((uVar12 & 0xfff8) << 5 | uVar7 & 0xfc) << 3);
      }
      else {
        uVar10 = (ushort)(((uVar12 & 0xf8) << 5 | uVar7 & 0xf8) << 2);
      }
      *local_30 = uVar10 | bVar4 >> 3;
      local_30 = local_30 + 1;
      local_28 = local_28 + -1;
    } while (local_28 != 0);
    if ((uVar11 != param_1 - 2U) && (local_2c = 1, uVar11 != 0)) {
      do {
        local_24 = 0x100;
        do {
          uVar12 = __ftol();
          uVar7 = __ftol();
          iVar13 = __ftol();
          if ((int)uVar12 < 0xff) {
            if ((int)uVar12 < 1) {
              uVar12 = 0;
            }
          }
          else {
            uVar12 = 0xff;
          }
          if ((int)uVar7 < 0xff) {
            if ((int)uVar7 < 1) {
              uVar7 = 0;
            }
          }
          else {
            uVar7 = 0xff;
          }
          if (iVar13 < 0xff) {
            if (iVar13 < 1) {
              iVar13 = 0;
            }
          }
          else {
            iVar13 = 0xff;
          }
          bVar4 = (byte)iVar13;
          if (0xff < (ushort)uVar12) {
            uVar12 = 0xff;
          }
          if (0xff < (ushort)uVar7) {
            uVar7 = 0xff;
          }
          local_40 = (ushort)iVar13;
          if (0xff < local_40) {
            bVar4 = 0xff;
          }
          if (DAT_006e1f88 == 0) {
            uVar10 = (ushort)(((uVar12 & 0xfff8) << 5 | uVar7 & 0xfc) << 3);
          }
          else {
            uVar10 = (ushort)(((uVar12 & 0xf8) << 5 | uVar7 & 0xf8) << 2);
          }
          *local_30 = uVar10 | bVar4 >> 3;
          local_30 = local_30 + 1;
          local_24 = local_24 + -1;
        } while (local_24 != 0);
        local_2c = local_2c + 1;
      } while (local_2c <= uVar11);
    }
    return 1;
  }
  return 0;
}

