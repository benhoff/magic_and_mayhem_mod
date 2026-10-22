
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004d3eb0(int *param_1,uint param_2,int param_3,int param_4)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  undefined4 uVar4;
  uint uVar5;
  
  if ((*(int *)((int)param_1 + 0x25) == 0) || ((param_2 & 1) == 0)) {
    if ((*(int *)((int)param_1 + 0x29) == 0) || ((param_2 & 4) == 0)) {
      if (((*(int *)((int)param_1 + 0x25) == 0) || ((param_2 & 2) == 0)) ||
         (*(int *)((int)param_1 + 0x39) != 1)) {
        if (((*(int *)((int)param_1 + 0x29) != 0) && ((param_2 & 8) != 0)) &&
           (*(int *)((int)param_1 + 0x39) == 1)) {
          *(undefined4 *)((int)param_1 + 0x39) = 0;
          (**(code **)(*param_1 + 0x10))();
        }
      }
      else {
        *(undefined4 *)((int)param_1 + 0x39) = 0;
        (**(code **)(*param_1 + 0x10))();
      }
    }
    else {
      iVar1 = *(int *)((int)param_1 + 0x21);
      if (iVar1 != 0) {
        (**(code **)(**(int **)(*(int *)(iVar1 + 4) + *(int *)(iVar1 + 0x10) * 4) + 0x24))();
        for (uVar5 = 0;
            (iVar2 = *(int *)(iVar1 + 4), iVar2 != 0 &&
            (uVar5 < (uint)(*(int *)(iVar1 + 8) - iVar2 >> 2))); uVar5 = uVar5 + 1) {
          piVar3 = *(int **)(iVar2 + uVar5 * 4);
          if (piVar3 == param_1) {
            (**(code **)(*piVar3 + 0x20))();
            *(uint *)(iVar1 + 0x10) = uVar5;
          }
        }
      }
      DAT_006e1f04 = 1;
      if (*(int *)((int)param_1 + 0x39) != 1) {
        FUN_0056f000(*(undefined4 *)((int)param_1 + 0x51),0,0,0,0,0xffffffff,0xffffffff);
        *(undefined4 *)((int)param_1 + 0x39) = 1;
      }
      (**(code **)(*param_1 + 0x10))();
      if (DAT_00657fec == 0) {
        _DAT_0065813e = 0;
      }
      else {
        _DAT_0065813e = GetTickCount();
      }
      DAT_00657fec = 0;
      _DAT_00658162 = 0;
      DAT_005e16fc = 2;
      if (((((*(int *)((int)param_1 + 0x11) <= param_3) && (param_3 < *(int *)((int)param_1 + 0x19))
            ) && (*(int *)((int)param_1 + 0x15) <= param_4)) &&
          (param_4 < *(int *)((int)param_1 + 0x1d))) || ((char)param_1[4] == '\0')) {
        (**(code **)**(undefined4 **)((int)param_1 + 0x29))(*(undefined4 *)((int)param_1 + 0x2d));
      }
    }
  }
  else {
    iVar1 = *(int *)((int)param_1 + 0x21);
    if (iVar1 != 0) {
      (**(code **)(**(int **)(*(int *)(iVar1 + 4) + *(int *)(iVar1 + 0x10) * 4) + 0x24))();
      for (uVar5 = 0;
          (iVar2 = *(int *)(iVar1 + 4), iVar2 != 0 &&
          (uVar5 < (uint)(*(int *)(iVar1 + 8) - iVar2 >> 2))); uVar5 = uVar5 + 1) {
        piVar3 = *(int **)(iVar2 + uVar5 * 4);
        if (piVar3 == param_1) {
          (**(code **)(*piVar3 + 0x20))();
          *(uint *)(iVar1 + 0x10) = uVar5;
        }
      }
    }
    DAT_006e1f04 = 1;
    if (*(int *)((int)param_1 + 0x39) != 1) {
      FUN_0056f000(*(undefined4 *)((int)param_1 + 0x51),0,0,0,0,0xffffffff,0xffffffff);
      *(undefined4 *)((int)param_1 + 0x39) = 1;
    }
    (**(code **)(*param_1 + 0x10))();
    if (DAT_00657fec == 0) {
      _DAT_0065813e = 0;
    }
    else {
      _DAT_0065813e = GetTickCount();
    }
    DAT_00657fec = 0;
    _DAT_00658162 = 0;
    DAT_005e16fc = 2;
    if ((((*(int *)((int)param_1 + 0x11) <= param_3) && (param_3 < *(int *)((int)param_1 + 0x19)))
        && ((*(int *)((int)param_1 + 0x15) <= param_4 && (param_4 < *(int *)((int)param_1 + 0x1d))))
        ) || ((char)param_1[4] == '\0')) {
      (**(code **)**(undefined4 **)((int)param_1 + 0x25))(*(undefined4 *)((int)param_1 + 0x2d));
    }
  }
  if (((param_2 & 0x20) != 0) && (*(int *)((int)param_1 + 0x35) != -1)) {
    uVar4 = FUN_0058a680(*(int *)((int)param_1 + 0x35));
    FUN_004dba60(uVar4);
  }
  if ((param_2 & 0x40) != 0) {
    if (DAT_00657fec == 0) {
      _DAT_0065813e = 0;
    }
    else {
      _DAT_0065813e = GetTickCount();
    }
    DAT_00657fec = 0;
    _DAT_00658162 = 0;
    DAT_005e16fc = 2;
  }
  if (((param_2 & 0x40) != 0) && (*(int *)((int)param_1 + 0x39) == 1)) {
    *(undefined4 *)((int)param_1 + 0x39) = 0;
    (**(code **)(*param_1 + 0x10))();
  }
  return;
}

