
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_005947b2(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5,int param_6,
            int param_7)

{
  int iVar1;
  undefined4 *puVar2;
  bool bVar3;
  int iVar4;
  undefined1 *puVar5;
  undefined1 *puVar6;
  int iVar7;
  undefined3 uVar10;
  uint uVar8;
  uint uVar9;
  undefined1 *puVar11;
  uint uVar12;
  int iVar13;
  uint *puVar14;
  uint *puVar15;
  
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
      puVar2 = *(undefined4 **)(param_3 + 0x1c);
      for (; param_7 != 0; param_7 = param_7 + -1) {
        puVar2 = (undefined4 *)*puVar2;
      }
      uVar8 = puVar2[1];
      if ((uVar8 != 0) &&
         (param_6 = param_6 >> ((byte)uVar8 & 0x1f), bVar3 = (uVar8 & 0x1f) == 0,
         bVar3 && (int)uVar8 < 0 || !bVar3 && param_6 < 0)) {
        param_6 = param_6 + 1;
      }
      iVar4 = (param_6 + puVar2[2]) * 0x200 + 0xc;
      _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
      puVar14 = (uint *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
      DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
      DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
      if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
        puVar11 = (undefined1 *)(param_3 + DAT_005f1e6c[1]);
        puVar5 = (undefined1 *)(*DAT_005f1e6c + param_3);
        DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
        do {
          DAT_005f1e74 = DAT_005f1e64;
          DAT_005f1e88 = 0;
          uVar8 = DAT_005f1e64;
          do {
            uVar10 = (undefined3)(uVar8 >> 8);
            iVar7 = CONCAT31(uVar10,*puVar5);
            iVar13 = DAT_005f1e88 + iVar7;
            puVar14 = (uint *)((int)puVar14 + iVar7 * 2);
            puVar6 = puVar5 + 1;
            if (DAT_005f1e74 - iVar7 == 0) break;
            uVar8 = CONCAT31(uVar10,puVar5[1]);
            puVar5 = puVar5 + 2;
            DAT_005f1e74 = (DAT_005f1e74 - iVar7) - uVar8;
            DAT_005f1e8c = 0;
            iVar7 = DAT_005f1e80 - iVar13;
            puVar15 = puVar14;
            if (iVar7 == 0 || DAT_005f1e80 < iVar13) {
LAB_00594bb2:
              if (iVar13 < (int)DAT_005f1e84) {
                iVar7 = (iVar13 + uVar8) - DAT_005f1e84;
                if (iVar7 != 0 && (int)DAT_005f1e84 <= (int)(iVar13 + uVar8)) {
                  uVar8 = uVar8 - iVar7;
                  DAT_005f1e8c = iVar7;
                }
                DAT_005f1e88 = iVar13 + uVar8;
                uVar9 = uVar8;
                uVar12 = uVar8;
                puVar14 = puVar15;
                DAT_005f1e7c = uVar8;
                if (((uint)puVar15 & 2) != 0) {
                  uVar9 = CONCAT31((int3)(uVar8 >> 8),*puVar11);
                  puVar14 = (uint *)((int)puVar15 + 2);
                  puVar11 = puVar11 + 1;
                  uVar12 = uVar8 - 1;
                  DAT_005f1e7c = uVar12;
                  *(short *)puVar15 = (short)*(undefined4 *)((int)puVar2 + uVar9 * 2 + iVar4);
                  uVar8 = uVar9;
                  if (uVar12 == 0) goto LAB_00594c71;
                }
                if ((uVar12 & 1) == 0) {
LAB_00594c23:
                  uVar10 = (undefined3)(uVar9 >> 8);
                  iVar7 = *(int *)((int)puVar2 + CONCAT31(uVar10,puVar11[uVar12 - 1]) * 2 + iVar4);
                  iVar13 = CONCAT31(uVar10,puVar11[uVar12 - 2]);
                  while( true ) {
                    uVar9 = uVar12 - 2;
                    uVar8 = iVar7 << 0x10;
                    if (uVar9 == 0) break;
                    iVar1 = iVar13 * 2;
                    uVar10 = (undefined3)((uint)iVar13 >> 8);
                    iVar7 = *(int *)((int)puVar2 + CONCAT31(uVar10,puVar11[uVar12 - 3]) * 2 + iVar4)
                    ;
                    iVar13 = CONCAT31(uVar10,puVar11[uVar12 - 4]);
                    *(uint *)((int)puVar14 + uVar9 * 2) =
                         *(ushort *)((int)puVar2 + iVar1 + iVar4) | uVar8;
                    uVar12 = uVar9;
                  }
                  *puVar14 = uVar8 | *(ushort *)((int)puVar2 + iVar13 * 2 + iVar4);
                }
                else {
                  uVar9 = CONCAT31((int3)(uVar9 >> 8),puVar11[uVar12 - 1]);
                  *(short *)((int)puVar14 + uVar12 * 2 + -2) =
                       (short)*(undefined4 *)((int)puVar2 + uVar9 * 2 + iVar4);
                  uVar12 = uVar12 - 1;
                  if (uVar12 != 0) goto LAB_00594c23;
                }
                puVar14 = (uint *)((int)puVar14 + DAT_005f1e7c * 2);
                puVar11 = puVar11 + DAT_005f1e7c;
                uVar8 = DAT_005f1e7c;
              }
              else {
                puVar11 = puVar11 + uVar8;
                DAT_005f1e88 = iVar13 + uVar8;
                puVar14 = (uint *)((int)puVar15 + uVar8 * 2);
              }
            }
            else {
              if (iVar7 < (int)uVar8) {
                puVar11 = puVar11 + iVar7;
                puVar15 = (uint *)((int)puVar14 + iVar7 * 2);
                uVar8 = uVar8 - iVar7;
                iVar13 = iVar13 + iVar7;
                goto LAB_00594bb2;
              }
              puVar11 = puVar11 + uVar8;
              DAT_005f1e88 = iVar13 + uVar8;
              puVar14 = (uint *)((int)puVar14 + uVar8 * 2);
            }
LAB_00594c71:
            if (DAT_005f1e8c != 0) {
              puVar11 = puVar11 + DAT_005f1e8c;
              puVar14 = (uint *)((int)puVar14 + DAT_005f1e8c * 2);
              DAT_005f1e88 = DAT_005f1e88 + DAT_005f1e8c;
            }
            puVar6 = puVar5;
          } while (DAT_005f1e74 != 0);
          puVar14 = (uint *)((int)puVar14 + DAT_005f1e58);
          DAT_005f1e70 = DAT_005f1e70 + -1;
          puVar5 = puVar6;
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
    puVar2 = *(undefined4 **)(param_3 + 0x1c);
    for (; param_7 != 0; param_7 = param_7 + -1) {
      puVar2 = (undefined4 *)*puVar2;
    }
    uVar8 = puVar2[1];
    if ((uVar8 != 0) &&
       (param_6 = param_6 >> ((byte)uVar8 & 0x1f), bVar3 = (uVar8 & 0x1f) == 0,
       bVar3 && (int)uVar8 < 0 || !bVar3 && param_6 < 0)) {
      param_6 = param_6 + 1;
    }
    iVar4 = (param_6 + puVar2[2]) * 0x200 + 0xc;
    _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
    puVar14 = (uint *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
    DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
    DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
    if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
      puVar11 = (undefined1 *)(param_3 + DAT_005f1e6c[1]);
      puVar5 = (undefined1 *)(*DAT_005f1e6c + param_3);
      DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
      do {
        DAT_005f1e74 = DAT_005f1e64;
        uVar8 = DAT_005f1e64;
        do {
          uVar10 = (undefined3)(uVar8 >> 8);
          iVar7 = DAT_005f1e74 - CONCAT31(uVar10,*puVar5);
          puVar15 = (uint *)((int)puVar14 + CONCAT31(uVar10,*puVar5) * 2);
          puVar6 = puVar5 + 1;
          puVar14 = puVar15;
          if (iVar7 == 0) break;
          uVar12 = CONCAT31(uVar10,puVar5[1]);
          puVar5 = puVar5 + 2;
          DAT_005f1e74 = iVar7 - uVar12;
          uVar8 = uVar12;
          DAT_005f1e7c = uVar12;
          if (((uint)puVar15 & 2) == 0) {
LAB_00594961:
            if ((uVar12 & 1) == 0) {
LAB_00594978:
              uVar10 = (undefined3)(uVar8 >> 8);
              iVar7 = *(int *)((int)puVar2 + CONCAT31(uVar10,puVar11[uVar12 - 1]) * 2 + iVar4);
              iVar13 = CONCAT31(uVar10,puVar11[uVar12 - 2]);
              while( true ) {
                uVar9 = uVar12 - 2;
                uVar8 = iVar7 << 0x10;
                if (uVar9 == 0) break;
                iVar1 = iVar13 * 2;
                uVar10 = (undefined3)((uint)iVar13 >> 8);
                iVar7 = *(int *)((int)puVar2 + CONCAT31(uVar10,puVar11[uVar12 - 3]) * 2 + iVar4);
                iVar13 = CONCAT31(uVar10,puVar11[uVar12 - 4]);
                *(uint *)((int)puVar14 + uVar9 * 2) =
                     *(ushort *)((int)puVar2 + iVar1 + iVar4) | uVar8;
                uVar12 = uVar9;
              }
              *puVar14 = uVar8 | *(ushort *)((int)puVar2 + iVar13 * 2 + iVar4);
            }
            else {
              uVar8 = CONCAT31((int3)(uVar8 >> 8),puVar11[uVar12 - 1]);
              *(short *)((int)puVar14 + uVar12 * 2 + -2) =
                   (short)*(undefined4 *)((int)puVar2 + uVar8 * 2 + iVar4);
              uVar12 = uVar12 - 1;
              if (uVar12 != 0) goto LAB_00594978;
            }
            puVar14 = (uint *)((int)puVar14 + DAT_005f1e7c * 2);
            puVar11 = puVar11 + DAT_005f1e7c;
            uVar8 = DAT_005f1e7c;
          }
          else {
            uVar8 = CONCAT31(uVar10,*puVar11);
            puVar14 = (uint *)((int)puVar15 + 2);
            puVar11 = puVar11 + 1;
            uVar12 = uVar12 - 1;
            DAT_005f1e7c = uVar12;
            *(short *)puVar15 = (short)*(undefined4 *)((int)puVar2 + uVar8 * 2 + iVar4);
            if (uVar12 != 0) goto LAB_00594961;
          }
          puVar6 = puVar5;
        } while (DAT_005f1e74 != 0);
        puVar14 = (uint *)((int)puVar14 + DAT_005f1e58);
        DAT_005f1e70 = DAT_005f1e70 + -1;
        puVar5 = puVar6;
      } while (DAT_005f1e70 != 0);
      DAT_005f1e70 = 0;
    }
  }
  return (ulonglong)param_2 << 0x20;
}

