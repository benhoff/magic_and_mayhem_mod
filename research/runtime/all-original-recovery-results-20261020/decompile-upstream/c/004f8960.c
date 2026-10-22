
/* WARNING (jumptable): Unable to track spacebase fully for stack */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004f8960(int param_1)

{
  undefined2 uVar1;
  char cVar2;
  ushort uVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  undefined4 *puVar8;
  uint uVar9;
  int iVar10;
  int iVar11;
  ushort *puVar12;
  int iVar13;
  uint uVar14;
  bool bVar15;
  float10 fVar16;
  undefined4 uVar17;
  int local_110;
  int local_10c;
  int local_108;
  int local_104;
  int local_100;
  int local_fc;
  int local_f8;
  int local_f4;
  int local_f0;
  int local_ec;
  int local_e8;
  int local_e4;
  int local_e0;
  int local_dc;
  int local_d8;
  uint local_d4;
  int local_d0;
  int local_cc;
  uint local_c8;
  int local_c4;
  uint local_c0;
  uint local_bc;
  uint local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  uint local_a8;
  int local_a4;
  double local_a0;
  int local_94;
  int local_90;
  undefined4 local_8c;
  int local_88;
  int local_84;
  int local_80;
  int local_7c;
  int local_78;
  undefined4 local_74;
  int local_70;
  undefined4 local_6c;
  int local_64;
  int local_60;
  int local_58;
  int local_54;
  int local_4c;
  int local_48;
  int local_40;
  int local_3c;
  int local_34;
  int local_30;
  int local_28;
  int local_24;
  int local_1c;
  int local_18;
  int local_10;
  
  uVar9 = *(uint *)(param_1 + 0x85);
  local_fc = 1;
  local_90 = 1;
  local_ac = 0;
  local_110 = 0;
  puVar12 = *(ushort **)(param_1 + 0x79);
  local_c4 = CONCAT22(local_c4._2_2_,
                      (short)*(char *)((&DAT_006cb8c2)[uVar9 >> 1] +
                                       (&DAT_006cb942)[*(int *)(param_1 + 0x81)] + DAT_006c5c5c +
                                      *(int *)(param_1 + 0x7d)));
  if ((((((uint)puVar12[4] & 8 << ((byte)*(undefined4 *)(param_1 + 0x31) & 0x1f)) == 0) ||
       (((uint)puVar12[5] & 0x200 << ((char)*(undefined4 *)(param_1 + 0x65) - 1U & 0x1f)) == 0)) &&
      (((puVar12[5] & 0x40) == 0 ||
       ((((puVar12[4] & 4) == 0 || ((puVar12[4] & 0x8000) == 0)) || (DAT_005e41b0 != 0)))))) ||
     (*(int *)(param_1 + 0x61) - 1U <= uVar9)) {
    local_d0 = *(int *)(param_1 + 0x89);
    iVar10 = *(int *)(param_1 + 0x8d);
    local_10c = *(int *)(param_1 + 0x81) * 0x20 + 0x10;
    local_108 = *(int *)(param_1 + 0x7d) * 0x20 + 0x10;
    local_100 = uVar9 << 4;
    iVar4 = *(int *)(param_1 + 0x31) + (uint)*puVar12 * 0x59;
    uVar9 = *(uint *)(DAT_0065660c + 0x84 + iVar4 * 4);
    local_d4 = *(uint *)(DAT_0065660c + 0xd0 + iVar4 * 4);
    local_a8 = *(uint *)(DAT_0065660c + 0xe0 + iVar4 * 4);
    local_cc = iVar10;
    if (((puVar12[3] != 0xffff) || (puVar12[2] != 0xffff)) || (uVar9 != 0)) {
      if ((*(int *)(param_1 + 0x6d) == 1) && (*(uint *)(param_1 + 0x65) < ((byte)puVar12[4] & 3))) {
        local_fc = 0;
      }
      if ((uVar9 != 0) && (local_fc != 0)) {
        iVar4 = FUN_0057dcf0(iVar10,param_1 + 0x91);
        if (((iVar4 == 0) &&
            ((local_d4 == 0 || (iVar4 = FUN_0057dcf0(iVar10,param_1 + 0x91), iVar4 == 0)))) &&
           ((local_a8 == 0 || (iVar4 = FUN_0057dcf0(iVar10,param_1 + 0x91), iVar4 == 0)))) {
          iVar4 = *(int *)(param_1 + 0x79);
          local_b4 = (int)(short)local_c4;
          if ((*(byte *)(iVar4 + 10) & 8) == 0) {
            iVar5 = local_b4 - *(int *)(param_1 + 0xb1);
            local_e8 = -0x7f;
            if (-0x80 < iVar5) {
              local_e8 = iVar5;
            }
            piVar6 = *(int **)(param_1 + 0x2d);
            if (*(uint *)(*piVar6 + 0xc) < uVar9) {
              uVar9 = 0;
            }
            uVar1 = *(undefined2 *)(param_1 + 0xa9);
            uVar17 = 0x21;
          }
          else {
            iVar5 = local_b4 - *(int *)(param_1 + 0xb1);
            local_e8 = -0x7f;
            if (-0x80 < iVar5) {
              local_e8 = iVar5;
            }
            piVar6 = *(int **)(param_1 + 0x2d);
            if (*(uint *)(*piVar6 + 0xc) < uVar9) {
              uVar9 = 0;
            }
            uVar1 = *(undefined2 *)(param_1 + 0xa9);
            uVar17 = 0x1f;
          }
          FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                       local_d0,iVar10,*(undefined4 *)(param_1 + 0x9d),local_e8,
                       CONCAT22((short)((uint)iVar4 >> 0x10),uVar1),uVar17,
                       CONCAT44(iVar4,0xffff8ad0),0xffff8ad0);
        }
        else {
          DAT_006c5488 = *(undefined4 *)(param_1 + 0x7d);
          DAT_006c5484 = *(undefined4 *)(param_1 + 0x81);
          DAT_006cbb64 = *(undefined4 *)(param_1 + 0x85);
          iVar4 = *(int *)(param_1 + 0x79);
          local_b4 = (int)(short)local_c4;
          if ((*(byte *)(iVar4 + 10) & 8) == 0) {
            iVar5 = local_b4 - *(int *)(param_1 + 0xb1);
            local_e8 = -0x7f;
            if (-0x80 < iVar5) {
              local_e8 = iVar5;
            }
            piVar6 = *(int **)(param_1 + 0x2d);
            if (*(uint *)(*piVar6 + 0xc) < uVar9) {
              uVar9 = 0;
            }
            uVar1 = *(undefined2 *)(param_1 + 0xa9);
            uVar17 = 0x21;
          }
          else {
            iVar5 = local_b4 - *(int *)(param_1 + 0xb1);
            local_e8 = -0x7f;
            if (-0x80 < iVar5) {
              local_e8 = iVar5;
            }
            piVar6 = *(int **)(param_1 + 0x2d);
            if (*(uint *)(*piVar6 + 0xc) < uVar9) {
              uVar9 = 0;
            }
            uVar1 = *(undefined2 *)(param_1 + 0xa9);
            uVar17 = 0x1f;
          }
          iVar5 = FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,
                               local_100,local_d0,iVar10,*(undefined4 *)(param_1 + 0x9d),local_e8,
                               CONCAT22((short)((uint)iVar4 >> 0x10),uVar1),uVar17,
                               CONCAT44(iVar4,0xffff8ad0),0xffff8ad0);
          iVar4 = *(int *)(*(int *)(param_1 + 0x229) + iVar5 * 0x24);
          if (*(int *)(param_1 + 0x1ed) < iVar4) {
            _DAT_006c3750 = iVar5;
            *(int *)(param_1 + 0x1ed) = iVar4;
          }
        }
        iVar4 = *(int *)(param_1 + 0x79);
        if ((*(byte *)(iVar4 + 8) & 4) == 0) {
          if (local_d4 == 0) {
            *(byte *)(iVar4 + 8) = *(byte *)(iVar4 + 8) | 4;
            puVar12 = (ushort *)(*(int *)(param_1 + 0x79) + 8);
            *puVar12 = *puVar12 | 0x8000;
          }
          else {
            iVar5 = local_b4 - *(int *)(param_1 + 0xb1);
            local_e8 = -0x7f;
            if (-0x80 < iVar5) {
              local_e8 = iVar5;
            }
            piVar6 = *(int **)(param_1 + 0x2d);
            local_bc = local_d4;
            uVar9 = local_d4;
            if (*(uint *)(*piVar6 + 0xc) < local_d4) {
              uVar9 = 0;
            }
            FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                         local_d0,iVar10,*(undefined4 *)(param_1 + 0x9d),local_e8,
                         CONCAT22((short)((uint)iVar4 >> 0x10),*(undefined2 *)(param_1 + 0xa9)),0x21
                         ,0xffff8ad0,iVar4,0xffff8ad0,0);
          }
          iVar4 = *(int *)(param_1 + 0x79);
          if ((*(ushort *)(iVar4 + 8) & 0x8000) == 0) {
            if (local_a8 == 0) {
              *(ushort *)(iVar4 + 8) = *(ushort *)(iVar4 + 8) | 0x8000;
            }
            else {
              iVar5 = local_b4 - *(int *)(param_1 + 0xb1);
              local_e8 = -0x7f;
              if (-0x80 < iVar5) {
                local_e8 = iVar5;
              }
              piVar6 = *(int **)(param_1 + 0x2d);
              local_bc = local_a8;
              uVar9 = local_a8;
              if (*(uint *)(*piVar6 + 0xc) < local_a8) {
                uVar9 = 0;
              }
              FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100
                           ,local_d0,iVar10,*(undefined4 *)(param_1 + 0x9d),local_e8,
                           CONCAT22((short)((uint)iVar4 >> 0x10),*(undefined2 *)(param_1 + 0xa9)),
                           0x21,0xffff8ad0,iVar4,0xffff8ad0,0);
            }
          }
        }
      }
    }
    if (((*(int *)(param_1 + 0x75) != 0) &&
        (iVar4 = *(int *)(*(int *)(param_1 + 0x21) + 0x7f0), iVar4 != 0)) &&
       (*(int *)(param_1 + 0x85) == iVar4)) {
      if ((*(byte *)(DAT_0065660c + 0xa8 + (uint)**(ushort **)(param_1 + 0x79) * 0x164) & 0x20) == 0
         ) {
        *(int *)(param_1 + 0x1f5) = *(int *)(param_1 + 0x1f5) + 1;
      }
      if (((-0x46 < (short)local_c4) || (iVar10 = local_cc, DAT_005e1404 == 0)) &&
         ((uVar3 = **(ushort **)(param_1 + 0x79), uVar3 == 0 ||
          (iVar10 = local_cc, (*(byte *)(DAT_0065660c + 0xa8 + (uint)uVar3 * 0x164) & 0x20) == 0))))
      {
        piVar6 = *(int **)(param_1 + 0x2d);
        uVar9 = *(uint *)(DAT_0065660c + 0x1e8);
        if (*(uint *)(*piVar6 + 0xc) < uVar9) {
          uVar9 = 0;
        }
        local_e0 = *(int *)(piVar6[2] + uVar9 * 4) + piVar6[3];
        FUN_004ffd30(local_e0,local_108,local_10c,local_100,local_d0,iVar10,
                     *(int *)(param_1 + 0x9d) + -1,local_c4,0,6,0xffff8ad0,0,0xffff8ad0,0);
      }
    }
  }
  uVar3 = *(ushort *)(*(int *)(param_1 + 0x79) + 2);
  iVar10 = local_f4;
  iVar4 = local_f8;
  if (uVar3 != 0xffff) {
    if (uVar3 < DAT_0068949c) {
      iVar5 = DAT_00689498 + (uint)uVar3 * 0x22e;
    }
    else {
      iVar5 = 0;
    }
    local_d4 = *(uint *)(iVar5 + 0x28);
    if (((*(int *)(iVar5 + 4) == 0) || (*(int *)(iVar5 + 8) != *(int *)(param_1 + 0x7d))) ||
       ((*(int *)(iVar5 + 0xc) != *(int *)(param_1 + 0x81) ||
        (*(int *)(iVar5 + 0x10) != *(int *)(param_1 + 0x85))))) {
      FUN_0049bd20();
      iVar10 = local_f4;
      iVar4 = local_f8;
    }
    else {
      iVar7 = FUN_0049a600();
      iVar10 = local_f4;
      iVar4 = local_f8;
      if (iVar7 != 0) goto LAB_004f952c;
      if (((*(int *)(iVar5 + 0x22a) != 0) &&
          ((DAT_005e18e8 < (short)local_c4 + 0xf || (cVar2 = FUN_0049aa30(), cVar2 != '\0')))) ||
         (iVar10 = local_f4, iVar4 = local_f8, DAT_00644520 == -1)) {
        local_c0 = (uint)(*(int *)(iVar5 + 0x10) < (int)DAT_006c5c80);
        iVar10 = *(int *)(iVar5 + 0x18) + *(int *)(iVar5 + 0xc) * -0x20 + -0x10;
        iVar4 = *(int *)(iVar5 + 0x14) + *(int *)(iVar5 + 8) * -0x20 + -0x10;
        switch(*(undefined4 *)(param_1 + 0x31)) {
        case 0:
          local_f8 = iVar4 - iVar10;
          local_f4 = (iVar4 + iVar10) / 2;
          break;
        case 1:
          local_f8 = iVar4 + iVar10;
          local_f4 = (iVar10 - iVar4) / 2;
          break;
        case 2:
          local_f8 = iVar10 - iVar4;
          local_f4 = -((iVar4 + iVar10) / 2);
          break;
        case 3:
          local_f8 = -(iVar4 + iVar10);
          local_f4 = (iVar4 - iVar10) / 2;
        }
        iVar13 = local_f4;
        iVar7 = local_f8;
        FUN_00487c40();
        if (((local_d4 == 0xf) || (local_d4 == 0xe)) || (local_d4 == 0x10)) {
          *(int *)(param_1 + 0x1fd) = *(int *)(param_1 + 0x1fd) + 1;
        }
        else if (((local_d4 == 0x19) || (local_d4 == 0x1a)) || (local_d4 == 0x1b)) {
          *(int *)(param_1 + 0x219) = *(int *)(param_1 + 0x219) + 1;
        }
        else if (local_d4 == 10) {
          *(int *)(param_1 + 0x205) = *(int *)(param_1 + 0x205) + 1;
        }
        iVar10 = local_f4;
        iVar4 = local_f8;
        if (local_e0 != 0) {
          local_108 = *(int *)(iVar5 + 0x14);
          local_10c = *(int *)(iVar5 + 0x18);
          local_100 = *(int *)(iVar5 + 0x1c);
          if (local_d4 == 8) {
            iVar10 = *(int *)(iVar5 + 8);
            iVar4 = *(int *)(iVar5 + 0xc);
            uVar9 = *(uint *)(iVar5 + 0x14) & 0x8000001f;
            bVar15 = uVar9 == 0;
            if ((int)uVar9 < 0) {
              bVar15 = (uVar9 - 1 | 0xffffffe0) == 0xffffffff;
            }
            if ((bVar15) && ((*(int *)(param_1 + 0x31) == 0 || (*(int *)(param_1 + 0x31) == 3)))) {
              iVar10 = iVar10 + 1;
            }
            uVar9 = *(uint *)(iVar5 + 0x18) & 0x8000001f;
            bVar15 = uVar9 == 0;
            if ((int)uVar9 < 0) {
              bVar15 = (uVar9 - 1 | 0xffffffe0) == 0xffffffff;
            }
            if (((bVar15) && (-1 < *(int *)(param_1 + 0x31))) && (*(int *)(param_1 + 0x31) < 2)) {
              iVar4 = iVar4 + 1;
            }
            local_10c = iVar4 * 0x20 + 0x10;
            local_108 = iVar10 * 0x20 + 0x10;
            local_100 = *(int *)(iVar5 + 0x10) << 4;
          }
          iVar10 = FUN_0049a5f0();
          if (iVar10 == 0) {
            iVar10 = 0;
          }
          else {
            iVar10 = (int)(short)local_c4;
          }
          FUN_004ffd30(local_e0,local_108,local_10c,local_100,
                       *(int *)(param_1 + 0x89) + local_dc + iVar7,
                       (*(int *)(param_1 + 0x8d) - (*(uint *)(iVar5 + 0x1c) & 0xf)) + local_d8 +
                       iVar13,*(int *)(param_1 + 0x9d) + 7,iVar10,local_c0,
                       *(undefined4 *)(iVar5 + 0x21e),0xffff8ad0,0,0xffff8ad0,0);
          iVar10 = local_f4;
          iVar4 = local_f8;
        }
      }
    }
LAB_004f9269:
    while (uVar3 = *(ushort *)(iVar5 + 0x1a0), uVar3 != 0xffff) {
      if (uVar3 < DAT_0068949c) {
        iVar5 = DAT_00689498 + (uint)uVar3 * 0x22e;
      }
      else {
        iVar5 = 0;
      }
      local_e8 = *(int *)(iVar5 + 0x28);
      if (((*(int *)(iVar5 + 4) == 0) || (*(int *)(iVar5 + 8) != *(int *)(param_1 + 0x7d))) ||
         ((*(int *)(iVar5 + 0xc) != *(int *)(param_1 + 0x81) ||
          (*(int *)(iVar5 + 0x10) != *(int *)(param_1 + 0x85))))) goto LAB_004f9514;
      iVar7 = FUN_0049a600();
      if (iVar7 != 0) break;
      if ((*(int *)(iVar5 + 0x22a) != 0) &&
         ((DAT_005e18e8 < (short)local_c4 + 0xf || (cVar2 = FUN_0049aa30(), cVar2 != '\0')))) {
        local_c0 = (uint)(*(int *)(iVar5 + 0x10) < (int)DAT_006c5c80);
        iVar7 = *(int *)(iVar5 + 0x18) + *(int *)(iVar5 + 0xc) * -0x20 + -0x10;
        iVar13 = *(int *)(iVar5 + 0x14) + *(int *)(iVar5 + 8) * -0x20 + -0x10;
        switch(*(undefined4 *)(param_1 + 0x31)) {
        case 0:
          iVar10 = iVar13 + iVar7;
          local_f8 = iVar13 - iVar7;
          break;
        case 1:
          local_f8 = iVar13 + iVar7;
          iVar10 = iVar7 - iVar13;
          break;
        case 2:
          iVar4 = iVar7 - iVar13;
          iVar10 = -((iVar13 + iVar7) / 2);
          local_f8 = iVar4;
          local_f4 = iVar10;
          goto switchD_004f935c_default;
        case 3:
          local_f8 = -(iVar13 + iVar7);
          iVar10 = iVar13 - iVar7;
          break;
        default:
          goto switchD_004f935c_default;
        }
        iVar10 = iVar10 / 2;
        iVar4 = local_f8;
        local_f4 = iVar10;
switchD_004f935c_default:
        FUN_00487c40();
        if (((local_e8 == 0xf) || (local_e8 == 0xe)) || (local_e8 == 0x10)) {
          *(int *)(param_1 + 0x1fd) = *(int *)(param_1 + 0x1fd) + 1;
        }
        else if (((local_e8 == 0x19) || (local_e8 == 0x1a)) || (local_e8 == 0x1b)) {
          *(int *)(param_1 + 0x219) = *(int *)(param_1 + 0x219) + 1;
        }
        else if (local_e8 == 10) {
          *(int *)(param_1 + 0x205) = *(int *)(param_1 + 0x205) + 1;
        }
        if (local_e0 != 0) {
          local_108 = *(int *)(iVar5 + 0x14);
          local_10c = *(int *)(iVar5 + 0x18);
          local_100 = *(int *)(iVar5 + 0x1c);
          if (*(int *)(iVar5 + 0x28) == 8) {
            iVar7 = *(int *)(iVar5 + 8);
            iVar13 = *(int *)(iVar5 + 0xc);
            uVar9 = *(uint *)(iVar5 + 0x14) & 0x8000001f;
            bVar15 = uVar9 == 0;
            if ((int)uVar9 < 0) {
              bVar15 = (uVar9 - 1 | 0xffffffe0) == 0xffffffff;
            }
            if ((bVar15) && ((*(int *)(param_1 + 0x31) == 0 || (*(int *)(param_1 + 0x31) == 3)))) {
              iVar7 = iVar7 + 1;
            }
            uVar9 = *(uint *)(iVar5 + 0x18) & 0x8000001f;
            bVar15 = uVar9 == 0;
            if ((int)uVar9 < 0) {
              bVar15 = (uVar9 - 1 | 0xffffffe0) == 0xffffffff;
            }
            if (((bVar15) && (-1 < *(int *)(param_1 + 0x31))) && (*(int *)(param_1 + 0x31) < 2)) {
              iVar13 = iVar13 + 1;
            }
            local_10c = iVar13 * 0x20 + 0x10;
            local_108 = iVar7 * 0x20 + 0x10;
            local_100 = *(int *)(iVar5 + 0x10) << 4;
          }
          iVar7 = FUN_0049a5f0();
          FUN_004ffd30(local_e0,local_108,local_10c,local_100,
                       *(int *)(param_1 + 0x89) + local_dc + iVar4,
                       (*(int *)(param_1 + 0x8d) - (*(uint *)(iVar5 + 0x1c) & 0xf)) + local_d8 +
                       iVar10,*(int *)(param_1 + 0x9d) + 7,
                       -(uint)(iVar7 != 0) & (int)(short)local_c4,local_c0,
                       *(undefined4 *)(iVar5 + 0x21e),0xffff8ad0,0,0xffff8ad0,0);
        }
      }
    }
  }
LAB_004f952c:
  if ((DAT_006a49cc != 0) && (iVar5 = FUN_005344e0(), iVar5 != 0)) {
    local_108 = *(int *)(param_1 + 0x7d) * 0x20 + 0x10;
    local_10c = *(int *)(param_1 + 0x81) * 0x20 + 0x10;
    local_100 = *(uint *)(param_1 + 0x85) << 4;
    bVar15 = *(uint *)(param_1 + 0x85) < DAT_006c5c80;
    *(int *)(param_1 + 0x205) = *(int *)(param_1 + 0x205) + 1;
    FUN_00534640(*(int *)(param_1 + 0x7d),*(int *)(param_1 + 0x81),&local_e0,&local_bc);
    switch(*(undefined4 *)(param_1 + 0x31)) {
    case 0:
    case 1:
    case 2:
    case 3:
      iVar4 = 0;
      iVar10 = 0;
      local_f8 = 0;
      local_f4 = 0;
    }
    FUN_004ffd30(local_e0,local_108,local_10c,local_100,*(int *)(param_1 + 0x89) + local_dc + iVar4,
                 *(int *)(param_1 + 0x8d) + local_d8 + iVar10,*(int *)(param_1 + 0x9d) + 7,local_c4,
                 bVar15,local_bc,0xffff8ad0,0,0xffff8ad0,0);
  }
  uVar3 = *(ushort *)(*(int *)(param_1 + 0x79) + 6);
  if (((uVar3 != 0xffff) && (local_fc != 0)) &&
     ((DAT_005e18f8 < (short)local_c4 || ((DAT_005e1404 == 0 || (DAT_00644520 == -1)))))) {
    if (uVar3 < DAT_006b124c) {
      iVar10 = DAT_006b1248 + (uint)uVar3 * 0x7e;
    }
    else {
      iVar10 = 0;
    }
    local_c0 = (uint)(*(int *)(iVar10 + 0x10) < (int)DAT_006c5c80);
    iVar4 = *(int *)(iVar10 + 0x32);
    if (iVar4 == 0) {
      local_e0 = 0;
      local_dc = 0;
      local_d8 = 0;
    }
    else {
      piVar6 = *(int **)(*(int *)(iVar10 + 0x4a) + 0x110);
      uVar9 = *(uint *)(*(int *)(iVar10 + 0x32) + 4);
      if (*(uint *)(*piVar6 + 0xc) < uVar9) {
        uVar9 = 0;
      }
      local_e0 = *(int *)(piVar6[2] + uVar9 * 4) + piVar6[3];
      local_dc = *(int *)(iVar4 + 8);
      local_d8 = *(int *)(iVar4 + 0xc);
      if (local_e0 != 0) {
        local_108 = *(int *)(iVar10 + 8) * 0x20 + 0x10;
        local_10c = *(int *)(iVar10 + 0xc) * 0x20 + 0x10;
        local_100 = *(int *)(iVar10 + 0x10) << 4;
        uVar9 = *(int *)(param_1 + 0x91) -
                (((*(uint *)(local_e0 + 4) >> 1) - *(int *)(local_e0 + 0xc)) + local_dc + local_d0);
        uVar14 = (int)uVar9 >> 0x1f;
        local_94 = (uVar9 ^ uVar14) - uVar14;
        uVar9 = *(int *)(param_1 + 0x95) -
                (((*(uint *)(local_e0 + 8) >> 2) - *(int *)(local_e0 + 0x10)) + local_d8 + local_cc)
        ;
        local_bc = (int)uVar9 >> 0x1f;
        local_bc = (uVar9 ^ local_bc) - local_bc;
        fVar16 = (float10)FUN_0059d60c((double)local_94,(double)(int)local_bc);
        if ((float10)_DAT_005c7140 <= fVar16) goto LAB_004f9834;
        goto LAB_004f97a2;
      }
    }
    while (uVar3 = FUN_005437f0(), uVar3 != 0xffff) {
      if ((uVar3 < DAT_006b124c) && (iVar10 = DAT_006b1248 + (uint)uVar3 * 0x7e, iVar10 != 0)) {
        iVar4 = *(int *)(iVar10 + 0x32);
        if (iVar4 == 0) {
          local_e0 = 0;
          local_dc = 0;
          local_d8 = 0;
        }
        else {
          piVar6 = *(int **)(*(int *)(iVar10 + 0x4a) + 0x110);
          uVar9 = *(uint *)(*(int *)(iVar10 + 0x32) + 4);
          if (*(uint *)(*piVar6 + 0xc) < uVar9) {
            uVar9 = 0;
          }
          local_e0 = *(int *)(piVar6[2] + uVar9 * 4) + piVar6[3];
          local_dc = *(int *)(iVar4 + 8);
          local_d8 = *(int *)(iVar4 + 0xc);
          if (local_e0 != 0) {
            iVar5 = *(int *)(iVar10 + 0x18) + *(int *)(iVar10 + 0xc) * -0x20;
            iVar4 = *(int *)(iVar10 + 0x14) + *(int *)(iVar10 + 8) * -0x20;
            switch(*(undefined4 *)(param_1 + 0x31)) {
            case 0:
              local_f8 = iVar4 - iVar5;
              iVar4 = iVar4 + iVar5;
              break;
            case 1:
              local_f8 = iVar4 + iVar5;
              iVar4 = iVar5 - iVar4;
              break;
            case 2:
              local_f8 = iVar5 - iVar4;
              local_f4 = -((iVar4 + iVar5) / 2);
              goto switchD_004f9942_default;
            case 3:
              local_f8 = -(iVar4 + iVar5);
              iVar4 = iVar4 - iVar5;
              break;
            default:
              goto switchD_004f9942_default;
            }
            local_f4 = iVar4 / 2;
switchD_004f9942_default:
            local_108 = *(int *)(iVar10 + 8) * 0x20 + 0x10;
            local_10c = *(int *)(iVar10 + 0xc) * 0x20 + 0x10;
            local_100 = *(int *)(iVar10 + 0x10) << 4;
            iVar4 = FUN_0057dcf0(local_d8 + local_cc,param_1 + 0x91);
            if (iVar4 == 0) {
LAB_004f9834:
              FUN_004ffd30(local_e0,local_108,local_10c,local_100,
                           *(int *)(param_1 + 0x89) + local_dc,
                           (*(int *)(param_1 + 0x8d) - (*(uint *)(iVar10 + 0x1c) & 0xf)) + local_d8,
                           *(int *)(param_1 + 0x9d) + 2,local_c4,local_c0,
                           *(undefined4 *)(iVar10 + 0x5e),0xffff8ad0,0,0xffff8ad0,0);
            }
            else {
LAB_004f97a2:
              DAT_006dbea4 = iVar10;
              iVar4 = FUN_004ffd30(local_e0,local_108,local_10c,local_100,
                                   *(int *)(param_1 + 0x89) + local_dc,
                                   (*(int *)(param_1 + 0x8d) - (*(uint *)(iVar10 + 0x1c) & 0xf)) +
                                   local_d8,*(int *)(param_1 + 0x9d) + 2,local_c4 + 0x20,local_c0,
                                   *(undefined4 *)(iVar10 + 0x5e),0xffff8ad0,0,0xffff8ad0,0);
              iVar10 = *(int *)(*(int *)(param_1 + 0x229) + iVar4 * 0x24);
              if (*(int *)(param_1 + 0x1ed) < iVar10) {
                _DAT_006c3750 = iVar4;
                *(int *)(param_1 + 0x1f1) = iVar10;
              }
            }
          }
        }
      }
    }
  }
  if ((*(byte *)(*(int *)(param_1 + 0x79) + 0xb) & 0x20) != 0) {
    FUN_0052b0b0();
  }
  uVar3 = *(ushort *)(*(int *)(param_1 + 0x79) + 4);
  if (uVar3 != 0xffff) {
    if (uVar3 < DAT_006def5c) {
      iVar10 = (uint)uVar3 * 0xe4b + DAT_006def58;
    }
    else {
      iVar10 = 0;
    }
    iVar4 = DAT_006c54dc +
            ((&DAT_006cb942)[*(int *)(iVar10 + 0xc)] + (&DAT_006cb8c2)[*(int *)(iVar10 + 0x10)] +
            *(int *)(iVar10 + 8)) * 0xc;
    uVar3 = *(ushort *)(iVar4 + 10);
    if (((uVar3 & 1) == 0) && (*(int *)(iVar10 + 0x5ec) == 1)) {
      *(ushort *)(iVar4 + 10) = uVar3 | 1;
    }
    iVar4 = FUN_00506220();
    if ((iVar4 != 0) &&
       (((DAT_00644520 < 0 || (7 < DAT_00644520)) ||
        (*(int *)(iVar10 + 0x24 + DAT_00644520 * 4) == 0)))) {
      FUN_00466870();
      FUN_00508ca0();
    }
  }
  uVar3 = *(ushort *)(*(int *)(param_1 + 0x79) + 4);
  if (uVar3 == 0xffff) goto LAB_004f9b13;
  if ((*(byte *)(*(int *)(param_1 + 0x79) + 10) & 1) == 0) goto LAB_004f9b13;
  if (local_fc == 0) {
    if (uVar3 < DAT_006def5c) {
      iVar10 = (uint)uVar3 * 0xe4b + DAT_006def58;
    }
    else {
      iVar10 = 0;
    }
    if (*(int *)(iVar10 + 0x108) < 1) goto LAB_004f9b13;
  }
  if (uVar3 < DAT_006def5c) {
    local_104 = (uint)uVar3 * 0xe4b + DAT_006def58;
  }
  else {
    local_104 = 0;
  }
  do {
    iVar10 = local_104;
    if ((((DAT_00644520 < 0) || (7 < DAT_00644520)) ||
        (*(int *)(local_104 + 0x24 + DAT_00644520 * 4) == 0)) &&
       ((DAT_005e1404 != 0 && (DAT_00644520 != -1)))) goto LAB_004f9b13;
    iVar4 = *(int *)(local_104 + 8);
    local_10c = *(int *)(local_104 + 0xc);
    local_100 = *(int *)(local_104 + 0x10);
    if ((*(int *)(local_104 + 0x618) == 0) || (*(int *)(local_104 + 0x610) != 0)) {
      local_cc = 0;
      local_d0 = 0;
      goto switchD_004fa026_default;
    }
    iVar7 = DAT_006c5498;
    iVar5 = iVar4;
    switch(*(undefined4 *)(local_104 + 0x608)) {
    case 0:
      local_d0 = 0;
      local_cc = 0x20;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
      case 1:
switchD_004f9c1d_caseD_0:
        local_cc = 0x20;
        if (local_10c != 0) goto switchD_004f9bfe_default;
        break;
      case 2:
      case 3:
        goto switchD_004f9c1d_caseD_2;
      default:
        goto switchD_004f9bfe_default;
      }
      goto LAB_004f9f7d;
    case 1:
      local_d0 = -0x20;
      local_cc = 0x20;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        if (iVar4 == DAT_006c5494 + -1) {
          iVar5 = -DAT_006c5494;
LAB_004f9f13:
          local_110 = local_110 + iVar5 * 0x20;
        }
LAB_004f9f20:
        local_cc = 0x20;
        if (local_10c == 0) goto LAB_004f9f7d;
        break;
      case 1:
        if (local_10c == 0) {
          local_110 = local_110 + DAT_006c5498 * 0x20;
        }
switchD_004f9caa_caseD_1:
        local_d0 = -0x20;
        iVar13 = iVar4;
        iVar5 = DAT_006c5494;
LAB_004f9e93:
        if (iVar13 == iVar5 + -1) goto LAB_004f9f7d;
        break;
      case 2:
        iVar5 = DAT_006c5494;
        if (iVar4 == DAT_006c5494 + -1) {
LAB_004f9f4d:
          local_110 = local_110 + iVar5 * 0x20;
        }
        goto switchD_004f9c1d_caseD_2;
      case 3:
        if (iVar4 == DAT_006c5494 + -1) {
LAB_004f9f49:
          iVar5 = -DAT_006c5494;
          goto LAB_004f9f4d;
        }
switchD_004f9c1d_caseD_2:
        local_cc = 0x20;
        iVar5 = local_10c;
joined_r0x004f9f79:
        if (iVar5 == 0) {
LAB_004f9f7b:
          iVar7 = -iVar7;
LAB_004f9f7d:
          local_110 = local_110 + iVar7 * 0x20;
        }
      }
      break;
    case 2:
      local_d0 = -0x20;
      local_cc = 0;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
      case 3:
        if (iVar4 == DAT_006c5494 + -1) {
          local_110 = local_110 + DAT_006c5494 * -0x20;
        }
        break;
      case 1:
        goto switchD_004f9caa_caseD_1;
      case 2:
        bVar15 = iVar4 == DAT_006c5494 + -1;
LAB_004f9cd7:
        local_cc = 0;
        if (bVar15) {
          local_110 = local_110 + DAT_006c5494 * 0x20;
        }
      }
      break;
    case 3:
      local_d0 = -0x20;
      local_cc = -0x20;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        if (iVar4 == DAT_006c5494 + -1) {
          local_110 = local_110 + DAT_006c5494 * -0x20;
        }
        goto joined_r0x004f9e1c;
      case 1:
        if (iVar4 == DAT_006c5494 + -1) {
          local_110 = local_110 + DAT_006c5498 * 0x20;
        }
joined_r0x004f9e1c:
        local_cc = -0x20;
        if (local_10c != DAT_006c5498 + -1) break;
        goto LAB_004f9f7b;
      case 2:
        if (iVar4 == DAT_006c5494 + -1) {
LAB_004f9e7d:
          local_110 = local_110 + DAT_006c5494 * 0x20;
        }
switchD_004f9db2_caseD_3:
        local_cc = -0x20;
        iVar13 = local_10c;
        iVar5 = DAT_006c5498;
        goto LAB_004f9e93;
      case 3:
        if (iVar4 == DAT_006c5494 + -1) {
          local_110 = local_110 + DAT_006c5494 * -0x20;
        }
joined_r0x004f9e6e:
        local_cc = -0x20;
        if (local_10c == DAT_006c5498 + -1) goto LAB_004f9f7d;
      }
      break;
    case 4:
      local_d0 = 0;
      local_cc = -0x20;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        break;
      case 1:
switchD_004f9db2_caseD_1:
        break;
      case 2:
        goto joined_r0x004f9e6e;
      case 3:
        goto switchD_004f9db2_caseD_3;
      default:
        goto switchD_004f9bfe_default;
      }
      goto joined_r0x004f9e1c;
    case 5:
      local_d0 = 0x20;
      local_cc = -0x20;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        if (iVar4 == 0) {
          local_110 = local_110 + DAT_006c5494 * 0x20;
        }
        goto switchD_004f9db2_caseD_1;
      case 1:
        if (local_10c == DAT_006c5498 + -1) {
          local_110 = local_110 + DAT_006c5498 * -0x20;
        }
switchD_004f9eb8_caseD_1:
        local_d0 = 0x20;
        break;
      case 2:
        if (iVar4 == 0) {
          local_110 = local_110 + DAT_006c5494 * -0x20;
        }
        goto joined_r0x004f9e6e;
      case 3:
        if (iVar4 == 0) goto LAB_004f9e7d;
        goto switchD_004f9db2_caseD_3;
      default:
        goto switchD_004f9bfe_default;
      }
      goto joined_r0x004f9f79;
    case 6:
      local_d0 = 0x20;
      local_cc = 0;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        if (iVar4 == 0) {
          local_110 = local_110 + DAT_006c5494 * 0x20;
        }
        break;
      case 1:
        goto switchD_004f9eb8_caseD_1;
      case 2:
        iVar7 = DAT_006c5494;
joined_r0x004f9f79:
        local_d0 = 0x20;
        goto joined_r0x004f9f79;
      case 3:
        bVar15 = iVar4 == 0;
        goto LAB_004f9cd7;
      }
      break;
    case 7:
      local_d0 = 0x20;
      local_cc = 0x20;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        iVar5 = DAT_006c5494;
        if (iVar4 == 0) goto LAB_004f9f13;
        goto LAB_004f9f20;
      case 1:
        if (iVar4 == 0) {
          local_110 = local_110 + DAT_006c5498 * -0x20;
        }
        goto switchD_004f9c1d_caseD_0;
      case 2:
        if (iVar4 == 0) goto LAB_004f9f49;
        goto switchD_004f9c1d_caseD_2;
      case 3:
        iVar5 = local_10c;
        if (iVar4 == 0) {
          local_110 = local_110 + DAT_006c5494 * 0x20;
        }
        goto joined_r0x004f9f79;
      }
    }
switchD_004f9bfe_default:
    iVar5 = *(int *)(local_104 + 0x977);
    iVar7 = *(int *)(local_104 + 8);
    iVar13 = *(int *)(local_104 + 0x983 + iVar5 * 0x1c);
    iVar11 = *(int *)(local_104 + 0x97f + iVar5 * 0x1c);
    local_fc = *(int *)(local_104 + iVar5 * 0x1c + 0x987);
    local_bc = iVar7 - iVar11 >> 0x1f;
    local_bc = (iVar7 - iVar11 ^ local_bc) - local_bc;
    if (DAT_006c5494 / 2 < (int)local_bc) {
      if (iVar11 < iVar7) {
        iVar11 = iVar11 + DAT_006c5494;
      }
      else {
        iVar11 = iVar11 - DAT_006c5494;
      }
    }
    uVar9 = *(int *)(local_104 + 0xc) - iVar13;
    uVar14 = (int)uVar9 >> 0x1f;
    if (DAT_006c5498 / 2 < (int)((uVar9 ^ uVar14) - uVar14)) {
      if (iVar13 < *(int *)(local_104 + 0xc)) {
        iVar13 = iVar13 + DAT_006c5498;
      }
      else {
        iVar13 = iVar13 - DAT_006c5498;
      }
    }
    switch(*(undefined4 *)(param_1 + 0x31)) {
    case 0:
      if (iVar11 <= iVar7) {
LAB_004fa037:
        iVar11 = iVar4;
      }
      break;
    case 1:
      if (iVar7 <= iVar11) goto LAB_004fa037;
      break;
    case 2:
      if (iVar11 < iVar7) {
        iVar4 = iVar11;
      }
      if (*(int *)(local_104 + 0xc) <= iVar13) goto LAB_004fa063;
      goto LAB_004fa05f;
    case 3:
      if (iVar7 < iVar11) {
        iVar4 = iVar11;
      }
      if (iVar13 < *(int *)(local_104 + 0xc)) goto LAB_004fa05f;
      goto LAB_004fa063;
    default:
      goto switchD_004fa026_default;
    }
    iVar4 = iVar11;
    if (*(int *)(local_104 + 0xc) < iVar13) {
LAB_004fa05f:
      local_10c = iVar13;
    }
LAB_004fa063:
    if (*(int *)(local_104 + 0x10) < local_fc) {
      local_100 = local_fc;
    }
switchD_004fa026_default:
    if (*(int *)(*(int *)(local_104 + 0xac) + 8) == 2) {
      iVar5 = *(int *)(param_1 + 0x31);
      if (iVar5 == 0) {
        iVar4 = iVar4 + 1;
      }
      else if (iVar5 != 1) {
        if (iVar5 == 3) {
          iVar4 = iVar4 + 1;
        }
        goto LAB_004fa0bf;
      }
      local_10c = local_10c + 1;
    }
LAB_004fa0bf:
    local_10c = local_10c * 0x20 + 0x10;
    local_108 = iVar4 * 0x20 + 0x10;
    local_100 = local_100 << 4;
    if ((*(int *)(local_104 + 0x5fc) == 10) || (*(int *)(local_104 + 0x5fc) == 0x14)) {
      FUN_004eadf0(&local_10c,&local_100);
    }
    if (*(int *)(iVar10 + 0xa8) == 0x11) {
      iVar4 = *(int *)(param_1 + 0x31);
      uVar9 = *(int *)(iVar10 + 0x608) + iVar4 * -2 & 7;
      if ((uVar9 == 0) || (uVar9 == 6)) {
        switch(iVar4) {
        case 0:
        case 3:
          local_108 = local_108 + 0x1e;
          break;
        case 1:
switchD_004fa18a_caseD_1:
          local_10c = local_10c + 0x1e;
          break;
        case 2:
switchD_004fa18a_caseD_2:
          local_108 = local_108 + -0x1e;
        }
      }
      else if (uVar9 == 7) {
        switch(iVar4) {
        case 0:
          local_108 = local_108 + 0x20;
          goto switchD_004fa18a_caseD_1;
        case 1:
          local_10c = local_10c + 0x20;
          local_108 = local_108 + -0x1e;
          break;
        case 2:
          local_10c = local_10c + -0x20;
          goto switchD_004fa18a_caseD_2;
        case 3:
          local_108 = local_108 + 0x20;
          local_10c = local_10c + -0x1e;
        }
      }
    }
    puVar12 = *(ushort **)(param_1 + 0x79);
    local_bc = *(uint *)(iVar10 + 0x1c) & 0xf;
    if ((*(int *)(iVar10 + 0x108) != 0) || (*(int *)(iVar10 + 0x5ec) == 0x10)) {
      while (((ushort *)
              (*(int *)(*(int *)(param_1 + 0x21) + 0x4c) +
              *(int *)(*(int *)(param_1 + 0x21) + 0x10) * 0xc) <= puVar12 &&
             ((*(byte *)(DAT_0065660c + 0xb0 + (uint)*puVar12 * 0x164) & 8) == 0))) {
        local_bc = local_bc + 0x10;
        puVar12 = puVar12 + DAT_006c54a0 * -6;
      }
    }
    local_bc = local_bc + (2 - *(int *)(DAT_0065660c + 0x94 + (uint)*puVar12 * 0x164));
    iVar4 = *(int *)(local_104 + 0xb7f) + *(int *)(local_104 + 0xc) * -0x20 + local_cc +
            *(int *)(local_104 + 0x18);
    iVar10 = *(int *)(local_104 + 0xb7b) + *(int *)(local_104 + 8) * -0x20 +
             *(int *)(local_104 + 0x14) + local_d0;
    switch(*(undefined4 *)(param_1 + 0x31)) {
    case 0:
      local_f8 = iVar10 - iVar4;
      iVar10 = iVar10 + iVar4;
      break;
    case 1:
      local_f8 = iVar10 + iVar4;
      iVar10 = iVar4 - iVar10;
      break;
    case 2:
      local_f8 = iVar4 - iVar10;
      local_f4 = -((iVar10 + iVar4) / 2);
      goto switchD_004fa26b_default;
    case 3:
      local_f8 = -(iVar10 + iVar4);
      iVar10 = iVar10 - iVar4;
      break;
    default:
      goto switchD_004fa26b_default;
    }
    local_f4 = iVar10 / 2;
switchD_004fa26b_default:
    if (*(int *)(local_104 + 0x855) == 0) {
      FUN_00507190();
    }
    else {
      FUN_00507330();
    }
    iVar4 = *(int *)(param_1 + 0x89) + local_f8;
    iVar10 = *(int *)(param_1 + 0x85) * 0x10;
    iVar5 = ((*(int *)(param_1 + 0x8d) + iVar10) - *(int *)(local_104 + 0x1c)) + local_f4;
    local_a8 = CONCAT22((short)((uint)iVar10 >> 0x10),
                        (short)*(int *)(local_104 + 0x1c) - (short)(DAT_006c5c80 << 4));
    local_e4 = local_c4;
    if (*(int *)(local_104 + 0x174) == DAT_00644520) {
      local_e4 = (int)(short)local_c4;
      if (local_e4 <= DAT_005e18e8) {
        local_e4 = DAT_005e18e8;
      }
    }
    local_d4 = 0;
    if (*(int *)(local_104 + 0xe4) == 0) {
      iVar10 = FUN_00466870();
      if (iVar10 != 0) {
        iVar10 = FUN_00466870();
        if ((*(int *)(iVar10 + 0x5e8) != -1) &&
           (*(int *)(iVar10 + 0x570 + *(int *)(iVar10 + 0x5e8) * 4) == 0x34)) goto LAB_004fa39c;
      }
    }
    else {
LAB_004fa39c:
      iVar10 = local_104;
      if ((DAT_006903e8 == 3) && (iVar7 = FUN_0057ddb0(local_d8 + iVar5,&DAT_00690438), iVar7 != 0))
      {
        local_e4 = local_e4 + 0x20;
        local_d4 = 1;
        *(undefined4 *)(iVar10 + 0x74) = 1;
      }
      else {
        iVar10 = FUN_0057dcf0(local_d8 + iVar5,param_1 + 0x91);
        if (iVar10 == 0) {
          if (*(int *)(param_1 + 0xbd) != 0) {
            uVar9 = *(int *)(param_1 + 0x91) -
                    (((*(uint *)(local_e0 + 4) >> 1) - *(int *)(local_e0 + 0xc)) + local_dc + iVar4)
            ;
            uVar14 = (int)uVar9 >> 0x1f;
            local_fc = (uVar9 ^ uVar14) - uVar14;
            uVar9 = *(int *)(param_1 + 0x95) -
                    (((*(uint *)(local_e0 + 8) >> 2) - *(int *)(local_e0 + 0x10)) + local_d8 + iVar5
                    );
            uVar14 = (int)uVar9 >> 0x1f;
            uVar14 = (uVar9 ^ uVar14) - uVar14;
            local_c8 = uVar14;
            fVar16 = (float10)FUN_0059d60c((double)local_fc,(double)(int)uVar14);
            local_a0 = (double)fVar16;
            fVar16 = (float10)FUN_0059d60c((double)*(int *)(param_1 + 0xb5),
                                           (double)*(int *)(param_1 + 0xb9));
            if ((float10)local_a0 < fVar16) {
              *(uint *)(param_1 + 0xb9) = uVar14;
              *(int *)(param_1 + 0xb5) = local_fc;
              local_d4 = 1;
            }
          }
        }
        else {
          local_d4 = 1;
          *(undefined4 *)(param_1 + 0xbd) = 0;
        }
      }
    }
    iVar10 = local_104;
    FUN_0051fa60(local_dc + iVar4,local_d8 + iVar5);
    local_b8 = (uint)(short)local_e4;
    if (((*(int *)(iVar10 + 0xddf) < 0) && (-0x6f < (short)local_e4)) &&
       (local_b8 = *(int *)(iVar10 + 0xddf) + local_b8, (int)local_b8 < -0x6e)) {
      local_b8 = 0xffffff91;
    }
    if (((*(int *)(iVar10 + 0x798) != 0) && (*(int *)(iVar10 + 0xa0) != 1)) &&
       (iVar7 = FUN_0051fef0(), iVar7 != 3)) {
      uVar9 = FUN_0054e200();
      uVar9 = (uVar9 & 0x7fffffff) % 10;
      if (uVar9 < 4) {
        iVar5 = iVar5 - uVar9;
      }
    }
    local_c8 = *(uint *)(iVar10 + 0xcff);
    if (local_c8 == 0) {
      local_c8 = 0xffff8ad0;
    }
    local_fc = *(int *)(*(int *)(iVar10 + 0xcf7) + 600 + *(int *)(iVar10 + 0xbf3) * 4) + local_b8;
    if (local_fc < -0x7e) {
LAB_004fa5a3:
      local_fc = -0x7f;
    }
    else if (local_fc < 0x7f) {
      if (local_fc < -0x7e) goto LAB_004fa5a3;
    }
    else {
      local_fc = 0x7f;
    }
    uVar17 = FUN_004ffd30(local_e0,local_108,local_10c,local_100,local_dc + iVar4,local_d8 + iVar5,
                          *(int *)(param_1 + 0x9d) + local_110 + 6 + local_ac,local_fc,
                          *(undefined2 *)(local_104 + 0xcfb),
                          *(undefined4 *)(*(int *)(iVar10 + 0xcf7) + *(int *)(iVar10 + 0xbf3) * 4),
                          local_a8,0,local_c8,0);
    local_a0 = (double)CONCAT44(local_a0._4_4_,uVar17);
    if (*(int *)(local_104 + 0x821) != 0) {
      local_b8 = (uint)(short)local_e4;
    }
    local_fc = *(int *)(local_104 + 0xcf7);
    iVar10 = *(int *)(local_fc + 0x78 + *(int *)(local_104 + 0xbf3) * 4);
    if (iVar10 != -1) {
      local_c8 = *(uint *)(local_104 + 0xcff);
      if (local_c8 == 0) {
        local_c8 = 0xffff8ad0;
      }
      iVar7 = local_b8 + *(int *)(local_fc + 600 + *(int *)(local_104 + 0xbf3) * 4);
      if (iVar7 < -0x7e) {
LAB_004fa697:
        iVar7 = -0x7f;
      }
      else if (iVar7 < 0x7f) {
        if (iVar7 < -0x7e) goto LAB_004fa697;
      }
      else {
        iVar7 = 0x7f;
      }
      FUN_004ffd30(local_e0,local_108,local_10c,local_100,local_dc + iVar4,local_d8 + iVar5,
                   *(int *)(param_1 + 0x9d) + local_110 + 7 + local_ac,iVar7,
                   *(undefined2 *)(local_104 + 0xcfb),iVar10,local_a8,0,local_c8,0);
    }
    local_fc = *(int *)(local_104 + 0xcf7);
    iVar10 = *(int *)(local_fc + 0xf0 + *(int *)(local_104 + 0xbf3) * 4);
    if (iVar10 != -1) {
      local_c8 = *(uint *)(local_104 + 0xcff);
      if (local_c8 == 0) {
        local_c8 = 0xffff8ad0;
      }
      iVar7 = local_b8 + *(int *)(local_fc + 600 + *(int *)(local_104 + 0xbf3) * 4);
      if (iVar7 < -0x7e) {
LAB_004fa75d:
        iVar7 = -0x7f;
      }
      else if (iVar7 < 0x7f) {
        if (iVar7 < -0x7e) goto LAB_004fa75d;
      }
      else {
        iVar7 = 0x7f;
      }
      FUN_004ffd30(local_e0,local_108,local_10c,local_100,local_dc + iVar4,local_d8 + iVar5,
                   *(int *)(param_1 + 0x9d) + local_110 + 8 + local_ac,iVar7,
                   *(undefined2 *)(local_104 + 0xcfb),iVar10,local_a8,0,local_c8,0);
    }
    iVar10 = *(int *)(*(int *)(local_104 + 0xcf7) + 0x168 + *(int *)(local_104 + 0xbf3) * 4);
    if (iVar10 != -1) {
      FUN_004ffd30(local_e0,local_108,local_10c,local_100,local_dc + iVar4,
                   local_bc + local_d8 + iVar5,*(int *)(param_1 + 0x9d) + local_110 + 3 + local_ac,
                   local_e4,0,iVar10,local_a8,0,0xffff8ad0,0);
    }
    if ((*(int *)(local_104 + 0x778) != 0) && (DAT_006899c9 == 0)) {
      local_fc = 0;
      do {
        FUN_0051faa0(local_fc,&local_74);
        if (local_70 == -1) break;
        if ((*(int *)(local_104 + 0xa8) == 0xd) || (*(int *)(local_104 + 0x8c) != 0)) {
          iVar10 = 3;
        }
        else if (local_fc == 0) {
          iVar10 = 5;
        }
        else {
          iVar10 = 4 - (uint)(local_fc != 1);
        }
        local_c8 = *(uint *)(local_104 + 0xcff);
        if (local_c8 == 0) {
          local_c8 = 0xffff8ad0;
        }
        iVar7 = *(int *)(*(int *)(local_104 + 0xcf7) + 600 + *(int *)(local_104 + 0xbf3) * 4) +
                local_b8;
        if (iVar7 < -0x7e) {
LAB_004fa904:
          iVar7 = -0x7f;
        }
        else if (iVar7 < 0x7f) {
          if (iVar7 < -0x7e) goto LAB_004fa904;
        }
        else {
          iVar7 = 0x7f;
        }
        FUN_004ffd30(local_74,local_108,local_10c,local_100,local_70,local_6c,
                     *(int *)(param_1 + 0x9d) + local_fc + local_110 + 6 + local_ac,iVar7,
                     *(undefined2 *)(local_104 + 0xcfb),iVar10,local_a8,0,local_c8,0);
        local_fc = local_fc + 1;
      } while (local_fc < 3);
    }
    if ((DAT_006903e8 != 3) && (local_d4 != 0)) {
      DAT_00657d40 = local_a0._0_4_;
      *(undefined4 *)(param_1 + 0x1e9) =
           *(undefined4 *)(*(int *)(param_1 + 0x229) + local_a0._0_4_ * 0x24);
      DAT_00656600 = local_104;
    }
    FUN_00507250(&local_f0,&local_ec);
    if ((*(int *)(local_104 + 0xe4) != 0) && (*(int *)(local_104 + 0x124) != 0)) {
      piVar6 = *(int **)(*(int *)(local_104 + 0x148) + 0x110);
      local_c8 = *(uint *)(*(int *)(local_104 + 0x130) + 4);
      if (*(uint *)(*piVar6 + 0xc) < local_c8) {
        local_c8 = 0;
      }
      FUN_004ffd30(*(int *)(piVar6[2] + local_c8 * 4) + piVar6[3],local_108,local_10c,local_100,
                   local_f0 + iVar4,local_ec + iVar5,*(int *)(param_1 + 0x9d) + 7 + local_110,
                   local_e4 + 0x20,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
    }
    if (*(int *)(local_104 + 0xe4) != 0) {
      iVar10 = FUN_0051fef0();
      if (iVar10 == 1) {
        iVar10 = *(int *)(local_104 + 0x7d9);
        local_a0 = (double)CONCAT44(local_a0._4_4_,iVar10);
        piVar6 = *(int **)(*(int *)(local_104 + 0x7f1) + 0x110);
        uVar9 = *(uint *)(iVar10 + 4);
        if (*(uint *)(*piVar6 + 0xc) < uVar9) {
          uVar9 = 0;
        }
        local_10 = *(int *)(iVar10 + 8);
        FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                     local_10 + local_f0 + iVar4,*(int *)(iVar10 + 0xc) + local_ec + iVar5,
                     *(int *)(param_1 + 0x9d) + 8,local_e4 + 0x20,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      }
      if (*(int *)(local_104 + 0xe4) != 0) {
        if (*(int *)(local_104 + 0x889) != 0) {
          iVar10 = *(int *)(local_104 + 0x85d);
          local_a0 = (double)CONCAT44(local_a0._4_4_,iVar10);
          uVar9 = *(uint *)(iVar10 + 4);
          piVar6 = *(int **)(*(int *)(local_104 + 0x875) + 0x110);
          if (*(uint *)(*piVar6 + 0xc) < uVar9) {
            uVar9 = 0;
          }
          local_4c = *(int *)(iVar10 + 8);
          local_48 = *(int *)(iVar10 + 0xc);
          FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                       local_4c + local_f0 + iVar4,local_48 + local_ec + iVar5,
                       *(int *)(param_1 + 0x9d) + 9 + local_110,local_e4 + 0x20,0,0x1f,0xffff8ad0,0,
                       0xffff8ad0,0);
        }
        if ((*(int *)(local_104 + 0xe4) != 0) && (*(int *)(local_104 + 0x8d1) != 0)) {
          piVar6 = *(int **)(DAT_006df168 + 0x110);
          uVar9 = *(uint *)(DAT_006df150 + 4);
          if (*(uint *)(*piVar6 + 0xc) < uVar9) {
            uVar9 = 0;
          }
          local_64 = *(int *)(DAT_006df150 + 8);
          local_60 = *(int *)(DAT_006df150 + 0xc);
          FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                       local_64 + local_f0 + iVar4,local_60 + local_ec + iVar5,
                       *(int *)(param_1 + 0x9d) + 10 + local_110,local_e4 + 0x20,0,0x1f,0xffff8ad0,0
                       ,0xffff8ad0,0);
        }
      }
    }
    if (*(int *)(local_104 + 0x8bd) != 0) {
      iVar10 = *(int *)(local_104 + 0x891);
      local_a0 = (double)CONCAT44(local_a0._4_4_,iVar10);
      piVar6 = *(int **)(*(int *)(local_104 + 0x8a9) + 0x110);
      uVar9 = *(uint *)(iVar10 + 4);
      if (*(uint *)(*piVar6 + 0xc) < uVar9) {
        uVar9 = 0;
      }
      local_1c = *(int *)(iVar10 + 8);
      local_18 = *(int *)(iVar10 + 0xc);
      FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                   local_1c + local_f0 + iVar4,local_18 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 0xb + local_110,local_e4 + 0x20,0,0x1f,0xffff8ad0,0,
                   0xffff8ad0,0);
    }
    if (*(int *)(local_104 + 0xe4) != 0) {
      if (*(int *)(local_104 + 0x780) != 0) {
        local_c8 = *(uint *)(DAT_006df060 + 4);
        piVar6 = *(int **)(DAT_006df078 + 0x110);
        if (*(uint *)(*piVar6 + 0xc) < local_c8) {
          local_c8 = 0;
        }
        FUN_004ffd30(*(int *)(piVar6[2] + local_c8 * 4) + piVar6[3],local_108,local_10c,local_100,
                     local_f0 + iVar4,local_ec + iVar5,*(int *)(param_1 + 0x9d) + 0xc + local_110,
                     local_e4 + 0x20,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      }
      if (*(int *)(local_104 + 0xe4) != 0) {
        if (*(int *)(local_104 + 0x8d5) != 0) {
          piVar6 = *(int **)(DAT_006df0a8 + 0x110);
          uVar9 = *(uint *)(DAT_006df090 + 4);
          if (*(uint *)(*piVar6 + 0xc) < uVar9) {
            uVar9 = 0;
          }
          local_34 = *(int *)(DAT_006df090 + 8);
          local_30 = *(int *)(DAT_006df090 + 0xc);
          FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                       local_34 + local_f0 + iVar4,local_30 + local_ec + iVar5,
                       *(int *)(param_1 + 0x9d) + 0xd + local_110,local_e4 + 0x20,0,2,0xffff8ad0,0,
                       0xffff8ad0,0);
        }
        if (*(int *)(local_104 + 0xe4) != 0) {
          if (*(int *)(local_104 + 0x963) != 0) {
            piVar6 = *(int **)(DAT_006df108 + 0x110);
            uVar9 = *(uint *)(DAT_006df0f0 + 4);
            if (*(uint *)(*piVar6 + 0xc) < uVar9) {
              uVar9 = 0;
            }
            local_58 = *(int *)(DAT_006df0f0 + 8);
            local_54 = *(int *)(DAT_006df0f0 + 0xc);
            FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                         local_58 + local_f0 + iVar4,local_54 + local_ec + iVar5,
                         *(int *)(param_1 + 0x9d) + 0xe + local_110,local_e4 + 0x20,0,2,0xffff8ad0,0
                         ,0xffff8ad0,0);
          }
          if (*(int *)(local_104 + 0xe4) != 0) {
            if (*(char *)(local_104 + 0x962) != '\0') {
              piVar6 = *(int **)(DAT_006df0d8 + 0x110);
              uVar9 = *(uint *)(DAT_006df0c0 + 4);
              if (*(uint *)(*piVar6 + 0xc) < uVar9) {
                uVar9 = 0;
              }
              local_40 = *(int *)(DAT_006df0c0 + 8);
              local_3c = *(int *)(DAT_006df0c0 + 0xc);
              FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100
                           ,local_40 + local_f0 + iVar4,local_3c + local_ec + iVar5,
                           *(int *)(param_1 + 0x9d) + 0xf + local_110,local_e4 + 0x20,0,0x1f,
                           0xffff8ad0,0,0xffff8ad0,0);
            }
            if (*(int *)(local_104 + 0xe4) != 0) {
              if (*(int *)(local_104 + 0x78c) != 0) {
                piVar6 = *(int **)(DAT_006df138 + 0x110);
                uVar9 = *(uint *)(DAT_006df120 + 4);
                if (*(uint *)(*piVar6 + 0xc) < uVar9) {
                  uVar9 = 0;
                }
                local_28 = *(int *)(DAT_006df120 + 8);
                local_24 = *(int *)(DAT_006df120 + 0xc);
                FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,
                             local_100,local_28 + local_f0 + iVar4,local_24 + local_ec + iVar5,
                             *(int *)(param_1 + 0x9d) + 0x10 + local_110,local_e4 + 0x20,0,0x1f,
                             0xffff8ad0,0,0xffff8ad0,0);
              }
              if ((*(int *)(local_104 + 0xe4) != 0) && (iVar10 = FUN_0051fef0(), iVar10 == 3)) {
                piVar6 = *(int **)(*(int *)(local_104 + 0x7f1) + 0x110);
                uVar9 = *(uint *)(*(int *)(local_104 + 0x7d9) + 4);
                if (*(uint *)(*piVar6 + 0xc) < uVar9) {
                  uVar9 = 0;
                }
                local_80 = *(int *)(piVar6[2] + uVar9 * 4) + piVar6[3];
                local_7c = *(int *)(*(int *)(local_104 + 0x7d9) + 8);
                local_78 = *(int *)(*(int *)(local_104 + 0x7d9) + 0xc);
                local_94 = local_f0;
                local_e8 = local_ec;
                if (*(int *)(*(int *)(local_104 + 0xac) + 8) == 2) {
                  FUN_005072c0(&local_94,&local_e8);
                }
                else {
                  local_e8 = (*(uint *)(local_80 + 8) >> 1) + local_ec;
                }
                FUN_004ffd30(local_80,local_108,local_10c,local_100,local_7c + local_94 + iVar4,
                             local_78 + local_e8 + iVar5,*(int *)(param_1 + 0x9d) + 5 + local_110,
                             local_e4 + 0x20,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
              }
            }
          }
        }
      }
    }
    if ((((((*(int *)(local_104 + 0x68) == 0) && (*(int *)(local_104 + 0x70) == 0)) &&
          (iVar10 = FUN_00506220(), iVar10 == 0)) && (DAT_005e18f4 <= (short)local_e4)) ||
        (DAT_006e1f0c != 0)) && (*(int *)(local_104 + 0x5fc) != 3)) {
      if (*(int *)(local_104 + 0x174) == -1) {
        puVar8 = (undefined4 *)0x0;
      }
      else {
        puVar8 = &DAT_00643a80 + *(int *)(local_104 + 0x174) * 0x55;
      }
      uVar9 = puVar8[0x54] + 0x1a;
      if (*(uint *)(DAT_006593e0 + 0xc) < uVar9) {
        uVar9 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + uVar9 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_f0 + iVar4,local_ec + iVar5,*(int *)(param_1 + 0x9d) + 0xb + local_110,
                   local_e4 + 0x20,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
    }
    if (*(int *)(*(int *)(local_104 + 0xac) + 8) != 2) {
      iVar7 = local_cc + *(int *)(local_104 + 0xc) * -0x20 + *(int *)(local_104 + 0x18);
      local_a0 = (double)CONCAT44(local_a0._4_4_,*(int *)(local_104 + 8) * 0x20);
      iVar10 = *(int *)(local_104 + 0x14) + *(int *)(local_104 + 8) * -0x20 + local_d0;
      switch(*(undefined4 *)(param_1 + 0x31)) {
      case 0:
        goto switchD_004fb38c_caseD_0;
      case 1:
        goto switchD_004fb38c_caseD_1;
      case 2:
        goto switchD_004fb38c_caseD_2;
      case 3:
        goto switchD_004fb38c_caseD_3;
      }
      goto switchD_004fb38c_default;
    }
    iVar7 = local_cc + *(int *)(local_104 + 0xc) * -0x20 + 0x10 + *(int *)(local_104 + 0x18);
    iVar10 = *(int *)(local_104 + 0x14) + *(int *)(local_104 + 8) * -0x20 + 0x10 + local_d0;
    switch(*(undefined4 *)(param_1 + 0x31)) {
    case 0:
switchD_004fb38c_caseD_0:
      local_f8 = iVar10 - iVar7;
      iVar10 = iVar10 + iVar7;
      break;
    case 1:
switchD_004fb38c_caseD_1:
      local_f8 = iVar10 + iVar7;
      iVar10 = iVar7 - iVar10;
      break;
    case 2:
switchD_004fb38c_caseD_2:
      local_f8 = iVar7 - iVar10;
      local_f4 = -((iVar10 + iVar7) / 2);
      goto switchD_004fb38c_default;
    case 3:
switchD_004fb38c_caseD_3:
      local_f8 = -(iVar10 + iVar7);
      iVar10 = iVar10 - iVar7;
      break;
    default:
      goto switchD_004fb38c_default;
    }
    local_f4 = iVar10 / 2;
switchD_004fb38c_default:
    FUN_005072c0(&local_f0,&local_ec);
    if ((*(int *)(local_104 + 0xe4) != 0) && (*(int *)(local_104 + 0x744) != 0)) {
      if ((*(int *)(local_104 + 0xa8) == 0xf) || (*(int *)(local_104 + 0xa8) == 1)) {
        piVar6 = *(int **)(DAT_006df018 + 0x110);
        uVar9 = *(uint *)(DAT_006df000 + 4);
        if (*(uint *)(*piVar6 + 0xc) < uVar9) goto LAB_004fb494;
      }
      else {
        uVar9 = *(uint *)(DAT_006df030 + 4);
        piVar6 = *(int **)(DAT_006df048 + 0x110);
        if (*(uint *)(*piVar6 + 0xc) < uVar9) {
LAB_004fb494:
          uVar9 = 0;
        }
      }
      FUN_004ffd30(*(int *)(piVar6[2] + uVar9 * 4) + piVar6[3],local_108,local_10c,local_100,
                   local_f0 + iVar4,local_ec + iVar5,*(int *)(param_1 + 0x9d) + 8 + local_110,
                   local_e4 + 0x20,0,2,0xffff8ad0,0,0xffff8ad0,0);
    }
    if ((*(int *)(local_104 + 0xe4) != 0) && (*(char *)(local_104 + 0x7cc) != '\0')) {
      FUN_00520540();
      local_a0 = (double)CONCAT44(local_a0._4_4_,local_e4 + 0x20);
      FUN_004ffd30(local_8c,local_108,local_10c,local_100,local_88 + local_f0 + iVar4,
                   local_84 + local_ec + iVar5,*(int *)(param_1 + 0x9d) + 5 + local_110,
                   local_e4 + 0x20,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      FUN_004ffd30(local_8c,local_108,local_10c,local_100,local_88 + local_f0 + iVar4,
                   local_bc + local_84 + local_ec + iVar5,*(int *)(param_1 + 0x9d) + 5 + local_110,
                   local_a0._0_4_,0,1,0xffff8ad0,0,0xffff8ad0,0);
    }
    if (*(int *)(local_104 + 0x6e4) != 0) {
      local_d4 = *(uint *)(local_104 + 0x608);
      local_fc = (int)&DAT_006a5f80 + *(int *)(local_104 + 0xa8) * 0x5c9;
      FUN_00505aa0(&local_b4,&local_a4,local_d4,0);
      iVar10 = 4;
      if (*(uint *)(DAT_006593e0 + 0xc) < 4) {
        iVar10 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + iVar10 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_b4 + local_f0 + iVar4,local_a4 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      FUN_00505aa0(&local_b4,&local_a4,local_d4,1);
      iVar10 = 5;
      if (*(uint *)(DAT_006593e0 + 0xc) < 5) {
        iVar10 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + iVar10 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_b4 + local_f0 + iVar4,local_a4 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      FUN_00505aa0(&local_b4,&local_a4,local_d4,2);
      iVar10 = 6;
      if (*(uint *)(DAT_006593e0 + 0xc) < 6) {
        iVar10 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + iVar10 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_b4 + local_f0 + iVar4,local_a4 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      FUN_00505aa0(&local_b4,&local_a4,local_d4,3);
      iVar10 = 7;
      if (*(uint *)(DAT_006593e0 + 0xc) < 7) {
        iVar10 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + iVar10 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_b4 + local_f0 + iVar4,local_a4 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
    }
    if ((*(int *)(local_104 + 0x68) != 0) || (*(int *)(local_104 + 0x70) != 0)) {
      local_d4 = *(uint *)(local_104 + 0x608);
      local_bc = *(uint *)(local_104 + 0x8e9);
      if (DAT_00644520 == -1) {
LAB_004fb943:
        local_b8 = 0xc;
      }
      else {
        if (*(int *)(local_104 + 0x174) != DAT_00644520) {
          if (DAT_00644520 != -1) {
            if (*(int *)(local_104 + 0x174) == -1) {
              puVar8 = (undefined4 *)0x0;
            }
            else {
              puVar8 = &DAT_00643a80 + *(int *)(local_104 + 0x174) * 0x55;
            }
            local_b8 = 0x22;
            if (*(char *)((int)puVar8 + DAT_00644520 + 0x18) != '\0') goto LAB_004fb94b;
          }
          goto LAB_004fb943;
        }
        local_b8 = (*(int *)(*(int *)(local_104 + 0xac) + 8) != 2) - 1 & 8;
      }
LAB_004fb94b:
      local_fc = (int)&DAT_006a5f80 + *(int *)(local_104 + 0xa8) * 0x5c9;
      FUN_00505aa0(&local_c0,&local_b0,local_d4,0);
      uVar9 = local_b8;
      if (*(uint *)(DAT_006593e0 + 0xc) < local_b8) {
        uVar9 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + uVar9 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_c0 + local_f0 + iVar4,local_b0 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      FUN_00505aa0(&local_c0,&local_b0,local_d4,1);
      uVar9 = local_b8 + 1;
      if (*(uint *)(DAT_006593e0 + 0xc) < uVar9) {
        uVar9 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + uVar9 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_c0 + local_f0 + iVar4,local_b0 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      if (local_bc != 0xffffffff) {
        uVar9 = local_bc + 0x10;
        if (*(uint *)(DAT_006593e0 + 0xc) < uVar9) {
          uVar9 = 0;
        }
        FUN_004ffd30(*(int *)(DAT_006593e8 + uVar9 * 4) + DAT_006593ec,local_108,local_10c,local_100
                     ,local_c0 + local_f0 + iVar4,local_b0 + local_ec + iVar5,
                     *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0)
        ;
      }
      *(int *)(param_1 + 0xf5 + *(int *)(param_1 + 0x1e5) * 4) = local_104 + 0x6b4;
      FUN_0058f7d0(local_c0 + local_f0 + 2 + iVar4,local_b0 + local_ec + iVar5);
      *(int *)(param_1 + 0x1e5) = *(int *)(param_1 + 0x1e5) + 1;
      FUN_00505aa0(&local_c0,&local_b0,local_d4,2);
      uVar9 = local_b8 + 2;
      if (*(uint *)(DAT_006593e0 + 0xc) < uVar9) {
        uVar9 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + uVar9 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_c0 + local_f0 + iVar4,local_b0 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
      FUN_00505aa0(&local_c0,&local_b0,local_d4,3);
      uVar9 = local_b8 + 3;
      if (*(uint *)(DAT_006593e0 + 0xc) < uVar9) {
        uVar9 = 0;
      }
      FUN_004ffd30(*(int *)(DAT_006593e8 + uVar9 * 4) + DAT_006593ec,local_108,local_10c,local_100,
                   local_c0 + local_f0 + iVar4,local_b0 + local_ec + iVar5,
                   *(int *)(param_1 + 0x9d) + 5000 + local_110,0,0,0x1f,0xffff8ad0,0,0xffff8ad0,0);
    }
LAB_004f9b13:
    if ((*(byte *)(*(int *)(param_1 + 0x79) + 0xb) & 0x20) == 0) {
      return;
    }
    local_104 = FUN_0052b0c0();
    if (local_90 == 0) {
      local_ac = local_ac + 1;
    }
    else {
      local_ac = -2;
      local_90 = 0;
    }
    if (local_104 == 0) {
      return;
    }
    if (*(int *)(param_1 + 0x6d) != 1) {
      return;
    }
    if (*(uint *)(param_1 + 0x65) < (*(byte *)(*(int *)(param_1 + 0x79) + 8) & 3)) {
      return;
    }
  } while( true );
LAB_004f9514:
  FUN_0049bd20();
  goto LAB_004f9269;
}

