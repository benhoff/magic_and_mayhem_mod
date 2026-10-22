
void __thiscall FUN_0058bac0(int param_1,uint param_2,uint param_3)

{
  int *piVar1;
  int iVar2;
  undefined4 *puVar3;
  uint uStack_88;
  uint uStack_84;
  undefined4 uStack_80;
  undefined4 uStack_7c;
  undefined4 *puStack_78;
  undefined4 local_64 [20];
  uint local_14;
  
  puVar3 = local_64;
  for (iVar2 = 0x19; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  local_64[0] = 100;
  local_14 = param_3 & 0xffff;
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 != (int *)0x0) {
    puStack_78 = local_64;
    uStack_7c = 0x1000400;
    uStack_80 = 0;
    uStack_84 = 0;
    uStack_88 = param_2;
    iVar2 = (**(code **)(*piVar1 + 0x14))(piVar1);
    if (iVar2 != 0) {
      FUN_00484c00(iVar2);
    }
    if ((iVar2 == -0x7789fe3e) && (piVar1 = *(int **)(param_1 + 8), piVar1 != (int *)0x0)) {
      iVar2 = (**(code **)(*piVar1 + 0x6c))(piVar1);
      if (iVar2 == 0) {
        if (DAT_006de6dd != 0) {
          uStack_88 = (uint)*(ushort *)(param_1 + 0x2c);
          uStack_84 = uStack_88;
          (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&uStack_88);
        }
        puVar3 = *(undefined4 **)(param_1 + 0x28);
        if (puVar3 == (undefined4 *)0xffffffff) {
          FUN_004a2f90();
          return;
        }
        if (puVar3 != (undefined4 *)0x0) {
          (**(code **)*puVar3)(0);
        }
      }
    }
  }
  return;
}

