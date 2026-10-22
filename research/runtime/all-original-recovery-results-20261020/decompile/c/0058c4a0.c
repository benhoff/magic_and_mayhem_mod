
void __thiscall FUN_0058c4a0(int param_1,uint param_2,int *param_3,int param_4,int param_5)

{
  int *piVar1;
  undefined4 *puVar2;
  uint uVar3;
  int *piVar4;
  HWND hWnd;
  int iVar5;
  undefined4 uVar6;
  tagPOINT local_20;
  uint uStack_18;
  uint uStack_14;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  piVar4 = param_3;
  uVar3 = param_2;
  if ((*(int *)(param_1 + 8) != 0) && (*(int *)(param_2 + 8) != 0)) {
    local_10 = param_4;
    local_c = param_5;
    local_8 = (param_3[2] - *param_3) + param_4;
    local_4 = (param_3[3] - param_3[1]) + param_5;
    if ((DAT_006f68d8 == 0) && (param_2 == DAT_006f98c0)) {
      local_20.x = 0;
      local_20.y = 0;
      if (DAT_006f98d0 == (int *)0x0) {
        hWnd = (HWND)0x0;
      }
      else {
        iVar5 = (**(code **)(*DAT_006f98d0 + 0x10))(DAT_006f98d0,&param_2);
        hWnd = (HWND)(~-(uint)(iVar5 != 0) & param_2);
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
          uVar6 = 0x1000000;
        }
        else {
          uVar6 = 0;
        }
        iVar5 = (**(code **)(**(int **)(uVar3 + 8) + 0x14))
                          (*(int **)(uVar3 + 8),&local_10,*(undefined4 *)(param_1 + 8),piVar4,uVar6,
                           0);
      }
      else {
        if (DAT_006f98d8 == 0) {
          uVar6 = 0x10;
        }
        else {
          uVar6 = 0;
        }
        iVar5 = (**(code **)(**(int **)(uVar3 + 8) + 0x1c))
                          (*(int **)(uVar3 + 8),param_4,param_5,*(undefined4 *)(param_1 + 8),piVar4,
                           uVar6);
      }
      if (iVar5 == 0) break;
      FUN_00484c00(iVar5);
      if (iVar5 == -0x7789fe3e) {
        piVar1 = *(int **)(param_1 + 8);
        if (piVar1 != (int *)0x0) {
          iVar5 = (**(code **)(*piVar1 + 0x6c))(piVar1);
          if (iVar5 == 0) {
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
        piVar1 = *(int **)(uVar3 + 8);
        if (piVar1 != (int *)0x0) {
          iVar5 = (**(code **)(*piVar1 + 0x6c))(piVar1);
          if (iVar5 == 0) {
            if (DAT_006de6dd != 0) {
              uStack_18 = (uint)*(ushort *)(uVar3 + 0x2c);
              uStack_14 = uStack_18;
              (**(code **)(**(int **)(uVar3 + 8) + 0x74))(*(int **)(uVar3 + 8),8,&uStack_18);
            }
            puVar2 = *(undefined4 **)(uVar3 + 0x28);
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
  }
  return;
}

