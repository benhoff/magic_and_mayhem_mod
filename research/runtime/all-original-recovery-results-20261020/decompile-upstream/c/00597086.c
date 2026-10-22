
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_00597086(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5)

{
  undefined2 uVar1;
  byte *pbVar2;
  int iVar3;
  byte *pbVar4;
  int iVar5;
  uint uVar6;
  undefined4 *puVar7;
  uint uVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  
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
        pbVar2 = (byte *)(*DAT_005f1e6c + param_3);
        DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
        do {
          DAT_005f1e74 = DAT_005f1e64;
          DAT_005f1e88 = 0;
          do {
            uVar8 = (uint)*pbVar2;
            iVar5 = DAT_005f1e88 + uVar8;
            puVar9 = (undefined4 *)((int)puVar9 + uVar8 * 2);
            pbVar4 = pbVar2 + 1;
            if (DAT_005f1e74 - uVar8 == 0) break;
            uVar6 = (uint)pbVar2[1];
            pbVar2 = pbVar2 + 2;
            DAT_005f1e74 = (DAT_005f1e74 - uVar8) - uVar6;
            DAT_005f1e8c = 0;
            iVar3 = DAT_005f1e80 - iVar5;
            puVar10 = puVar9;
            if (iVar3 == 0 || DAT_005f1e80 < iVar5) {
LAB_005973f0:
              if (iVar5 < DAT_005f1e84) {
                iVar3 = (iVar5 + uVar6) - DAT_005f1e84;
                if (iVar3 != 0 && DAT_005f1e84 <= (int)(iVar5 + uVar6)) {
                  uVar6 = uVar6 - iVar3;
                  DAT_005f1e8c = iVar3;
                }
                DAT_005f1e88 = iVar5 + uVar6;
                puVar9 = puVar10;
                DAT_005f1e7c = uVar6;
                if (((uint)puVar10 & 2) != 0) {
                  uVar1 = *(undefined2 *)puVar7;
                  puVar9 = (undefined4 *)((int)puVar10 + 2);
                  puVar7 = (undefined4 *)((int)puVar7 + 2);
                  uVar6 = uVar6 - 1;
                  DAT_005f1e7c = uVar6;
                  *(undefined2 *)puVar10 = uVar1;
                  if (uVar6 == 0) goto LAB_00597480;
                }
                if ((uVar6 & 1) == 0) {
LAB_00597462:
                  while (uVar6 = uVar6 - 2, uVar6 != 0) {
                    *(undefined4 *)((int)puVar9 + uVar6 * 2) =
                         *(undefined4 *)((int)puVar7 + uVar6 * 2);
                  }
                  *puVar9 = *puVar7;
                }
                else {
                  *(undefined2 *)((int)puVar9 + uVar6 * 2 + -2) =
                       *(undefined2 *)((int)puVar7 + uVar6 * 2 + -2);
                  uVar6 = uVar6 - 1;
                  if (uVar6 != 0) goto LAB_00597462;
                }
                puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e7c * 2);
                puVar7 = (undefined4 *)((int)puVar7 + DAT_005f1e7c * 2);
              }
              else {
                puVar7 = (undefined4 *)((int)puVar7 + uVar6 * 2);
                DAT_005f1e88 = iVar5 + uVar6;
                puVar9 = (undefined4 *)((int)puVar10 + uVar6 * 2);
              }
            }
            else {
              if (iVar3 < (int)uVar6) {
                puVar7 = (undefined4 *)((int)puVar7 + iVar3 * 2);
                puVar10 = (undefined4 *)((int)puVar9 + iVar3 * 2);
                uVar6 = uVar6 - iVar3;
                iVar5 = iVar5 + iVar3;
                goto LAB_005973f0;
              }
              puVar7 = (undefined4 *)((int)puVar7 + uVar6 * 2);
              DAT_005f1e88 = iVar5 + uVar6;
              puVar9 = (undefined4 *)((int)puVar9 + uVar6 * 2);
            }
LAB_00597480:
            if (DAT_005f1e8c != 0) {
              puVar7 = (undefined4 *)((int)puVar7 + DAT_005f1e8c * 2);
              puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e8c * 2);
              DAT_005f1e88 = DAT_005f1e88 + DAT_005f1e8c;
            }
            pbVar4 = pbVar2;
          } while (DAT_005f1e74 != 0);
          puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e58);
          DAT_005f1e70 = DAT_005f1e70 + -1;
          pbVar2 = pbVar4;
        } while (DAT_005f1e70 != 0);
        DAT_005f1e70 = 0;
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
      pbVar2 = (byte *)(*DAT_005f1e6c + param_3);
      DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
      do {
        DAT_005f1e74 = DAT_005f1e64;
        do {
          iVar5 = DAT_005f1e74 - (uint)*pbVar2;
          puVar10 = (undefined4 *)((int)puVar9 + (uint)*pbVar2 * 2);
          pbVar4 = pbVar2 + 1;
          puVar9 = puVar10;
          if (iVar5 == 0) break;
          uVar8 = (uint)pbVar2[1];
          pbVar2 = pbVar2 + 2;
          DAT_005f1e74 = iVar5 - uVar8;
          DAT_005f1e7c = uVar8;
          if (((uint)puVar10 & 2) == 0) {
LAB_005971fe:
            if ((uVar8 & 1) == 0) {
LAB_00597215:
              while (uVar8 = uVar8 - 2, uVar8 != 0) {
                *(undefined4 *)((int)puVar9 + uVar8 * 2) = *(undefined4 *)((int)puVar7 + uVar8 * 2);
              }
              *puVar9 = *puVar7;
            }
            else {
              *(undefined2 *)((int)puVar9 + uVar8 * 2 + -2) =
                   *(undefined2 *)((int)puVar7 + uVar8 * 2 + -2);
              uVar8 = uVar8 - 1;
              if (uVar8 != 0) goto LAB_00597215;
            }
            puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e7c * 2);
            puVar7 = (undefined4 *)((int)puVar7 + DAT_005f1e7c * 2);
          }
          else {
            uVar1 = *(undefined2 *)puVar7;
            puVar9 = (undefined4 *)((int)puVar10 + 2);
            puVar7 = (undefined4 *)((int)puVar7 + 2);
            uVar8 = uVar8 - 1;
            DAT_005f1e7c = uVar8;
            *(undefined2 *)puVar10 = uVar1;
            if (uVar8 != 0) goto LAB_005971fe;
          }
          pbVar4 = pbVar2;
        } while (DAT_005f1e74 != 0);
        puVar9 = (undefined4 *)((int)puVar9 + DAT_005f1e58);
        DAT_005f1e70 = DAT_005f1e70 + -1;
        pbVar2 = pbVar4;
      } while (DAT_005f1e70 != 0);
      DAT_005f1e70 = 0;
    }
  }
  return (ulonglong)param_2 << 0x20;
}

