
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00588bf0(uint param_1,int *param_2,char *param_3)

{
  char cVar1;
  char cVar2;
  char cVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined4 uVar9;
  uint uVar10;
  float10 fVar11;
  char *local_34;
  uint *local_24;
  int local_1c;
  
  if (((((param_1 != 2) && (param_1 != 4)) && (param_1 != 8)) &&
      ((param_1 != 0x10 && (param_1 != 0x20)))) &&
     ((param_1 != 0x40 && ((param_1 != 0x80 && (param_1 != 0x100)))))) {
    return 0;
  }
  uVar10 = param_1 - 1 >> 1;
  iVar4 = FUN_00597880((param_1 >> 1) * 0x400 + 0x10);
  *param_2 = iVar4;
  if (iVar4 == 0) {
    return 0;
  }
  switch(param_1) {
  case 2:
    uVar9 = 7;
    break;
  default:
    uVar9 = 0;
    break;
  case 4:
    uVar9 = 6;
    break;
  case 8:
    uVar9 = 5;
    break;
  case 0x10:
    uVar9 = 4;
    break;
  case 0x20:
    uVar9 = 3;
    break;
  case 0x40:
    uVar9 = 2;
    break;
  case 0x80:
    uVar9 = 1;
  }
  *(undefined4 *)(iVar4 + 4) = uVar9;
  *(uint *)(*param_2 + 8) = uVar10;
  *(undefined4 *)*param_2 = 0;
  fVar11 = (float10)FUN_0059bcc0();
  _DAT_005f1910 = (double)fVar11;
  fVar11 = (float10)FUN_0059bcc0();
  _DAT_005f1918 = (double)fVar11;
  local_24 = (uint *)(*param_2 + 0xc);
  if (uVar10 != 0) {
    for (; uVar10 != 0; uVar10 = uVar10 - 1) {
      FUN_0059bcc0();
      local_34 = param_3;
      local_1c = 0x100;
      do {
        cVar1 = *local_34;
        cVar2 = local_34[1];
        cVar3 = local_34[2];
        local_34 = local_34 + 3;
        if (cVar1 == '\0') {
          uVar5 = 0;
        }
        else {
          uVar5 = __ftol();
        }
        if (cVar2 == '\0') {
          uVar6 = 0;
        }
        else {
          uVar6 = __ftol();
        }
        if (cVar3 == '\0') {
          uVar7 = 0;
        }
        else {
          uVar7 = __ftol();
        }
        uVar8 = uVar5;
        if (0 < (int)uVar5) {
          uVar8 = __ftol();
        }
        if ((int)uVar6 < 1) {
          if (0 < (int)uVar5) goto LAB_00588eb8;
        }
        else {
          uVar6 = __ftol();
LAB_00588eb8:
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
        if (0xff < (ushort)uVar8) {
          uVar8 = 0xff;
        }
        if (0xff < (ushort)uVar6) {
          uVar6 = 0xff;
        }
        if (0xff < (ushort)uVar7) {
          uVar7 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          uVar5 = ((uVar8 & 0xfff8) << 5 | uVar6 & 0xfc) << 3;
        }
        else {
          uVar5 = ((uVar8 & 0xf8) << 5 | uVar6 & 0xf8) << 2;
        }
        *local_24 = uVar5 & 0xffff | uVar7 >> 3 & 0x1f;
        local_24 = local_24 + 1;
        local_1c = local_1c + -1;
      } while (local_1c != 0);
    }
  }
  local_1c = 0x100;
  local_34 = param_3;
  do {
    cVar1 = *local_34;
    cVar2 = local_34[1];
    uVar5 = (uint)(byte)local_34[2];
    local_34 = local_34 + 3;
    uVar10 = 0;
    if (cVar1 != '\0') {
      uVar10 = __ftol();
    }
    if (cVar2 == '\0') {
      uVar6 = 0;
      if (cVar1 != '\0') goto LAB_00589060;
    }
    else {
      uVar6 = __ftol();
LAB_00589060:
      uVar5 = __ftol();
    }
    if ((int)uVar10 < 0xff) {
      if ((int)uVar10 < 1) {
        uVar10 = 0;
      }
    }
    else {
      uVar10 = 0xff;
    }
    if ((int)uVar6 < 0xff) {
      if ((int)uVar6 < 1) {
        uVar6 = 0;
      }
    }
    else {
      uVar6 = 0xff;
    }
    if ((int)uVar5 < 0xff) {
      if ((int)uVar5 < 1) {
        uVar5 = 0;
      }
    }
    else {
      uVar5 = 0xff;
    }
    if (0xff < (ushort)uVar10) {
      uVar10 = 0xff;
    }
    if (0xff < (ushort)uVar6) {
      uVar6 = 0xff;
    }
    if (0xff < (ushort)uVar5) {
      uVar5 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar10 = ((uVar10 & 0xfff8) << 5 | uVar6 & 0xfc) << 3;
    }
    else {
      uVar10 = ((uVar10 & 0xf8) << 5 | uVar6 & 0xf8) << 2;
    }
    *local_24 = uVar10 & 0xffff | uVar5 >> 3 & 0x1f;
    local_24 = local_24 + 1;
    local_1c = local_1c + -1;
    if (local_1c == 0) {
      return 1;
    }
  } while( true );
}

