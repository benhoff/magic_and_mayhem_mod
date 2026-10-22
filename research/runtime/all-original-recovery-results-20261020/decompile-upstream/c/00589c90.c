
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint FUN_00589c90(int param_1,int *param_2,uint param_3)

{
  ushort uVar1;
  short sVar2;
  byte bVar3;
  uint uVar4;
  int iVar5;
  short *psVar6;
  uint uVar7;
  byte bVar8;
  int iVar9;
  byte *pbVar10;
  ushort *puVar11;
  uint uVar12;
  int iVar13;
  short *psVar14;
  uint local_1c;
  int local_18;
  uint local_c;
  
  bVar8 = 0xb - (DAT_006e1f88 != 0);
  bVar3 = 6 - (DAT_006e1f88 != 0);
  uVar4 = param_3 - *(int *)(param_1 + 0x10);
  uVar7 = *(uint *)(param_1 + 4);
  iVar9 = (int)param_2 - *(int *)(param_1 + 0xc);
  param_3 = uVar4;
  if (DAT_006e0008 < (int)(uVar7 + iVar9)) {
    iVar5 = *(uint *)(param_1 + 8) + uVar4;
    if (((DAT_006cbb6c < iVar5) && (iVar9 < DAT_006a49b8)) && ((int)uVar4 < DAT_00656618)) {
      uVar12 = 0;
      if ((int)uVar4 < DAT_006cbb6c) {
        uVar12 = DAT_006cbb6c - uVar4;
      }
      param_3 = *(uint *)(param_1 + 8);
      if (DAT_00656618 < iVar5) {
        param_3 = DAT_00656618 - uVar4;
      }
      iVar5 = DAT_006a2dc8 - uVar7;
      param_2 = (int *)(param_1 + 0x28 + uVar12 * 8);
      local_c = 0;
      psVar14 = (short *)(DAT_00658174 + ((uVar4 + uVar12) * DAT_006a2dc8 + iVar9) * 2);
      if (iVar9 < DAT_006e0008) {
        local_c = DAT_006e0008 - iVar9;
      }
      if (DAT_006a49b8 < (int)(uVar7 + iVar9)) {
        local_1c = DAT_006a49b8 - iVar9;
      }
      else {
        local_1c = uVar7;
        if (iVar9 >= DAT_006e0008) {
          if (uVar12 < param_3) {
            local_18 = param_3 - uVar12;
            do {
              pbVar10 = (byte *)(*param_2 + param_1);
              puVar11 = (ushort *)(param_2[1] + param_1);
              uVar4 = 0;
              if (uVar7 != 0) {
                do {
                  uVar4 = uVar4 + *pbVar10;
                  psVar14 = psVar14 + *pbVar10;
                  if (uVar7 <= uVar4) break;
                  uVar7 = (uint)pbVar10[1];
                  pbVar10 = pbVar10 + 2;
                  uVar4 = uVar4 + uVar7;
                  for (; uVar7 != 0; uVar7 = uVar7 - 1) {
                    uVar1 = *puVar11;
                    puVar11 = puVar11 + 1;
                    uVar12 = ((uint)(uVar1 & _DAT_006e1f80) +
                              (uint)(ushort)((uVar1 & DAT_006e1f7e) >> (bVar3 & 0x1f)) +
                             (uint)(ushort)((uVar1 & DAT_006e1f7c) >> (bVar8 & 0x1f))) / 3;
                    *psVar14 = (short)(uVar12 << (bVar3 & 0x1f)) +
                               (short)uVar12 + (short)(uVar12 << (bVar8 & 0x1f));
                    psVar14 = psVar14 + 1;
                  }
                  uVar7 = *(uint *)(param_1 + 4);
                } while (uVar4 < uVar7);
              }
              param_2 = param_2 + 2;
              psVar14 = psVar14 + iVar5;
              local_18 = local_18 + -1;
              if (local_18 == 0) {
                return 0;
              }
            } while( true );
          }
          goto LAB_0058a003;
        }
      }
      if (uVar12 < param_3) {
        iVar9 = param_3 - uVar12;
        do {
          param_3 = iVar9;
          uVar4 = 0;
          pbVar10 = (byte *)(*param_2 + param_1);
          psVar6 = (short *)(param_2[1] + param_1);
          if (uVar7 != 0) {
            do {
              uVar4 = uVar4 + *pbVar10;
              psVar14 = psVar14 + *pbVar10;
              if (uVar7 <= uVar4) break;
              uVar7 = (uint)pbVar10[1];
              pbVar10 = pbVar10 + 2;
              if (uVar4 < local_c) {
                uVar12 = local_c - uVar4;
                if (uVar7 < uVar12) {
                  psVar14 = psVar14 + uVar7;
                  psVar6 = psVar6 + uVar7;
                }
                else {
                  psVar14 = psVar14 + uVar12;
                  psVar6 = psVar6 + uVar12;
                  iVar9 = uVar7 - uVar12;
                  if (local_1c < uVar4 + uVar7) {
                    iVar13 = (iVar9 - (uVar4 + uVar7)) + local_1c;
                    uVar12 = iVar9 - iVar13;
                    for (; iVar13 != 0; iVar13 = iVar13 + -1) {
                      sVar2 = *psVar6;
                      psVar6 = psVar6 + 1;
                      *psVar14 = sVar2;
                      psVar14 = psVar14 + 1;
                    }
                    goto LAB_00589fcd;
                  }
                  for (; iVar9 != 0; iVar9 = iVar9 + -1) {
                    sVar2 = *psVar6;
                    psVar6 = psVar6 + 1;
                    *psVar14 = sVar2;
                    psVar14 = psVar14 + 1;
                  }
                }
              }
              else {
                uVar12 = uVar7;
                if (uVar4 < local_1c) {
                  if (uVar4 + uVar7 < local_1c) {
                    for (; uVar12 != 0; uVar12 = uVar12 - 1) {
                      sVar2 = *psVar6;
                      psVar6 = psVar6 + 1;
                      *psVar14 = sVar2;
                      psVar14 = psVar14 + 1;
                    }
                    goto LAB_00589fd3;
                  }
                  iVar9 = (uVar7 - (uVar4 + uVar7)) + local_1c;
                  uVar12 = uVar7 - iVar9;
                  for (; iVar9 != 0; iVar9 = iVar9 + -1) {
                    sVar2 = *psVar6;
                    psVar6 = psVar6 + 1;
                    *psVar14 = sVar2;
                    psVar14 = psVar14 + 1;
                  }
                }
LAB_00589fcd:
                psVar14 = psVar14 + uVar12;
                psVar6 = psVar6 + uVar12;
              }
LAB_00589fd3:
              uVar4 = uVar4 + uVar7;
              uVar7 = *(uint *)(param_1 + 4);
            } while (uVar4 < uVar7);
          }
          param_2 = param_2 + 2;
          iVar9 = param_3 + -1;
          psVar14 = psVar14 + iVar5;
          param_3 = 0;
        } while (iVar9 != 0);
      }
    }
  }
LAB_0058a003:
  return param_3 & 0xffff0000;
}

