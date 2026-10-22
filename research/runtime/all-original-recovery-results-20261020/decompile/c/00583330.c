
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00583330(int param_1,int *param_2,byte *param_3)

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
  undefined4 uVar11;
  uint uVar12;
  byte *pbVar13;
  float10 fVar14;
  byte *local_3c;
  ushort *local_2c;
  uint local_28;
  int local_20;
  
  if (((((param_1 != 2) && (param_1 != 4)) && (param_1 != 8)) &&
      ((param_1 != 0x10 && (param_1 != 0x20)))) &&
     ((param_1 != 0x40 && ((param_1 != 0x80 && (param_1 != 0x100)))))) {
    return 0;
  }
  uVar12 = param_1 - 1U >> 1;
  iVar6 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar6;
  if (iVar6 == 0) {
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
  *(undefined4 *)(iVar6 + 4) = uVar11;
  *(uint *)(*param_2 + 8) = uVar12;
  *(undefined4 *)*param_2 = 0;
  fVar14 = (float10)FUN_0059bcc0();
  _DAT_005f18e8 = (double)fVar14;
  fVar14 = (float10)FUN_0059bcc0();
  _DAT_005f18f0 = (double)fVar14;
  local_2c = (ushort *)(*param_2 + 0xc);
  uVar10 = uVar12;
  if (uVar12 != 0) {
    for (; uVar10 != 0; uVar10 = uVar10 - 1) {
      FUN_0059bcc0();
      local_3c = param_3;
      local_20 = 0x100;
      do {
        bVar1 = *local_3c;
        bVar2 = local_3c[1];
        bVar3 = local_3c[2];
        local_3c = local_3c + 3;
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
        bVar4 = 0 < (int)uVar9;
        if (bVar4) {
          uVar9 = __ftol();
        }
        if ((int)uVar8 < 1) {
          if (bVar4) goto LAB_005835f5;
        }
        else {
          uVar8 = __ftol();
LAB_005835f5:
          uVar7 = __ftol();
        }
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
          uVar5 = (ushort)(((uVar7 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
        }
        else {
          uVar5 = (ushort)(((uVar7 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
        }
        *local_2c = uVar5 | (ushort)(uVar9 >> 3) & 0x1f;
        local_2c = local_2c + 1;
        local_20 = local_20 + -1;
      } while (local_20 != 0);
    }
  }
  local_20 = 0x100;
  local_3c = param_3;
  do {
    uVar7 = (uint)*local_3c;
    bVar1 = local_3c[1];
    bVar2 = local_3c[2];
    local_3c = local_3c + 3;
    uVar10 = 0;
    if (bVar2 != 0) {
      uVar10 = __ftol();
    }
    if (bVar1 == 0) {
      uVar8 = 0;
      if (bVar2 != 0) goto LAB_00583785;
    }
    else {
      uVar8 = __ftol();
LAB_00583785:
      uVar7 = __ftol();
    }
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
    if ((int)uVar10 < 0xff) {
      if ((int)uVar10 < 1) {
        uVar10 = 0;
      }
    }
    else {
      uVar10 = 0xff;
    }
    if (0xff < (ushort)uVar7) {
      uVar7 = 0xff;
    }
    if (0xff < (ushort)uVar8) {
      uVar8 = 0xff;
    }
    if (0xff < (ushort)uVar10) {
      uVar10 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar5 = (ushort)(((uVar7 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
    }
    else {
      uVar5 = (ushort)(((uVar7 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
    }
    *local_2c = uVar5 | (ushort)(uVar10 >> 3) & 0x1f;
    local_2c = local_2c + 1;
    local_20 = local_20 + -1;
    if (local_20 == 0) {
      if ((uVar12 != param_1 - 2U) && (local_28 = 1, uVar12 != 0)) {
        do {
          local_20 = 0x100;
          local_3c = param_3;
          do {
            uVar8 = ((0xff - (uint)*local_3c) * local_28) / uVar12 + (uint)*local_3c;
            pbVar13 = local_3c + 2;
            uVar7 = ((0xff - (uint)local_3c[1]) * local_28) / uVar12 + (uint)local_3c[1];
            local_3c = local_3c + 3;
            uVar10 = ((0xff - (uint)*pbVar13) * local_28) / uVar12 + (uint)*pbVar13;
            bVar4 = 0 < (int)uVar10;
            if (bVar4) {
              uVar10 = __ftol();
            }
            if ((int)uVar7 < 1) {
              if (bVar4) goto LAB_00583977;
            }
            else {
              uVar7 = __ftol();
LAB_00583977:
              uVar8 = __ftol();
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
            if ((int)uVar10 < 0xff) {
              if ((int)uVar10 < 1) {
                uVar10 = 0;
              }
            }
            else {
              uVar10 = 0xff;
            }
            if (0xff < (ushort)uVar8) {
              uVar8 = 0xff;
            }
            if (0xff < (ushort)uVar7) {
              uVar7 = 0xff;
            }
            if (0xff < (ushort)uVar10) {
              uVar10 = 0xff;
            }
            if (DAT_006e1f88 == 0) {
              uVar5 = (ushort)(((uVar8 & 0xfff8) << 5 | uVar7 & 0xfc) << 3);
            }
            else {
              uVar5 = (ushort)(((uVar8 & 0xf8) << 5 | uVar7 & 0xf8) << 2);
            }
            *local_2c = uVar5 | (ushort)(uVar10 >> 3) & 0x1f;
            local_2c = local_2c + 1;
            local_20 = local_20 + -1;
          } while (local_20 != 0);
          local_28 = local_28 + 1;
        } while (local_28 <= uVar12);
      }
      return 1;
    }
  } while( true );
}

