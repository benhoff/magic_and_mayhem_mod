
void __thiscall FUN_004dd6c0(int param_1,int param_2)

{
  int *piVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int local_10;
  int local_c;
  int local_8;
  
  iVar3 = FUN_0058b660();
  if (iVar3 != 0) {
    FUN_0057dc20(*(undefined4 *)(param_2 + 0x14));
    FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    piVar1 = *(int **)(param_1 + 0x20);
    uVar4 = *(uint *)(param_1 + 0x24);
    iVar7 = *(int *)(param_1 + 4);
    iVar5 = *(int *)(param_1 + 8);
    if (*(uint *)(*piVar1 + 0xc) < uVar4) {
      uVar4 = 0;
    }
    iVar6 = *(int *)(piVar1[2] + uVar4 * 4) + piVar1[3];
    if (*(int *)(param_1 + 0xc) == 0) {
      local_c = 0;
      local_10 = *(int *)(iVar6 + 4) + 1;
    }
    else if (*(int *)(param_1 + 0xc) == 1) {
      local_10 = 0;
      local_c = *(int *)(iVar6 + 8) + 1;
    }
    local_8 = *(int *)(param_1 + 0x10);
    if (local_8 < *(int *)(param_1 + 0x18)) {
      do {
        FUN_0040a150(iVar6,iVar7,iVar5,0,0);
        iVar5 = iVar5 + local_c;
        iVar7 = iVar7 + local_10;
        local_8 = local_8 + 1;
      } while (local_8 < *(int *)(param_1 + 0x18));
    }
    piVar1 = *(int **)(param_1 + 0x20);
    uVar4 = *(uint *)(param_1 + 0x28);
    if (*(uint *)(*piVar1 + 0xc) < uVar4) {
      uVar4 = 0;
    }
    iVar6 = *(int *)(piVar1[2] + uVar4 * 4) + piVar1[3];
    if (*(int *)(param_1 + 0xc) == 0) {
      local_c = 0;
      local_10 = *(int *)(iVar6 + 4) + 1;
    }
    else if (*(int *)(param_1 + 0xc) == 1) {
      local_10 = 0;
      local_c = *(int *)(iVar6 + 8) + 1;
    }
    if (local_8 < *(int *)(param_1 + 0x1c)) {
      do {
        if (*(int *)(iVar6 + 0x1c) == -1) {
          (*DAT_006903dc)(iVar6,iVar7,iVar5);
        }
        else {
          sVar2 = (*DAT_006c4e3c)(iVar6,iVar7,iVar5,0,0);
          if (sVar2 == 1) {
            FUN_0057de00(iVar6,iVar7,iVar5,0,0);
          }
        }
        iVar5 = iVar5 + local_c;
        iVar7 = iVar7 + local_10;
        local_8 = local_8 + 1;
      } while (local_8 < *(int *)(param_1 + 0x1c));
    }
    (**(code **)(**(int **)(param_2 + 4) + 0x80))(*(int **)(param_2 + 4),iVar3);
  }
  return;
}

