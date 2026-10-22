
undefined4 __thiscall FUN_005457e0(undefined4 *param_1,int *param_2)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 *puStack_4;
  
  iVar3 = -1;
  if ((int)param_1[0x700] < 0x700) {
    param_1[param_1[0x700]] = param_2;
    param_1[0x700] = param_1[0x700] + 1;
    return 1;
  }
  iVar1 = (**(code **)(*param_2 + 0x14))();
  if ((0x400 < iVar1) && ((int)param_1[0x701] < iVar1)) {
    iVar4 = 0;
    puStack_4 = param_1;
    if (0 < (int)param_1[0x700]) {
      do {
        iVar2 = (**(code **)(*(int *)*puStack_4 + 0x14))();
        if (iVar2 < iVar1) {
          iVar3 = iVar4;
          iVar1 = iVar2;
        }
        iVar4 = iVar4 + 1;
        puStack_4 = puStack_4 + 1;
      } while (iVar4 < (int)param_1[0x700]);
      if (iVar3 != -1) {
        if ((undefined4 *)param_1[iVar3] != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)param_1[iVar3])(1);
        }
        param_1[iVar3] = param_2;
        if ((int)param_1[0x701] < iVar3 + -1) {
          param_1[0x701] = iVar3 + -1;
        }
        return 1;
      }
    }
    param_1[0x701] = iVar1;
  }
  return 0;
}

