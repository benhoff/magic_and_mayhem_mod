
void __thiscall FUN_00520100(uint param_1,int param_2,int param_3)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  int *piVar6;
  uint uVar7;
  int local_198;
  int local_184;
  undefined4 local_160 [4];
  undefined4 local_150 [81];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c24a3;
  local_c = ExceptionList;
  if (*(int *)(param_1 + 0xa8) != 0x1b) {
    iVar1 = *(int *)(param_1 + 0x14);
    iVar2 = *(int *)(*(int *)(param_1 + 0xac) + 0x198);
    ExceptionList = &local_c;
    if (*(int *)(param_1 + 0xa8) != 6) {
      ExceptionList = &local_c;
      FUN_00571310();
    }
    if (param_2 == 0) {
      FUN_00549e80();
    }
    else {
      uVar7 = -(uint)(DAT_006def5c != 0) & DAT_006def58;
      FUN_0040e650();
      FUN_004132e0();
      puVar5 = local_150;
      for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
        *puVar5 = 0;
        puVar5 = puVar5 + 1;
      }
      local_150[5] = 0xffffffff;
      local_150[4] = 0xffffffff;
      local_150[3] = 0xffffffff;
      local_150[2] = 0xffffffff;
      local_150[1] = 0xffffffff;
      local_150[0] = 0xffffffff;
      local_150[9] = 0xffffffff;
      local_150[8] = 0xffffffff;
      local_150[7] = 0xffffffff;
      local_150[6] = 0xffffffff;
      local_150[0xc] = 7;
      local_150[10] = 1;
      local_150[0xb] = 0xffffffff;
      FUN_004e1360();
      *(undefined4 *)(param_1 + 0xe4) = 0;
      local_4 = 1;
      if (0 < DAT_006df180) {
        local_184 = DAT_006df180;
        do {
          if (((*(int *)(uVar7 + 4) != 0) && (*(int *)(uVar7 + 0xe4) != 0)) && (uVar7 != param_1)) {
            FUN_0040e290();
            FUN_0040e4e0();
            FUN_0040e8a0();
            FUN_0040eb70();
            iVar4 = FUN_004eaca0();
            if ((int)(iVar4 + (iVar4 >> 0x1f & 3U)) >> 2 < param_2 / param_3) {
              FUN_0044cd00();
              FUN_00411530();
              FUN_0044ccf0();
              FUN_00411500();
              FUN_0044ccf0();
              FUN_004114c0();
              FUN_00411530();
              FUN_00411500();
              FUN_004114c0(iVar1 + iVar2);
              iVar4 = FUN_004e1db0();
              if (iVar4 != 0) {
                FUN_0052cda0();
                puVar5 = local_160;
                piVar6 = (int *)(param_1 + 0x678);
                local_198 = 2;
                do {
                  iVar4 = *piVar6;
                  if (iVar4 == 0) {
LAB_00520431:
                    *puVar5 = 0xffffffff;
                  }
                  else {
                    for (iVar3 = *(int *)(iVar4 + 0x39); iVar3 != 0; iVar3 = *(int *)(iVar3 + 0x39))
                    {
                      iVar4 = iVar3;
                    }
                    if (*(int *)(iVar4 + 5) == 0) goto LAB_00520431;
                    *puVar5 = *(undefined4 *)(*(int *)(iVar4 + 5) + 0x1f);
                  }
                  piVar6 = piVar6 + 1;
                  puVar5 = puVar5 + 1;
                  local_198 = local_198 + -1;
                } while (local_198 != 0);
                FUN_00514860();
              }
            }
          }
          uVar7 = uVar7 + 0xe4b;
          local_184 = local_184 + -1;
        } while (local_184 != 0);
      }
      FUN_00549e80();
      FUN_00597870();
      local_4 = 0xffffffff;
    }
    FUN_0051f5e0();
    *(undefined **)(param_1 + 0x5f4) = &DAT_0050aca0;
    *(undefined4 *)(param_1 + 0x5f8) = 0;
    *(undefined4 *)(param_1 + 0x5f0) = 0;
    *(undefined4 *)(param_1 + 0x5ec) = 0;
    FUN_00506d30();
  }
  ExceptionList = local_c;
  return;
}

