
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00589280(int param_1,int param_2,char *param_3)

{
  char cVar1;
  char cVar2;
  char cVar3;
  undefined4 uVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  float10 fVar10;
  char *local_34;
  uint *local_24;
  int local_1c;
  
  if (((((param_1 != 2) && (param_1 != 4)) && (param_1 != 8)) &&
      ((param_1 != 0x10 && (param_1 != 0x20)))) &&
     ((param_1 != 0x40 && ((param_1 != 0x80 && (param_1 != 0x100)))))) {
    return 0;
  }
  uVar9 = param_1 - 1U >> 1;
  if (param_2 == 0) {
    return 0;
  }
  switch(param_1) {
  case 2:
    uVar4 = 7;
    break;
  default:
    uVar4 = 0;
    break;
  case 4:
    uVar4 = 6;
    break;
  case 8:
    uVar4 = 5;
    break;
  case 0x10:
    uVar4 = 4;
    break;
  case 0x20:
    uVar4 = 3;
    break;
  case 0x40:
    uVar4 = 2;
    break;
  case 0x80:
    uVar4 = 1;
  }
  *(undefined4 *)(param_2 + 4) = uVar4;
  *(uint *)(param_2 + 8) = uVar9;
  fVar10 = (float10)FUN_0059bcc0();
  _DAT_005f1910 = (double)fVar10;
  fVar10 = (float10)FUN_0059bcc0();
  _DAT_005f1918 = (double)fVar10;
  local_24 = (uint *)(param_2 + 0xc);
  if (uVar9 != 0) {
    for (; uVar9 != 0; uVar9 = uVar9 - 1) {
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
          if (0 < (int)uVar5) goto LAB_0058951d;
        }
        else {
          uVar6 = __ftol();
LAB_0058951d:
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
    uVar9 = 0;
    if (cVar1 != '\0') {
      uVar9 = __ftol();
    }
    if (cVar2 == '\0') {
      uVar6 = 0;
      if (cVar1 != '\0') goto LAB_005896bc;
    }
    else {
      uVar6 = __ftol();
LAB_005896bc:
      uVar5 = __ftol();
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
    if ((int)uVar5 < 0xff) {
      if ((int)uVar5 < 1) {
        uVar5 = 0;
      }
    }
    else {
      uVar5 = 0xff;
    }
    if (0xff < (ushort)uVar9) {
      uVar9 = 0xff;
    }
    if (0xff < (ushort)uVar6) {
      uVar6 = 0xff;
    }
    if (0xff < (ushort)uVar5) {
      uVar5 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar9 = ((uVar9 & 0xfff8) << 5 | uVar6 & 0xfc) << 3;
    }
    else {
      uVar9 = ((uVar9 & 0xf8) << 5 | uVar6 & 0xf8) << 2;
    }
    *local_24 = uVar9 & 0xffff | uVar5 >> 3 & 0x1f;
    local_24 = local_24 + 1;
    local_1c = local_1c + -1;
    if (local_1c == 0) {
      return 1;
    }
  } while( true );
}

