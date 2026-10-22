
void __fastcall FUN_005606f0(undefined4 *param_1)

{
  ushort *puVar1;
  byte bVar2;
  undefined4 uVar3;
  byte *pbVar4;
  int iVar5;
  byte *pbVar6;
  int iVar7;
  int iVar8;
  uint uVar9;
  uint *puVar10;
  byte *pbVar11;
  uint uVar12;
  int *piVar13;
  undefined4 *puVar14;
  uint *puVar15;
  byte *pbVar16;
  bool bVar17;
  byte *local_30;
  int local_2c;
  int *local_28;
  uint local_24;
  int local_1c;
  uint local_18;
  int local_10;
  int local_8;
  
  local_28 = (int *)*param_1;
  local_1c = 0;
  local_8 = 0;
  uVar9 = DAT_006c5494;
  if (0 < (int)param_1[2]) {
LAB_00560728:
    if (*local_28 != 0) {
      local_2c = 0;
LAB_0056073e:
      switch(local_2c) {
      case 0:
        local_30 = (byte *)(local_28 + 0x22);
        break;
      case 1:
        local_30 = (byte *)(local_28 + 0x39);
        break;
      case 2:
      case 3:
        local_30 = &DAT_005fc874;
        switch(local_28[1]) {
        case 1:
        case 2:
          local_30 = (byte *)(local_28 + 0xb3);
          break;
        case 3:
          local_30 = (byte *)(local_28 + 0xb3);
          break;
        case 4:
          local_30 = (byte *)(local_28 + 0xb4);
          break;
        case 5:
        case 6:
          local_30 = (byte *)(local_28 + 0xb3);
          break;
        case 8:
          local_30 = (byte *)(local_28 + 0xf6);
        }
        break;
      case 4:
        local_30 = &DAT_005fc874;
        if (local_28[1] == 1) {
          local_30 = (byte *)(local_28 + 0xbf);
        }
        else if (local_28[1] == 3) {
          local_30 = (byte *)(local_28 + 0xc6);
        }
        break;
      case 5:
        local_30 = &DAT_005fc874;
        if (local_28[1] == 1) {
          local_30 = (byte *)(local_28 + 0xd7);
        }
      }
      pbVar16 = &DAT_005fc874;
      pbVar4 = local_30;
      do {
        bVar2 = *pbVar4;
        bVar17 = bVar2 < *pbVar16;
        if (bVar2 != *pbVar16) {
LAB_0056082c:
          iVar5 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
          goto LAB_00560831;
        }
        if (bVar2 == 0) break;
        bVar2 = pbVar4[1];
        bVar17 = bVar2 < pbVar16[1];
        if (bVar2 != pbVar16[1]) goto LAB_0056082c;
        pbVar4 = pbVar4 + 2;
        pbVar16 = pbVar16 + 2;
      } while (bVar2 != 0);
      iVar5 = 0;
LAB_00560831:
      pbVar4 = DAT_006f3804;
      if (iVar5 != 0) {
        DAT_006f3800 = 0;
        DAT_006f3804 = DAT_006f37f8;
        pbVar4 = DAT_006f3804;
        do {
          do {
            DAT_006f3804 = pbVar4;
            pbVar4 = DAT_006f3804;
            if (DAT_006f37fc <= DAT_006f3800) {
LAB_005608c9:
              DAT_006f3804 = pbVar4;
              iVar5 = 0;
              pbVar16 = DAT_006f37f8;
              pbVar4 = DAT_006f3804;
              if (0 < DAT_006f37fc) goto LAB_005608db;
              goto LAB_0056093a;
            }
            pbVar4 = DAT_006f3804 + 0x40;
            DAT_006f3800 = DAT_006f3800 + 1;
            if (DAT_006f3804 == (byte *)0x0) goto LAB_005608c9;
          } while (*(int *)DAT_006f3804 == 0);
          pbVar16 = DAT_006f3804 + 4;
          pbVar6 = local_30;
          do {
            bVar2 = *pbVar6;
            bVar17 = bVar2 < *pbVar16;
            if (bVar2 != *pbVar16) {
LAB_005608af:
              iVar5 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
              goto LAB_005608b4;
            }
            if (bVar2 == 0) break;
            bVar2 = pbVar6[1];
            bVar17 = bVar2 < pbVar16[1];
            if (bVar2 != pbVar16[1]) goto LAB_005608af;
            pbVar6 = pbVar6 + 2;
            pbVar16 = pbVar16 + 2;
          } while (bVar2 != 0);
          iVar5 = 0;
LAB_005608b4:
          uVar9 = DAT_006c5494;
        } while (iVar5 != 0);
        if (local_28[1] == 3) {
          pbVar16 = DAT_006f3804 + 0x38;
          DAT_006f3804 = pbVar4;
          *(int *)pbVar16 = local_28[0xbb];
          uVar9 = DAT_006c5494;
          pbVar4 = DAT_006f3804;
        }
      }
      goto LAB_0056093a;
    }
    goto LAB_00560db3;
  }
LAB_00560ddb:
  DAT_006f3800 = 0;
  DAT_006f3804 = DAT_006f37f8;
  pbVar4 = DAT_006f3804;
  while (DAT_006f3804 = pbVar4, pbVar16 = DAT_006f3804, pbVar4 = DAT_006f3804,
        DAT_006f3800 < DAT_006f37fc) {
    pbVar4 = DAT_006f3804 + 0x40;
    DAT_006f3800 = DAT_006f3800 + 1;
    if (DAT_006f3804 == (byte *)0x0) break;
    if (((*(int *)DAT_006f3804 != 0) &&
        (pbVar6 = DAT_006f3804 + 4, DAT_006f3804 = pbVar4, iVar5 = FUN_00466900(pbVar6),
        pbVar4 = DAT_006f3804, iVar5 != 0)) &&
       (iVar5 = *(int *)(iVar5 + 0xc), *(int *)(pbVar16 + 0x28) = iVar5, pbVar4 = DAT_006f3804,
       iVar5 != -1)) {
      uVar3 = *(undefined4 *)(pbVar16 + 0x28);
      pbVar16[0x24] = 1;
      pbVar16[0x25] = 0;
      pbVar16[0x26] = 0;
      pbVar16[0x27] = 0;
      FUN_00564a40(uVar3);
      pbVar4 = DAT_006f3804;
    }
  }
  DAT_006f3804 = pbVar4;
  if (DAT_006e1f40 != 0) {
    FUN_00560eb0();
  }
  return;
  while( true ) {
    iVar5 = iVar5 + 1;
    pbVar16 = pbVar16 + 0x40;
    if (DAT_006f37fc <= iVar5) break;
LAB_005608db:
    if (*(int *)pbVar16 == 0) {
      if (pbVar16 != (byte *)0x0) {
        uVar9 = 0xffffffff;
        pbVar16[0] = 1;
        pbVar16[1] = 0;
        pbVar16[2] = 0;
        pbVar16[3] = 0;
        pbVar4 = local_30;
        goto code_r0x005608fd;
      }
      break;
    }
  }
  goto LAB_0056093a;
  while( true ) {
    uVar9 = uVar9 - 1;
    pbVar6 = pbVar4 + 1;
    bVar2 = *pbVar4;
    pbVar4 = pbVar6;
    if (bVar2 == 0) break;
code_r0x005608fd:
    pbVar6 = pbVar4;
    if (uVar9 == 0) break;
  }
  uVar9 = ~uVar9;
  pbVar6 = pbVar6 + -uVar9;
  pbVar4 = pbVar16;
  for (uVar12 = uVar9 >> 2; pbVar4 = pbVar4 + 4, uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pbVar4 = *(undefined4 *)pbVar6;
    pbVar6 = pbVar6 + 4;
  }
  for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
    *pbVar4 = *pbVar6;
    pbVar6 = pbVar6 + 1;
    pbVar4 = pbVar4 + 1;
  }
  pbVar16[0x24] = 3;
  pbVar16[0x25] = 0;
  pbVar16[0x26] = 0;
  pbVar16[0x27] = 0;
  pbVar16[0x28] = 0xff;
  pbVar16[0x29] = 0xff;
  pbVar16[0x2a] = 0xff;
  pbVar16[0x2b] = 0xff;
  uVar9 = DAT_006c5494;
  pbVar4 = DAT_006f3804;
  if (local_28[1] == 3) {
    *(int *)(pbVar16 + 0x38) = local_28[0xbb];
    uVar9 = DAT_006c5494;
    pbVar4 = DAT_006f3804;
  }
LAB_0056093a:
  DAT_006f3804 = pbVar4;
  local_2c = local_2c + 1;
  if (5 < local_2c) goto code_r0x0056094c;
  goto LAB_0056073e;
code_r0x0056094c:
  pbVar4 = (byte *)(local_28 + 0x61);
  pbVar16 = &DAT_005fc874;
  do {
    bVar2 = *pbVar4;
    bVar17 = bVar2 < *pbVar16;
    if (bVar2 != *pbVar16) {
LAB_0056097b:
      iVar5 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
      goto LAB_00560980;
    }
    if (bVar2 == 0) break;
    bVar2 = pbVar4[1];
    bVar17 = bVar2 < pbVar16[1];
    if (bVar2 != pbVar16[1]) goto LAB_0056097b;
    pbVar4 = pbVar4 + 2;
    pbVar16 = pbVar16 + 2;
  } while (bVar2 != 0);
  iVar5 = 0;
LAB_00560980:
  if (iVar5 != 0) {
    local_28[8] = local_1c;
    iVar5 = DAT_006cbb56;
    local_2c = 0;
    uVar9 = DAT_006c5494;
    if (0 < DAT_006cbb56) {
      local_10 = 0;
      do {
        if ((local_10 < 0) || (DAT_006cbb56 <= local_2c)) {
          puVar14 = (undefined4 *)0x0;
        }
        else {
          puVar14 = (undefined4 *)(local_10 + DAT_006cbb52);
        }
        pbVar4 = (byte *)(puVar14 + 6);
        pbVar16 = (byte *)(local_28 + 0x61);
        do {
          bVar2 = *pbVar4;
          bVar17 = bVar2 < *pbVar16;
          if (bVar2 != *pbVar16) {
LAB_00560a04:
            iVar7 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
            goto LAB_00560a09;
          }
          if (bVar2 == 0) break;
          bVar2 = pbVar4[1];
          bVar17 = bVar2 < pbVar16[1];
          if (bVar2 != pbVar16[1]) goto LAB_00560a04;
          pbVar4 = pbVar4 + 2;
          pbVar16 = pbVar16 + 2;
        } while (bVar2 != 0);
        iVar7 = 0;
LAB_00560a09:
        if (iVar7 == 0) {
          piVar13 = &DAT_006cb8c2 + puVar14[2];
          local_18 = puVar14[2];
          do {
            local_24 = FUN_004132e0(puVar14[1]);
            do {
              iVar7 = FUN_0040e650(*puVar14);
              puVar15 = param_1 + local_1c + 10;
              do {
                if (local_1c < 100) {
                  local_1c = local_1c + 1;
                  *puVar15 = (uint)*(ushort *)
                                    (DAT_006c54dc +
                                    ((&DAT_006cb942)[local_24] + *piVar13 + iVar7) * 0xc);
                  puVar15 = puVar15 + 1;
                }
                iVar8 = FUN_0040e650(iVar7 + 1);
                bVar17 = iVar7 != puVar14[3];
                iVar7 = iVar8;
              } while (bVar17);
              uVar9 = local_24 + 1;
              if ((int)uVar9 < 0) {
                do {
                  uVar9 = uVar9 + DAT_006c5498;
                } while ((int)uVar9 < 0);
              }
              else {
                for (; DAT_006c5498 <= uVar9; uVar9 = uVar9 - DAT_006c5498) {
                }
              }
              bVar17 = local_24 != puVar14[4];
              local_24 = uVar9;
            } while (bVar17);
            piVar13 = piVar13 + 1;
            bVar17 = local_18 != puVar14[5];
            local_18 = local_18 + 1;
          } while (bVar17);
        }
        local_2c = local_2c + 1;
        local_10 = local_10 + 0x48;
        uVar9 = DAT_006c5494;
      } while (local_2c < iVar5);
    }
  }
  if ((local_28[0x9f] != 0) || ((local_28[1] == 2 && (local_28[0x11c] != 0)))) {
    param_1[8] = 1;
    uVar9 = DAT_006c5494;
  }
  iVar5 = DAT_006cbb56;
  pbVar16 = &DAT_005fc874;
  pbVar4 = (byte *)(local_28 + 0x75);
  do {
    bVar2 = *pbVar4;
    bVar17 = bVar2 < *pbVar16;
    if (bVar2 != *pbVar16) {
LAB_00560b69:
      iVar7 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
      goto LAB_00560b6e;
    }
    if (bVar2 == 0) break;
    bVar2 = pbVar4[1];
    bVar17 = bVar2 < pbVar16[1];
    if (bVar2 != pbVar16[1]) goto LAB_00560b69;
    pbVar4 = pbVar4 + 2;
    pbVar16 = pbVar16 + 2;
  } while (bVar2 != 0);
  iVar7 = 0;
LAB_00560b6e:
  if ((iVar7 != 0) && (local_2c = 0, 0 < DAT_006cbb56)) {
    local_10 = 0;
    do {
      if ((local_10 < 0) || (DAT_006cbb56 <= local_2c)) {
        puVar15 = (uint *)0x0;
      }
      else {
        puVar15 = (uint *)(local_10 + DAT_006cbb52);
      }
      puVar10 = puVar15 + 6;
      pbVar4 = (byte *)(local_28 + 0x75);
      do {
        bVar2 = (byte)*puVar10;
        bVar17 = bVar2 < *pbVar4;
        if (bVar2 != *pbVar4) {
LAB_00560bdc:
          iVar7 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
          goto LAB_00560be1;
        }
        if (bVar2 == 0) break;
        bVar2 = *(byte *)((int)puVar10 + 1);
        bVar17 = bVar2 < pbVar4[1];
        if (bVar2 != pbVar4[1]) goto LAB_00560bdc;
        puVar10 = (uint *)((int)puVar10 + 2);
        pbVar4 = pbVar4 + 2;
      } while (bVar2 != 0);
      iVar7 = 0;
LAB_00560be1:
      if (iVar7 == 0) {
        piVar13 = &DAT_006cb8c2 + puVar15[2];
        local_18 = puVar15[2];
        do {
          local_24 = puVar15[1];
          if ((int)local_24 < 0) {
            do {
              local_24 = local_24 + DAT_006c5498;
            } while ((int)local_24 < 0);
          }
          else {
            for (; DAT_006c5498 <= local_24; local_24 = local_24 - DAT_006c5498) {
            }
          }
          do {
            uVar12 = *puVar15;
            if ((int)uVar12 < 0) {
              do {
                uVar12 = uVar12 + uVar9;
              } while ((int)uVar12 < 0);
            }
            else {
              for (; uVar9 <= uVar12; uVar12 = uVar12 - uVar9) {
              }
            }
            do {
              puVar1 = (ushort *)
                       (DAT_006c54dc + 8 + ((&DAT_006cb942)[local_24] + *piVar13 + uVar12) * 0xc);
              *puVar1 = *puVar1 | 0x4000;
              uVar9 = uVar12 + 1;
              if ((int)uVar9 < 0) {
                do {
                  uVar9 = uVar9 + DAT_006c5494;
                } while ((int)uVar9 < 0);
              }
              else {
                for (; DAT_006c5494 <= uVar9; uVar9 = uVar9 - DAT_006c5494) {
                }
              }
              bVar17 = uVar12 != puVar15[3];
              uVar12 = uVar9;
            } while (bVar17);
            uVar12 = local_24 + 1;
            if ((int)uVar12 < 0) {
              do {
                uVar12 = uVar12 + DAT_006c5498;
              } while ((int)uVar12 < 0);
            }
            else {
              for (; DAT_006c5498 <= uVar12; uVar12 = uVar12 - DAT_006c5498) {
              }
            }
            bVar17 = local_24 != puVar15[4];
            uVar9 = DAT_006c5494;
            local_24 = uVar12;
          } while (bVar17);
          piVar13 = piVar13 + 1;
          bVar17 = local_18 != puVar15[5];
          local_18 = local_18 + 1;
        } while (bVar17);
      }
      local_2c = local_2c + 1;
      local_10 = local_10 + 0x48;
    } while (local_2c < iVar5);
  }
  if (((local_28[1] == 3) && (local_28[0xf2] != 0)) && (iVar5 = 0, 0 < DAT_006f37fc)) {
    pbVar4 = DAT_006f37f8 + 4;
    pbVar16 = DAT_006f37f8;
    do {
      if (*(int *)pbVar16 != 0) {
        pbVar6 = (byte *)(local_28 + 0xb3);
        pbVar11 = pbVar4;
        do {
          bVar2 = *pbVar11;
          bVar17 = bVar2 < *pbVar6;
          if (bVar2 != *pbVar6) {
LAB_00560d5e:
            iVar7 = (1 - (uint)bVar17) - (uint)(bVar17 != 0);
            goto LAB_00560d63;
          }
          if (bVar2 == 0) break;
          bVar2 = pbVar11[1];
          bVar17 = bVar2 < pbVar6[1];
          if (bVar2 != pbVar6[1]) goto LAB_00560d5e;
          pbVar11 = pbVar11 + 2;
          pbVar6 = pbVar6 + 2;
        } while (bVar2 != 0);
        iVar7 = 0;
LAB_00560d63:
        uVar9 = DAT_006c5494;
        if (iVar7 == 0) {
          if (((iVar5 < DAT_006f37fc) && (pbVar16 != (byte *)0x0)) &&
             (iVar5 = FUN_00466900(pbVar16 + 4), uVar9 = DAT_006c5494, iVar5 != 0)) {
            FUN_00465650(DAT_00644520,1);
            uVar9 = DAT_006c5494;
          }
          break;
        }
      }
      iVar5 = iVar5 + 1;
      pbVar16 = pbVar16 + 0x40;
      pbVar4 = pbVar4 + 0x40;
    } while (iVar5 < DAT_006f37fc);
  }
LAB_00560db3:
  local_28 = local_28 + 0x11f;
  local_8 = local_8 + 1;
  if ((int)param_1[2] <= local_8) goto LAB_00560ddb;
  goto LAB_00560728;
}

