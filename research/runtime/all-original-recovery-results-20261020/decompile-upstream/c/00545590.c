
void __thiscall FUN_00545590(undefined4 *param_1,int param_2,undefined4 param_3,int param_4)

{
  int iVar1;
  int iVar2;
  
  param_1[2] = param_3;
  param_1[1] = param_2;
  iVar1 = DAT_005eb488 * 2;
  iVar2 = param_4;
  if (iVar1 <= param_4) {
    iVar2 = iVar1;
  }
  if (iVar2 < 0) {
    iVar1 = 0;
  }
  else if (param_4 < iVar1) {
    iVar1 = param_4;
  }
  param_1[3] = iVar1;
  *param_1 = &PTR_FUN_005c73a8;
  if (param_2 < 0) {
    do {
      iVar1 = param_1[1] + DAT_005eb480;
      param_1[1] = iVar1;
    } while (iVar1 < 0);
  }
  else if (DAT_005eb480 <= param_2) {
    do {
      iVar1 = param_1[1] - DAT_005eb480;
      param_1[1] = iVar1;
    } while (DAT_005eb480 <= iVar1);
  }
  if (-1 < (int)param_1[2]) {
    if (DAT_005eb484 <= (int)param_1[2]) {
      do {
        iVar1 = param_1[2] - DAT_005eb484;
        param_1[2] = iVar1;
      } while (DAT_005eb484 <= iVar1);
    }
    return;
  }
  do {
    iVar1 = param_1[2] + DAT_005eb484;
    param_1[2] = iVar1;
  } while (iVar1 < 0);
  return;
}

