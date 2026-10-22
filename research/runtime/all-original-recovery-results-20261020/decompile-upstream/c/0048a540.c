
/* WARNING: Removing unreachable block (ram,0x0048a68e) */

void __fastcall FUN_0048a540(int param_1)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  uint uVar5;
  ushort *puVar6;
  int *piVar7;
  uint uVar8;
  uint local_108;
  uint local_104;
  int local_fc [63];
  
  local_108 = *(uint *)(param_1 + 8);
  uVar5 = *(uint *)(param_1 + 0xc);
  uVar3 = *(int *)(param_1 + 0x10) + 1;
  if (((((-1 < (int)local_108) && ((int)local_108 < (int)DAT_006c5494)) && (-1 < (int)uVar5)) &&
      (((int)uVar5 < (int)DAT_006c5498 && (-1 < (int)uVar3)))) && ((int)uVar3 < (int)DAT_006c549c))
  {
    iVar1 = (&DAT_006cb942)[uVar5] + (&DAT_006cb8c6)[*(int *)(param_1 + 0x10)] + local_108;
    puVar6 = (ushort *)(DAT_006c54dc + iVar1 * 0xc);
    if ((*(short *)(DAT_006c54dc + iVar1 * 0xc) == 0) || ((puVar6[5] & 0x10) != 0)) {
      uVar3 = FUN_0054e200();
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x827);
      }
      uVar2 = *(int *)(&DAT_005dda08 + (uVar3 & 3) * 8) + *(int *)(param_1 + 8);
      uVar5 = *(int *)(&DAT_005dda0c + (uVar3 & 3) * 8) + *(int *)(param_1 + 0xc);
      uVar3 = *(int *)(param_1 + 0x10) + 1;
      uVar8 = 0;
      local_104 = *(int *)(param_1 + 0x10) + 2;
      piVar4 = &DAT_006cb8c2 + uVar3;
      local_108 = uVar2;
      do {
        if ((uVar3 < DAT_006c549c) && (local_104 < DAT_006c549c)) {
          if ((int)uVar2 < 0) {
            uVar2 = uVar2 + DAT_006c5494;
            local_108 = uVar2;
          }
          if (DAT_006c5494 <= uVar2) {
            uVar2 = uVar2 - DAT_006c5494;
            local_108 = uVar2;
          }
          if ((int)uVar5 < 0) {
            uVar5 = uVar5 + DAT_006c5498;
          }
          if (DAT_006c5498 <= uVar5) {
            uVar5 = uVar5 - DAT_006c5498;
          }
          puVar6 = (ushort *)(DAT_006c54dc + ((&DAT_006cb942)[uVar5] + *piVar4 + uVar2) * 0xc);
          if (((puVar6[5] & 0x10) == 0) && (uVar2 = local_108, *puVar6 != 0)) break;
        }
        uVar8 = uVar8 + 1;
        uVar3 = uVar3 - 1;
        local_104 = local_104 - 1;
        piVar4 = piVar4 + -1;
        if (2 < uVar8) {
          return;
        }
      } while( true );
    }
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x850);
    }
    if (((uVar2 & 0x7fffffff) % 99 + 1 < *(uint *)(DAT_0065660c + 0xa0 + (uint)*puVar6 * 0x164)) &&
       (iVar1 = FUN_0049cdb0(), iVar1 != 0)) {
      piVar4 = (int *)(param_1 + 0x2c);
      piVar7 = local_fc;
      for (iVar1 = 0x3f; iVar1 != 0; iVar1 = iVar1 + -1) {
        *piVar7 = *piVar4;
        piVar4 = piVar4 + 1;
        piVar7 = piVar7 + 1;
      }
      local_fc[0] = local_108 << 5;
      local_fc[1] = uVar5 << 5;
      local_fc[2] = uVar3 << 4;
      FUN_00493f20(0xe,local_fc);
    }
  }
  return;
}

