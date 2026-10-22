
uint __thiscall FUN_00547d80(undefined4 *param_1,ushort *param_2,ushort *param_3)

{
  ushort uVar1;
  byte bVar2;
  ushort uVar3;
  ushort uVar4;
  uint uVar5;
  ushort *puVar6;
  ushort *puVar7;
  ushort *puVar9;
  uint uVar10;
  undefined2 uVar12;
  uint uVar11;
  uint uVar13;
  ushort *puVar14;
  ushort *puVar15;
  int iVar16;
  int iVar17;
  uint uVar18;
  int iVar19;
  int iVar20;
  undefined2 uVar21;
  ushort uVar22;
  int iVar23;
  uint uVar24;
  int local_10;
  int local_c;
  int local_8;
  ushort *local_4;
  ushort *puVar8;
  
  iVar20 = 0xb - (uint)(DAT_006e1f88 != 0);
  bVar2 = (byte)iVar20;
  uVar13 = 0x1f << (bVar2 & 0x1f);
  uVar3 = (-(ushort)(DAT_006e1f88 != 0) & 0xfc00) + 0x7e0;
  uVar10 = (0 < (int)param_2 - DAT_006e0008) - 1 & (int)param_2 - DAT_006e0008;
  iVar23 = -uVar10;
  uVar5 = (int)param_3 - DAT_006cbb6c & (0 < (int)param_3 - DAT_006cbb6c) - 1;
  iVar17 = param_1[1];
  puVar6 = (ushort *)-uVar5;
  iVar16 = iVar17;
  if (DAT_006a49b8 - (int)param_2 <= iVar17) {
    iVar16 = DAT_006a49b8 - (int)param_2;
  }
  local_c = DAT_00656618 - (int)param_3;
  if ((int)param_1[2] < local_c) {
    local_c = param_1[2];
  }
  puVar15 = (ushort *)
            (DAT_00658174 +
            (((int)puVar6 + (int)param_3) * DAT_006a2dc8 + iVar23 + (int)param_2) * 2);
  uVar12 = (undefined2)(uVar13 >> 0x10);
  param_3 = puVar6;
  if (*(int *)((int)param_1 + 0x12) == 0) {
    puVar8 = (ushort *)*param_1;
    switch(puVar8) {
    case (ushort *)0x0:
      uVar4 = (ushort)((uint)puVar8 >> 0x10);
      uVar3 = *(short *)(param_1 + 3) << (bVar2 & 0x1f) | *(short *)((int)param_1 + 0xe) << 5 |
              *(ushort *)(param_1 + 4);
      puVar8 = (ushort *)CONCAT22(uVar4,uVar3);
      if ((int)puVar6 < local_c) {
        param_3 = (ushort *)(local_c + uVar5);
        do {
          if (iVar23 < iVar16) {
            uVar5 = iVar16 + uVar10;
            puVar6 = puVar15;
            for (uVar13 = uVar5 >> 1; uVar13 != 0; uVar13 = uVar13 - 1) {
              *(uint *)puVar6 = CONCAT22(uVar3,uVar3);
              puVar6 = puVar6 + 2;
            }
            for (uVar13 = (uint)((uVar5 & 1) != 0); uVar13 != 0; uVar13 = uVar13 - 1) {
              *puVar6 = uVar3;
              puVar6 = puVar6 + 1;
            }
            puVar15 = puVar15 + uVar5;
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          param_3 = (ushort *)((int)param_3 + -1);
        } while (param_3 != (ushort *)0x0);
        return (uint)uVar4 << 0x10;
      }
      break;
    case (ushort *)0x1:
      puVar8 = puVar6;
      if ((int)puVar6 < local_c) {
        local_4 = (ushort *)(local_c + uVar5);
        do {
          if (iVar23 < iVar16) {
            uVar4 = uVar3 >> 2;
            puVar9 = (ushort *)CONCAT22((short)((uint)puVar6 >> 0x10),uVar4);
            param_3 = (ushort *)CONCAT22(param_3._2_2_,uVar4);
            puVar6 = puVar15;
            puVar8 = (ushort *)(iVar16 + uVar10);
            param_2 = (ushort *)(iVar16 + uVar10);
            do {
              uVar5 = CONCAT22((short)((uint)puVar9 >> 0x10),*puVar6);
              uVar11 = uVar5 & uVar13;
              puVar15 = puVar6 + 1;
              *puVar6 = (ushort)(CONCAT22((short)(uVar11 >> 0x10),
                                          (ushort)((ushort)uVar11 >> (bVar2 & 0x1f)) >> 2) +
                                 CONCAT22(uVar12,*(ushort *)(param_1 + 3) >> 2) +
                                 CONCAT22((short)((uint)puVar8 >> 0x10),
                                          *(ushort *)(param_1 + 3) >> 1) << (bVar2 & 0x1f)) |
                        ((ushort)(uVar5 >> 2) & uVar4 & 0x3fe0) +
                        ((*(ushort *)((int)param_1 + 0xe) & 0x1ffc) +
                        (*(ushort *)((int)param_1 + 0xe) & 0xffe) * 2) * 8 |
                        (*puVar6 >> 2 & 7) + (*(ushort *)(param_1 + 4) >> 2) +
                        (*(ushort *)(param_1 + 4) >> 1);
              puVar9 = (ushort *)((int)param_2 + -1);
              puVar6 = puVar15;
              puVar8 = param_3;
              param_2 = puVar9;
            } while (puVar9 != (ushort *)0x0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          puVar6 = (ushort *)((int)local_4 + -1);
          local_4 = puVar6;
        } while (puVar6 != (ushort *)0x0);
        return 0;
      }
      break;
    case (ushort *)0x2:
      puVar8 = puVar6;
      if ((int)puVar6 < local_c) {
        param_3 = (ushort *)(local_c + uVar5);
        do {
          if (iVar23 < iVar16) {
            iVar17 = CONCAT22((short)((uint)puVar6 >> 0x10),uVar3 >> 1);
            puVar6 = puVar15;
            param_2 = (ushort *)(iVar16 + uVar10);
            do {
              uVar5 = CONCAT22((short)((uint)iVar17 >> 0x10),*puVar6);
              puVar15 = puVar6 + 1;
              uVar11 = uVar5 & uVar13;
              *puVar6 = (ushort)(CONCAT22((short)(uVar11 >> 0x10),
                                          (ushort)((ushort)uVar11 >> (bVar2 & 0x1f)) >> 1) +
                                 CONCAT22(uVar12,*(ushort *)(param_1 + 3) >> 1) << (bVar2 & 0x1f)) |
                        ((ushort)(uVar5 >> 1) & uVar3 >> 1 & 0x7fe0) +
                        (*(ushort *)((int)param_1 + 0xe) & 0xffe) * 0x10 |
                        (*puVar6 >> 1 & 0xf) + (*(ushort *)(param_1 + 4) >> 1);
              iVar17 = (int)param_2 + -1;
              puVar6 = puVar15;
              param_2 = (ushort *)iVar17;
            } while (iVar17 != 0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          puVar6 = (ushort *)((int)param_3 + -1);
          param_3 = puVar6;
        } while (puVar6 != (ushort *)0x0);
        return 0;
      }
      break;
    case (ushort *)0x3:
      puVar8 = puVar6;
      if ((int)puVar6 < local_c) {
        local_4 = (ushort *)(local_c + uVar5);
        do {
          if (iVar23 < iVar16) {
            iVar17 = iVar16 + uVar10;
            param_2 = (ushort *)CONCAT22(param_2._2_2_,uVar3 >> 6);
            puVar6 = puVar15;
            param_3 = (ushort *)iVar17;
            do {
              uVar4 = *puVar6;
              uVar5 = CONCAT22((short)((uint)iVar17 >> 0x10),uVar4) & uVar13;
              uVar22 = (ushort)uVar5 >> (bVar2 & 0x1f);
              puVar15 = puVar6 + 1;
              uVar11 = CONCAT22(uVar12,uVar4 >> 6) & (uint)param_2;
              iVar19 = ((*(ushort *)((int)param_1 + 0xe) & 0x1ffc) +
                       ((uVar11 & 0xffe) + uVar11 * 2) * 2) * 8;
              uVar11 = (byte)((byte)uVar4 >> 1) & 0xf;
              iVar17 = uVar11 + (uVar11 >> 1);
              *puVar6 = (ushort)iVar19 |
                        (ushort)(CONCAT22((short)((uint)iVar20 >> 0x10),uVar22 >> 2) +
                                 CONCAT22((short)(uVar5 >> 0x10),uVar22 >> 1) +
                                 CONCAT22((short)((uint)iVar19 >> 0x10),
                                          *(ushort *)(param_1 + 3) >> 2) << (bVar2 & 0x1f)) |
                        (*(ushort *)(param_1 + 4) >> 2) + (short)iVar17;
              param_3 = (ushort *)((int)param_3 + -1);
              puVar6 = puVar15;
            } while (param_3 != (ushort *)0x0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          local_4 = (ushort *)((int)local_4 + -1);
          puVar8 = (ushort *)0x0;
        } while (local_4 != (ushort *)0x0);
      }
    }
  }
  else {
    puVar8 = (ushort *)(*(int *)((int)param_1 + 0x12) + (iVar17 * (int)puVar6 + iVar23) * 6);
    switch(*param_1) {
    case 0:
      if ((int)puVar6 < local_c) {
        local_10 = local_c + uVar5;
        do {
          if (iVar23 < iVar16) {
            param_2 = (ushort *)(iVar16 + uVar10);
            puVar9 = puVar15;
            do {
              uVar3 = *puVar8;
              local_c = CONCAT22(local_c._2_2_,puVar8[1]);
              puVar14 = puVar8 + 2;
              puVar15 = puVar9 + 1;
              puVar8 = puVar8 + 3;
              param_3 = (ushort *)CONCAT22(param_3._2_2_,*puVar14);
              puVar6 = (ushort *)
                       (CONCAT22((short)((uint)puVar6 >> 0x10),uVar3) << (bVar2 & 0x1f) |
                        local_c << 5 | (uint)param_3);
              *puVar9 = (ushort)puVar6;
              param_2 = (ushort *)((int)param_2 + -1);
              puVar9 = puVar15;
            } while (param_2 != (ushort *)0x0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          puVar8 = puVar8 + ((param_1[1] - iVar16) + iVar23) * 3;
          local_10 = local_10 + -1;
        } while (local_10 != 0);
        return (uint)puVar8 & 0xffff0000;
      }
      break;
    case 1:
      if ((int)puVar6 < local_c) {
        local_8 = local_c + uVar5;
        do {
          if (iVar23 < iVar16) {
            local_c = iVar16 + uVar10;
            param_3 = (ushort *)(uint)(uVar3 >> 2);
            puVar9 = puVar15;
            do {
              uVar4 = *puVar9;
              uVar22 = *puVar8;
              puVar7 = puVar8 + 1;
              puVar14 = puVar8 + 2;
              uVar5 = CONCAT22((short)((uint)puVar6 >> 0x10),uVar4) & uVar13;
              puVar8 = puVar8 + 3;
              puVar15 = puVar9 + 1;
              puVar6 = (ushort *)
                       (CONCAT22((short)(uVar5 >> 0x10),
                                 (ushort)((ushort)uVar5 >> (bVar2 & 0x1f)) >> 2) +
                        ((int)(uint)uVar22 >> 2) + ((int)(uint)uVar22 >> 1) << (bVar2 & 0x1f) |
                        (((int)(uint)*puVar7 >> 2) + ((int)(uint)*puVar7 >> 1)) * 0x20 +
                        (((uint)uVar4 & (int)param_3 << 2) >> 2 & 0x3fe0) |
                       ((int)(uint)*puVar14 >> 2) + ((int)(uint)*puVar14 >> 1) +
                       ((uVar4 & 0x1c) >> 2));
              local_c = local_c + -1;
              *puVar9 = (ushort)puVar6;
              puVar9 = puVar15;
            } while (local_c != 0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          puVar8 = puVar8 + ((param_1[1] - iVar16) + iVar23) * 3;
          local_8 = local_8 + -1;
        } while (local_8 != 0);
        return (uint)puVar8 & 0xffff0000;
      }
      break;
    case 2:
      if ((int)puVar6 < local_c) {
        local_8 = local_c + uVar5;
        do {
          if (iVar23 < iVar16) {
            iVar17 = iVar16 + uVar10;
            uVar4 = param_2._2_2_;
            param_2 = (ushort *)((uint)param_2._2_2_ << 0x10);
            puVar6 = puVar15;
            param_3 = (ushort *)iVar17;
            do {
              uVar22 = *puVar6;
              uVar5 = CONCAT22((short)((uint)iVar17 >> 0x10),uVar22) & uVar13;
              uVar1 = *puVar8;
              puVar9 = puVar8 + 1;
              iVar17 = ((byte)((byte)uVar22 >> 1) & 0xf) + CONCAT22(uVar4,puVar8[2] >> 1);
              puVar8 = puVar8 + 3;
              puVar15 = puVar6 + 1;
              param_3 = (ushort *)((int)param_3 + -1);
              *puVar6 = (ushort)(CONCAT22((short)(uVar5 >> 0x10),
                                          (ushort)((ushort)uVar5 >> (bVar2 & 0x1f)) >> 1) +
                                 CONCAT22(uVar12,uVar1 >> 1) << (bVar2 & 0x1f)) |
                        (((uVar22 & uVar3) >> 6) + (*puVar9 >> 1)) * 0x20 | (ushort)iVar17;
              puVar6 = puVar15;
            } while (param_3 != (ushort *)0x0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          puVar8 = puVar8 + ((param_1[1] - iVar16) + iVar23) * 3;
          local_8 = local_8 + -1;
        } while (local_8 != 0);
        return (uint)puVar8 & 0xffff0000;
      }
      break;
    case 3:
      if ((int)puVar6 < local_c) {
        local_4 = (ushort *)(local_c + uVar5);
        do {
          if (iVar23 < iVar16) {
            iVar17 = iVar16 + uVar10;
            param_2 = (ushort *)CONCAT22(param_2._2_2_,uVar3 >> 6);
            puVar9 = puVar8;
            puVar14 = puVar15;
            uVar5 = CONCAT22((short)((uint)puVar6 >> 0x10),uVar3 >> 6);
            local_8 = iVar17;
            do {
              uVar4 = *puVar14;
              uVar24 = CONCAT22((short)((uint)iVar17 >> 0x10),uVar4) & uVar13;
              uVar22 = (ushort)uVar24 >> (bVar2 & 0x1f);
              uVar18 = (byte)((byte)uVar4 >> 1) & 0xf;
              puVar8 = puVar9 + 3;
              uVar11 = CONCAT22(uVar12,uVar4 >> 6) & uVar5;
              uVar21 = (undefined2)(uVar5 >> 0x10);
              iVar20 = (uint)(*puVar9 >> 2) + CONCAT22((short)(uVar24 >> 0x10),uVar22 >> 1);
              puVar15 = puVar14 + 1;
              iVar19 = CONCAT22((short)((uint)iVar20 >> 0x10),puVar9[1] >> 2) + uVar11 +
                       CONCAT22(uVar21,(ushort)uVar11 >> 1);
              puVar6 = (ushort *)CONCAT22(uVar21,(ushort)uVar18 >> 1);
              iVar17 = CONCAT22((short)((uint)iVar19 >> 0x10),puVar9[2] >> 2) + uVar18 + (int)puVar6
              ;
              *puVar14 = (ushort)(iVar20 + CONCAT22(uVar21,uVar22 >> 2) << (bVar2 & 0x1f)) |
                         (short)iVar19 * 0x20 | (ushort)iVar17;
              local_8 = local_8 + -1;
              puVar9 = puVar8;
              puVar14 = puVar15;
              uVar5 = (uint)param_2;
            } while (local_8 != 0);
          }
          puVar15 = puVar15 + (DAT_006a2dc8 - iVar16) + iVar23;
          puVar8 = puVar8 + ((param_1[1] - iVar16) + iVar23) * 3;
          local_4 = (ushort *)((int)local_4 + -1);
        } while (local_4 != (ushort *)0x0);
        return (uint)puVar8 & 0xffff0000;
      }
    }
  }
  return (uint)puVar8 & 0xffff0000;
}

