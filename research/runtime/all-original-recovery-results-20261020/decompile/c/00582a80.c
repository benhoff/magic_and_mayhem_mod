
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00582a80(int param_1,int *param_2,byte *param_3)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  bool bVar4;
  ushort uVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  uint uVar10;
  uint uVar11;
  undefined4 uVar12;
  uint uVar13;
  byte *pbVar14;
  float10 fVar15;
  byte *local_38;
  ushort *local_28;
  int local_24;
  uint local_20;
  int local_1c;
  
  if (((((param_1 != 2) && (param_1 != 4)) && (param_1 != 8)) &&
      ((param_1 != 0x10 && (param_1 != 0x20)))) &&
     ((param_1 != 0x40 && ((param_1 != 0x80 && (param_1 != 0x100)))))) {
    return 0;
  }
  uVar13 = param_1 - 1U >> 1;
  iVar6 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar6;
  if (iVar6 == 0) {
    return 0;
  }
  switch(param_1) {
  case 2:
    uVar12 = 7;
    break;
  default:
    uVar12 = 0;
    break;
  case 4:
    uVar12 = 6;
    break;
  case 8:
    uVar12 = 5;
    break;
  case 0x10:
    uVar12 = 4;
    break;
  case 0x20:
    uVar12 = 3;
    break;
  case 0x40:
    uVar12 = 2;
    break;
  case 0x80:
    uVar12 = 1;
  }
  *(undefined4 *)(iVar6 + 4) = uVar12;
  *(uint *)(*param_2 + 8) = uVar13;
  *(undefined4 *)*param_2 = 0;
  fVar15 = (float10)FUN_0059bcc0();
  _DAT_005f1910 = (double)fVar15;
  fVar15 = (float10)FUN_0059bcc0();
  _DAT_005f1918 = (double)fVar15;
  local_28 = (ushort *)(*param_2 + 0xc);
  uVar11 = uVar13;
  if (uVar13 != 0) {
    for (; uVar11 != 0; uVar11 = uVar11 - 1) {
      FUN_0059bcc0();
      local_38 = param_3;
      local_24 = 0x100;
      do {
        bVar1 = *local_38;
        bVar2 = local_38[1];
        bVar3 = local_38[2];
        local_38 = local_38 + 3;
        if (bVar1 == 0) {
          uVar7 = 0;
        }
        else {
          uVar7 = __ftol();
        }
        if (bVar2 == 0) {
          uVar8 = 0;
        }
        else {
          uVar8 = __ftol();
        }
        if (bVar3 == 0) {
          uVar9 = 0;
        }
        else {
          uVar9 = __ftol();
        }
        uVar10 = uVar7;
        if (0 < (int)uVar7) {
          uVar10 = __ftol();
        }
        if ((int)uVar8 < 1) {
          if (0 < (int)uVar7) goto LAB_00582d50;
        }
        else {
          uVar8 = __ftol();
LAB_00582d50:
          uVar9 = __ftol();
        }
        if ((int)uVar10 < 0xff) {
          if ((int)uVar10 < 1) {
            uVar10 = 0;
          }
        }
        else {
          uVar10 = 0xff;
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
        if (0xff < (ushort)uVar10) {
          uVar10 = 0xff;
        }
        if (0xff < (ushort)uVar8) {
          uVar8 = 0xff;
        }
        if (0xff < (ushort)uVar9) {
          uVar9 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar5 = (ushort)(((uVar10 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
        }
        else {
          uVar5 = (ushort)(((uVar10 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
        }
        *local_28 = uVar5 | (ushort)(uVar9 >> 3) & 0x1f;
        local_28 = local_28 + 1;
        local_24 = local_24 + -1;
      } while (local_24 != 0);
    }
  }
  local_1c = 0x100;
  local_38 = param_3;
  do {
    bVar1 = *local_38;
    bVar2 = local_38[1];
    uVar7 = (uint)local_38[2];
    local_38 = local_38 + 3;
    uVar11 = 0;
    if (bVar1 != 0) {
      uVar11 = __ftol();
    }
    if (bVar2 == 0) {
      uVar8 = 0;
      if (bVar1 != 0) goto LAB_00582ef4;
    }
    else {
      uVar8 = __ftol();
LAB_00582ef4:
      uVar7 = __ftol();
    }
    if ((int)uVar11 < 0xff) {
      if ((int)uVar11 < 1) {
        uVar11 = 0;
      }
    }
    else {
      uVar11 = 0xff;
    }
    if ((int)uVar8 < 0xff) {
      if ((int)uVar8 < 1) {
        uVar8 = 0;
      }
    }
    else {
      uVar8 = 0xff;
    }
    if ((int)uVar7 < 0xff) {
      if ((int)uVar7 < 1) {
        uVar7 = 0;
      }
    }
    else {
      uVar7 = 0xff;
    }
    if (0xff < (ushort)uVar11) {
      uVar11 = 0xff;
    }
    if (0xff < (ushort)uVar8) {
      uVar8 = 0xff;
    }
    if (0xff < (ushort)uVar7) {
      uVar7 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar5 = (ushort)(((uVar11 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
    }
    else {
      uVar5 = (ushort)(((uVar11 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
    }
    *local_28 = uVar5 | (ushort)(uVar7 >> 3) & 0x1f;
    local_28 = local_28 + 1;
    local_1c = local_1c + -1;
    if (local_1c == 0) {
      if ((uVar13 != param_1 - 2U) && (local_20 = 1, uVar13 != 0)) {
        do {
          local_1c = 0x100;
          local_38 = param_3;
          do {
            uVar8 = ((0xff - (uint)*local_38) * local_20) / uVar13 + (uint)*local_38;
            pbVar14 = local_38 + 2;
            uVar11 = ((0xff - (uint)local_38[1]) * local_20) / uVar13 + (uint)local_38[1];
            local_38 = local_38 + 3;
            uVar7 = ((0xff - (uint)*pbVar14) * local_20) / uVar13 + (uint)*pbVar14;
            bVar4 = 0 < (int)uVar8;
            if (bVar4) {
              uVar8 = __ftol();
            }
            if ((int)uVar11 < 1) {
              if (bVar4) goto LAB_005830fc;
            }
            else {
              uVar11 = __ftol();
LAB_005830fc:
              uVar7 = __ftol();
            }
            if ((int)uVar8 < 0xff) {
              if ((int)uVar8 < 1) {
                uVar8 = 0;
              }
            }
            else {
              uVar8 = 0xff;
            }
            if ((int)uVar11 < 0xff) {
              if ((int)uVar11 < 1) {
                uVar11 = 0;
              }
            }
            else {
              uVar11 = 0xff;
            }
            if ((int)uVar7 < 0xff) {
              if ((int)uVar7 < 1) {
                uVar7 = 0;
              }
            }
            else {
              uVar7 = 0xff;
            }
            if (0xff < (ushort)uVar8) {
              uVar8 = 0xff;
            }
            if (0xff < (ushort)uVar11) {
              uVar11 = 0xff;
            }
            if (0xff < (ushort)uVar7) {
              uVar7 = 0xff;
            }
            if (DAT_006e1f88 == 0) {
              uVar5 = (ushort)(((uVar8 & 0xfff8) << 5 | uVar11 & 0xfc) << 3);
            }
            else {
              uVar5 = (ushort)(((uVar8 & 0xf8) << 5 | uVar11 & 0xf8) << 2);
            }
            *local_28 = uVar5 | (ushort)(uVar7 >> 3) & 0x1f;
            local_28 = local_28 + 1;
            local_1c = local_1c + -1;
          } while (local_1c != 0);
          local_20 = local_20 + 1;
        } while (local_20 <= uVar13);
      }
      return 1;
    }
  } while( true );
}

