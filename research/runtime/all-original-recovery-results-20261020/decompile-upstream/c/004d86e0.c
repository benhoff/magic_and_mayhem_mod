
void __thiscall FUN_004d86e0(int param_1,undefined4 param_2,uint param_3,int param_4)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 local_10;
  int local_c;
  undefined4 local_8;
  int local_4;
  
  if (param_4 < 2) {
    param_4 = 1;
  }
  if ((param_3 < *(uint *)(param_1 + 0x15a)) && (*(int *)(param_1 + 4) != 0)) {
    local_4 = (param_3 + 1) * *(int *)(param_1 + 0x156);
    local_c = *(int *)(param_1 + 0x156) * param_3;
    local_10 = 0;
    local_8 = 0x194;
    if (DAT_006de6dd == 0) {
      uVar3 = 0;
    }
    else {
      uVar3 = 0x1f;
    }
    FUN_0058bac0(&local_10,uVar3);
    iVar1 = FUN_0058b660();
    if (iVar1 != 0) {
      FUN_0057dc20(*(undefined4 *)(param_1 + 0x138));
      FUN_0057dc60(local_8,local_4);
      uVar2 = FUN_004a5ce0(param_2,&local_10,0,0,0,0,0);
      *(int *)(*(int *)(param_1 + 0xc) + param_3 * 4) = param_4;
      (**(code **)(**(int **)(param_1 + 0x128) + 0x80))(*(int **)(param_1 + 0x128),iVar1);
      iVar1 = param_3 * 0x10;
      *(undefined4 *)(*(int *)(param_1 + 0x10) + iVar1) = 0;
      *(uint *)(*(int *)(param_1 + 0x10) + 4 + iVar1) = *(int *)(param_1 + 0x156) * param_3;
      *(uint *)(*(int *)(param_1 + 0x10) + 8 + iVar1) = uVar2 + 3 & 0xfffffffc;
      iVar1 = iVar1 + *(int *)(param_1 + 0x10);
      *(int *)(iVar1 + 0xc) = *(int *)(iVar1 + 4) + *(int *)(param_1 + 0x156);
      *(uint *)(*(int *)(param_1 + 0x14) + param_3 * 4) =
           (((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280) / 2 - (uVar2 >> 1)) + 3 & 0xfffffffc;
    }
  }
  return;
}

