
void __fastcall FUN_00554e00(int param_1)

{
  int iVar1;
  undefined2 *puVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  char *pcVar6;
  int iVar7;
  char *pcVar8;
  uint uVar9;
  undefined2 *puVar10;
  undefined2 *local_80;
  int local_7c;
  undefined4 local_6c;
  undefined4 local_68;
  int local_64;
  int local_60;
  undefined4 local_4;
  
  *(undefined4 *)(param_1 + 0x6207) = 1;
  FUN_0058b3e0();
  if (*(int *)(param_1 + 0x61df) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x61df));
    *(undefined4 *)(param_1 + 0x61df) = 0;
    *(undefined4 *)(param_1 + 0x61e3) = 0;
    *(undefined4 *)(param_1 + 0x61e7) = 0;
    *(undefined4 *)(param_1 + 0x61eb) = 0;
  }
  piVar5 = (int *)(param_1 + 0x61ef);
  iVar7 = 4;
  do {
    if (*piVar5 != 0) {
      FUN_0057d0d0(*piVar5);
      *piVar5 = 0;
    }
    piVar5 = piVar5 + 1;
    iVar7 = iVar7 + -1;
  } while (iVar7 != 0);
  if (*(int *)(param_1 + 0x61ff) != 0) {
    FUN_005979b0(*(int *)(param_1 + 0x61ff));
    *(undefined4 *)(param_1 + 0x61ff) = 0;
  }
  iVar7 = DAT_006c5498;
  local_6c = 0x6c;
  local_68 = 7;
  if ((DAT_006c5494 == 100) && (DAT_006c5498 == 100)) {
    pcVar6 = s_bitmaps_scan5by5Middle_bmp_005ebfd8;
    pcVar8 = s_bitmaps_scanLargeEdge_spr_005ebfbc;
    local_60 = 200;
    local_64 = 100;
  }
  else {
    if ((DAT_006c5494 != 0x50) || (DAT_006c5498 != 0x50)) {
      iVar1 = DAT_006c5494 * 2;
      *(undefined4 *)(param_1 + 0x6207) = 0;
      local_64 = iVar7;
      local_4 = 0x840;
      local_60 = iVar1;
      FUN_0058ad90(&local_6c,s_c__Projects_Chaos_Scanner_cpp_005ebf64,0x5c3);
      FUN_0058d1a0(s_bitmaps_Scanner_BackGround_bmp_005ebf44);
      puVar2 = (undefined2 *)FUN_0058b660();
      uVar9 = *(uint *)(param_1 + 0x61c1) >> 1;
      local_7c = 0;
      local_80 = puVar2;
      if (0 < iVar7) {
        do {
          iVar3 = 0;
          if (0 < iVar1) {
            puVar10 = local_80;
            do {
              iVar4 = iVar3 % 0x50;
              iVar3 = iVar3 + 1;
              *puVar10 = puVar2[iVar4 + (local_7c % 0x28) * uVar9];
              puVar10 = puVar10 + 1;
            } while (iVar3 < iVar1);
          }
          local_7c = local_7c + 1;
          local_80 = local_80 + uVar9;
        } while (local_7c < iVar7);
      }
      (**(code **)(**(int **)(param_1 + 0x61ad) + 0x80))(*(int **)(param_1 + 0x61ad),puVar2);
      return;
    }
    pcVar6 = s_bitmaps_scan4by4Middle_bmp_005ebfa0;
    pcVar8 = s_bitmaps_scanSmallEdge_spr_005ebf84;
    local_60 = 0xa0;
    local_64 = DAT_006c5494;
  }
  local_4 = 0x840;
  FUN_0058ad90(&local_6c,s_c__Projects_Chaos_Scanner_cpp_005ebf64,0x5d3);
  FUN_0058d1a0(pcVar6);
  FUN_0057d310(pcVar8,2);
  return;
}

