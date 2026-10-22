
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 __thiscall FUN_004eb490(ushort *param_1,uint param_2)

{
  int iVar1;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 *puVar8;
  int local_2c;
  undefined4 local_28;
  int local_24;
  ushort *local_20;
  ushort *local_1c;
  undefined4 local_18;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_8;
  undefined4 local_4;
  
  if ((DAT_006e8180 & 1) == 0) {
    DAT_006e8180 = DAT_006e8180 | 1;
    DAT_006e8d68 = -DAT_006c5494;
    DAT_006e8d6c = -1 - DAT_006c5494;
    _DAT_006e8d7c = DAT_006c5494 + -1;
    _DAT_006e8d88 = DAT_006c54a0;
    _DAT_006e8d84 = 1 - DAT_006c5494;
    _DAT_006e8d74 = DAT_006c5494 + 1;
    _DAT_006e8d8c = -DAT_006c54a0;
    _DAT_006e8d70 = 1;
    _DAT_006e8d78 = DAT_006c5494;
    _DAT_006e8d80 = 0xffffffff;
  }
  iVar1 = DAT_0065660c + (uint)*param_1 * 0x164;
  if (*(int *)(DAT_0065660c + 0xac + (uint)*param_1 * 0x164) != 0xff) {
    local_20 = DAT_006c54dc;
    local_1c = DAT_006c54dc + DAT_006c54a4 * 6;
    if (*(uint *)(iVar1 + 0x9c) <= param_2) {
      local_28 = 0;
      local_2c = 0;
      iVar2 = ((int)param_1 - (int)DAT_006c54dc) / 0xc;
      local_24 = iVar2 / DAT_006c54a0;
      iVar2 = iVar2 - (&DAT_006cb8c2)[local_24];
      local_2c = FUN_004132e0(iVar2 / DAT_006c5494);
      local_28 = FUN_0040e650(iVar2 - (&DAT_006cb942)[local_2c]);
      puVar6 = &local_c;
      puVar8 = puVar6;
      FUN_0040eec0(&local_24);
      uVar5 = extraout_ECX;
      FUN_0040e8d0(&local_2c);
      uVar4 = extraout_ECX_00;
      FUN_0040e2c0(&local_28);
      FUN_004f49e0(uVar4,uVar5,puVar6,puVar8);
      param_1[5] = param_1[5] & 0xfe7f;
      FUN_004eb710(*(undefined4 *)(iVar1 + 0xa4));
      if (DAT_006e1f3c != 0) {
        puVar6 = &local_18;
        puVar8 = puVar6;
        FUN_0040eec0(&local_24);
        uVar5 = extraout_ECX_01;
        FUN_0040e8d0(&local_2c);
        uVar4 = extraout_ECX_02;
        FUN_0040e2c0(&local_28);
        FUN_004f49e0(uVar4,uVar5,puVar6,puVar8);
        iVar1 = FUN_00540430(local_c,local_8,local_4,local_18,local_14,local_10);
        if (iVar1 != -1) {
          uVar7 = extraout_ECX_03;
          FUN_0040eec0(&local_24);
          uVar5 = extraout_ECX_04;
          FUN_0040e8d0(&local_2c);
          uVar4 = extraout_ECX_05;
          FUN_0040e2c0(&local_28);
          FUN_00540140(uVar4,uVar5,uVar7,iVar1);
        }
      }
      piVar3 = &DAT_006e8d68;
      do {
        if ((local_20 < param_1 + *piVar3 * 6) && (param_1 + *piVar3 * 6 < local_1c)) {
          FUN_004eb490(param_2);
        }
        piVar3 = piVar3 + 1;
      } while ((int)piVar3 < 0x6e8d90);
      return 1;
    }
  }
  return 0;
}

