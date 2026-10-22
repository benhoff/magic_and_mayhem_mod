
undefined4 __thiscall
FUN_00559390(int *param_1,undefined4 param_2,int param_3,int param_4,undefined4 param_5)

{
  int iVar1;
  int *piVar2;
  
  if (param_3 == 0x100) {
    if (param_4 == 0xd) {
      iVar1 = *(int *)((int)param_1 + 0x23);
      if ((iVar1 == 0) || (*(int *)((int)param_1 + 0x27) - iVar1 >> 2 == 0)) {
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = *(int **)(iVar1 + *(int *)((int)param_1 + 0x2f) * 4);
      }
      if (*(int *)((int)param_1 + 0x3b) != 0) {
        FUN_004cfbf0(1);
        return 0;
      }
      if (piVar2 != (int *)0x0) {
        (**(code **)(*piVar2 + 0x28))(1);
      }
    }
    else {
      if (param_4 == 0x1b) {
        FUN_004a39a0();
        (**(code **)(*param_1 + 0x18))();
        if ((*(char *)((int)param_1 + 0x11) != '\0') && ((char)param_1[3] == '\0')) {
          *(undefined1 *)(param_1 + 3) = 1;
          *(undefined4 *)((int)param_1 + 0xd) = 0;
        }
        *(undefined4 *)((int)param_1 + 0x43) = 1;
        return 0;
      }
      iVar1 = *(int *)((int)param_1 + 0x23);
      if ((iVar1 == 0) || (*(int *)((int)param_1 + 0x27) - iVar1 >> 2 == 0)) {
        piVar2 = (int *)0x0;
      }
      else {
        piVar2 = *(int **)(iVar1 + *(int *)((int)param_1 + 0x2f) * 4);
      }
      if (*(int *)((int)param_1 + 0x3b) != 0) {
        FUN_004cfcb0(param_2,0x100,param_4,param_5);
        return 0;
      }
      if (piVar2 != (int *)0x0) {
        (**(code **)(*piVar2 + 0x2c))(param_2,0x100,param_4,param_5);
        return 0;
      }
    }
  }
  else if (param_3 == 0x101) {
    iVar1 = *(int *)((int)param_1 + 0x23);
    if ((iVar1 == 0) || (*(int *)((int)param_1 + 0x27) - iVar1 >> 2 == 0)) {
      piVar2 = (int *)0x0;
    }
    else {
      piVar2 = *(int **)(iVar1 + *(int *)((int)param_1 + 0x2f) * 4);
    }
    if (param_4 == 0xd) {
      if (*(int *)((int)param_1 + 0x3b) == 0) {
        if (piVar2 != (int *)0x0) {
          (**(code **)(*piVar2 + 0x28))(0);
        }
      }
      else {
        FUN_004cfbf0(0);
      }
      (**(code **)(*param_1 + 0x28))();
      if (*(char *)((int)param_1 + 0x12) == '\0') {
        DAT_00642c60 = 0;
        if ((char)param_1[3] == '\0') {
          FUN_004a45b0();
        }
        FUN_0058bf40(&DAT_006a4fe0,0,0);
        FUN_004a4380();
        DAT_00642c60 = 1;
        return 0;
      }
    }
  }
  else if ((((param_3 == 0x102) && (iVar1 = *(int *)((int)param_1 + 0x23), iVar1 != 0)) &&
           (*(int *)((int)param_1 + 0x27) - iVar1 >> 2 != 0)) &&
          (piVar2 = *(int **)(iVar1 + *(int *)((int)param_1 + 0x2f) * 4), piVar2 != (int *)0x0)) {
    iVar1 = (**(code **)(*piVar2 + 0xc))();
    if (iVar1 == 1) {
      (**(code **)(*piVar2 + 0x2c))(param_2,0x102,param_4,param_5);
      return 0;
    }
  }
  return 0;
}

