
/* WARNING: Type propagation algorithm not settling */

void __thiscall FUN_004d9f30(int param_1,undefined4 param_2)

{
  int iVar1;
  uint local_14 [4];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0x16d) != 0) {
    FUN_0058d1a0(s_bitmaps_smallscroll_bmp_005e3030);
    local_4 = *(undefined4 *)(param_1 + 0x17d);
    local_14[3] = *(int *)(param_1 + 0x179) + -0x18;
    local_14[0] = local_14[0] & 0xffffff00;
    local_14[1] = 0x18;
    local_14[2] = 0;
    iVar1 = FUN_0058b660();
    if (iVar1 != 0) {
      FUN_0057dc20(*(undefined4 *)(param_1 + 0x17d));
      FUN_0057dc60(local_14[3],local_4);
      FUN_004a5a90(local_14,8);
      FUN_004a5a90(local_14,4);
      FUN_004a5a90(local_14,0x10);
      FUN_004a5ce0(param_2,local_14 + 1,0,0,0,local_14[0],0);
      (**(code **)(**(int **)(param_1 + 0x16d) + 0x80))(*(int **)(param_1 + 0x16d),iVar1);
      *(undefined4 *)(param_1 + 0x165) = 0x32;
      *(undefined4 *)(param_1 + 0x1b3) = 0x18;
      *(undefined4 *)(param_1 + 0x10) = 1;
      FUN_0056f000(0x328,0,0,0,0,0xffffffff,0xffffffff);
    }
  }
  return;
}

