
/* WARNING: Removing unreachable block (ram,0x00588a5d) */
/* WARNING: Removing unreachable block (ram,0x00588a51) */
/* WARNING: Removing unreachable block (ram,0x00588a69) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00588730(uint param_1,int *param_2,byte *param_3)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  int iVar4;
  byte *pbVar5;
  uint uVar6;
  uint uVar7;
  uint uVar8;
  undefined4 uVar9;
  byte *pbVar10;
  uint uVar11;
  float10 fVar12;
  uint local_30;
  uint *local_2c;
  int local_28;
  
  uVar11 = param_1 - 1 >> 1;
  iVar4 = FUN_00597880((param_1 >> 1) * 0x400 + 0x10);
  *param_2 = iVar4;
  if (iVar4 != 0) {
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
    *(uint *)(*param_2 + 8) = uVar11;
    *(undefined4 *)*param_2 = 0;
    fVar12 = (float10)FUN_0059bcc0();
    _DAT_005f18e8 = (double)fVar12;
    fVar12 = (float10)FUN_0059bcc0();
    _DAT_005f18f0 = (double)fVar12;
    local_2c = (uint *)(*param_2 + 0xc);
    if ((uVar11 != 0) && (local_30 = 1, uVar11 != 0)) {
      do {
        FUN_0059bcc0();
        local_28 = 0x100;
        pbVar5 = param_3;
        do {
          bVar1 = *pbVar5;
          bVar2 = pbVar5[1];
          bVar3 = pbVar5[2];
          pbVar5 = pbVar5 + 3;
          if (bVar1 == 0) {
            uVar6 = 0;
          }
          else {
            uVar6 = __ftol();
          }
          if (bVar2 == 0) {
            uVar7 = 0;
          }
          else {
            uVar7 = __ftol();
          }
          if (bVar3 == 0) {
            uVar8 = 0;
          }
          else {
            uVar8 = __ftol();
          }
          uVar6 = ((int)uVar6 < 1) - 1 & uVar6;
          if (0xfe < (int)uVar6) {
            uVar6 = 0xff;
          }
          uVar7 = ((int)uVar7 < 1) - 1 & uVar7;
          if (0xfe < (int)uVar7) {
            uVar7 = 0xff;
          }
          uVar8 = ((int)uVar8 < 1) - 1 & uVar8;
          if (0xfe < (int)uVar8) {
            uVar8 = 0xff;
          }
          if (local_30 == 1) {
            uVar6 = 0;
            uVar7 = 0;
            uVar8 = 0;
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
            uVar6 = ((uVar6 & 0xfff8) << 5 | uVar7 & 0xfc) << 3;
          }
          else {
            uVar6 = ((uVar6 & 0xf8) << 5 | uVar7 & 0xf8) << 2;
          }
          *local_2c = uVar6 & 0xffff | uVar8 >> 3 & 0x1f;
          local_2c = local_2c + 1;
          local_28 = local_28 + -1;
        } while (local_28 != 0);
        local_30 = local_30 + 1;
      } while (local_30 <= uVar11);
    }
    iVar4 = 0x100;
    do {
      bVar1 = *param_3;
      pbVar5 = param_3 + 1;
      pbVar10 = param_3 + 2;
      param_3 = param_3 + 3;
      if (DAT_006e1f88 == 0) {
        uVar11 = ((bVar1 & 0xfff8) << 5 | *pbVar5 & 0xfc) << 3;
      }
      else {
        uVar11 = ((bVar1 & 0xf8) << 5 | *pbVar5 & 0xf8) << 2;
      }
      iVar4 = iVar4 + -1;
      *local_2c = uVar11 | *pbVar10 >> 3;
      local_2c = local_2c + 1;
    } while (iVar4 != 0);
    return 1;
  }
  return 0;
}

