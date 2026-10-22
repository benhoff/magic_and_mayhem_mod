
/* WARNING: Removing unreachable block (ram,0x00582772) */
/* WARNING: Removing unreachable block (ram,0x00582766) */
/* WARNING: Removing unreachable block (ram,0x0058277e) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __fastcall FUN_00582440(int param_1,int *param_2,byte *param_3)

{
  byte bVar1;
  byte bVar2;
  byte bVar3;
  ushort uVar4;
  int iVar5;
  byte *pbVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  undefined4 uVar10;
  uint uVar11;
  byte *pbVar12;
  byte *pbVar13;
  ushort *puVar14;
  float10 fVar15;
  uint local_38;
  ushort *local_30;
  ushort local_2c;
  int local_28;
  
  uVar11 = param_1 - 1U >> 1;
  iVar5 = FUN_00597880(param_1 * 0x200 + -0x1f0);
  *param_2 = iVar5;
  if (iVar5 != 0) {
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
    *(undefined4 *)(iVar5 + 4) = uVar10;
    *(uint *)(*param_2 + 8) = uVar11;
    *(undefined4 *)*param_2 = 0;
    fVar15 = (float10)FUN_0059bcc0();
    _DAT_005f18e8 = (double)fVar15;
    fVar15 = (float10)FUN_0059bcc0();
    _DAT_005f18f0 = (double)fVar15;
    local_30 = (ushort *)(*param_2 + 0xc);
    if ((uVar11 != 0) && (local_38 = 1, uVar11 != 0)) {
      do {
        FUN_0059bcc0();
        local_28 = 0x100;
        pbVar6 = param_3;
        do {
          bVar1 = *pbVar6;
          bVar2 = pbVar6[1];
          bVar3 = pbVar6[2];
          pbVar6 = pbVar6 + 3;
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
          uVar7 = ((int)uVar7 < 1) - 1 & uVar7;
          if (0xfe < (int)uVar7) {
            uVar7 = 0xff;
          }
          uVar8 = ((int)uVar8 < 1) - 1 & uVar8;
          if (0xfe < (int)uVar8) {
            uVar8 = 0xff;
          }
          uVar9 = ((int)uVar9 < 1) - 1 & uVar9;
          if (0xfe < (int)uVar9) {
            uVar9 = 0xff;
          }
          if (local_38 == 1) {
            uVar7 = 0;
            uVar8 = 0;
            uVar9 = 0;
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
            uVar4 = (ushort)(((uVar7 & 0xfff8) << 5 | uVar8 & 0xfc) << 3);
          }
          else {
            uVar4 = (ushort)(((uVar7 & 0xf8) << 5 | uVar8 & 0xf8) << 2);
          }
          *local_30 = uVar4 | (ushort)(uVar9 >> 3) & 0x1f;
          local_30 = local_30 + 1;
          local_28 = local_28 + -1;
        } while (local_28 != 0);
        local_38 = local_38 + 1;
      } while (local_38 <= uVar11);
    }
    local_28 = 0x100;
    pbVar6 = param_3;
    puVar14 = local_30;
    do {
      bVar1 = *pbVar6;
      pbVar13 = pbVar6 + 1;
      pbVar12 = pbVar6 + 2;
      pbVar6 = pbVar6 + 3;
      if (DAT_006e1f88 == 0) {
        uVar4 = (ushort)(((bVar1 & 0xfff8) << 5 | *pbVar13 & 0xfc) << 3);
      }
      else {
        uVar4 = (ushort)(((bVar1 & 0xf8) << 5 | *pbVar13 & 0xf8) << 2);
      }
      local_30 = puVar14 + 1;
      *puVar14 = uVar4 | *pbVar12 >> 3;
      local_28 = local_28 + -1;
      puVar14 = local_30;
    } while (local_28 != 0);
    if ((uVar11 != param_1 - 2U) && (local_38 = 1, uVar11 != 0)) {
      local_28 = 0x100;
      pbVar6 = param_3;
      while( true ) {
        do {
          uVar8 = ((0xff - (uint)*pbVar6) * local_38) / uVar11 + (uint)*pbVar6;
          pbVar13 = pbVar6 + 2;
          uVar9 = ((0xff - (uint)pbVar6[1]) * local_38) / uVar11 + (uint)pbVar6[1];
          pbVar6 = pbVar6 + 3;
          uVar7 = ((0xff - (uint)*pbVar13) * local_38) / uVar11 + (uint)*pbVar13;
          uVar8 = uVar8 & ((int)uVar8 < 1) - 1;
          if (0xfe < (int)uVar8) {
            uVar8 = 0xff;
          }
          uVar9 = ((int)uVar9 < 1) - 1 & uVar9;
          if (0xfe < (int)uVar9) {
            uVar9 = 0xff;
          }
          uVar7 = ((int)uVar7 < 1) - 1 & uVar7;
          if (0xfe < (int)uVar7) {
            uVar7 = 0xff;
          }
          if (0xff < (ushort)uVar8) {
            uVar8 = 0xff;
          }
          local_2c = (ushort)uVar9;
          if (0xff < local_2c) {
            uVar9 = 0xff;
          }
          if (0xff < (ushort)uVar7) {
            uVar7 = 0xff;
          }
          if (DAT_006e1f88 == 0) {
            uVar4 = (ushort)(((uVar8 & 0xfff8) << 5 | uVar9 & 0xfc) << 3);
          }
          else {
            uVar4 = (ushort)(((uVar8 & 0xf8) << 5 | uVar9 & 0xf8) << 2);
          }
          *local_30 = uVar4 | (ushort)(uVar7 >> 3) & 0x1f;
          local_30 = local_30 + 1;
          local_28 = local_28 + -1;
        } while (local_28 != 0);
        local_38 = local_38 + 1;
        if (uVar11 < local_38) break;
        local_28 = 0x100;
        pbVar6 = param_3;
      }
    }
    return 1;
  }
  return 0;
}

