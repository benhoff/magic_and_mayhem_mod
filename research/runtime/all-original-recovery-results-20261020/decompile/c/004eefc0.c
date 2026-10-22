
void __thiscall
FUN_004eefc0(int param_1,int param_2,undefined4 param_3,undefined4 param_4,int param_5,int param_6,
            int param_7)

{
  ushort *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int *piVar9;
  char *pcVar10;
  int local_c;
  int local_4;
  
  local_c = 0;
  if (DAT_006e1ee8 != 0) {
    if (DAT_005e4278 == 0) {
      local_c = FUN_0059cae2(s_Section_plc_005e48a4,&DAT_005e4888);
    }
    else {
      local_c = FUN_0059cae2(s_Section_plc_005e48a4,&DAT_005dc824);
      DAT_005e4278 = 0;
      if (local_c != 0) {
        FUN_0059d50b(local_c,s_Section_Placement_log_005e488c);
        FUN_0059d50b(local_c,s____________________005e47c0);
      }
    }
    if (DAT_006e1ee8 != 0) {
      FUN_0059d50b(local_c,s____New_Section_List____005e4870);
    }
  }
  local_4 = 0;
  piVar9 = *(int **)(param_2 + 0x10);
  iVar7 = param_2;
  iVar8 = param_2;
  do {
    if (*(int *)(param_2 + 0x14) <= local_4) {
      if (DAT_006e1ee8 != 0) {
        FUN_0059c4c3(local_c);
      }
      return;
    }
    iVar6 = piVar9[1];
    if ((iVar6 < 0) || (iVar2 = *piVar9, iVar2 < 0)) goto LAB_004ef278;
    iVar3 = *(int *)(param_1 + 0x50) / 2;
    iVar4 = iVar3;
    if (iVar2 < iVar3) {
      if (iVar6 < iVar3) {
        iVar6 = 1;
        iVar4 = iVar3 + -1;
        iVar3 = iVar3 + -1;
      }
      else {
        if (iVar3 <= iVar2) goto LAB_004ef0ac;
LAB_004ef0be:
        if (iVar6 < iVar3) goto LAB_004ef0ca;
        iVar6 = 3;
        iVar4 = iVar3 + -1;
      }
    }
    else {
LAB_004ef0ac:
      if (iVar6 < iVar3) {
        iVar6 = 2;
        iVar3 = iVar3 + -1;
      }
      else {
        if (iVar2 < iVar3) goto LAB_004ef0be;
LAB_004ef0ca:
        iVar6 = 4;
      }
    }
    *piVar9 = *piVar9 - iVar4;
    piVar9[1] = piVar9[1] - iVar3;
    switch(param_7) {
    case 0:
      iVar8 = *piVar9;
      iVar7 = piVar9[1];
      break;
    case 1:
      iVar7 = *piVar9;
      iVar8 = -piVar9[1];
      break;
    case 2:
      iVar7 = piVar9[1];
      iVar8 = -*piVar9;
      goto LAB_004ef10b;
    case 3:
      iVar8 = piVar9[1];
      iVar7 = *piVar9;
LAB_004ef10b:
      iVar7 = -iVar7;
    }
    iVar6 = param_7 + iVar6 * 4;
    iVar8 = iVar8 + *(int *)(&DAT_005e425c + iVar6 * 8) + iVar4;
    iVar7 = iVar7 + *(int *)(&DAT_005e4260 + iVar6 * 8) + iVar3;
    *piVar9 = param_5 + iVar8;
    piVar9[1] = iVar7 + param_6;
    if (param_5 + iVar8 < 0) {
      *piVar9 = *piVar9 + *(int *)(param_1 + 4);
    }
    if (*(int *)(param_1 + 4) <= *piVar9) {
      *piVar9 = *piVar9 - *(int *)(param_1 + 4);
    }
    if (piVar9[1] < 0) {
      piVar9[1] = *(int *)(param_1 + 8) + piVar9[1];
    }
    if (*(int *)(param_1 + 8) <= piVar9[1]) {
      piVar9[1] = piVar9[1] - *(int *)(param_1 + 8);
    }
    puVar1 = (ushort *)
             (*(int *)(param_1 + 0x4c) +
             (*(int *)(param_1 + 0x6432 + piVar9[2] * 4) +
              *(int *)(param_1 + 0x64b2 + piVar9[1] * 4) + *piVar9) * 0xc);
    if ((puVar1[5] & 8) == 0) {
      uVar5 = (uint)*puVar1;
    }
    else {
      uVar5 = FUN_00464bd0(*puVar1);
    }
    if ((uVar5 == 0) || (*(int *)(DAT_0065660c + 0x94 + uVar5 * 0x164) != 0x10)) {
      if ((piVar9[3] == 4) &&
         ((((iVar6 = piVar9[4], iVar6 == 2 || (iVar6 == 4)) || (iVar6 == 7)) ||
          (((iVar6 == 8 || (iVar6 == 9)) || ((iVar6 == 0x11 || (iVar6 == 0x12)))))))) {
        if (param_7 == 1) {
          *piVar9 = *piVar9 + -1;
        }
        else if (param_7 == 2) {
          *piVar9 = *piVar9 + -1;
          piVar9[1] = piVar9[1] + -1;
        }
        else if (param_7 == 3) {
          piVar9[1] = piVar9[1] + -1;
        }
      }
      FUN_00533250(piVar9);
    }
    else {
      MessageBoxA((HWND)0x0,s_Deployment_Rotation_error__Objec_005e4844,s_DEPLOYMENT_ERROR_005d95c0,
                  0x42030);
    }
    if (DAT_006e1ee8 != 0) {
      if (piVar9[3] == 1) {
        pcVar10 = s_Friendly_Wizard_Site_005e482c;
      }
      else {
        if (piVar9[3] != 2) goto LAB_004ef278;
        pcVar10 = s_Enemy_Wizard_Site_005e4818;
      }
      FUN_0059d50b(local_c,pcVar10);
    }
LAB_004ef278:
    piVar9 = piVar9 + 10;
    local_4 = local_4 + 1;
  } while( true );
}

