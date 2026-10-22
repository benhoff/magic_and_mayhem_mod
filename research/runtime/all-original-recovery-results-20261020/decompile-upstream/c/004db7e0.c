
void __fastcall FUN_004db7e0(int param_1)

{
  int iVar1;
  int iVar2;
  DWORD DVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  bool bVar8;
  
  DVar3 = GetTickCount();
  if ((*(int *)(param_1 + 0x17a) != 0) && (*(uint *)(param_1 + 0x156) <= DVar3)) {
    iVar1 = *(int *)(param_1 + 0x14);
    iVar6 = (&DAT_00641ab8)[DAT_00641fbc * 10];
    iVar7 = *(int *)(&DAT_00641abc + DAT_00641fbc * 0x28);
    if (iVar7 + 0x14 + iVar1 < (int)((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0)) {
      iVar7 = iVar7 + 0x14;
    }
    else {
      iVar7 = iVar7 - iVar1;
    }
    iVar2 = *(int *)(param_1 + 0x17e);
    *(int *)(param_1 + 0x15e) = iVar7;
    uVar4 = iVar6 - iVar2 / 2;
    *(uint *)(param_1 + 0x15a) = uVar4;
    *(int *)(param_1 + 0x166) = iVar7 + iVar1;
    *(uint *)(param_1 + 0x162) = uVar4 + iVar2;
    if ((int)uVar4 < 0) {
      iVar5 = (uVar4 ^ (int)uVar4 >> 0x1f) - ((int)uVar4 >> 0x1f);
    }
    else {
      uVar4 = (uVar4 + iVar2) - ((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280);
      iVar5 = -(((int)uVar4 < 1) - 1 & uVar4);
    }
    bVar8 = DAT_006de6d5 != 0;
    *(int *)(param_1 + 0x162) = iVar2;
    *(int *)(param_1 + 0x166) = iVar1;
    uVar4 = (iVar7 + iVar1) - ((-(uint)bVar8 & 0x78) + 0x1e0);
    *(undefined4 *)(param_1 + 0x15a) = 0;
    *(undefined4 *)(param_1 + 0x15e) = 0;
    iVar6 = iVar6 + (iVar5 - iVar2 / 2);
    iVar7 = iVar7 - (uVar4 & ((int)uVar4 < 1) - 1);
    *(int *)(param_1 + 0x16a) = iVar6;
    *(int *)(param_1 + 0x16e) = iVar7;
    *(int *)(param_1 + 0x172) = iVar6 + iVar2;
    *(int *)(param_1 + 0x176) = iVar7 + iVar1;
    *(int *)(param_1 + 0x144) = iVar6;
    *(int *)(param_1 + 0x148) = iVar7;
    *(undefined4 *)(param_1 + 0x17a) = 0;
    *(undefined4 *)(param_1 + 4) = 1;
  }
  if (*(int *)(param_1 + 4) != 0) {
    if (DAT_006e1ee4 != 0) {
      FUN_0058d5b0(2,&DAT_006a49e0,param_1 + 0x15a,*(undefined4 *)(param_1 + 0x144),
                   *(undefined4 *)(param_1 + 0x148),2);
      return;
    }
    FUN_0058c4a0(&DAT_006a49e0,param_1 + 0x15a,*(undefined4 *)(param_1 + 0x144),
                 *(undefined4 *)(param_1 + 0x148));
  }
  return;
}

