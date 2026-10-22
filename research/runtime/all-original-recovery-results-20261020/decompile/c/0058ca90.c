
void __thiscall FUN_0058ca90(int param_1,int param_2,int *param_3,uint param_4,int param_5)

{
  uint uVar1;
  int iVar2;
  HWND hWnd;
  int iVar3;
  undefined4 uVar4;
  tagPOINT local_18;
  uint local_10;
  int local_c;
  int local_8;
  int local_4;
  
  iVar2 = param_5;
  uVar1 = param_4;
  local_8 = (param_3[2] - *param_3) + param_4;
  local_10 = param_4;
  local_4 = (param_3[3] - param_3[1]) + param_5;
  local_c = param_5;
  if (DAT_006f68d8 != 0) {
LAB_0058cb8f:
    if (DAT_006f98d8 == 0) {
      uVar4 = 0x11;
    }
    else {
      uVar4 = 1;
    }
    (**(code **)(**(int **)(param_2 + 8) + 0x1c))
              (*(int **)(param_2 + 8),uVar1,iVar2,*(undefined4 *)(param_1 + 8),param_3,uVar4);
    return;
  }
  if (param_2 == DAT_006f98c0) {
    local_18.x = 0;
    local_18.y = 0;
    if (DAT_006f98d0 == (int *)0x0) {
      hWnd = (HWND)0x0;
    }
    else {
      iVar3 = (**(code **)(*DAT_006f98d0 + 0x10))(DAT_006f98d0,&param_4);
      hWnd = (HWND)(~-(uint)(iVar3 != 0) & param_4);
    }
    ClientToScreen(hWnd,&local_18);
    local_c = local_c + local_18.y;
    local_10 = local_10 + local_18.x;
    local_4 = local_4 + local_18.y;
    local_8 = local_8 + local_18.x;
    if (DAT_006f68d8 != 0) goto LAB_0058cb8f;
  }
  if (DAT_006f98d8 == 0) {
    uVar4 = 0x1008000;
  }
  else {
    uVar4 = 0x8000;
  }
  (**(code **)(**(int **)(param_2 + 8) + 0x14))
            (*(int **)(param_2 + 8),&local_10,*(undefined4 *)(param_1 + 8),param_3,uVar4,0);
  return;
}

