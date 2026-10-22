
void __fastcall FUN_004cb5a0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  short sVar4;
  int iVar5;
  int *piVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  
  if (*(char *)(param_1 + 0x79) == '\0') {
    return;
  }
  iVar9 = *(int *)(param_1 + 0x31);
  if (iVar9 == 0) {
    return;
  }
  iVar1 = *(int *)(iVar9 + 0x20);
  iVar2 = *(int *)(param_1 + 0x19);
  iVar9 = *(int *)(iVar9 + 0x24);
  iVar10 = *(int *)(param_1 + 0x11) - iVar1;
  iVar3 = *(int *)(param_1 + 0x1d);
  iVar8 = *(int *)(param_1 + 0x15) - iVar9;
  iVar5 = FUN_0058b660();
  if (iVar5 == 0) {
    return;
  }
  FUN_0057dc20(*(undefined4 *)(*(int *)(param_1 + 0x31) + 0x14));
  FUN_0057dc60((iVar2 - iVar1) + 1,(iVar3 - iVar9) + 1);
  if (((*(int *)(param_1 + 0x3d) == 1) && (*(int *)(param_1 + 0x39) != 1)) &&
     (uVar7 = *(uint *)(param_1 + 0x69), uVar7 != 0xffffffff)) {
    piVar6 = *(int **)(param_1 + 0x55);
    if (*(uint *)(*piVar6 + 0xc) < uVar7) {
LAB_004cb637:
      uVar7 = 0;
    }
LAB_004cb639:
    iVar9 = *(int *)(piVar6[2] + uVar7 * 4) + piVar6[3];
    if (*(int *)(iVar9 + 0x1c) == -1) {
      (*DAT_006903dc)(iVar9,iVar10,iVar8);
      goto LAB_004cb741;
    }
  }
  else {
    iVar9 = *(int *)(param_1 + 0x39);
    if ((iVar9 == 1) && (uVar7 = *(uint *)(param_1 + 0x5d), uVar7 != 0xffffffff)) {
      piVar6 = *(int **)(param_1 + 0x55);
      if (*(uint *)(*piVar6 + 0xc) < uVar7) goto LAB_004cb637;
      goto LAB_004cb639;
    }
    if ((iVar9 == 0) && (uVar7 = *(uint *)(param_1 + 0x59), uVar7 != 0xffffffff)) {
      piVar6 = *(int **)(param_1 + 0x55);
      if (*(uint *)(*piVar6 + 0xc) < uVar7) goto LAB_004cb637;
      goto LAB_004cb639;
    }
    if (iVar9 != 2) {
      uVar7 = *(uint *)(param_1 + 0x65);
      if (uVar7 != 0xffffffff) {
        piVar6 = *(int **)(param_1 + 0x55);
        if (*(uint *)(*piVar6 + 0xc) < uVar7) {
          uVar7 = 0;
        }
        FUN_0040a150(*(int *)(piVar6[2] + uVar7 * 4) + piVar6[3],iVar10,iVar8,0,0);
      }
      goto LAB_004cb741;
    }
    uVar7 = *(uint *)(param_1 + 0x61);
    if (uVar7 == 0xffffffff) {
      uVar7 = *(uint *)(param_1 + 0x59);
      if (uVar7 != 0xffffffff) {
        piVar6 = *(int **)(param_1 + 0x55);
        if (*(uint *)(*piVar6 + 0xc) < uVar7) {
          uVar7 = 0;
        }
        FUN_00589c90(*(int *)(piVar6[2] + uVar7 * 4) + piVar6[3],iVar10,iVar8);
      }
      goto LAB_004cb741;
    }
    piVar6 = *(int **)(param_1 + 0x55);
    if (*(uint *)(*piVar6 + 0xc) < uVar7) {
      uVar7 = 0;
    }
    iVar9 = *(int *)(piVar6[2] + uVar7 * 4) + piVar6[3];
    if (*(int *)(iVar9 + 0x1c) == -1) {
      (*DAT_006903dc)(iVar9,iVar10,iVar8);
      goto LAB_004cb741;
    }
  }
  sVar4 = (*DAT_006c4e3c)(iVar9,iVar10,iVar8,0,0);
  if (sVar4 == 1) {
    FUN_0057de00(iVar9,iVar10,iVar8,0,0);
  }
LAB_004cb741:
  piVar6 = *(int **)(*(int *)(param_1 + 0x31) + 4);
  (**(code **)(*piVar6 + 0x80))(piVar6,iVar5);
  return;
}

