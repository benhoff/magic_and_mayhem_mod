
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_0059521a(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5,int param_6,
            int param_7)

{
  byte bVar1;
  undefined4 *puVar2;
  bool bVar3;
  int iVar4;
  byte *pbVar5;
  int iVar6;
  byte *pbVar7;
  byte *pbVar8;
  uint uVar9;
  uint *puVar10;
  
  DAT_005f1e78 = param_3;
  if ((*(int *)(param_3 + 8) != 0) && (*(int *)(param_3 + 4) != 0)) {
    param_4 = param_4 - *(int *)(param_3 + 0xc);
    if ((param_4 < DAT_006e0008) ||
       (DAT_005f1e64 = *(int *)(param_3 + 4), DAT_006a49b8 <= param_4 + DAT_005f1e64)) {
      DAT_005f1e80 = 0;
      if (param_4 < DAT_006e0008) {
        DAT_005f1e80 = DAT_006e0008 - param_4;
      }
      DAT_005f1e64 = *(int *)(param_3 + 4);
      DAT_005f1e84 = DAT_005f1e64;
      if (DAT_006a49b8 <= param_4 + DAT_005f1e64) {
        DAT_005f1e84 = DAT_005f1e64 - ((param_4 + DAT_005f1e64) - DAT_006a49b8);
      }
      DAT_005f1e54 = *(int *)(param_3 + 8);
      DAT_005f1e50 = 0;
      param_5 = param_5 - *(int *)(param_3 + 0x10);
      _DAT_005f1e68 = param_5;
      if (param_5 < DAT_006cbb6c) {
        DAT_005f1e50 = -(param_5 - DAT_006cbb6c);
        _DAT_005f1e68 = param_5 - (param_5 - DAT_006cbb6c);
      }
      if (DAT_00656618 <= param_5 + DAT_005f1e54) {
        DAT_005f1e54 = DAT_00656618 - param_5;
      }
      puVar2 = *(undefined4 **)(param_3 + 0x1c);
      for (; param_7 != 0; param_7 = param_7 + -1) {
        puVar2 = (undefined4 *)*puVar2;
      }
      uVar9 = puVar2[1];
      if ((uVar9 != 0) &&
         (param_6 = param_6 >> ((byte)uVar9 & 0x1f), bVar3 = (uVar9 & 0x1f) == 0,
         bVar3 && (int)uVar9 < 0 || !bVar3 && param_6 < 0)) {
        param_6 = param_6 + 1;
      }
      iVar4 = (param_6 + puVar2[2]) * 0x200 + 0xc;
      _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
      puVar10 = (uint *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
      DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
      DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
      if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
        pbVar8 = (byte *)(param_3 + DAT_005f1e6c[1]);
        pbVar5 = (byte *)(*DAT_005f1e6c + param_3);
        DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
        do {
          DAT_005f1e74 = DAT_005f1e64;
          DAT_005f1e88 = 0;
          do {
            uVar9 = (uint)*pbVar5;
            puVar10 = (uint *)((int)puVar10 + uVar9 * 2);
            DAT_005f1e88 = DAT_005f1e88 + uVar9;
            DAT_005f1e74 = DAT_005f1e74 - uVar9;
            pbVar7 = pbVar5 + 1;
            if (DAT_005f1e74 == 0) break;
            uVar9 = (uint)pbVar5[1];
            pbVar5 = pbVar5 + 2;
            DAT_005f1e74 = DAT_005f1e74 - uVar9;
            DAT_005f1e8c = 0;
            iVar6 = DAT_005f1e80 - DAT_005f1e88;
            if (iVar6 == 0 || DAT_005f1e80 < DAT_005f1e88) {
LAB_005955bc:
              if (DAT_005f1e88 < DAT_005f1e84) {
                if (DAT_005f1e84 < (int)(DAT_005f1e88 + uVar9)) {
                  DAT_005f1e8c = (DAT_005f1e88 + uVar9) - DAT_005f1e84;
                  uVar9 = uVar9 - DAT_005f1e8c;
                }
                DAT_005f1e88 = DAT_005f1e88 + uVar9;
                if ((uVar9 & 1) != 0) {
                  bVar1 = *pbVar8;
                  pbVar8 = pbVar8 + 1;
                  *(short *)puVar10 = (short)*(undefined4 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
                  puVar10 = (uint *)((int)puVar10 + 2);
                  uVar9 = uVar9 - 1;
                  if (uVar9 == 0) goto LAB_00595632;
                }
                do {
                  pbVar7 = pbVar8 + 1;
                  bVar1 = *pbVar8;
                  pbVar8 = pbVar8 + 2;
                  *puVar10 = *(int *)((int)puVar2 + (uint)*pbVar7 * 2 + iVar4) << 0x10 |
                             (uint)*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
                  puVar10 = puVar10 + 1;
                  uVar9 = uVar9 - 2;
                } while (uVar9 != 0);
              }
              else {
                pbVar8 = pbVar8 + uVar9;
                puVar10 = (uint *)((int)puVar10 + uVar9 * 2);
                DAT_005f1e88 = DAT_005f1e88 + uVar9;
              }
            }
            else {
              if (iVar6 < (int)uVar9) {
                pbVar8 = pbVar8 + iVar6;
                puVar10 = (uint *)((int)puVar10 + iVar6 * 2);
                uVar9 = uVar9 - iVar6;
                DAT_005f1e88 = DAT_005f1e88 + iVar6;
                goto LAB_005955bc;
              }
              pbVar8 = pbVar8 + uVar9;
              puVar10 = (uint *)((int)puVar10 + uVar9 * 2);
              DAT_005f1e88 = DAT_005f1e88 + uVar9;
            }
LAB_00595632:
            if (DAT_005f1e8c != 0) {
              pbVar8 = pbVar8 + DAT_005f1e8c;
              puVar10 = (uint *)((int)puVar10 + DAT_005f1e8c * 2);
              DAT_005f1e88 = DAT_005f1e88 + DAT_005f1e8c;
            }
            pbVar7 = pbVar5;
          } while (DAT_005f1e74 != 0);
          puVar10 = (uint *)((int)puVar10 + DAT_005f1e58);
          DAT_005f1e70 = DAT_005f1e70 + -1;
          pbVar5 = pbVar7;
        } while (DAT_005f1e70 != 0);
      }
      return (ulonglong)param_2 << 0x20;
    }
    DAT_005f1e54 = *(int *)(param_3 + 8);
    DAT_005f1e50 = 0;
    param_5 = param_5 - *(int *)(param_3 + 0x10);
    _DAT_005f1e68 = param_5;
    if (param_5 < DAT_006cbb6c) {
      DAT_005f1e50 = -(param_5 - DAT_006cbb6c);
      _DAT_005f1e68 = param_5 - (param_5 - DAT_006cbb6c);
    }
    if (DAT_00656618 <= param_5 + DAT_005f1e54) {
      DAT_005f1e54 = DAT_00656618 - param_5;
    }
    puVar2 = *(undefined4 **)(param_3 + 0x1c);
    for (; param_7 != 0; param_7 = param_7 + -1) {
      puVar2 = (undefined4 *)*puVar2;
    }
    uVar9 = puVar2[1];
    if ((uVar9 != 0) &&
       (param_6 = param_6 >> ((byte)uVar9 & 0x1f), bVar3 = (uVar9 & 0x1f) == 0,
       bVar3 && (int)uVar9 < 0 || !bVar3 && param_6 < 0)) {
      param_6 = param_6 + 1;
    }
    iVar4 = (param_6 + puVar2[2]) * 0x200 + 0xc;
    _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
    puVar10 = (uint *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
    DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
    DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
    if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
      pbVar8 = (byte *)(param_3 + DAT_005f1e6c[1]);
      pbVar5 = (byte *)(*DAT_005f1e6c + param_3);
      DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
      do {
        DAT_005f1e74 = DAT_005f1e64;
        do {
          puVar10 = (uint *)((int)puVar10 + (uint)*pbVar5 * 2);
          DAT_005f1e74 = DAT_005f1e74 - (uint)*pbVar5;
          pbVar7 = pbVar5 + 1;
          if (DAT_005f1e74 == 0) break;
          bVar1 = pbVar5[1];
          uVar9 = (uint)bVar1;
          pbVar5 = pbVar5 + 2;
          DAT_005f1e74 = DAT_005f1e74 - uVar9;
          if ((bVar1 & 1) == 0) goto LAB_005953bb;
          bVar1 = *pbVar8;
          pbVar8 = pbVar8 + 1;
          *(short *)puVar10 = (short)*(undefined4 *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
          puVar10 = (uint *)((int)puVar10 + 2);
          for (uVar9 = uVar9 - 1; uVar9 != 0; uVar9 = uVar9 - 2) {
LAB_005953bb:
            pbVar7 = pbVar8 + 1;
            bVar1 = *pbVar8;
            pbVar8 = pbVar8 + 2;
            *puVar10 = *(int *)((int)puVar2 + (uint)*pbVar7 * 2 + iVar4) << 0x10 |
                       (uint)*(ushort *)((int)puVar2 + (uint)bVar1 * 2 + iVar4);
            puVar10 = puVar10 + 1;
          }
          pbVar7 = pbVar5;
        } while (DAT_005f1e74 != 0);
        puVar10 = (uint *)((int)puVar10 + DAT_005f1e58);
        DAT_005f1e70 = DAT_005f1e70 + -1;
        pbVar5 = pbVar7;
      } while (DAT_005f1e70 != 0);
    }
  }
  return (ulonglong)param_2 << 0x20;
}

