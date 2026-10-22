
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00585d80(int param_1,int *param_2,int param_3)

{
  byte bVar1;
  ushort uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  uint uVar11;
  float10 fVar12;
  float10 extraout_ST1;
  float10 extraout_ST1_00;
  float10 extraout_ST1_01;
  float10 extraout_ST1_02;
  ushort local_40;
  ushort *local_2c;
  uint local_24;
  int local_1c;
  int local_18;
  
  if (((((param_1 != 2) && (param_1 != 4)) && (param_1 != 8)) &&
      ((param_1 != 0x10 && (param_1 != 0x20)))) &&
     ((param_1 != 0x40 && ((param_1 != 0x80 && (param_1 != 0x100)))))) {
    return 0;
  }
  uVar11 = param_1 - 1U >> 1;
  iVar3 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar3;
  if (iVar3 == 0) {
    return 0;
  }
  switch(param_1) {
  case 2:
    uVar10 = 7;
    break;
  default:
    uVar10 = 0;
    break;
  case 4:
    uVar10 = 6;
    break;
  case 8:
    uVar10 = 5;
    break;
  case 0x10:
    uVar10 = 4;
    break;
  case 0x20:
    uVar10 = 3;
    break;
  case 0x40:
    uVar10 = 2;
    break;
  case 0x80:
    uVar10 = 1;
  }
  *(undefined4 *)(iVar3 + 4) = uVar10;
  *(uint *)(*param_2 + 8) = uVar11;
  *(undefined4 *)*param_2 = 0;
  fVar12 = (float10)FUN_0059bcc0();
  _DAT_005f18e8 = (double)fVar12;
  fVar12 = (float10)FUN_0059bcc0();
  _DAT_005f18f0 = (double)fVar12;
  local_2c = (ushort *)(*param_2 + 0xc);
  uVar9 = uVar11;
  if (uVar11 != 0) {
    for (; uVar9 != 0; uVar9 = uVar9 - 1) {
      FUN_0059bcc0();
      local_1c = 0x100;
      do {
        iVar3 = __ftol();
        fVar12 = extraout_ST1;
        iVar4 = __ftol();
        if (fVar12 <= extraout_ST1_00) {
          iVar5 = __ftol();
        }
        else {
          iVar5 = __ftol();
        }
        if (iVar3 < 0xff) {
          if (iVar3 < 1) {
            iVar3 = 0;
          }
        }
        else {
          iVar3 = 0xff;
        }
        if (iVar4 < 0xff) {
          if (iVar4 < 1) {
            iVar4 = 0;
          }
        }
        else {
          iVar4 = 0xff;
        }
        if (iVar5 < 0xff) {
          if (iVar5 < 1) {
            iVar5 = 0;
          }
        }
        else {
          iVar5 = 0xff;
        }
        uVar6 = 0;
        if (iVar3 != 0) {
          uVar6 = __ftol();
        }
        uVar7 = 0;
        if (iVar4 != 0) {
          uVar7 = __ftol();
        }
        uVar8 = 0;
        if (iVar5 != 0) {
          uVar8 = __ftol();
        }
        if (0xff < (ushort)uVar6) {
          uVar6 = 0xff;
        }
        if (0xff < (ushort)uVar7) {
          uVar7 = 0xff;
        }
        if (0xff < (ushort)uVar8) {
          uVar8 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar2 = (ushort)(((uVar6 & 0xfff8) << 5 | uVar7 & 0xfc) << 3);
        }
        else {
          uVar2 = (ushort)(((uVar6 & 0xf8) << 5 | uVar7 & 0xf8) << 2);
        }
        *local_2c = uVar2 | (ushort)(uVar8 >> 3) & 0x1f;
        local_2c = local_2c + 1;
        local_1c = local_1c + -1;
      } while (local_1c != 0);
    }
  }
  local_18 = 0x100;
  iVar3 = param_3;
  do {
    bVar1 = *(byte *)(iVar3 + 2);
    iVar3 = iVar3 + 3;
    uVar9 = __ftol();
    uVar6 = __ftol();
    if ((float10)bVar1 <= extraout_ST1_01) {
      uVar7 = __ftol();
    }
    else {
      uVar7 = __ftol();
    }
    if ((int)uVar9 < 0xff) {
      if ((int)uVar9 < 1) {
        uVar9 = 0;
      }
    }
    else {
      uVar9 = 0xff;
    }
    if ((int)uVar6 < 0xff) {
      if ((int)uVar6 < 1) {
        uVar6 = 0;
      }
    }
    else {
      uVar6 = 0xff;
    }
    if ((int)uVar7 < 0xff) {
      if ((int)uVar7 < 1) {
        uVar7 = 0;
      }
    }
    else {
      uVar7 = 0xff;
    }
    if (0xff < (ushort)uVar9) {
      uVar9 = 0xff;
    }
    if (0xff < (ushort)uVar6) {
      uVar6 = 0xff;
    }
    if (0xff < (ushort)uVar7) {
      uVar7 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar2 = (ushort)(((uVar9 & 0xfff8) << 5 | uVar6 & 0xfc) << 3);
    }
    else {
      uVar2 = (ushort)(((uVar9 & 0xf8) << 5 | uVar6 & 0xf8) << 2);
    }
    *local_2c = uVar2 | (ushort)(uVar7 >> 3) & 0x1f;
    local_2c = local_2c + 1;
    local_18 = local_18 + -1;
  } while (local_18 != 0);
  if ((uVar11 != param_1 - 2U) && (local_24 = 1, uVar11 != 0)) {
    do {
      local_18 = 0x100;
      iVar3 = param_3;
      do {
        bVar1 = *(byte *)(iVar3 + 2);
        iVar3 = iVar3 + 3;
        uVar9 = __ftol();
        uVar6 = __ftol();
        if ((float10)(int)(((0xff - (uint)bVar1) * local_24) / uVar11 + (uint)bVar1) <=
            extraout_ST1_02) {
          iVar4 = __ftol();
        }
        else {
          iVar4 = __ftol();
        }
        if ((int)uVar9 < 0xff) {
          if ((int)uVar9 < 1) {
            uVar9 = 0;
          }
        }
        else {
          uVar9 = 0xff;
        }
        if ((int)uVar6 < 0xff) {
          if ((int)uVar6 < 1) {
            uVar6 = 0;
          }
        }
        else {
          uVar6 = 0xff;
        }
        if (iVar4 < 0xff) {
          if (iVar4 < 1) {
            iVar4 = 0;
          }
        }
        else {
          iVar4 = 0xff;
        }
        bVar1 = (byte)iVar4;
        if (0xff < (ushort)uVar9) {
          uVar9 = 0xff;
        }
        if (0xff < (ushort)uVar6) {
          uVar6 = 0xff;
        }
        local_40 = (ushort)iVar4;
        if (0xff < local_40) {
          bVar1 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar2 = (ushort)(((uVar9 & 0xfff8) << 5 | uVar6 & 0xfc) << 3);
        }
        else {
          uVar2 = (ushort)(((uVar9 & 0xf8) << 5 | uVar6 & 0xf8) << 2);
        }
        *local_2c = uVar2 | bVar1 >> 3;
        local_2c = local_2c + 1;
        local_18 = local_18 + -1;
      } while (local_18 != 0);
      local_24 = local_24 + 1;
    } while (local_24 <= uVar11);
  }
  return 1;
}

