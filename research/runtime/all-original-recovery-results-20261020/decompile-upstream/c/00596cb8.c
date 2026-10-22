
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_00596cb8(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5)

{
  byte bVar1;
  undefined2 uVar2;
  undefined4 uVar3;
  byte *pbVar4;
  int iVar5;
  byte *pbVar6;
  undefined4 *puVar7;
  uint uVar8;
  undefined4 *puVar9;
  
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
      _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
      puVar9 = (undefined4 *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
      DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
      DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
      if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
        puVar7 = (undefined4 *)(param_3 + DAT_005f1e6c[1]);
        pbVar4 = (byte *)(*DAT_005f1e6c + param_3);
        DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
        do {
          DAT_005f1e74 = DAT_005f1e64;
          DAT_005f1e88 = 0;
          do {
            uVar8 = (uint)*pbVar4;
            puVar9 = (undefined4 *)((int)puVar9 + uVar8 * 2);
            DAT_005f1e88 = DAT_005f1e88 + uVar8;
            DAT_005f1e74 = DAT_005f1e74 - uVar8;
            pbVar6 = pbVar4 + 1;
            if (DAT_005f1e74 == 0) break;
            uVar8 = (uint)pbVar4[1];
            pbVar4 = pbVar4 + 2;
            DAT_005f1e74 = DAT_005f1e74 - uVar8;
            DAT_005f1e8c = 0;
            iVar5 = DAT_005f1e80 - DAT_005f1e88;
            if (iVar5 == 0 || DAT_005f1e80 < DAT_005f1e88) {
LAB_00596fda:
              if (DAT_005f1e88 < DAT_005f1e84) {
                if (DAT_005f1e84 < (int)(DAT_005f1e88 + uVar8)) {
                  DAT_005f1e8c = (DAT_005f1e88 + uVar8) - DAT_005f1e84;
                  uVar8 = uVar8 - DAT_005f1e8c;
                }
                DAT_005f1e88 = DAT_005f1e88 + uVar8;
                if ((uVar8 & 1) != 0) {
                  uVar2 = *(undefined2 *)puVar7;
                  puVar7 = (undefined4 *)((int)puVar7 + 2);
                  *(undefined2 *)puVar9 = uVar2;
                  puVar9 = (undefined4 *)((int)puVar9 + 2);
                  uVar8 = uVar8 - 1;
                  if (uVar8 == 0) goto LAB_0059703f;
                }
                do {
                  uVar3 = *puVar7;
                  puVar7 = puVar7 + 1;
                  *puVar9 = uVar3;
                  puVar9 = puVar9 + 1;
                  uVar8 = uVar8 - 2;
                } while (uVar8 != 0);
              }
              else {
                puVar7 = (undefined4 *)((int)puVar7 + uVar8 * 2);
                puVar9 = (undefined4 *)((int)puVar9 + uVar8 * 2);
                DAT_005f1e88 = DAT_005f1e88 + uVar8;
              }
            }
            else {
              if (iVar5 < (int)uVar8) {
                puVar7 = (undefined4 *)((int)puVar7 + iVar5 * 2);
                puVar9 = (undefined4 *)((int)puVar9 + iVar5 * 2);
                uVar8 = uVar8 - iVar5;
                DAT_005f1e88 = DAT_005f1e88 + iVar5;
                goto LAB_00596fda;
              }
              puVar7 = (undefined4 *)((int)puVar7 + uVar8 * 2);
              puVar9 = (undefined4 *)((int)puVar9 + uVar8 * 2);
              DAT_005f1e88 = DAT_005f1e88 + uVar8;
            }
LAB_0059703f:
            if (DAT_005f1e8c != 0) {
              puVar7 = (undefined4 *)((int)puVar7 + DAT_005f1e8c * 2);
              puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e8c * 2);
              DAT_005f1e88 = DAT_005f1e88 + DAT_005f1e8c;
            }
            pbVar6 = pbVar4;
          } while (DAT_005f1e74 != 0);
          puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e58);
          DAT_005f1e70 = DAT_005f1e70 + -1;
          pbVar4 = pbVar6;
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
    _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
    puVar9 = (undefined4 *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
    DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
    DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
    if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
      puVar7 = (undefined4 *)(param_3 + DAT_005f1e6c[1]);
      pbVar4 = (byte *)(*DAT_005f1e6c + param_3);
      DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
      do {
        DAT_005f1e74 = DAT_005f1e64;
        do {
          puVar9 = (undefined4 *)((int)puVar9 + (uint)*pbVar4 * 2);
          DAT_005f1e74 = DAT_005f1e74 - (uint)*pbVar4;
          pbVar6 = pbVar4 + 1;
          if (DAT_005f1e74 == 0) break;
          bVar1 = pbVar4[1];
          uVar8 = (uint)bVar1;
          pbVar4 = pbVar4 + 2;
          DAT_005f1e74 = DAT_005f1e74 - uVar8;
          if ((bVar1 & 1) == 0) goto LAB_00596e22;
          uVar2 = *(undefined2 *)puVar7;
          puVar7 = (undefined4 *)((int)puVar7 + 2);
          *(undefined2 *)puVar9 = uVar2;
          puVar9 = (undefined4 *)((int)puVar9 + 2);
          for (uVar8 = uVar8 - 1; uVar8 != 0; uVar8 = uVar8 - 2) {
LAB_00596e22:
            uVar3 = *puVar7;
            puVar7 = puVar7 + 1;
            *puVar9 = uVar3;
            puVar9 = puVar9 + 1;
          }
          pbVar6 = pbVar4;
        } while (DAT_005f1e74 != 0);
        puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e58);
        DAT_005f1e70 = DAT_005f1e70 + -1;
        pbVar4 = pbVar6;
      } while (DAT_005f1e70 != 0);
    }
  }
  return (ulonglong)param_2 << 0x20;
}

