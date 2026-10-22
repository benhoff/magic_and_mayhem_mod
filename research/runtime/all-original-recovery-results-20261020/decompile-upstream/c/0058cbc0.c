
void __thiscall FUN_0058cbc0(int param_1,int param_2,undefined4 param_3,int *param_4)

{
  int *piVar1;
  undefined4 *puVar2;
  int *piVar3;
  HWND hWnd;
  int iVar4;
  tagPOINT local_10;
  uint uStack_8;
  uint uStack_4;
  
  piVar3 = param_4;
  if ((DAT_006f68d8 == 0) && (param_2 == DAT_006f98c0)) {
    local_10.x = 0;
    local_10.y = 0;
    if (DAT_006f98d0 == (int *)0x0) {
      hWnd = (HWND)0x0;
    }
    else {
      iVar4 = (**(code **)(*DAT_006f98d0 + 0x10))(DAT_006f98d0,&param_4);
      hWnd = (HWND)(~-(uint)(iVar4 != 0) & (uint)param_4);
    }
    ClientToScreen(hWnd,&local_10);
    piVar3[1] = piVar3[1] + local_10.y;
    piVar3[3] = piVar3[3] + local_10.y;
    *piVar3 = *piVar3 + local_10.x;
    piVar3[2] = piVar3[2] + local_10.x;
  }
  while( true ) {
    iVar4 = (**(code **)(**(int **)(param_2 + 8) + 0x14))
                      (*(int **)(param_2 + 8),piVar3,*(undefined4 *)(param_1 + 8),param_3,0x1008000,
                       0);
    if (iVar4 == 0) break;
    FUN_00484c00(iVar4);
    if (iVar4 == -0x7789fe3e) {
      piVar1 = *(int **)(param_1 + 8);
      if (piVar1 != (int *)0x0) {
        iVar4 = (**(code **)(*piVar1 + 0x6c))(piVar1);
        if (iVar4 == 0) {
          if (DAT_006de6dd != 0) {
            local_10.x = (LONG)*(ushort *)(param_1 + 0x2c);
            local_10.y = local_10.x;
            (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&local_10);
          }
          puVar2 = *(undefined4 **)(param_1 + 0x28);
          if (puVar2 == (undefined4 *)0xffffffff) {
            FUN_004a2f90();
          }
          else if (puVar2 != (undefined4 *)0x0) {
            (**(code **)*puVar2)(0);
          }
        }
      }
      piVar1 = *(int **)(param_2 + 8);
      if (piVar1 != (int *)0x0) {
        iVar4 = (**(code **)(*piVar1 + 0x6c))(piVar1);
        if (iVar4 == 0) {
          if (DAT_006de6dd != 0) {
            uStack_8 = (uint)*(ushort *)(param_2 + 0x2c);
            uStack_4 = uStack_8;
            (**(code **)(**(int **)(param_2 + 8) + 0x74))(*(int **)(param_2 + 8),8,&uStack_8);
          }
          puVar2 = *(undefined4 **)(param_2 + 0x28);
          if (puVar2 == (undefined4 *)0xffffffff) {
            FUN_004a2f90();
          }
          else if (puVar2 != (undefined4 *)0x0) {
            (**(code **)*puVar2)(0);
          }
        }
      }
    }
  }
  return;
}

