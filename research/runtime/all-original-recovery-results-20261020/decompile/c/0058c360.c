
void __thiscall FUN_0058c360(int param_1,uint param_2,int *param_3,int param_4,int param_5)

{
  int *piVar1;
  uint uVar2;
  int *piVar3;
  int iVar4;
  HWND hWnd;
  int iVar5;
  undefined4 uVar6;
  tagPOINT local_18;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  iVar4 = param_4;
  piVar3 = param_3;
  uVar2 = param_2;
  if ((*(int *)(param_1 + 8) != 0) && (*(int *)(param_2 + 8) != 0)) {
    local_10 = param_4;
    local_c = param_5;
    local_8 = (param_3[2] - *param_3) + param_4;
    local_4 = (param_3[3] - param_3[1]) + param_5;
    if (DAT_006f68d8 == 0) {
      if (param_2 != DAT_006f98c0) {
LAB_0058c434:
        piVar1 = *(int **)(uVar2 + 8);
        if (DAT_006f98d8 == 0) {
          uVar6 = 0x1000000;
        }
        else {
          uVar6 = 0;
        }
        (**(code **)(*piVar1 + 0x14))(piVar1,&local_10,*(undefined4 *)(param_1 + 8),piVar3,uVar6,0);
        return;
      }
      local_18.x = 0;
      local_18.y = 0;
      if (DAT_006f98d0 == (int *)0x0) {
        hWnd = (HWND)0x0;
      }
      else {
        iVar5 = (**(code **)(*DAT_006f98d0 + 0x10))(DAT_006f98d0,&param_2);
        hWnd = (HWND)(~-(uint)(iVar5 != 0) & param_2);
      }
      ClientToScreen(hWnd,&local_18);
      local_c = local_c + local_18.y;
      local_10 = local_10 + local_18.x;
      local_4 = local_4 + local_18.y;
      local_8 = local_8 + local_18.x;
      if (DAT_006f68d8 == 0) goto LAB_0058c434;
    }
    piVar1 = *(int **)(uVar2 + 8);
    if (DAT_006f98d8 == 0) {
      uVar6 = 0x10;
    }
    else {
      uVar6 = 0;
    }
    (**(code **)(*piVar1 + 0x1c))(piVar1,iVar4,param_5,*(undefined4 *)(param_1 + 8),piVar3,uVar6);
  }
  return;
}

