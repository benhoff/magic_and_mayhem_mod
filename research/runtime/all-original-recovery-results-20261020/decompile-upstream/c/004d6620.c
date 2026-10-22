
void __thiscall FUN_004d6620(int param_1,uint *param_2,undefined4 param_3)

{
  int *piVar1;
  int *piVar2;
  int iVar3;
  uint *puVar4;
  int iVar5;
  uint uVar6;
  char cVar7;
  uint uVar8;
  
  if (0x60 < *param_2) {
    return;
  }
  if ((param_2[4] == 0) && (0 < (int)param_2[1])) {
    param_2[4] = 1;
  }
  uVar6 = *(uint *)(param_1 + 0x1518);
  uVar8 = 0;
  if (uVar6 != 0) {
    puVar4 = (uint *)(param_1 + 0x1b1);
    do {
      if ((puVar4[1] == 1) && (*puVar4 == *param_2)) {
        iVar5 = param_1 + uVar8 * 0xc6;
        cVar7 = *(char *)(param_1 + 0x1c1 + uVar8 * 0xc6) + '\x01';
        *(char *)(iVar5 + 0x1c1) = cVar7;
        *(int *)(iVar5 + 0x176) = (int)cVar7;
        (**(code **)(*(int *)(iVar5 + 0xfc) + 0x10))();
        return;
      }
      uVar8 = uVar8 + 1;
      puVar4 = (uint *)((int)puVar4 + 0xc6);
    } while (uVar8 < uVar6);
  }
  if (0x19 < uVar6) {
    if (*(int *)(param_1 + 0x183c) == 0) {
      if (DAT_006e2032 == '\0') {
        return;
      }
      MessageBoxA((HWND)0x0,s_Too_many_popup_buttons_005e2f20,s_SELECT_BAR_MESSAGE_ERROR_005e2ea0,
                  0x42030);
      return;
    }
    if (0x19 < uVar6) {
      piVar1 = (int *)(param_1 + 0x17ec);
      piVar2 = (int *)(param_1 + 0xfc + *(int *)(param_1 + 0x17ec) * 0xc6);
      iVar5 = *(int *)(param_1 + 0x139 + *(int *)(param_1 + 0x17ec) * 0xc6);
      *(undefined4 *)((int)piVar2 + 0x39) = 0;
      if (iVar5 != 2) {
        piVar2[2] = 1;
      }
      (**(code **)(*piVar2 + 0x10))();
      iVar5 = param_1 + 0xfc + *piVar1 * 0xc6;
      *(undefined4 *)(iVar5 + 0x3d) = 0;
      *(undefined4 *)(iVar5 + 8) = 1;
      FUN_004d53d0(*piVar1,*(undefined4 *)(param_1 + 0x1b1 + *piVar1 * 0xc6));
      iVar5 = *(int *)(param_1 + 0x183c) + -1;
      *(int *)(param_1 + 0x183c) = iVar5;
      FUN_0059cb60(piVar1,param_1 + 0x17f4,iVar5 * 8);
    }
  }
  *(uint *)(param_1 + 0x1b1 + *(int *)(param_1 + 0x1518) * 0xc6) = *param_2;
  *(undefined4 *)(param_1 + 0x1b5 + *(int *)(param_1 + 0x1518) * 0xc6) = 1;
  *(undefined4 *)(param_1 + 0x1bd + *(int *)(param_1 + 0x1518) * 0xc6) = param_3;
  *(undefined1 *)(param_1 + 0x1c1 + *(int *)(param_1 + 0x1518) * 0xc6) = 1;
  *(undefined4 *)(param_1 + 0x176 + *(int *)(param_1 + 0x1518) * 0xc6) = 1;
  iVar5 = param_1 + 0xfc + *(int *)(param_1 + 0x1518) * 0xc6;
  *(undefined4 *)(iVar5 + 0x3d) = 0;
  *(undefined4 *)(iVar5 + 8) = 1;
  iVar5 = param_1 + 0xfc + *(int *)(param_1 + 0x1518) * 0xc6;
  *(uint *)(iVar5 + 0x59) = *param_2 + 0x86;
  *(undefined4 *)(iVar5 + 0x5d) = 0xffffffff;
  *(undefined4 *)(iVar5 + 0x61) = 0xffffffff;
  *(uint *)(param_1 + 0x131 + *(int *)(param_1 + 0x1518) * 0xc6) = *param_2 + 0x62;
  if (*param_2 < DAT_006b1254) {
    iVar5 = *param_2 * 0x20 + DAT_006b1250;
  }
  else {
    iVar5 = 0;
  }
  if ((param_2[4] == *(uint *)(iVar5 + 0xc)) ||
     (uVar6 = (int)(param_2[4] * 5) / (int)*(uint *)(iVar5 + 0xc), 4 < uVar6)) {
    uVar6 = 5;
  }
  *(uint *)(param_1 + 0x161 + *(int *)(param_1 + 0x1518) * 0xc6) = uVar6 + 0x80;
  *(uint *)(param_1 + 0x1b9 + *(int *)(param_1 + 0x1518) * 0xc6) = uVar6;
  piVar1 = (int *)(param_1 + 0xfc + *(int *)(param_1 + 0x1518) * 0xc6);
  iVar5 = *(int *)(param_1 + 0x139 + *(int *)(param_1 + 0x1518) * 0xc6);
  *(undefined4 *)((int)piVar1 + 0x39) = 0;
  if (iVar5 != 2) {
    piVar1[2] = 1;
  }
  (**(code **)(*piVar1 + 0x10))();
  if (*(int *)(param_1 + 0x1910) == 0) {
    *(undefined4 *)(param_1 + *(int *)(param_1 + 0x1518) * 0xc6 + 0x135) = 3;
    FUN_004a5000();
  }
  if (*(int *)(*(int *)(param_1 + 0x18ec) + 0x8f5) == *(int *)(*(int *)(param_1 + 0x18ec) + 0x8f1))
  {
    iVar5 = *(int *)(param_1 + 0x1518);
    iVar3 = *(int *)(param_1 + 0x1b1 + iVar5 * 0xc6);
    if (*(int *)(param_1 + 0x1b5 + iVar5 * 0xc6) == 0) {
      if (((0x1a < iVar3) && (iVar3 != 0x34)) && ((iVar3 != 0x36 && (iVar3 != 0x56))))
      goto LAB_004d69a8;
    }
    else if ((((iVar3 < 0x1c) || (0x31 < iVar3)) && (iVar3 != 0x3a)) && (iVar3 != 0xc))
    goto LAB_004d69a8;
    *(undefined4 *)(param_1 + iVar5 * 0xc6 + 0x139) = 2;
    FUN_004a5000();
  }
LAB_004d69a8:
  (**(code **)(*(int *)(param_1 + 0xfc + *(int *)(param_1 + 0x1518) * 0xc6) + 0x10))();
  *(int *)(param_1 + 0x1518) = *(int *)(param_1 + 0x1518) + 1;
  return;
}

