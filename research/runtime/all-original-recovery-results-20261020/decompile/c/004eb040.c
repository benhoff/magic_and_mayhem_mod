
undefined4 __thiscall FUN_004eb040(ushort *param_1,uint param_2)

{
  ushort *puVar1;
  ushort uVar2;
  uint uVar3;
  uint uVar4;
  char cVar5;
  undefined4 uVar6;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  int iVar7;
  int iVar8;
  undefined4 uVar9;
  undefined4 uVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  undefined4 local_30;
  uint local_2c;
  uint local_28;
  int local_24;
  ushort *local_20;
  int local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  uVar4 = param_2;
  uVar2 = *param_1;
  local_20 = (ushort *)(uint)uVar2;
  local_24 = DAT_0065660c + (int)local_20 * 0x164;
  uVar3 = *(uint *)(local_24 + 0x9c);
  if ((uVar3 == 0xff) || (param_2 < uVar3)) {
    return 0;
  }
  local_1c = param_2 - uVar3;
  local_30 = 0;
  param_2 = 0;
  local_2c = 0;
  if (*(int *)(local_24 + 0xac) != 0xff) {
    uVar6 = FUN_004eb490(uVar4);
    return uVar6;
  }
  if ((uVar2 != 0) &&
     ((((((0x14c < (short)uVar2 && ((short)uVar2 < 0x154)) ||
         ((0x152 < (short)uVar2 && ((short)uVar2 < 0x15a)))) ||
        ((0x158 < (short)uVar2 && ((short)uVar2 < 0x161)))) ||
       ((0x15f < (short)uVar2 && ((short)uVar2 < 0x168)))) ||
      ((0x166 < (short)uVar2 && ((short)uVar2 < 0x16f)))))) {
    iVar7 = ((int)param_1 - DAT_006c54dc) / 0xc;
    local_2c = iVar7 / DAT_006c54a0;
    iVar7 = iVar7 - (&DAT_006cb8c2)[local_2c];
    param_2 = FUN_004132e0(iVar7 / DAT_006c5494);
    local_30 = FUN_0040e650(iVar7 - (&DAT_006cb942)[param_2]);
    puVar1 = param_1;
    uVar3 = local_2c;
    while (local_28 = uVar3 + 1, uVar3 < DAT_006c549c) {
      uVar2 = puVar1[1];
      uVar3 = local_28;
      if (uVar2 != 0) {
        while ((uVar2 < DAT_0068949c && (iVar7 = DAT_00689498 + (uint)uVar2 * 0x22e, iVar7 != 0))) {
          if (*(int *)(iVar7 + 0x28) == 0x21) {
            FUN_00490260();
            return 1;
          }
          uVar2 = *(ushort *)(iVar7 + 0x1a0);
        }
        puVar1 = puVar1 + DAT_006c54a0 * 6;
      }
    }
  }
  puVar1 = local_20;
  iVar7 = local_24;
  if (DAT_006e1f3c != 0) {
    iVar8 = ((int)param_1 - DAT_006c54dc) / 0xc;
    local_2c = iVar8 / DAT_006c54a0;
    iVar8 = iVar8 - (&DAT_006cb8c2)[local_2c];
    param_2 = FUN_004132e0(iVar8 / DAT_006c5494);
    local_30 = FUN_0040e650(iVar8 - (&DAT_006cb942)[param_2]);
    puVar11 = &local_c;
    puVar12 = puVar11;
    FUN_0040eec0(&local_2c);
    uVar9 = extraout_ECX;
    FUN_0040e8d0(&param_2);
    uVar6 = extraout_ECX_00;
    FUN_0040e2c0(&local_30);
    FUN_004f49e0(uVar6,uVar9,puVar11,puVar12);
  }
  if ((param_1[5] & 8) == 0) {
    FUN_004eb710(*(undefined4 *)(iVar7 + 0xa4));
  }
  else {
    uVar6 = *(undefined4 *)(iVar7 + 0xa4);
    for (puVar11 = (undefined4 *)*DAT_006c5c78; puVar11 != DAT_006c5c78;
        puVar11 = (undefined4 *)*puVar11) {
      if ((ushort *)puVar11[2] == param_1) {
        param_1[5] = param_1[5] & 0xfff7;
        FUN_004eb710(uVar6);
        FUN_004bfeb0(&local_20,puVar11);
        break;
      }
    }
  }
  local_20 = param_1;
  cVar5 = FUN_00502840();
  uVar6 = extraout_ECX_01;
  if (cVar5 != '\0') {
    FUN_00472b30(&local_20);
    uVar6 = extraout_ECX_02;
  }
  if (DAT_006e1f3c != 0) {
    puVar11 = &local_18;
    FUN_0040eec0(&local_2c);
    uVar10 = extraout_ECX_03;
    FUN_0040e8d0(&param_2);
    uVar9 = extraout_ECX_04;
    FUN_0040e2c0(&local_30);
    FUN_004f49e0(uVar9,uVar10,uVar6,puVar11);
    iVar7 = FUN_00540430(local_c,local_8,local_4,local_18,local_14,local_10);
    if (iVar7 != -1) {
      uVar10 = extraout_ECX_05;
      FUN_0040eec0(&local_2c);
      uVar9 = extraout_ECX_06;
      FUN_0040e8d0(&param_2);
      uVar6 = extraout_ECX_07;
      FUN_0040e2c0(&local_30);
      FUN_00540140(uVar6,uVar9,uVar10,iVar7);
    }
  }
  uVar2 = param_1[1];
  if (uVar2 != 0xffff) {
    if (uVar2 < DAT_0068949c) {
      iVar7 = DAT_00689498 + (uint)uVar2 * 0x22e;
    }
    else {
      iVar7 = 0;
    }
    if (*(int *)(iVar7 + 0x28) == 0xe) {
      if (*param_1 != 0) goto LAB_004eb457;
      FUN_004943f0();
    }
    else if (*(int *)(iVar7 + 0x28) == 0x21) {
      FUN_00490260();
    }
  }
  if ((*param_1 == 0) && (FUN_004ec8b0(puVar1,local_2c), *param_1 == 0)) {
    return 1;
  }
LAB_004eb457:
  if ((0 < local_1c) && ((*(byte *)((int)param_1 + 9) & 0x40) == 0)) {
    FUN_004eb040(local_1c);
  }
  return 1;
}

