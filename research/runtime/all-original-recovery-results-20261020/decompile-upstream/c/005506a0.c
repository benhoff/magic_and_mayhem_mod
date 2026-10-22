
void __thiscall FUN_005506a0(int param_1,int param_2)

{
  int iVar1;
  short sVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  int iVar6;
  int *piVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  int *local_18;
  int local_14;
  int local_10;
  int local_8;
  
  iVar3 = FUN_0058b660();
  if (iVar3 != 0) {
    FUN_0057dc20(*(undefined4 *)(param_2 + 0x14));
    FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    iVar10 = *(int *)(param_1 + 0x823);
    local_10 = 0;
    if (0 < iVar10) {
      piVar7 = (int *)(param_1 + 0x1e3);
      local_14 = 0;
      local_8 = 0;
      local_18 = &DAT_0065b1dd;
      do {
        if ((DAT_006e0080 == 0) && (*local_18 != DAT_00659f51)) {
          iVar6 = 1;
          if (1 < iVar10 + 1) {
            piVar4 = &DAT_0065b1dd;
            piVar8 = piVar7;
            do {
              if ((*piVar4 == DAT_00659f51) && (*piVar8 != 0)) goto LAB_00550773;
              iVar6 = iVar6 + 1;
              piVar4 = piVar4 + 1;
              piVar8 = piVar8 + 0x14;
            } while (iVar6 < iVar10 + 1);
          }
        }
        else {
LAB_00550773:
          iVar10 = *(int *)(*(int *)(param_1 + 0x16) + 8 + local_8);
          iVar6 = *(int *)(*(int *)(param_1 + 0x16) + 0xc + local_8);
          if (DAT_006de6d5 == 0) {
            iVar10 = (iVar10 * 0x280) / 800;
            iVar6 = (iVar6 * 0x1e0) / 600;
          }
          iVar1 = *(int *)(*(int *)(param_1 + 0x1d7) + 4 + local_14);
          iVar6 = *(int *)(iVar1 + 0xc) + iVar6;
          uVar5 = *(uint *)(iVar1 + 4);
          iVar10 = *(int *)(iVar1 + 8) + iVar10;
          if (*(uint *)(*(int *)(param_1 + 0x9f) + 0xc) < uVar5) {
            uVar5 = 0;
          }
          iVar1 = *(int *)(*(int *)(param_1 + 0xa7) + uVar5 * 4);
          iVar9 = iVar1 + *(int *)(param_1 + 0xab);
          if (*(int *)(iVar1 + 0x1c + *(int *)(param_1 + 0xab)) == -1) {
            (*DAT_006903dc)(iVar9,iVar10,iVar6);
          }
          else {
            sVar2 = (*DAT_006c4e3c)(iVar9,iVar10,iVar6,0,0);
            if (sVar2 == 1) {
              FUN_0057de00(iVar9,iVar10,iVar6,0,0);
            }
          }
        }
        FUN_00464ec0();
        iVar10 = *(int *)(param_1 + 0x823);
        local_18 = local_18 + 1;
        local_10 = local_10 + 1;
        piVar7 = piVar7 + 1;
        local_8 = local_8 + 0x8c;
        local_14 = local_14 + 0x30;
      } while (local_10 < iVar10);
    }
    (**(code **)(**(int **)(param_2 + 4) + 0x80))(*(int **)(param_2 + 4),iVar3);
  }
  return;
}

