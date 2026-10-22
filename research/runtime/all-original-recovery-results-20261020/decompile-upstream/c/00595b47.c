
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_00595b47(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5,int param_6,
            int param_7)

{
  undefined4 *puVar1;
  bool bVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined1 *puVar5;
  int iVar6;
  undefined3 uVar9;
  uint uVar7;
  uint uVar8;
  undefined1 *puVar10;
  uint uVar11;
  int iVar12;
  uint *puVar13;
  uint *puVar14;
  
  DAT_005f1e78 = param_3;
  if ((*(int *)(param_3 + 8) != 0) && (*(int *)(param_3 + 4) != 0)) {
    param_4 = param_4 - *(int *)(param_3 + 0xc);
    if ((param_4 < DAT_006e0008) ||
       (DAT_005f1e64 = *(uint *)(param_3 + 4), DAT_006a49b8 <= (int)(param_4 + DAT_005f1e64))) {
      DAT_005f1e80 = 0;
      if (param_4 < DAT_006e0008) {
        DAT_005f1e80 = DAT_006e0008 - param_4;
      }
      DAT_005f1e64 = *(uint *)(param_3 + 4);
      DAT_005f1e84 = DAT_005f1e64;
      if (DAT_006a49b8 <= (int)(param_4 + DAT_005f1e64)) {
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
      puVar1 = *(undefined4 **)(param_3 + 0x1c);
      for (; param_7 != 0; param_7 = param_7 + -1) {
        puVar1 = (undefined4 *)*puVar1;
      }
      uVar7 = puVar1[1];
      if ((uVar7 != 0) &&
         (param_6 = param_6 >> ((byte)uVar7 & 0x1f), bVar2 = (uVar7 & 0x1f) == 0,
         bVar2 && (int)uVar7 < 0 || !bVar2 && param_6 < 0)) {
        param_6 = param_6 + 1;
      }
      param_6 = param_6 + puVar1[2];
      _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
      puVar13 = (uint *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
      DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
      DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
      if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
        puVar10 = (undefined1 *)(param_3 + DAT_005f1e6c[1]);
        puVar4 = (undefined1 *)(*DAT_005f1e6c + param_3);
        DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
        do {
          DAT_005f1e74 = DAT_005f1e64;
          DAT_005f1e88 = 0;
          uVar7 = DAT_005f1e64;
          do {
            uVar9 = (undefined3)(uVar7 >> 8);
            iVar6 = CONCAT31(uVar9,*puVar4);
            iVar12 = DAT_005f1e88 + iVar6;
            puVar13 = (uint *)((int)puVar13 + iVar6 * 2);
            puVar5 = puVar4 + 1;
            if (DAT_005f1e74 - iVar6 == 0) break;
            uVar7 = CONCAT31(uVar9,puVar4[1]);
            puVar4 = puVar4 + 2;
            DAT_005f1e74 = (DAT_005f1e74 - iVar6) - uVar7;
            DAT_005f1e8c = 0;
            iVar6 = DAT_005f1e80 - iVar12;
            puVar14 = puVar13;
            if (iVar6 == 0 || DAT_005f1e80 < iVar12) {
LAB_00595f3d:
              if (iVar12 < (int)DAT_005f1e84) {
                iVar6 = (iVar12 + uVar7) - DAT_005f1e84;
                if (iVar6 != 0 && (int)DAT_005f1e84 <= (int)(iVar12 + uVar7)) {
                  uVar7 = uVar7 - iVar6;
                  DAT_005f1e8c = iVar6;
                }
                DAT_005f1e88 = iVar12 + uVar7;
                uVar8 = uVar7;
                uVar11 = uVar7;
                puVar13 = puVar14;
                DAT_005f1e7c = uVar7;
                if (((uint)puVar14 & 2) != 0) {
                  uVar8 = CONCAT31((int3)(uVar7 >> 8),*puVar10);
                  puVar13 = (uint *)((int)puVar14 + 2);
                  puVar10 = puVar10 + 1;
                  uVar11 = uVar7 - 1;
                  DAT_005f1e7c = uVar11;
                  *(short *)puVar14 = (short)puVar1[param_6 * 0x100 + uVar8 + 3];
                  uVar7 = uVar8;
                  if (uVar11 == 0) goto LAB_00595ff0;
                }
                if ((uVar11 & 1) == 0) {
LAB_00595fae:
                  uVar9 = (undefined3)(uVar8 >> 8);
                  iVar6 = puVar1[param_6 * 0x100 + CONCAT31(uVar9,puVar10[uVar11 - 1]) + 3];
                  iVar12 = CONCAT31(uVar9,puVar10[uVar11 - 2]);
                  while( true ) {
                    uVar8 = uVar11 - 2;
                    uVar7 = iVar6 << 0x10;
                    if (uVar8 == 0) break;
                    iVar3 = iVar12 + 3;
                    uVar9 = (undefined3)((uint)iVar12 >> 8);
                    iVar6 = puVar1[param_6 * 0x100 + CONCAT31(uVar9,puVar10[uVar11 - 3]) + 3];
                    iVar12 = CONCAT31(uVar9,puVar10[uVar11 - 4]);
                    *(uint *)((int)puVar13 + uVar8 * 2) = puVar1[param_6 * 0x100 + iVar3] | uVar7;
                    uVar11 = uVar8;
                  }
                  *puVar13 = uVar7 | puVar1[param_6 * 0x100 + iVar12 + 3];
                }
                else {
                  uVar8 = CONCAT31((int3)(uVar8 >> 8),puVar10[uVar11 - 1]);
                  *(short *)((int)puVar13 + uVar11 * 2 + -2) =
                       (short)puVar1[param_6 * 0x100 + uVar8 + 3];
                  uVar11 = uVar11 - 1;
                  if (uVar11 != 0) goto LAB_00595fae;
                }
                puVar13 = (uint *)((int)puVar13 + DAT_005f1e7c * 2);
                puVar10 = puVar10 + DAT_005f1e7c;
                uVar7 = DAT_005f1e7c;
              }
              else {
                puVar10 = puVar10 + uVar7;
                DAT_005f1e88 = iVar12 + uVar7;
                puVar13 = (uint *)((int)puVar14 + uVar7 * 2);
              }
            }
            else {
              if (iVar6 < (int)uVar7) {
                puVar10 = puVar10 + iVar6;
                puVar14 = (uint *)((int)puVar13 + iVar6 * 2);
                uVar7 = uVar7 - iVar6;
                iVar12 = iVar12 + iVar6;
                goto LAB_00595f3d;
              }
              puVar10 = puVar10 + uVar7;
              DAT_005f1e88 = iVar12 + uVar7;
              puVar13 = (uint *)((int)puVar13 + uVar7 * 2);
            }
LAB_00595ff0:
            if (DAT_005f1e8c != 0) {
              puVar10 = puVar10 + DAT_005f1e8c;
              puVar13 = (uint *)((int)puVar13 + DAT_005f1e8c * 2);
              DAT_005f1e88 = DAT_005f1e88 + DAT_005f1e8c;
            }
            puVar5 = puVar4;
          } while (DAT_005f1e74 != 0);
          puVar13 = (uint *)((int)puVar13 + DAT_005f1e58);
          DAT_005f1e70 = DAT_005f1e70 + -1;
          puVar4 = puVar5;
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
    puVar1 = *(undefined4 **)(param_3 + 0x1c);
    for (; param_7 != 0; param_7 = param_7 + -1) {
      puVar1 = (undefined4 *)*puVar1;
    }
    uVar7 = puVar1[1];
    if ((uVar7 != 0) &&
       (param_6 = param_6 >> ((byte)uVar7 & 0x1f), bVar2 = (uVar7 & 0x1f) == 0,
       bVar2 && (int)uVar7 < 0 || !bVar2 && param_6 < 0)) {
      param_6 = param_6 + 1;
    }
    param_6 = param_6 + puVar1[2];
    _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
    puVar13 = (uint *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
    DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
    DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
    if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
      puVar10 = (undefined1 *)(param_3 + DAT_005f1e6c[1]);
      puVar4 = (undefined1 *)(*DAT_005f1e6c + param_3);
      DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
      do {
        DAT_005f1e74 = DAT_005f1e64;
        uVar7 = DAT_005f1e64;
        do {
          uVar9 = (undefined3)(uVar7 >> 8);
          iVar6 = DAT_005f1e74 - CONCAT31(uVar9,*puVar4);
          puVar14 = (uint *)((int)puVar13 + CONCAT31(uVar9,*puVar4) * 2);
          puVar5 = puVar4 + 1;
          puVar13 = puVar14;
          if (iVar6 == 0) break;
          uVar11 = CONCAT31(uVar9,puVar4[1]);
          puVar4 = puVar4 + 2;
          DAT_005f1e74 = iVar6 - uVar11;
          uVar7 = uVar11;
          DAT_005f1e7c = uVar11;
          if (((uint)puVar14 & 2) == 0) {
LAB_00595cf7:
            if ((uVar11 & 1) == 0) {
LAB_00595d0e:
              uVar9 = (undefined3)(uVar7 >> 8);
              iVar6 = puVar1[param_6 * 0x100 + CONCAT31(uVar9,puVar10[uVar11 - 1]) + 3];
              iVar12 = CONCAT31(uVar9,puVar10[uVar11 - 2]);
              while( true ) {
                uVar8 = uVar11 - 2;
                uVar7 = iVar6 << 0x10;
                if (uVar8 == 0) break;
                iVar3 = iVar12 + 3;
                uVar9 = (undefined3)((uint)iVar12 >> 8);
                iVar6 = puVar1[param_6 * 0x100 + CONCAT31(uVar9,puVar10[uVar11 - 3]) + 3];
                iVar12 = CONCAT31(uVar9,puVar10[uVar11 - 4]);
                *(uint *)((int)puVar13 + uVar8 * 2) = puVar1[param_6 * 0x100 + iVar3] | uVar7;
                uVar11 = uVar8;
              }
              *puVar13 = uVar7 | puVar1[param_6 * 0x100 + iVar12 + 3];
            }
            else {
              uVar7 = CONCAT31((int3)(uVar7 >> 8),puVar10[uVar11 - 1]);
              *(short *)((int)puVar13 + uVar11 * 2 + -2) =
                   (short)puVar1[param_6 * 0x100 + uVar7 + 3];
              uVar11 = uVar11 - 1;
              if (uVar11 != 0) goto LAB_00595d0e;
            }
            puVar13 = (uint *)((int)puVar13 + DAT_005f1e7c * 2);
            puVar10 = puVar10 + DAT_005f1e7c;
            uVar7 = DAT_005f1e7c;
          }
          else {
            uVar7 = CONCAT31(uVar9,*puVar10);
            puVar13 = (uint *)((int)puVar14 + 2);
            puVar10 = puVar10 + 1;
            uVar11 = uVar11 - 1;
            DAT_005f1e7c = uVar11;
            *(short *)puVar14 = (short)puVar1[param_6 * 0x100 + uVar7 + 3];
            if (uVar11 != 0) goto LAB_00595cf7;
          }
          puVar5 = puVar4;
        } while (DAT_005f1e74 != 0);
        puVar13 = (uint *)((int)puVar13 + DAT_005f1e58);
        DAT_005f1e70 = DAT_005f1e70 + -1;
        puVar4 = puVar5;
      } while (DAT_005f1e70 != 0);
      DAT_005f1e70 = 0;
    }
  }
  return (ulonglong)param_2 << 0x20;
}

