
void __fastcall FUN_004d7a90(int param_1)

{
  uint uVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  undefined1 auStack_100 [256];
  
  if (*(int *)(param_1 + 0x18ec) == 0) {
    return;
  }
  if (DAT_006de6d9 == 0) {
    FUN_0058bc10(0x1f);
    FUN_0058bc10(0x1f);
    if (*(int *)(*(int *)(param_1 + 0x18ec) + 0xe4) == 0) goto LAB_004d7be7;
    (**(code **)(*(int *)(param_1 + 0x151c) + 0x1c))();
    iVar3 = *(int *)(param_1 + 0x151c);
  }
  else {
    FUN_0058bc10(0);
    FUN_0058bc10(0);
    (**(code **)(*(int *)(param_1 + 0x151c) + 0x1c))();
    if (*(int *)(param_1 + 0x1910) == 0) {
      FUN_004d72d0();
    }
    FUN_0059c1fd(auStack_100,s_bitmaps_border_ix_i_1_bmp_005e2f54,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    iVar3 = FUN_0058d1a0(auStack_100);
    if (iVar3 == 0) {
      FUN_0058bc10(0);
    }
    FUN_0059c1fd(auStack_100,s_bitmaps_border_ix_i_2_bmp_005e2f38,
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    iVar3 = FUN_0058d1a0(auStack_100);
    if (iVar3 == 0) {
      FUN_0058bc10(0);
    }
    (**(code **)(*(int *)(param_1 + 0x151c) + 0x18))();
    iVar3 = *(int *)(param_1 + 0x151c);
  }
  (**(code **)(iVar3 + 0x10))();
LAB_004d7be7:
  if (*(int *)(*(int *)(param_1 + 0x18ec) + 0xe4) != 0) {
    piVar4 = (int *)(param_1 + 0xfc);
    iVar3 = 0x1a;
    do {
      FUN_004d47f0();
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    iVar3 = 0x1a;
    do {
      (**(code **)(*piVar4 + 0x10))();
      piVar4 = (int *)((int)piVar4 + 0xc6);
      iVar3 = iVar3 + -1;
    } while (iVar3 != 0);
    piVar4 = (int *)(param_1 + 0x1596);
    uVar2 = 0;
    while( true ) {
      uVar1 = *(uint *)(param_1 + 0x17e8);
      if (*(uint *)(param_1 + 0x1921) <= *(uint *)(param_1 + 0x17e8)) {
        uVar1 = *(uint *)(param_1 + 0x1921);
      }
      if (uVar1 <= uVar2) break;
      FUN_004d47f0();
      uVar2 = uVar2 + 1;
    }
    uVar2 = 0;
    while( true ) {
      uVar1 = *(uint *)(param_1 + 0x17e8);
      if (*(uint *)(param_1 + 0x1921) <= *(uint *)(param_1 + 0x17e8)) {
        uVar1 = *(uint *)(param_1 + 0x1921);
      }
      if (uVar1 <= uVar2) break;
      (**(code **)(*piVar4 + 0x10))();
      uVar2 = uVar2 + 1;
      piVar4 = (int *)((int)piVar4 + 0xc6);
    }
    FUN_004d3b40();
  }
  return;
}

