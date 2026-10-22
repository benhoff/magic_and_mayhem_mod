
void __fastcall FUN_00512460(int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  uint uVar6;
  int *piVar7;
  undefined4 *puVar8;
  int iVar9;
  undefined4 *puVar10;
  undefined4 uVar11;
  undefined4 *puVar12;
  undefined4 uVar13;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  undefined4 local_30 [4];
  undefined1 local_20;
  undefined4 local_1f;
  undefined1 local_1b;
  undefined4 local_18 [6];
  
  if ((*(char *)(param_1 + 0x71c) != '\0') && (*(int *)(param_1 + 0x10c) == 3)) {
    if ((*(int *)(param_1 + 0x110) != 0) || (*(char *)(param_1 + 0x71d) != '\0')) {
      *(undefined4 *)(param_1 + 0x104) = 0;
      if ((*(int *)(param_1 + 0x7cd) == 3) && (*(char *)(param_1 + 0x71d) != '\0')) {
        *(undefined4 *)(param_1 + 0x7cd) = 0;
        *(undefined4 *)(param_1 + 0x809) = 0;
        FUN_00464c80();
        if (*(int *)(param_1 + 0x805) != 0) {
          FUN_004de090(0,1);
          *(undefined4 *)(param_1 + 0x805) = 0;
        }
      }
      *(undefined1 *)(param_1 + 0x71d) = 0;
    }
    uVar13 = 0;
    *(undefined1 *)(param_1 + 0x71c) = 0;
    uVar11 = 0xffffffff;
    if (*(int *)(param_1 + 0xa8) == 0xf) {
      uVar4 = *(undefined4 *)(param_1 + 0xe4);
    }
    else {
      uVar4 = FUN_00513c20(*(undefined4 *)(param_1 + 0xa4),*(undefined4 *)(param_1 + 0x10));
    }
    FUN_00514860(uVar4,uVar11,uVar13);
    if (*(int *)(param_1 + 4) == 0) {
      return;
    }
  }
  iVar5 = *(int *)(param_1 + 0x977);
  *(undefined4 *)(param_1 + 0xb5b) = 0;
  *(undefined4 *)(param_1 + 0xb57) = 0;
  *(undefined4 *)(param_1 + 0x618) = 0;
  FUN_005070e0(*(undefined4 *)(param_1 + 0x97f + iVar5 * 0x1c),
               *(undefined4 *)(param_1 + 0x983 + iVar5 * 0x1c),
               *(undefined4 *)(param_1 + 0x987 + iVar5 * 0x1c),*(undefined4 *)(param_1 + 0xa8));
  if (*(int *)(param_1 + 0xfc) != 0) {
    *(undefined4 *)(param_1 + 0xfc) = 0;
    iVar5 = FUN_00416d80(0,0,0,(float)DAT_005e1908 / (float)DAT_005e1904);
    piVar7 = (int *)(param_1 + 0x678);
    iVar9 = 2;
    do {
      iVar2 = *piVar7;
      if (iVar2 != 0) {
        for (iVar3 = *(int *)(iVar2 + 0x39); iVar3 != 0; iVar3 = *(int *)(iVar3 + 0x39)) {
          iVar2 = iVar3;
        }
        if (*(int *)(iVar2 + 5) != 0) {
          piVar1 = (int *)(*(int *)(iVar2 + 5) + 0x33);
          *piVar1 = *piVar1 + iVar5;
        }
      }
      piVar7 = piVar7 + 1;
      iVar9 = iVar9 + -1;
    } while (iVar9 != 0);
  }
  FUN_0051a8f0();
  FUN_0050f850();
  FUN_0051aa30();
  if (*(int *)(param_1 + 4) == 0) {
    return;
  }
  uVar6 = *(int *)(param_1 + 0x977) + 1;
  *(uint *)(param_1 + 0x977) = uVar6;
  if (((uVar6 < *(uint *)(param_1 + 0x97b)) ||
      (*(undefined4 *)(param_1 + 0xb8b) = 0, DAT_006e1f3c == 0)) || (*(int *)(param_1 + 0xd03) == 0)
     ) goto LAB_005127c3;
  local_34 = 0;
  iVar5 = *(int *)(param_1 + 0xd07 + *(int *)(param_1 + 0xdcf) * 4);
  if (((iVar5 < DAT_006eaf0c) && (-1 < iVar5)) && (iVar5 = DAT_006eaf08 + iVar5 * 0x294, iVar5 != 0)
     ) {
    local_3c = *(int *)(iVar5 + 4);
    local_40 = *(int *)(iVar5 + 8);
    local_38 = *(int *)(iVar5 + 0xc);
    FUN_00513cf0(&local_3c,&local_40,&local_38,*(undefined4 *)(iVar5 + 0x20c),
                 *(undefined4 *)(param_1 + 0x96b),*(undefined4 *)(param_1 + 0x96f),
                 *(undefined4 *)(param_1 + 0x973));
    if (((*(int *)(param_1 + 8) == local_3c) && (*(int *)(param_1 + 0xc) == local_40)) &&
       (*(int *)(param_1 + 0x10) == local_38)) {
      local_34 = 1;
    }
    uVar6 = *(int *)(param_1 + 0x97b) + 2;
    if (((((0xf < uVar6) || ((int)uVar6 < 0)) ||
         (iVar5 = param_1 + uVar6 * 0x1c, *(int *)(param_1 + 0x97f + uVar6 * 0x1c) != local_3c)) ||
        ((*(int *)(iVar5 + 0x983) != local_40 || (*(int *)(iVar5 + 0x987) != local_38)))) &&
       (local_34 == 0)) {
      if ((*(char *)(param_1 + 0x723) == '\0') && (*(int *)(param_1 + 0x5ec) != 0xe)) {
        local_1b = 0;
      }
      else {
        local_1b = 1;
      }
      iVar5 = *(int *)(param_1 + 0xac);
      local_20 = 0;
      local_30[0] = *(undefined4 *)(iVar5 + 0xc);
      local_30[1] = *(undefined4 *)(iVar5 + 8);
      local_30[3] = *(undefined4 *)(iVar5 + 0x44);
      local_30[2] = *(undefined4 *)(param_1 + 0x104);
      local_1f = *(undefined4 *)(param_1 + 0xa8);
      puVar8 = local_30;
      puVar10 = local_18;
      for (iVar5 = 5; iVar5 != 0; iVar5 = iVar5 + -1) {
        *puVar10 = *puVar8;
        puVar8 = puVar8 + 1;
        puVar10 = puVar10 + 1;
      }
      puVar12 = local_18;
      *(undefined2 *)puVar10 = *(undefined2 *)puVar8;
      puVar8 = puVar12;
      FUN_0040eeb0(local_38);
      uVar13 = extraout_ECX;
      FUN_0040e8a0(local_40);
      uVar11 = extraout_ECX_00;
      FUN_0040e290(local_3c);
      iVar5 = FUN_004f4900(uVar11,uVar13,puVar12,puVar8);
      if (iVar5 != 0) goto LAB_005127bd;
    }
    *(int *)(param_1 + 0xdcf) = *(int *)(param_1 + 0xdcf) + 1;
  }
  else {
LAB_005127bd:
    *(undefined4 *)(param_1 + 0xd03) = 0;
  }
LAB_005127c3:
  if ((0 < *(int *)(param_1 + 0x963)) &&
     (iVar5 = *(int *)(param_1 + 0x963) + -1, *(int *)(param_1 + 0x963) = iVar5, iVar5 == 0)) {
    *(undefined4 *)(param_1 + 0xde3) = 0;
    *(undefined1 *)(param_1 + 0x962) = 0;
    *(undefined4 *)(param_1 + 0x963) = 0;
    FUN_0050ab30();
  }
  return;
}

