
void __thiscall FUN_004d8cb0(int param_1,undefined4 param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  int local_4;
  
  if (*(int *)(param_1 + 0x15e) != 0) {
    iVar3 = *(int *)(param_1 + 0x15a) + -1;
    if (*(int *)(param_1 + 4) != 0) {
      local_4 = *(int *)(param_1 + 0x15a) * *(int *)(param_1 + 0x156);
      local_c = *(int *)(param_1 + 0x156) * iVar3;
      local_10 = 0;
      local_8 = 0x194;
      if (DAT_006de6dd == 0) {
        uVar4 = 0;
      }
      else {
        uVar4 = 0x1f;
      }
      FUN_0058bac0(&local_10,uVar4);
      iVar1 = FUN_0058b660();
      if (iVar1 != 0) {
        FUN_0057dc20(*(undefined4 *)(param_1 + 0x138));
        FUN_0057dc60(local_8,local_4);
        uVar2 = FUN_004a5ce0(param_2,&local_10,0,0,0,0,0);
        *(undefined4 *)(*(int *)(param_1 + 0xc) + iVar3 * 4) = 100;
        (**(code **)(**(int **)(param_1 + 0x128) + 0x80))(*(int **)(param_1 + 0x128),iVar1);
        iVar1 = iVar3 * 0x10;
        *(undefined4 *)(iVar1 + *(int *)(param_1 + 0x10)) = 0;
        *(int *)(iVar1 + 4 + *(int *)(param_1 + 0x10)) = iVar3 * *(int *)(param_1 + 0x156);
        *(uint *)(iVar1 + 8 + *(int *)(param_1 + 0x10)) = uVar2 + 3 & 0xfffffffc;
        iVar1 = iVar1 + *(int *)(param_1 + 0x10);
        *(int *)(iVar1 + 0xc) = *(int *)(iVar1 + 4) + *(int *)(param_1 + 0x156);
        *(uint *)(*(int *)(param_1 + 0x14) + iVar3 * 4) =
             (((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280) / 2 - (uVar2 >> 1)) + 3 & 0xfffffffc;
      }
    }
  }
  return;
}

