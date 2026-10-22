
void __fastcall FUN_004d2a40(int *param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  short sVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  
  if ((param_1[6] != -1) && (iVar10 = *param_1, iVar10 != 0)) {
    iVar1 = *(int *)(iVar10 + 0x24);
    iVar10 = *(int *)(iVar10 + 0x20);
    iVar2 = param_1[5];
    iVar9 = param_1[3] - iVar1;
    iVar3 = param_1[4];
    iVar8 = param_1[2] - iVar10;
    iVar6 = FUN_0058b660();
    if (iVar6 != 0) {
      FUN_0057dc20(*(undefined4 *)(*param_1 + 0x14));
      FUN_0057dc60((iVar3 - iVar10) + 1,(iVar2 - iVar1) + 1);
      piVar4 = (int *)param_1[1];
      uVar7 = param_1[6];
      if (*(uint *)(*piVar4 + 0xc) < uVar7) {
        uVar7 = 0;
      }
      iVar10 = *(int *)(piVar4[2] + uVar7 * 4) + piVar4[3];
      if (*(int *)(iVar10 + 0x1c) == -1) {
        (*DAT_006903dc)(iVar10,iVar8,iVar9);
      }
      else {
        sVar5 = (*DAT_006c4e3c)(iVar10,iVar8,iVar9,0,0);
        if (sVar5 == 1) {
          FUN_0057de00(iVar10,iVar8,iVar9,0,0);
        }
      }
      (**(code **)(**(int **)(*param_1 + 4) + 0x80))(*(int **)(*param_1 + 4),iVar6);
    }
  }
  return;
}

