
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_005870d0(int param_1,int *param_2,byte *param_3)

{
  byte *pbVar1;
  char cVar2;
  byte bVar3;
  byte bVar4;
  double dVar5;
  byte bVar6;
  DWORD DVar7;
  uint uVar8;
  uint uVar9;
  ushort uVar10;
  uint uVar11;
  uint uVar12;
  int iVar13;
  undefined4 uVar14;
  byte *pbVar15;
  byte *pbVar16;
  uint uVar17;
  int iVar18;
  char *pcVar19;
  double *pdVar20;
  char *pcVar21;
  char *pcVar22;
  float10 fVar23;
  float10 fVar24;
  float10 extraout_ST1;
  float10 extraout_ST1_00;
  int local_c5c;
  uint local_c58;
  ushort local_c54;
  ushort *local_c4c;
  double local_c48;
  int local_c34;
  double local_c30;
  double local_814 [256];
  void *local_14;
  undefined1 *puStack_10;
  undefined4 local_c;
  
  local_c = 0xffffffff;
  puStack_10 = &LAB_005c2e2b;
  local_14 = ExceptionList;
  ExceptionList = &local_14;
  GetCurrentDirectoryA(0x100,&DAT_006f5aac);
  uVar11 = 0xffffffff;
  pcVar19 = &DAT_006f5aac;
  do {
    pcVar22 = pcVar19;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar22 = pcVar19 + 1;
    cVar2 = *pcVar19;
    pcVar19 = pcVar22;
  } while (cVar2 != '\0');
  uVar11 = ~uVar11;
  pcVar19 = pcVar22 + -uVar11;
  pcVar22 = (char *)&DAT_006f5cac;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar22 = *(undefined4 *)pcVar19;
    pcVar19 = pcVar19 + 4;
    pcVar22 = pcVar22 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar22 = *pcVar19;
    pcVar19 = pcVar19 + 1;
    pcVar22 = pcVar22 + 1;
  }
  uVar11 = 0xffffffff;
  pcVar19 = (char *)&DAT_006f5cac;
  do {
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    cVar2 = *pcVar19;
    pcVar19 = pcVar19 + 1;
  } while (cVar2 != '\0');
  if (((~uVar11 != 1) && ((&DAT_006f5caa)[~uVar11] != '\\')) && ((&DAT_006f5caa)[~uVar11] != '/')) {
    uVar11 = 0xffffffff;
    pcVar19 = &DAT_005d9058;
    do {
      pcVar22 = pcVar19;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar22 = pcVar19 + 1;
      cVar2 = *pcVar19;
      pcVar19 = pcVar22;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    iVar13 = -1;
    pcVar19 = (char *)&DAT_006f5cac;
    do {
      pcVar21 = pcVar19;
      if (iVar13 == 0) break;
      iVar13 = iVar13 + -1;
      pcVar21 = pcVar19 + 1;
      cVar2 = *pcVar19;
      pcVar19 = pcVar21;
    } while (cVar2 != '\0');
    pcVar19 = pcVar22 + -uVar11;
    pcVar22 = pcVar21 + -1;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar22 = *(undefined4 *)pcVar19;
      pcVar19 = pcVar19 + 4;
      pcVar22 = pcVar22 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar22 = *pcVar19;
      pcVar19 = pcVar19 + 1;
      pcVar22 = pcVar22 + 1;
    }
  }
  uVar11 = 0xffffffff;
  pcVar19 = s_cfg_tables_cfg_005f194c;
  do {
    pcVar22 = pcVar19;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar22 = pcVar19 + 1;
    cVar2 = *pcVar19;
    pcVar19 = pcVar22;
  } while (cVar2 != '\0');
  uVar11 = ~uVar11;
  iVar13 = -1;
  pcVar19 = (char *)&DAT_006f5cac;
  do {
    pcVar21 = pcVar19;
    if (iVar13 == 0) break;
    iVar13 = iVar13 + -1;
    pcVar21 = pcVar19 + 1;
    cVar2 = *pcVar19;
    pcVar19 = pcVar21;
  } while (cVar2 != '\0');
  pcVar19 = pcVar22 + -uVar11;
  pcVar22 = pcVar21 + -1;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar22 = *(undefined4 *)pcVar19;
    pcVar19 = pcVar19 + 4;
    pcVar22 = pcVar22 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar22 = *pcVar19;
    pcVar19 = pcVar19 + 1;
    pcVar22 = pcVar22 + 1;
  }
  DVar7 = GetPrivateProfileStringA
                    (s_METAL_EFFECT_005f1974,s_Reducer_005f1944,&DAT_005faf28,&DAT_006f57ac,0x100,
                     (LPCSTR)&DAT_006f5cac);
  if (DVar7 != 0) {
    FUN_0059d4be(&DAT_006f57ac);
  }
  DVar7 = GetPrivateProfileStringA
                    (s_METAL_EFFECT_005f1974,s_SpecalValue_005f1928,&DAT_005faf28,&DAT_006f57ac,
                     0x100,(LPCSTR)&DAT_006f5cac);
  if (DVar7 != 0) {
    FUN_0059d4be(&DAT_006f57ac);
  }
  DVar7 = GetPrivateProfileStringA
                    (s_METAL_EFFECT_005f1974,s__Darkest_005f191f + 1,&DAT_005faf28,&DAT_006f57ac,
                     0x100,(LPCSTR)&DAT_006f5cac);
  if (DVar7 != 0) {
    FUN_0059d4be(&DAT_006f57ac);
  }
  DVar7 = GetPrivateProfileStringA
                    (s_METAL_EFFECT_005f1974,s_BlueTint_005f1968,&DAT_005faf28,&DAT_006f57ac,0x100,
                     (LPCSTR)&DAT_006f5cac);
  if (DVar7 != 0) {
    FUN_0059d4be(&DAT_006f57ac);
  }
  DVar7 = GetPrivateProfileStringA
                    (s_METAL_EFFECT_005f1974,s_Contrast_005f195c,&DAT_005faf28,&DAT_006f57ac,0x100,
                     (LPCSTR)&DAT_006f5cac);
  if (DVar7 != 0) {
    FUN_0059d4be(&DAT_006f57ac);
  }
  FUN_0054e170(0x499602d2);
  local_c = 0;
  FUN_0054e170(2000);
  iVar13 = __ftol();
  iVar18 = 0x100;
  pdVar20 = local_814;
  do {
    if (iVar13 == 1) {
      local_c58 = 1;
    }
    else {
      uVar11 = FUN_0054e200();
      local_c58 = (int)((longlong)(ulonglong)(uVar11 & 0x7fffffff) % (longlong)(iVar13 + -1)) + 1;
    }
    iVar18 = iVar18 + -1;
    *pdVar20 = (double)(int)local_c58;
    pdVar20 = pdVar20 + 1;
  } while (iVar18 != 0);
  if (((((param_1 != 2) && (param_1 != 4)) &&
       ((param_1 != 8 && ((param_1 != 0x10 && (param_1 != 0x20)))))) && (param_1 != 0x40)) &&
     ((param_1 != 0x80 && (param_1 != 0x100)))) {
    ExceptionList = local_14;
    return 0;
  }
  uVar11 = param_1 - 1U >> 1;
  iVar13 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar13;
  if (iVar13 == 0) {
    ExceptionList = local_14;
    return 0;
  }
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
  fVar23 = (float10)FUN_0059bcc0();
  _DAT_005f18e8 = (double)fVar23;
  fVar23 = (float10)FUN_0059bcc0();
  _DAT_005f18f0 = (double)fVar23;
  local_c30 = 0.0;
  local_c48 = 0.0;
  iVar13 = 0x100;
  pbVar16 = param_3;
  do {
    bVar6 = *pbVar16;
    pbVar1 = pbVar16 + 1;
    pbVar15 = pbVar16 + 2;
    pbVar16 = pbVar16 + 3;
    dVar5 = (double)((uint)*pbVar15 + (uint)*pbVar1 + (uint)bVar6);
    if (dVar5 < local_c30) {
      local_c30 = dVar5;
    }
    if (local_c48 < dVar5) {
      local_c48 = dVar5;
    }
    iVar13 = iVar13 + -1;
  } while (iVar13 != 0);
  iVar13 = 0x100;
  fVar23 = ((float10)local_c48 - (float10)local_c30) * (float10)_DAT_005c5a88;
  pbVar16 = param_3;
  do {
    fVar24 = (float10)((uint)pbVar16[2] + (uint)pbVar16[1] + (uint)*pbVar16);
    if (fVar23 <= fVar24) {
      if (fVar23 < fVar24) {
        bVar6 = __ftol();
        if (bVar6 == 0) {
          bVar6 = 0;
        }
        *pbVar16 = bVar6;
        fVar23 = extraout_ST1_00;
        bVar6 = __ftol();
        if (bVar6 == 0) {
          bVar6 = 0;
        }
        pbVar16[1] = bVar6;
        bVar6 = __ftol();
        if (bVar6 == 0) {
          bVar6 = 0;
        }
        goto LAB_00587699;
      }
    }
    else {
      bVar6 = __ftol();
      if (bVar6 == 0xff) {
        bVar6 = 0xff;
      }
      *pbVar16 = bVar6;
      fVar23 = extraout_ST1;
      bVar6 = __ftol();
      if (bVar6 == 0xff) {
        bVar6 = 0xff;
      }
      pbVar16[1] = bVar6;
      bVar6 = __ftol();
      if (bVar6 == 0xff) {
        bVar6 = 0xff;
      }
LAB_00587699:
      pbVar16[2] = bVar6;
      pbVar16 = pbVar16 + 3;
    }
    iVar13 = iVar13 + -1;
    if (iVar13 == 0) {
      local_c4c = (ushort *)(*param_2 + 0xc);
      uVar12 = uVar11;
      if (uVar11 != 0) {
        for (; uVar12 != 0; uVar12 = uVar12 - 1) {
          FUN_0059bcc0();
          local_c34 = 0x100;
          pbVar16 = param_3;
          do {
            bVar6 = *pbVar16;
            bVar3 = pbVar16[1];
            bVar4 = pbVar16[2];
            pbVar16 = pbVar16 + 3;
            if (bVar6 != 0) {
              __ftol();
            }
            if (bVar3 != 0) {
              __ftol();
            }
            if (bVar4 != 0) {
              __ftol();
            }
            uVar8 = __ftol();
            uVar9 = __ftol();
            if ((int)uVar8 < 0xff) {
              uVar17 = uVar8;
              if ((int)uVar8 < 1) {
                uVar17 = 0;
              }
            }
            else {
              uVar17 = 0xff;
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
            if (0xff < (ushort)uVar17) {
              uVar17 = 0xff;
            }
            if (0xff < (ushort)uVar8) {
              uVar8 = 0xff;
            }
            if (0xff < (ushort)uVar9) {
              uVar9 = 0xff;
            }
            if (DAT_006e1f88 == 0) {
              uVar10 = (ushort)(((uVar17 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
            }
            else {
              uVar10 = (ushort)(((uVar17 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
            }
            *local_c4c = uVar10 | (ushort)(uVar9 >> 3) & 0x1f;
            local_c4c = local_c4c + 1;
            local_c34 = local_c34 + -1;
          } while (local_c34 != 0);
        }
      }
      local_c5c = 0x100;
      do {
        uVar12 = __ftol();
        uVar8 = __ftol();
        if ((int)uVar12 < 0xff) {
          uVar9 = uVar12;
          if ((int)uVar12 < 1) {
            uVar9 = 0;
          }
        }
        else {
          uVar9 = 0xff;
        }
        if ((int)uVar12 < 0xff) {
          if ((int)uVar12 < 1) {
            uVar12 = 0;
          }
        }
        else {
          uVar12 = 0xff;
        }
        if ((int)uVar8 < 0xff) {
          if ((int)uVar8 < 1) {
            uVar8 = 0;
          }
        }
        else {
          uVar8 = 0xff;
        }
        if (0xff < (ushort)uVar9) {
          uVar9 = 0xff;
        }
        if (0xff < (ushort)uVar12) {
          uVar12 = 0xff;
        }
        if (0xff < (ushort)uVar8) {
          uVar8 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar10 = (ushort)(((uVar9 & 0xfff8) << 5 | uVar12 & 0xfc) << 3);
        }
        else {
          uVar10 = (ushort)(((uVar9 & 0xf8) << 5 | uVar12 & 0xf8) << 2);
        }
        *local_c4c = uVar10 | (ushort)(uVar8 >> 3) & 0x1f;
        local_c4c = local_c4c + 1;
        local_c5c = local_c5c + -1;
      } while (local_c5c != 0);
      if ((uVar11 != param_1 - 2U) && (local_c58 = 1, uVar11 != 0)) {
        do {
          local_c5c = 0x100;
          do {
            uVar12 = __ftol();
            iVar13 = __ftol();
            if ((int)uVar12 < 0xff) {
              uVar8 = uVar12;
              if ((int)uVar12 < 1) {
                uVar8 = 0;
              }
            }
            else {
              uVar8 = 0xff;
            }
            if ((int)uVar12 < 0xff) {
              if ((int)uVar12 < 1) {
                uVar12 = 0;
              }
            }
            else {
              uVar12 = 0xff;
            }
            if (iVar13 < 0xff) {
              if (iVar13 < 1) {
                iVar13 = 0;
              }
            }
            else {
              iVar13 = 0xff;
            }
            bVar6 = (byte)iVar13;
            if (0xff < (ushort)uVar8) {
              uVar8 = 0xff;
            }
            if (0xff < (ushort)uVar12) {
              uVar12 = 0xff;
            }
            local_c54 = (ushort)iVar13;
            if (0xff < local_c54) {
              bVar6 = 0xff;
            }
            if (DAT_006e1f88 == 0) {
              uVar10 = (ushort)(((uVar8 & 0xfff8) << 5 | uVar12 & 0xfc) << 3);
            }
            else {
              uVar10 = (ushort)(((uVar8 & 0xf8) << 5 | uVar12 & 0xf8) << 2);
            }
            *local_c4c = uVar10 | bVar6 >> 3;
            local_c4c = local_c4c + 1;
            local_c5c = local_c5c + -1;
          } while (local_c5c != 0);
          local_c58 = local_c58 + 1;
        } while (local_c58 <= uVar11);
      }
      ExceptionList = local_14;
      return 1;
    }
  } while( true );
}

