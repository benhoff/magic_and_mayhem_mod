
undefined4 __thiscall FUN_00513e40(int param_1,uint param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  
  if (*(int *)(param_1 + 0xa8) == 0x1b) {
    return 0;
  }
  uVar5 = (int)param_2 >> 0x1f;
  if (((*(int *)(param_1 + 0xa0) != 1) || (DAT_006b9577 <= (int)((param_2 ^ uVar5) - uVar5))) &&
     ((*(int *)(param_1 + 0xa0) != 2 || (DAT_006c305e <= (int)((param_2 ^ uVar5) - uVar5))))) {
    iVar1 = *(int *)(param_1 + 0x18);
    iVar2 = *(int *)(*(int *)(param_1 + 0xac) + 0x198);
    iVar3 = *(int *)(*(int *)(param_1 + 0xac) + 0x19c);
    iVar4 = *(int *)(param_1 + 0x1c);
    FUN_0051fba0(param_2);
    FUN_00571310(0x400,*(undefined4 *)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),
                 *(undefined4 *)(param_1 + 0x10),0,0,0);
    if (*(int *)(param_1 + 0xa0) == 1) {
      uVar6 = 4;
    }
    else {
      uVar6 = 1;
    }
    FUN_00549e80(iVar1 + iVar2,iVar3 + iVar4,0x80,0x80,0x80,0x20,uVar6,0);
    *(undefined **)(param_1 + 0x5f4) = &DAT_0050aca0;
    *(undefined4 *)(param_1 + 0x5f0) = 0;
    *(undefined4 *)(param_1 + 0x5ec) = 0;
    *(undefined4 *)(param_1 + 0x5f8) = 0;
    FUN_0051f5e0(0);
    FUN_00506d30();
    return 1;
  }
  return 0;
}

