
uint __fastcall FUN_0057e540(int param_1,int param_2,int *param_3)

{
  int *piVar1;
  ushort *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  int iVar8;
  byte *pbVar9;
  byte *pbVar10;
  bool bVar11;
  uint local_18;
  undefined2 local_14;
  
  bVar11 = DAT_006e1f88 != 0;
  uVar5 = *(uint *)(param_1 + 8);
  local_14 = 0;
  if ((uVar5 != 0) || (*(int *)(param_1 + 4) != 0)) {
    uVar7 = *(uint *)(param_1 + 4);
    param_2 = param_2 - *(int *)(param_1 + 0xc);
    iVar3 = uVar7 + (uVar5 >> 1) + param_2;
    iVar6 = (int)param_3 + ((uVar5 >> 1) - *(int *)(param_1 + 0x10));
    if ((DAT_006e0008 < iVar3) &&
       (((DAT_006cbb6c < (int)(uVar5 + iVar6) && (param_2 < DAT_006a49b8)) && (iVar6 < DAT_00656618)
        ))) {
      uVar4 = 0;
      if (iVar6 < DAT_006cbb6c) {
        uVar4 = DAT_006cbb6c - iVar6;
      }
      local_18 = uVar5;
      if (DAT_00656618 < (int)(uVar5 + iVar6)) {
        local_18 = DAT_00656618 - iVar6;
      }
      if (uVar5 <= uVar4) {
        local_14 = 4;
      }
      if (uVar5 < local_18) {
        local_14 = 5;
      }
      iVar8 = DAT_006a2dc8 - uVar7;
      uVar5 = DAT_00658174 + ((uVar4 + iVar6) * DAT_006a2dc8 + param_2) * 2;
      if ((iVar3 <= DAT_006a49b8) && (DAT_006e0008 <= param_2)) {
        puVar2 = (ushort *)(uVar5 + (local_18 & 0xfffffffe));
        piVar1 = (int *)(param_1 + 0x28 + uVar4 * 8);
        while (uVar4 < local_18) {
          uVar5 = 0;
          pbVar9 = (byte *)(*piVar1 + param_1);
          if (uVar7 != 0) {
            do {
              uVar5 = uVar5 + *pbVar9;
              puVar2 = puVar2 + *pbVar9;
              if (uVar7 <= uVar5) break;
              uVar7 = (uint)pbVar9[1];
              pbVar9 = pbVar9 + 2;
              uVar5 = uVar5 + uVar7;
              for (; uVar7 != 0; uVar7 = uVar7 - 1) {
                *puVar2 = *puVar2 >> 1 & (-(ushort)bVar11 & 0x200) - 0x411;
                puVar2 = puVar2 + 1;
              }
              uVar7 = *(uint *)(param_1 + 4);
            } while (uVar5 < uVar7);
          }
          uVar7 = *(uint *)(param_1 + 4);
          if (uVar5 != uVar7) {
            local_14 = 3;
          }
          param_3 = piVar1 + 2;
          puVar2 = puVar2 + iVar8 + -1;
          if (uVar4 + 1 < local_18) {
            uVar5 = 0;
            pbVar9 = (byte *)(*param_3 + param_1);
            if (uVar7 != 0) {
              do {
                pbVar10 = pbVar9 + 1;
                uVar5 = uVar5 + *pbVar9;
                if (uVar7 <= uVar5) break;
                pbVar9 = pbVar9 + 2;
                uVar5 = uVar5 + *pbVar10;
              } while (uVar5 < uVar7);
            }
            if (uVar5 != uVar7) {
              local_14 = 3;
            }
            param_3 = piVar1 + 4;
          }
          uVar5 = uVar4 + 2;
          piVar1 = param_3;
          uVar4 = uVar5;
        }
      }
      return CONCAT22((short)(uVar5 >> 0x10),local_14);
    }
  }
  return uVar5 & 0xffff0000;
}

