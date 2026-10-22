
void __thiscall FUN_0058c140(int param_1,uint param_2,int param_3,int param_4)

{
  int *piVar1;
  undefined4 *puVar2;
  uint uVar3;
  int iVar4;
  HWND hWnd;
  int iVar5;
  undefined4 uVar6;
  tagPOINT local_30;
  uint uStack_28;
  uint uStack_24;
  int local_20;
  int local_1c;
  int local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  int local_4;
  
  iVar4 = param_3;
  uVar3 = param_2;
  if ((*(int *)(param_1 + 8) != 0) && (*(int *)(param_2 + 8) != 0)) {
    local_8 = *(int *)(param_1 + 0x10);
    local_4 = *(int *)(param_1 + 0x14);
    local_18 = local_8 + param_3;
    local_14 = local_4 + param_4;
    local_10 = 0;
    local_c = 0;
    local_20 = param_3;
    local_1c = param_4;
    if ((DAT_006f68d8 == 0) && (param_2 == DAT_006f98c0)) {
      local_30.x = 0;
      local_30.y = 0;
      if (DAT_006f98d0 == (int *)0x0) {
        hWnd = (HWND)0x0;
      }
      else {
        iVar5 = (**(code **)(*DAT_006f98d0 + 0x10))(DAT_006f98d0,&param_2);
        hWnd = (HWND)(~-(uint)(iVar5 != 0) & param_2);
      }
      ClientToScreen(hWnd,&local_30);
      local_1c = local_1c + local_30.y;
      local_14 = local_14 + local_30.y;
      local_20 = local_20 + local_30.x;
      local_18 = local_18 + local_30.x;
    }
    while( true ) {
      if (DAT_006f68d8 == 0) {
        piVar1 = *(int **)(uVar3 + 8);
        if (DAT_006f98d8 == 0) {
          iVar5 = (**(code **)(*piVar1 + 0x14))
                            (piVar1,&local_20,*(undefined4 *)(param_1 + 8),&local_10,0x1008000,0);
        }
        else {
          iVar5 = (**(code **)(*piVar1 + 0x14))
                            (piVar1,&local_20,*(undefined4 *)(param_1 + 8),&local_10,0x8000);
        }
      }
      else {
        if (DAT_006f98d8 == 0) {
          uVar6 = 0x11;
        }
        else {
          uVar6 = 1;
        }
        iVar5 = (**(code **)(**(int **)(uVar3 + 8) + 0x1c))
                          (*(int **)(uVar3 + 8),iVar4,param_4,*(undefined4 *)(param_1 + 8),&local_10
                           ,uVar6);
      }
      if (iVar5 == 0) break;
      FUN_00484c00(iVar5);
      if (iVar5 == -0x7789fe3e) {
        piVar1 = *(int **)(param_1 + 8);
        if (piVar1 != (int *)0x0) {
          iVar5 = (**(code **)(*piVar1 + 0x6c))(piVar1);
          if (iVar5 == 0) {
            if (DAT_006de6dd != 0) {
              local_30.x = (LONG)*(ushort *)(param_1 + 0x2c);
              local_30.y = local_30.x;
              (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&local_30);
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
              uStack_28 = (uint)*(ushort *)(uVar3 + 0x2c);
              uStack_24 = uStack_28;
              (**(code **)(**(int **)(uVar3 + 8) + 0x74))(*(int **)(uVar3 + 8),8,&uStack_28);
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

