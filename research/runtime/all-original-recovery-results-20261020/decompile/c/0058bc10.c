
void __thiscall FUN_0058bc10(int param_1,uint param_2)

{
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  uint uStack_84;
  uint uStack_80;
  undefined4 uStack_7c;
  undefined4 *puStack_78;
  undefined4 local_64 [20];
  uint local_14;
  
  puVar4 = local_64;
  for (iVar3 = 0x19; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  local_64[0] = 100;
  local_14 = param_2 & 0xffff;
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 != (int *)0x0) {
    puStack_78 = local_64;
    uStack_7c = 0x1000400;
    uStack_80 = 0;
    uStack_84 = 0;
    iVar3 = (**(code **)(*piVar1 + 0x14))(piVar1,0);
    FUN_00484c00(iVar3);
    if (iVar3 == -0x7789fe3e) {
      piVar1 = *(int **)(param_1 + 8);
      if (piVar1 == (int *)0x0) {
        iVar3 = -0x7fffbffb;
      }
      else {
        iVar3 = (**(code **)(*piVar1 + 0x6c))(piVar1);
        if (iVar3 == 0) {
          if (DAT_006de6dd != 0) {
            uStack_84 = (uint)*(ushort *)(param_1 + 0x2c);
            uStack_80 = uStack_84;
            (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&uStack_84);
          }
          puVar4 = *(undefined4 **)(param_1 + 0x28);
          if (puVar4 == (undefined4 *)0xffffffff) {
            FUN_004a2f90();
          }
          else if (puVar4 != (undefined4 *)0x0) {
            (**(code **)*puVar4)(0);
          }
        }
      }
      FUN_00484c00(iVar3);
      uVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x14))
                        (*(int **)(param_1 + 8),0,0,0,0x1000400,&uStack_7c);
      FUN_00484c00(uVar2);
    }
  }
  return;
}

