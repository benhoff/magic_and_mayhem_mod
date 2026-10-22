
undefined4 __fastcall FUN_0058b660(int param_1)

{
  int *piVar1;
  undefined4 uVar2;
  int iVar3;
  undefined4 *puVar4;
  uint uStack_1c;
  uint uStack_18;
  
  puVar4 = &DAT_006f68e0;
  for (iVar3 = 0x1b; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  DAT_006f68e0 = 0x6c;
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 == (int *)0x0) {
    return 0;
  }
  uStack_18 = 0;
  uStack_1c = 1;
  DAT_006f8150 = (**(code **)(*piVar1 + 100))(piVar1,0,&DAT_006f68e0);
  FUN_00484c00(DAT_006f8150);
  *(undefined4 *)(param_1 + 0x18) = DAT_006f68f0;
  if (DAT_006f8150 == -0x7789fe3e) {
    piVar1 = *(int **)(param_1 + 8);
    if (piVar1 == (int *)0x0) {
      iVar3 = -0x7fffbffb;
    }
    else {
      iVar3 = (**(code **)(*piVar1 + 0x6c))(piVar1);
      if (iVar3 == 0) {
        if (DAT_006de6dd != 0) {
          uStack_1c = (uint)*(ushort *)(param_1 + 0x2c);
          uStack_18 = uStack_1c;
          (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&uStack_1c);
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
    DAT_006f8150 = iVar3;
    (**(code **)(**(int **)(param_1 + 8) + 100))(*(int **)(param_1 + 8),0,&DAT_006f68e0,1,0);
  }
  if (DAT_006f8150 == 0) goto LAB_0058b7e5;
  if (DAT_006f98c8 != (int *)0x0) {
    while (DAT_006f6950 != 0) {
      FUN_0058b3e0();
    }
    if (DAT_006f68d8 == 0) {
      if (DAT_006f98d0 != (int *)0x0) {
        (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
        DAT_006f98d0 = (int *)0x0;
        if (DAT_006f68d8 != 0) goto LAB_0058b78f;
      }
    }
    else {
LAB_0058b78f:
      (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
      DAT_006f68d8 = 0;
    }
    (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
    (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
    (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
    DAT_006f98c4 = (int *)0x0;
    DAT_006f98c8 = (int *)0x0;
    DAT_006f98cc = (int *)0x0;
    DAT_006f98c0 = 0;
  }
  DestroyWindow(DAT_00656614);
LAB_0058b7e5:
  *(undefined4 *)(param_1 + 0x18) = DAT_006f68f0;
  uVar2 = DAT_006f6904;
  if (DAT_006e1f1c != 0) {
    (**(code **)(**(int **)(param_1 + 4) + 0x80))(*(int **)(param_1 + 4),DAT_006f6904);
  }
  return uVar2;
}

