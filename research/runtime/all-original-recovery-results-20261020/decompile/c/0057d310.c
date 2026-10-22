
undefined4 __thiscall FUN_0057d310(int *param_1,LPCSTR param_2,int param_3)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int *piVar10;
  int local_1c;
  undefined1 local_18 [8];
  int local_10;
  
  if (*param_1 != 0) {
    FUN_005979b0(*param_1);
    *param_1 = 0;
    param_1[1] = 0;
    param_1[2] = 0;
    param_1[3] = 0;
  }
  piVar10 = param_1 + 4;
  local_1c = 4;
  do {
    piVar2 = (int *)*piVar10;
    if ((int *)*piVar10 != (int *)0x0) {
      do {
        piVar1 = (int *)*piVar2;
        FUN_005979b0(piVar2);
        piVar2 = piVar1;
      } while (piVar1 != (int *)0x0);
      *piVar10 = 0;
    }
    piVar10 = piVar10 + 1;
    local_1c = local_1c + -1;
  } while (local_1c != 0);
  if (param_1[8] != 0) {
    FUN_005979b0(param_1[8]);
    param_1[8] = 0;
  }
  iVar3 = FUN_004a1120();
  if (iVar3 == 0) {
    return 0;
  }
  uVar4 = FUN_005b74d8(iVar3);
  iVar5 = FUN_0059d9eb(uVar4);
  if (iVar5 == 0) {
    FUN_0059c4c3();
    return 0;
  }
  iVar6 = FUN_0059c359(local_18,1,0x18,iVar3);
  if (iVar6 != 0x18) {
    FUN_0059c4c3(iVar3);
    return 0;
  }
  FUN_0059cf25(iVar3,0,0);
  if (local_10 != 4) {
    MessageBoxA((HWND)0x0,param_2,s_Incorrect_Sprite_Version_005f1430,0x42030);
    FUN_0059c4c3(iVar3);
    return 0;
  }
  iVar6 = FUN_00597880(iVar5);
  *param_1 = iVar6;
  if (iVar6 == 0) {
    FUN_0059c4c3();
    return 0;
  }
  iVar6 = FUN_0059c359(iVar6,1,iVar5,iVar3);
  if (iVar6 != iVar5) {
    FUN_0059c4c3(iVar3);
    return 0;
  }
  iVar5 = *param_1;
  if (*(int *)(iVar5 + 0x10) == 0) {
    param_1[1] = 0;
    param_1[2] = iVar5 + 0x18;
    param_1[3] = iVar5 + 0x18 + *(int *)(iVar5 + 0xc) * 4;
  }
  else {
    param_1[1] = iVar5 + 0x18;
    iVar6 = iVar5 + 0x18 + *(int *)(iVar5 + 0x10) * 0x300;
    param_1[2] = iVar6;
    param_1[3] = iVar6 + *(int *)(iVar5 + 0xc) * 4;
  }
  FUN_0059c4c3(iVar3);
  uVar7 = *(uint *)(*param_1 + 0x10);
  if (((uVar7 == 0) || (param_3 != 0)) && (uVar7 < 5)) {
    if (uVar7 == 0) {
      if (DAT_006e1f88 != 0) {
        FUN_0057d560();
      }
    }
    else {
      iVar3 = FUN_0057d690(0,param_3);
      if (iVar3 == 0) {
        return 0;
      }
      uVar8 = 0;
      uVar7 = *(uint *)(*param_1 + 0xc);
      if (uVar7 != 0) {
        do {
          uVar9 = uVar8;
          if (uVar7 < uVar8) {
            uVar9 = 0;
          }
          iVar3 = *(int *)(param_1[2] + uVar9 * 4);
          uVar8 = uVar8 + 1;
          *(int *)(iVar3 + param_1[3] + 0x1c) = param_1[*(int *)(iVar3 + 0x1c + param_1[3]) + 4];
          uVar7 = *(uint *)(*param_1 + 0xc);
        } while (uVar8 < uVar7);
        return 1;
      }
    }
    return 1;
  }
  return 0;
}

