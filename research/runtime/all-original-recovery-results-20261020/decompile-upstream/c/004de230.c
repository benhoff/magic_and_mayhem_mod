
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004de230(int param_1)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  
  *(undefined4 *)(param_1 + 4) = 0;
  if (*(int **)(param_1 + 0x53) != (int *)0x0) {
    (**(code **)(**(int **)(param_1 + 0x53) + 4))(1);
    *(undefined4 *)(param_1 + 0x53) = 0;
  }
  if (*(int *)(param_1 + 0xc) != 0) {
    FUN_0058b3e0();
  }
  if (DAT_006e2090 != 0) {
    FUN_005979b0(DAT_006e2090);
    DAT_006e2090 = 0;
    _DAT_006e2094 = 0;
    DAT_006e2098 = 0;
    DAT_006e209c = 0;
  }
  iVar5 = 0x10;
  do {
    if (*(int *)((int)&DAT_006e2090 + iVar5) != 0) {
      FUN_0057d0d0(*(int *)((int)&DAT_006e2090 + iVar5));
      *(undefined4 *)((int)&DAT_006e2090 + iVar5) = 0;
    }
    iVar5 = iVar5 + 4;
  } while (iVar5 < 0x20);
  if (DAT_006e20b0 != 0) {
    FUN_005979b0(DAT_006e20b0);
    DAT_006e20b0 = 0;
  }
  piVar1 = *(int **)(param_1 + 0x4f);
  if (piVar1 != (int *)0x0) {
    if (((*(int *)(DAT_006f437c + 0x1c) != 0) && (*(int *)(DAT_006f437c + 0x18) != 0)) &&
       (*piVar1 != 0)) {
      _DAT_006fa794 = 0;
      iVar5 = (**(code **)(*(int *)*piVar1 + 0x24))((int *)*piVar1,&DAT_006fa794);
      if ((iVar5 != 0) || ((_DAT_006fa794 & 1) != 0)) {
        DAT_006fa798 = (**(code **)(*(int *)*piVar1 + 0x48))((int *)*piVar1);
        piVar2 = *(int **)(DAT_006f437c + 0x260);
        piVar3 = (int *)piVar2[4];
        piVar4 = piVar2;
        while (piVar3 != piVar1) {
          piVar4 = (int *)*piVar4;
          if (piVar4 == piVar2) {
            piVar4 = (int *)0x0;
            break;
          }
          piVar3 = (int *)piVar4[4];
        }
        FUN_00571d20(piVar4);
        if (DAT_006fa798 == 0) {
          DAT_006fa798 = (**(code **)(*(int *)*piVar1 + 0x34))((int *)*piVar1,0);
        }
      }
    }
    *(undefined4 *)(param_1 + 0x4f) = 0;
  }
  return;
}

