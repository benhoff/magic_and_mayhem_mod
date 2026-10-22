
void __thiscall FUN_004dba60(int param_1,char *param_2)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  DWORD DVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint uVar8;
  int iVar9;
  int iVar10;
  uint local_30;
  int local_2c;
  int local_28;
  int local_24;
  undefined4 local_20;
  undefined4 local_1c;
  uint local_18;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  int local_4;
  
  if ((*(int *)(param_1 + 4) == 0) && (*param_2 != '\0')) {
    local_20 = 0;
    local_1c = 0;
    local_18 = *(undefined4 *)(param_1 + 0x10);
    local_14 = *(int *)(param_1 + 0xc) + 8;
    *(int *)(param_1 + 0x14) = local_14;
    local_30 = local_30 & 0xffffff00;
    FUN_004a5a90(&local_30,4);
    FUN_004a5a90(&local_30,8);
    FUN_004a5a90(&local_30,0x10);
    uVar3 = *(uint *)(param_1 + 0x10);
    iVar2 = FUN_004a6630(param_2,&local_20,0xc,local_30);
    if (iVar2 + 0xcU < uVar3) {
      iVar2 = FUN_004a6630(param_2,&local_20,0xc,local_30);
      uVar3 = iVar2 + 0xc;
    }
    *(uint *)(param_1 + 0x17e) = uVar3;
    local_4 = *(int *)(*(int *)(param_1 + 8) + 0xe38);
    if (7 < local_4) {
      local_4 = 8;
    }
    local_4 = *(int *)(param_1 + 0xc) * local_4;
    local_8 = uVar3 - 1;
    local_14 = local_4 + 8;
    *(int *)(param_1 + 0x14) = local_14;
    local_4 = local_4 + 7;
    local_10 = 1;
    local_c = 1;
    local_18 = uVar3;
    if ((DAT_006e2108 == 0) || (*(int *)(param_1 + 0x182) == -1)) {
      FUN_0058bc10(0);
      FUN_0058bac0(&local_10,(-(uint)(DAT_006e1f88 != 0) & 0xffffff00) + 0x208);
    }
    iVar2 = FUN_0058b660();
    if (iVar2 != 0) {
      local_24 = iVar2;
      FUN_0057dc20(*(undefined4 *)(param_1 + 0x138));
      FUN_0057dc60(local_18,local_14);
      if ((DAT_006e2108 != 0) && (*(int *)(param_1 + 0x182) != -1)) {
        uVar3 = *(int *)(param_1 + 0x182) * 9 + 8;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        iVar6 = 0;
        iVar7 = *(int *)(DAT_006e2110 + uVar3 * 4) + DAT_006e2114;
        local_28 = *(int *)(iVar7 + 4);
        local_2c = *(int *)(iVar7 + 8);
        iVar2 = DAT_006e2114;
        iVar9 = DAT_006e2110;
        uVar3 = local_18;
        if (0 < local_14) {
          do {
            iVar10 = 0;
            if (0 < (int)uVar3) {
              do {
                if (*(int *)(iVar7 + 0x1c) == -1) {
                  (*DAT_006903dc)(iVar7,iVar10,iVar6);
                }
                else {
                  sVar1 = (*DAT_006c4e3c)(iVar7,iVar10,iVar6,0,0);
                  if (sVar1 == 1) {
                    FUN_0057de00(iVar7,iVar10,iVar6,0,0);
                  }
                }
                iVar10 = iVar10 + local_28;
                iVar2 = DAT_006e2114;
                iVar9 = DAT_006e2110;
                uVar3 = local_18;
              } while (iVar10 < (int)local_18);
            }
            iVar6 = iVar6 + local_2c;
          } while (iVar6 < local_14);
        }
        uVar5 = *(int *)(param_1 + 0x182) * 9;
        uVar8 = uVar5 + 4;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar8) {
          uVar8 = 0;
        }
        iVar6 = *(int *)(iVar9 + uVar8 * 4) + iVar2;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar5) {
          uVar5 = 0;
        }
        iVar7 = *(int *)(iVar6 + 4);
        iVar10 = *(int *)(*(int *)(iVar9 + uVar5 * 4) + 4 + iVar2);
        if (iVar10 < (int)uVar3) {
          do {
            if (*(int *)(iVar6 + 0x1c) == -1) {
              (*DAT_006903dc)(iVar6,iVar10);
            }
            else {
              sVar1 = (*DAT_006c4e3c)(iVar6,iVar10,0,0,0);
              if (sVar1 == 1) {
                FUN_0057de00(iVar6,iVar10,0,0,0);
              }
            }
            iVar10 = iVar10 + iVar7;
            iVar2 = DAT_006e2114;
            iVar9 = DAT_006e2110;
            uVar3 = local_18;
          } while (iVar10 < (int)local_18);
        }
        uVar8 = *(int *)(param_1 + 0x182) * 9 + 6;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar8) {
          uVar8 = 0;
        }
        uVar5 = *(int *)(param_1 + 0x182) * 9 + 2;
        iVar6 = *(int *)(iVar9 + uVar8 * 4) + iVar2;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar5) {
          uVar5 = 0;
        }
        iVar7 = *(int *)(*(int *)(iVar9 + uVar5 * 4) + 4 + iVar2);
        local_28 = *(int *)(iVar6 + 4);
        if (iVar7 < (int)uVar3) {
          do {
            iVar2 = local_14 + -1;
            if (*(int *)(iVar6 + 0x1c) == -1) {
              (*DAT_006903dc)(iVar6,iVar7,iVar2);
            }
            else {
              sVar1 = (*DAT_006c4e3c)(iVar6,iVar7,iVar2,0,0);
              if (sVar1 == 1) {
                FUN_0057de00(iVar6,iVar7,iVar2,0,0);
              }
            }
            iVar7 = iVar7 + local_28;
            iVar2 = DAT_006e2114;
            iVar9 = DAT_006e2110;
            uVar3 = local_18;
          } while (iVar7 < (int)local_18);
        }
        uVar5 = *(int *)(param_1 + 0x182) * 9;
        uVar8 = uVar5 + 7;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar8) {
          uVar8 = 0;
        }
        iVar6 = *(int *)(iVar9 + uVar8 * 4) + iVar2;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar5) {
          uVar5 = 0;
        }
        iVar7 = *(int *)(*(int *)(iVar9 + uVar5 * 4) + 8 + iVar2);
        iVar10 = *(int *)(iVar6 + 8);
        local_2c = iVar10;
        if (iVar7 < local_14) {
          do {
            if (*(int *)(iVar6 + 0x1c) == -1) {
              (*DAT_006903dc)(iVar6,0,iVar7);
            }
            else {
              sVar1 = (*DAT_006c4e3c)(iVar6,0,iVar7,0,0);
              if (sVar1 == 1) {
                FUN_0057de00(iVar6,0,iVar7,0,0);
              }
            }
            iVar7 = iVar7 + iVar10;
            iVar2 = DAT_006e2114;
            iVar9 = DAT_006e2110;
            uVar3 = local_18;
          } while (iVar7 < local_14);
        }
        uVar8 = *(int *)(param_1 + 0x182) * 9 + 5;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar8) {
          uVar8 = 0;
        }
        uVar5 = *(int *)(param_1 + 0x182) * 9 + 1;
        iVar6 = *(int *)(iVar9 + uVar8 * 4) + iVar2;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar5) {
          uVar5 = 0;
        }
        iVar7 = *(int *)(*(int *)(iVar9 + uVar5 * 4) + 8 + iVar2);
        local_2c = *(int *)(iVar6 + 8);
        if (iVar7 < local_14) {
          do {
            iVar2 = uVar3 - 1;
            if (*(int *)(iVar6 + 0x1c) == -1) {
              (*DAT_006903dc)(iVar6,iVar2,iVar7);
            }
            else {
              sVar1 = (*DAT_006c4e3c)(iVar6,iVar2,iVar7,0,0);
              if (sVar1 == 1) {
                FUN_0057de00(iVar6,iVar2,iVar7,0,0);
              }
            }
            iVar7 = iVar7 + local_2c;
            iVar2 = DAT_006e2114;
            iVar9 = DAT_006e2110;
            uVar3 = local_18;
          } while (iVar7 < local_14);
        }
        uVar3 = *(int *)(param_1 + 0x182) * 9;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        iVar2 = *(int *)(iVar9 + uVar3 * 4) + iVar2;
        if (*(int *)(iVar2 + 0x1c) == -1) {
          (*DAT_006903dc)(iVar2);
        }
        else {
          sVar1 = (*DAT_006c4e3c)(iVar2,0,0,0,0);
          if (sVar1 == 1) {
            FUN_0057de00(iVar2,0,0,0,0);
          }
        }
        uVar3 = *(int *)(param_1 + 0x182) * 9 + 1;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        iVar9 = *(int *)(DAT_006e2110 + uVar3 * 4) + DAT_006e2114;
        iVar2 = local_18 - 1;
        if (*(int *)(iVar9 + 0x1c) == -1) {
          (*DAT_006903dc)(iVar9,iVar2);
        }
        else {
          sVar1 = (*DAT_006c4e3c)(iVar9,iVar2,0,0,0);
          if (sVar1 == 1) {
            FUN_0057de00(iVar9,iVar2,0,0,0);
          }
        }
        uVar3 = *(int *)(param_1 + 0x182) * 9 + 2;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        iVar2 = local_14 + -1;
        iVar9 = *(int *)(DAT_006e2110 + uVar3 * 4) + DAT_006e2114;
        if (*(int *)(iVar9 + 0x1c) == -1) {
          (*DAT_006903dc)(iVar9,0,iVar2);
        }
        else {
          sVar1 = (*DAT_006c4e3c)(iVar9,0,iVar2,0,0);
          if (sVar1 == 1) {
            FUN_0057de00(iVar9,0,iVar2,0,0);
          }
        }
        uVar3 = *(int *)(param_1 + 0x182) * 9 + 3;
        if (*(uint *)(DAT_006e2108 + 0xc) < uVar3) {
          uVar3 = 0;
        }
        FUN_0040a150(*(int *)(DAT_006e2110 + uVar3 * 4) + DAT_006e2114,local_18 - 1,local_14 + -1,0,
                     0);
        iVar2 = local_24;
      }
      FUN_004a5ce0(param_2,&local_20,0,0,0,local_30,0);
      (**(code **)(**(int **)(param_1 + 0x128) + 0x80))(*(int **)(param_1 + 0x128),iVar2);
      DVar4 = GetTickCount();
      iVar2 = DAT_006de745;
      if (DVar4 <= (uint)(*(int *)(param_1 + 0x156) + DAT_006de741)) {
        *(DWORD *)(param_1 + 0x156) = DVar4;
        *(undefined4 *)(param_1 + 0x17a) = 1;
        return;
      }
      *(undefined4 *)(param_1 + 0x17a) = 1;
      *(DWORD *)(param_1 + 0x156) = iVar2 + DVar4;
    }
  }
  return;
}

