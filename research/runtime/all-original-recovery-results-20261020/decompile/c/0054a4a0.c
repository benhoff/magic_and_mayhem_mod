
uint __thiscall FUN_0054a4a0(int param_1,undefined4 param_2,undefined4 param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  uint uVar6;
  
  iVar1 = *(int *)(param_1 + 0x76);
  uVar6 = iVar1 * 9;
  if (((&DAT_006f1d58)[iVar1 * 9] != 0) && (iVar2 = *(int *)(param_1 + 0x5a), iVar2 != 0)) {
    iVar3 = *(int *)(param_1 + 0x46);
    iVar5 = (*(int *)(param_1 + 0x56) * 100) / DAT_005e1428;
    if (iVar1 == 0xd) {
      iVar5 = iVar5 / 2;
    }
    if (iVar5 < 0x4c) {
      if (0x32 < iVar5) {
        uVar6 = FUN_0057f0f0(param_3,iVar3,*(undefined4 *)(param_1 + 0x72));
        return uVar6 & 0xffff0000;
      }
      if (0x19 < iVar5) {
        uVar6 = FUN_0057ec90(param_3,iVar3,*(undefined4 *)(param_1 + 0x72));
        return uVar6 & 0xffff0000;
      }
      uVar6 = FUN_0057f5f0(param_3,iVar3,*(undefined4 *)(param_1 + 0x72));
    }
    else {
      uVar4 = *(undefined4 *)(param_1 + 0x72);
      if (*(int *)(iVar2 + 0x1c) == -1) {
        uVar6 = (*DAT_006903dc)(iVar2,param_2,param_3);
        return uVar6 & 0xffff0000;
      }
      if (iVar3 == -0x7f) {
        uVar6 = FUN_00595677(iVar2,param_2,param_3);
        if ((short)uVar6 == 1) {
          uVar6 = FUN_0057de00(iVar2,param_2,param_3,0xffffff81,uVar4);
          return uVar6 & 0xffff0000;
        }
      }
      else {
        uVar6 = (*DAT_006c4e3c)(iVar2,param_2,param_3,iVar3,uVar4);
        if ((short)uVar6 == 1) {
          uVar6 = FUN_0057de00(iVar2,param_2,param_3,iVar3,uVar4);
          return uVar6 & 0xffff0000;
        }
      }
    }
  }
  return uVar6 & 0xffff0000;
}

