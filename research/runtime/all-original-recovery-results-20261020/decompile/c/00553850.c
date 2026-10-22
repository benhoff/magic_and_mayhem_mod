
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00553850(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  uint uVar4;
  uint uVar5;
  short *psVar6;
  uint uVar7;
  int iVar8;
  short sVar9;
  uint uVar10;
  uint local_8;
  
  iVar2 = DAT_006e1f88;
  *(uint *)(param_1 + 0x6199) = DAT_006c5494;
  *(uint *)(param_1 + 0x619d) = DAT_006c5498;
  sVar9 = (-(ushort)(iVar2 != 0) & 0x8200) - 0x400;
  iVar2 = 0;
  for (local_8 = 0;
      (*(int *)(param_1 + 0x614d) != 0 &&
      (local_8 < (uint)((*(int *)(param_1 + 0x6151) - *(int *)(param_1 + 0x614d)) / 0xc)));
      local_8 = local_8 + 1) {
    piVar3 = (int *)(iVar2 + *(int *)(param_1 + 0x614d));
    if ((_DAT_005e18f0 !=
         *(char *)((&DAT_006cb8c2)[(uint)piVar3[2] >> 1] + (&DAT_006cb942)[piVar3[1]] + *piVar3 +
                  DAT_006c5c5c)) || (DAT_005e1404 == 0)) {
      uVar4 = ((DAT_006c5494 >> 1) - *(int *)(param_1 + 0x61a1)) + *piVar3;
      uVar7 = ((DAT_006c5498 >> 1) - *(int *)(param_1 + 0x61a5)) + piVar3[1];
      if ((int)uVar4 < 0) {
        uVar4 = uVar4 + DAT_006c5494;
      }
      else if (DAT_006c5494 <= uVar4) {
        uVar4 = uVar4 - DAT_006c5494;
      }
      if ((int)uVar7 < 0) {
        uVar7 = uVar7 + DAT_006c5498;
      }
      else if (DAT_006c5498 <= uVar7) {
        uVar7 = uVar7 - DAT_006c5498;
      }
      iVar1 = *(int *)(param_1 + 0xc5);
      if (iVar1 == 1) {
        uVar10 = *(int *)(param_1 + 0x103) - uVar4;
        uVar5 = uVar7;
      }
      else if (iVar1 == 2) {
        uVar10 = *(int *)(param_1 + 0x107) - uVar7;
        uVar5 = *(int *)(param_1 + 0x103) - uVar4;
      }
      else {
        uVar5 = uVar4;
        uVar10 = uVar7;
        if (iVar1 == 3) {
          uVar5 = *(int *)(param_1 + 0x107) - uVar7;
          uVar10 = uVar4;
        }
      }
      iVar8 = (uVar5 + *(int *)(param_1 + 0xeb) * -2) - uVar10;
      iVar1 = *(int *)(*(int *)(param_1 + 0xf3) + ((int)(uVar5 + uVar10) / 2) * 4);
      *(short *)(iVar1 + -2 + iVar8 * 2) = sVar9;
      iVar1 = iVar1 + -2 + iVar8 * 2;
      *(short *)(iVar1 + 2) = sVar9;
      *(short *)(iVar1 + 4) = sVar9;
      iVar8 = *(int *)(param_1 + 0xeb);
      *(short *)(iVar1 + 6 + iVar8 * 2) = sVar9;
      psVar6 = (short *)(iVar1 + iVar8 * 2 + -2);
      *psVar6 = sVar9;
      iVar1 = *(int *)(param_1 + 0xeb);
      psVar6[iVar1] = sVar9;
      psVar6[iVar1 + 2] = sVar9;
      psVar6 = psVar6 + iVar1 + 4;
      *psVar6 = sVar9;
      iVar1 = *(int *)(param_1 + 0xeb);
      psVar6[iVar1] = sVar9;
      psVar6[iVar1 + -4] = sVar9;
      iVar8 = *(int *)(param_1 + 0xeb);
      psVar6[iVar1 + iVar8 + -3] = sVar9;
      psVar6[iVar1 + iVar8 + -2] = sVar9;
      psVar6[iVar1 + iVar8 + -1] = sVar9;
    }
    iVar2 = iVar2 + 0xc;
  }
  return;
}

