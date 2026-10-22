
void __fastcall FUN_0054f0c0(int param_1)

{
  char *pcVar1;
  int *piVar2;
  int *piVar3;
  undefined4 uVar4;
  byte *pbVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  byte *pbVar11;
  int iVar12;
  int local_198;
  int local_178 [26];
  undefined4 local_110;
  undefined1 local_10c [256];
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_005c2a29;
  local_c = ExceptionList;
  piVar2 = local_178;
  for (iVar8 = 0x1b; iVar8 != 0; iVar8 = iVar8 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  local_178[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
  local_4 = 0;
  local_178[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
  local_178[0] = 0x6c;
  local_178[1] = 7;
  local_110 = 0x840;
  ExceptionList = &local_c;
  FUN_0058ad90(local_178,s_c__Projects_Chaos_RealmView_cpp_005ebe4c,0x286);
  FUN_0058d0e0(0,0,0xff);
  iVar8 = *(int *)(param_1 + 0x823);
  *(undefined4 *)(param_1 + 0x54) = 0;
  piVar2 = (int *)FUN_00597850(iVar8 * 0x14 + 4);
  local_4._0_1_ = 1;
  if (piVar2 == (int *)0x0) {
    piVar3 = (int *)0x0;
  }
  else {
    piVar3 = piVar2 + 1;
    *piVar2 = iVar8;
    FUN_0059c441(piVar3,0x14,iVar8,&LAB_005524f0,FUN_004c1130);
  }
  local_4 = (uint)local_4._1_3_ << 8;
  *(int **)(param_1 + 0x1a) = piVar3;
  uVar4 = FUN_00597850(*(int *)(param_1 + 0x823) << 4);
  *(undefined4 *)(param_1 + 0x1e) = uVar4;
  iVar9 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
  pbVar5 = (byte *)FUN_00597880(iVar9 * ((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0));
  iVar8 = 0;
  if (0 < *(int *)(param_1 + 0x823)) {
    iVar12 = 0;
    local_198 = 0;
    do {
      iVar8 = iVar8 + 1;
      FUN_0059c1fd(local_10c,s__s_ix_i__s_Border__02i_PCX_005ebe30,"Interface\\RealmViewer\\",
                   (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                   (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,*(undefined4 *)(param_1 + 0x12),iVar8
                  );
      iVar6 = FUN_0054ad50(local_10c);
      if (iVar6 != 0) {
        *(uint *)(iVar12 + *(int *)(param_1 + 0x1e)) = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
        *(undefined4 *)(iVar12 + 8 + *(int *)(param_1 + 0x1e)) = 0;
        *(uint *)(iVar12 + 4 + *(int *)(param_1 + 0x1e)) =
             (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
        *(undefined4 *)(iVar12 + 0xc + *(int *)(param_1 + 0x1e)) = 0;
        FUN_0054b510(pbVar5,iVar9);
        iVar6 = *(int *)(local_198 + 8 + *(int *)(param_1 + 0x1a));
        iVar10 = 0;
        pbVar11 = pbVar5;
        if ((-(uint)(DAT_006de6d5 != 0) & 0x78) != 0xfffffe20) {
          do {
            iVar7 = 0;
            if ((-(uint)(DAT_006de6d5 != 0) & 0xa0) != 0xfffffd80) {
              do {
                pcVar1 = (char *)(iVar6 + (uint)*pbVar11 * 3);
                if (((*pcVar1 != '\0') || (pcVar1[1] != '\0')) || (pcVar1[2] != -1)) {
                  if (*(int *)(iVar12 + 8 + *(int *)(param_1 + 0x1e)) < iVar7) {
                    *(int *)(iVar12 + 8 + *(int *)(param_1 + 0x1e)) = iVar7;
                  }
                  piVar2 = (int *)(*(int *)(param_1 + 0x1e) + iVar12);
                  if (iVar7 < *piVar2) {
                    *piVar2 = iVar7;
                  }
                  if (*(int *)(iVar12 + 0xc + *(int *)(param_1 + 0x1e)) < iVar10) {
                    *(int *)(iVar12 + 0xc + *(int *)(param_1 + 0x1e)) = iVar10;
                  }
                  if (iVar10 < *(int *)(iVar12 + 4 + *(int *)(param_1 + 0x1e))) {
                    *(int *)(iVar12 + 4 + *(int *)(param_1 + 0x1e)) = iVar10;
                  }
                }
                pbVar11 = pbVar11 + 1;
                iVar7 = iVar7 + 1;
              } while (iVar7 < (int)((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280));
            }
            iVar10 = iVar10 + 1;
          } while (iVar10 < (int)((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0));
        }
      }
      local_198 = local_198 + 0x14;
      iVar12 = iVar12 + 0x10;
    } while (iVar8 < *(int *)(param_1 + 0x823));
  }
  if (pbVar5 != (byte *)0x0) {
    FUN_005979b0(pbVar5);
  }
  ExceptionList = local_c;
  return;
}

