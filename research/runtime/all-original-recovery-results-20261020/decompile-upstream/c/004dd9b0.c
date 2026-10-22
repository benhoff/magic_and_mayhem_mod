
void __thiscall FUN_004dd9b0(int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int local_3c [2];
  int local_34;
  int local_30 [3];
  int local_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  int local_4;
  
  local_3c[0] = *(int *)(param_1 + 0x20) - *(int *)(param_1 + 0x1c);
  if (*(int *)(param_1 + 0x18) == 0) {
    iVar1 = *(int *)(param_1 + 0xc);
    iVar4 = *(int *)(param_1 + 8);
    uVar2 = *(uint *)(*(int *)(param_1 + 4) + 0x14);
    local_34 = *(int *)(param_1 + 0x10) - iVar4;
    local_14 = uVar2 + iVar1;
    local_4 = (uVar2 >> 1) + iVar1;
    iVar3 = ((*(int *)(param_1 + 0x24) - *(int *)(param_1 + 0x1c)) * local_34) / local_3c[0] + iVar4
    ;
    local_24 = local_14 - iVar1;
    local_30[0] = 0;
    local_30[1] = 0;
    iVar5 = (((*(int *)(param_1 + 0x28) - *(int *)(param_1 + 0x24)) - *(int *)(param_1 + 0x1c)) *
            local_34) / local_3c[0] + iVar3;
    local_30[2] = iVar3 - iVar4;
    if (0 < local_30[2]) {
      if (DAT_006e1ee4 == 0) {
        FUN_0058c4a0(param_2,local_30,iVar4,iVar1);
      }
      else {
        FUN_0058d5b0(2,param_2,local_30,iVar4,iVar1,1);
      }
    }
    iVar4 = param_2;
    if (iVar3 < iVar5) {
      local_30[0] = (*(uint *)(*(int *)(param_1 + 4) + 0x10) >> 1) + local_30[2];
      local_30[2] = (local_30[0] - iVar3) + iVar5;
      if (DAT_006e1ee4 == 0) {
        FUN_0058c4a0(param_2,local_30,iVar3,iVar1);
        iVar4 = param_2;
      }
      else {
        FUN_0058d5b0(2,param_2,local_30,iVar3,iVar1,1);
      }
    }
    local_34 = FUN_0058b660();
    if (local_34 != 0) {
      local_14 = local_4;
      local_18 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
      local_20 = iVar5;
      local_1c = iVar1;
      FUN_0057dc20(*(undefined4 *)(iVar4 + 0x14));
      FUN_0057dc60(local_18,(-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
      FUN_0059de2f(*(undefined4 *)(param_1 + 0x28),local_3c,10);
      FUN_004a5a90(&param_2,1);
      FUN_004a5a90(&param_2,8);
      FUN_004a5ce0(local_3c,&local_20,0,0,0,param_2,0);
      (**(code **)(**(int **)(iVar4 + 4) + 0x80))(*(int **)(iVar4 + 4),local_34);
    }
  }
  return;
}

