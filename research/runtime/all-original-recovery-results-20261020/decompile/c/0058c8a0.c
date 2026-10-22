
void __thiscall FUN_0058c8a0(int param_1,int param_2,int *param_3,int param_4,int param_5)

{
  int *piVar1;
  undefined4 *puVar2;
  int *piVar3;
  HWND hWnd;
  int iVar4;
  undefined4 uVar5;
  tagPOINT local_20;
  uint uStack_18;
  uint uStack_14;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  piVar3 = param_3;
  local_10 = param_4;
  local_8 = (param_3[2] - *param_3) + param_4;
  local_4 = (param_3[3] - param_3[1]) + param_5;
  local_c = param_5;
  if ((DAT_006f68d8 == 0) && (param_2 == DAT_006f98c0)) {
    local_20.x = 0;
    local_20.y = 0;
    if (DAT_006f98d0 == (int *)0x0) {
      hWnd = (HWND)0x0;
    }
    else {
      iVar4 = (**(code **)(*DAT_006f98d0 + 0x10))(DAT_006f98d0,&param_3);
      hWnd = (HWND)(~-(uint)(iVar4 != 0) & (uint)param_3);
    }
    ClientToScreen(hWnd,&local_20);
    local_c = local_c + local_20.y;
    local_4 = local_4 + local_20.y;
    local_10 = local_10 + local_20.x;
    local_8 = local_8 + local_20.x;
  }
  while( true ) {
    if (DAT_006f68d8 == 0) {
      if (DAT_006f98d8 == 0) {
        uVar5 = 0x1008000;
      }
      else {
        uVar5 = 0x8000;
      }
      iVar4 = (**(code **)(**(int **)(param_2 + 8) + 0x14))
                        (*(int **)(param_2 + 8),&local_10,*(undefined4 *)(param_1 + 8),piVar3,uVar5,
                         0);
    }
    else {
      if (DAT_006f98d8 == 0) {
        uVar5 = 0x11;
      }
      else {
        uVar5 = 1;
      }
      iVar4 = (**(code **)(**(int **)(param_2 + 8) + 0x1c))
                        (*(int **)(param_2 + 8),param_4,param_5,*(undefined4 *)(param_1 + 8),piVar3,
                         uVar5);
    }
    if (iVar4 == 0) break;
    FUN_00484c00(iVar4);
    if (iVar4 == -0x7789fe3e) {
      piVar1 = *(int **)(param_1 + 8);
      if (piVar1 != (int *)0x0) {
        iVar4 = (**(code **)(*piVar1 + 0x6c))(piVar1);
        if (iVar4 == 0) {
          if (DAT_006de6dd != 0) {
            local_20.x = (LONG)*(ushort *)(param_1 + 0x2c);
            local_20.y = local_20.x;
            (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&local_20);
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
            uStack_18 = (uint)*(ushort *)(param_2 + 0x2c);
            uStack_14 = uStack_18;
            (**(code **)(**(int **)(param_2 + 8) + 0x74))(*(int **)(param_2 + 8),8,&uStack_18);
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

