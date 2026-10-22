
void __fastcall FUN_0048aa50(int param_1)

{
  ushort uVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int *piVar7;
  ushort *puVar8;
  ushort *puVar9;
  int *piVar10;
  int local_170;
  int *local_16c;
  uint *local_168;
  uint local_160;
  uint local_15c [24];
  int local_fc [9];
  int local_d8;
  undefined4 local_cc;
  undefined4 local_c;
  
  iVar6 = DAT_006c54a0;
  local_170 = 0;
  if (*(uint *)(&DAT_006b1274 + *(int *)(param_1 + 0x4c) * 0x2d1) <= *(uint *)(param_1 + 0x1d2)) {
    local_168 = local_15c + 1;
    local_16c = &DAT_005dda2c;
    do {
      uVar2 = FUN_0040e650(local_16c[-1] + *(int *)(param_1 + 8));
      uVar3 = FUN_004132e0(*local_16c + *(int *)(param_1 + 0xc));
      uVar4 = *(int *)(param_1 + 0x10) + 1;
      local_160 = 0;
      puVar8 = (ushort *)
               (DAT_006c54dc + ((&DAT_006cb942)[uVar3] + (&DAT_006cb8c2)[uVar4] + uVar2) * 0xc);
      do {
        puVar9 = puVar8;
        if (((-1 < (int)uVar4) && (uVar4 < DAT_006c549c)) && (0 < (int)uVar4)) {
          puVar9 = puVar8 + iVar6 * -6;
          iVar5 = DAT_0065660c + (uint)*puVar9 * 0x164;
          uVar1 = *puVar8;
          if ((((uVar1 == 0) && (*puVar9 != 0)) &&
              ((*(int *)(iVar5 + 0x94) == 0x10 && ((*(byte *)(iVar5 + 0xb0) & 8) != 0)))) ||
             (((iVar5 = DAT_0065660c + (uint)uVar1 * 0x164, uVar1 != 0 &&
               (*(int *)(iVar5 + 0x94) != 0x10)) && ((*(byte *)(iVar5 + 0xb0) & 8) != 0)))) {
            local_168[-1] = uVar2;
            *local_168 = uVar3;
            local_168[1] = uVar4;
            local_168 = local_168 + 3;
            local_170 = local_170 + 1;
            break;
          }
        }
        local_160 = local_160 + 1;
        uVar4 = uVar4 - 1;
        puVar8 = puVar9;
      } while (local_160 < 3);
      local_16c = local_16c + 2;
    } while ((int)local_16c < 0x5dda6c);
    if (local_170 != 0) {
      uVar2 = FUN_0054e200();
      iVar6 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)local_170);
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x902);
      }
      iVar5 = FUN_0049cdb0();
      if (iVar5 != 0) {
        uVar2 = local_15c[iVar6 * 3];
        if ((int)uVar2 < 0) {
          do {
            uVar2 = uVar2 + DAT_006c5494;
          } while ((int)uVar2 < 0);
        }
        else {
          for (; DAT_006c5494 <= uVar2; uVar2 = uVar2 - DAT_006c5494) {
          }
        }
        uVar3 = local_15c[iVar6 * 3 + 1];
        if ((int)uVar3 < 0) {
          do {
            uVar3 = uVar3 + DAT_006c5498;
          } while ((int)uVar3 < 0);
        }
        else {
          for (; DAT_006c5498 <= uVar3; uVar3 = uVar3 - DAT_006c5498) {
          }
        }
        piVar7 = (int *)(param_1 + 0x2c);
        piVar10 = local_fc;
        for (iVar5 = 0x3f; iVar5 != 0; iVar5 = iVar5 + -1) {
          *piVar10 = *piVar7;
          piVar7 = piVar7 + 1;
          piVar10 = piVar10 + 1;
        }
        local_fc[2] = local_15c[iVar6 * 3 + 2] << 4;
        local_fc[1] = uVar3 << 5;
        local_cc = 1;
        local_c = 0;
        local_fc[0] = uVar2 << 5;
        local_d8 = *(int *)(param_1 + 0x50) -
                   *(int *)(&DAT_006b1274 + *(int *)(param_1 + 0x4c) * 0x2d1);
        FUN_00493f20(0x19,local_fc);
      }
    }
  }
  return;
}

