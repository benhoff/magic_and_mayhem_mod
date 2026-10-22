
void __thiscall FUN_00550970(int param_1,int param_2)

{
  int iVar1;
  short sVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  int iVar6;
  uint uVar7;
  int *piVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  int *local_34;
  int local_30;
  int local_2c;
  int *local_24;
  undefined1 local_10 [8];
  undefined1 local_8 [8];
  
  iVar3 = FUN_0058b660();
  if (iVar3 != 0) {
    FUN_0057dc20(*(undefined4 *)(param_2 + 0x14));
    FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    local_2c = 0;
    if (0 < DAT_00659f55) {
      local_30 = 0;
      local_24 = &DAT_0065ae1d;
      local_34 = &DAT_0065a061;
      do {
        iVar6 = DAT_00659461;
        if (local_24[-0xa0] != 0) {
          if (((local_2c == DAT_00659f51) || (DAT_006e0080 != 0)) ||
             (*(int *)(&DAT_0065b1d9 + *local_24 * 4) == DAT_00659f51)) {
LAB_00550a73:
            if (local_34[1] == 0) {
              iVar6 = *local_24;
              if (*(int *)(&DAT_0065b1d9 + iVar6 * 4) == local_2c) {
                iVar10 = *(int *)(*(int *)(param_1 + 0x16) + -0x84 + iVar6 * 0x8c);
                iVar6 = *(int *)(*(int *)(param_1 + 0x16) + -0x80 + iVar6 * 0x8c);
              }
              else {
                iVar10 = local_34[2];
                iVar6 = local_34[3];
              }
            }
            else {
              iVar10 = *local_34;
              iVar4 = iVar10;
              if (iVar10 == -1) {
                iVar1 = *local_24;
                iVar11 = 0;
                iVar9 = *(int *)(local_34[-1] * 0x8c + -0x7c + DAT_0065941e);
                if (0 < iVar9) {
                  do {
                    piVar5 = (int *)FUN_004a2cf0(local_10,iVar11);
                    iVar4 = iVar11;
                    if (*(char *)(piVar5[1] * 800 + *piVar5 + iVar6) == (char)iVar1) break;
                    iVar11 = iVar11 + 1;
                    iVar4 = iVar10;
                  } while (iVar11 < iVar9);
                }
              }
              iVar6 = FUN_004a2c30(iVar4);
              piVar5 = (int *)FUN_004a2d30(local_8,*local_34,iVar6 - local_34[1]);
              iVar10 = *piVar5;
              iVar6 = piVar5[1];
            }
            if (DAT_006de6d5 == 0) {
              iVar10 = (iVar10 * 0x280) / 800;
              iVar6 = (iVar6 * 0x1e0) / 600;
            }
            iVar4 = *(int *)(*(int *)(param_1 + 0x1db) + 4 + local_30);
            iVar6 = *(int *)(iVar4 + 0xc) + iVar6;
            uVar7 = *(uint *)(iVar4 + 4);
            iVar10 = *(int *)(iVar4 + 8) + iVar10;
            if (*(uint *)(*(int *)(param_1 + 0x9f) + 0xc) < uVar7) {
              uVar7 = 0;
            }
            iVar4 = *(int *)(*(int *)(param_1 + 0xa7) + uVar7 * 4);
            iVar9 = iVar4 + *(int *)(param_1 + 0xab);
            if (*(int *)(iVar4 + 0x1c + *(int *)(param_1 + 0xab)) == -1) {
              (*DAT_006903dc)(iVar9,iVar10,iVar6);
            }
            else {
              sVar2 = (*DAT_006c4e3c)(iVar9,iVar10,iVar6,0,0);
              if (sVar2 == 1) {
                FUN_0057de00(iVar9,iVar10,iVar6,0,0);
              }
            }
          }
          else {
            iVar4 = 1;
            iVar10 = *(int *)(param_1 + 0x823) + 1;
            if (1 < iVar10) {
              piVar8 = &DAT_0065b1dd;
              piVar5 = (int *)(param_1 + 0x1df + *local_24 * 4);
              do {
                if ((*piVar8 == DAT_00659f51) && (*piVar5 != 0)) goto LAB_00550a73;
                iVar4 = iVar4 + 1;
                piVar8 = piVar8 + 1;
                piVar5 = piVar5 + 0x14;
              } while (iVar4 < iVar10);
            }
          }
          FUN_00464ec0();
        }
        local_2c = local_2c + 1;
        local_30 = local_30 + 0x30;
        local_24 = local_24 + 1;
        local_34 = local_34 + 5;
      } while (local_2c < DAT_00659f55);
    }
    (**(code **)(**(int **)(param_2 + 4) + 0x80))(*(int **)(param_2 + 4),iVar3);
  }
  return;
}

