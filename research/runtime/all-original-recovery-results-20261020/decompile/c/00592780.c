
void __thiscall
FUN_00592780(int param_1,int param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5)

{
  char cVar1;
  char cVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int *piVar5;
  int iVar6;
  undefined4 uVar7;
  int iVar8;
  char *pcVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  undefined4 *puVar13;
  char *pcVar14;
  char *pcVar15;
  undefined4 *puVar16;
  undefined4 *puVar17;
  undefined4 local_128;
  undefined4 local_124;
  undefined4 local_120;
  int local_11c;
  undefined1 local_118 [24];
  char local_100 [256];
  
  puVar13 = (undefined4 *)0x0;
  local_124 = 0;
  local_128 = 0;
  local_120 = 0;
  puVar4 = (undefined4 *)FUN_0052a640();
  piVar5 = &DAT_006e980c;
  while (*piVar5 != 0) {
    piVar5 = piVar5 + 1;
    if (0x6e981b < (int)piVar5) {
      return;
    }
  }
  local_11c = FUN_00505b40();
  if (local_11c != -1) {
    iVar6 = FUN_00505b80();
    (&DAT_006e980c)[iVar6] = 1;
    if (param_2 != -1) {
      puVar13 = &DAT_00643a80 + param_2 * 0x55;
    }
    FUN_00465490(*puVar4);
    iVar6 = (param_1 + -0x65b250) / 0x93a;
    if ((iVar6 < 0) || (0x4f < iVar6)) {
      iVar6 = -1;
    }
    puVar13[4] = iVar6;
    puVar13[5] = *(undefined4 *)(param_1 + 0x84);
    uVar11 = 0xffffffff;
    pcVar14 = (char *)(param_1 + 0xc);
    do {
      pcVar9 = pcVar14;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar14 + 1;
      cVar1 = *pcVar14;
      pcVar14 = pcVar9;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar14 = pcVar9 + -uVar11;
    pcVar9 = (char *)(puVar13 + 0x14);
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar14;
      pcVar14 = pcVar14 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar14;
      pcVar14 = pcVar14 + 1;
      pcVar9 = pcVar9 + 1;
    }
    local_124 = FUN_0040e650(param_3);
    local_128 = FUN_004132e0(param_4);
    iVar6 = local_11c;
    puVar13 = &local_120;
    puVar17 = &local_128;
    local_120 = param_5;
    puVar16 = &local_124;
    uVar3 = *puVar4;
    uVar7 = FUN_00491280(local_118);
    iVar8 = FUN_004f5230(uVar3,uVar7,puVar16,puVar17,puVar13);
    if (iVar8 != 0) {
      FUN_005070e0(local_124,local_128,local_120,iVar6);
      cVar1 = DAT_006dbe9d;
      uVar11 = 0xffffffff;
      pcVar14 = s_creatures__005f1dc0;
      do {
        pcVar9 = pcVar14;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar9 = pcVar14 + 1;
        cVar2 = *pcVar14;
        pcVar14 = pcVar9;
      } while (cVar2 != '\0');
      uVar11 = ~uVar11;
      pcVar14 = pcVar9 + -uVar11;
      pcVar9 = (char *)&DAT_006f9de0;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar14;
        pcVar14 = pcVar14 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar9 = *pcVar14;
        pcVar14 = pcVar14 + 1;
        pcVar9 = pcVar9 + 1;
      }
      if (cVar1 != '\0') {
        uVar11 = 0xffffffff;
        pcVar14 = s_16Mb_Anims__005f1db4;
        do {
          pcVar9 = pcVar14;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pcVar9 = pcVar14 + 1;
          cVar1 = *pcVar14;
          pcVar14 = pcVar9;
        } while (cVar1 != '\0');
        uVar11 = ~uVar11;
        iVar6 = -1;
        pcVar14 = (char *)&DAT_006f9de0;
        do {
          pcVar15 = pcVar14;
          if (iVar6 == 0) break;
          iVar6 = iVar6 + -1;
          pcVar15 = pcVar14 + 1;
          cVar1 = *pcVar14;
          pcVar14 = pcVar15;
        } while (cVar1 != '\0');
        pcVar14 = pcVar9 + -uVar11;
        pcVar9 = pcVar15 + -1;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar9 = *(undefined4 *)pcVar14;
          pcVar14 = pcVar14 + 4;
          pcVar9 = pcVar9 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar9 = *pcVar14;
          pcVar14 = pcVar14 + 1;
          pcVar9 = pcVar9 + 1;
        }
      }
      uVar11 = 0xffffffff;
      pcVar14 = (char *)(param_1 + 0x104);
      do {
        pcVar9 = pcVar14;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar9 = pcVar14 + 1;
        cVar1 = *pcVar14;
        pcVar14 = pcVar9;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      iVar6 = -1;
      pcVar14 = (char *)&DAT_006f9de0;
      do {
        pcVar15 = pcVar14;
        if (iVar6 == 0) break;
        iVar6 = iVar6 + -1;
        pcVar15 = pcVar14 + 1;
        cVar1 = *pcVar14;
        pcVar14 = pcVar15;
      } while (cVar1 != '\0');
      pcVar14 = pcVar9 + -uVar11;
      pcVar9 = pcVar15 + -1;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar14;
        pcVar14 = pcVar14 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar9 = *pcVar14;
        pcVar14 = pcVar14 + 1;
        pcVar9 = pcVar9 + 1;
      }
      uVar11 = 0xffffffff;
      pcVar14 = &DAT_005da060;
      do {
        pcVar9 = pcVar14;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar9 = pcVar14 + 1;
        cVar1 = *pcVar14;
        pcVar14 = pcVar9;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      iVar6 = -1;
      pcVar14 = (char *)&DAT_006f9de0;
      do {
        pcVar15 = pcVar14;
        if (iVar6 == 0) break;
        iVar6 = iVar6 + -1;
        pcVar15 = pcVar14 + 1;
        cVar1 = *pcVar14;
        pcVar14 = pcVar15;
      } while (cVar1 != '\0');
      pcVar14 = pcVar9 + -uVar11;
      pcVar9 = pcVar15 + -1;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar14;
        pcVar14 = pcVar14 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar9 = *pcVar14;
        pcVar14 = pcVar14 + 1;
        pcVar9 = pcVar9 + 1;
      }
      iVar6 = *(int *)(param_1 + 0x54);
      if ((iVar6 != 0) && (DAT_006e1f0c == 0)) {
        *(int *)(param_1 + 0x40) =
             (((*(int *)(&DAT_006e1fac + DAT_00689924 * 4) * iVar6) / 100) *
             *(int *)(param_1 + 0x40)) / 100 + *(int *)(param_1 + 0x40);
        *(int *)(param_1 + 0x3c) =
             (((*(int *)(&DAT_006e1fbc + DAT_00689924 * 4) * iVar6) / 100) *
             *(int *)(param_1 + 0x3c)) / 100 + *(int *)(param_1 + 0x3c);
        *(int *)(param_1 + 0x44) =
             (((*(int *)(&DAT_006e1fcc + DAT_00689924 * 4) * iVar6) / 100) *
             *(int *)(param_1 + 0x44)) / 100 + *(int *)(param_1 + 0x44);
        *(int *)(param_1 + 0x48) =
             (((*(int *)(&DAT_006e1fdc + DAT_00689924 * 4) * iVar6) / 100) *
             *(int *)(param_1 + 0x48)) / 100 + *(int *)(param_1 + 0x48);
        *(int *)(param_1 + 0x4c) =
             (((*(int *)(&DAT_006e1fec + DAT_00689924 * 4) * iVar6) / 100) *
             *(int *)(param_1 + 0x4c)) / 100 + *(int *)(param_1 + 0x4c);
        *(int *)(param_1 + 0x50) =
             (((*(int *)(&DAT_006e1ffc + DAT_00689924 * 4) * iVar6) / 100) *
             *(int *)(param_1 + 0x50)) / 100 + *(int *)(param_1 + 0x50);
      }
      *(undefined4 *)((int)puVar4 + 0x8c1) = *(undefined4 *)(param_1 + 0x40);
      FUN_00506290(local_11c,param_2,0);
      FUN_00507c20(*(undefined4 *)(param_1 + 0x3c));
      FUN_00507f00(*(int *)(param_1 + 0x3c) << 8);
      puVar4[0x39] = *(undefined4 *)(param_1 + 0x40);
      *(undefined4 *)((int)puVar4 + 0x8f1) = *(undefined4 *)(param_1 + 0x44);
      *(undefined4 *)((int)puVar4 + 0x8ed) = *(undefined4 *)(param_1 + 0x50);
      puVar4[0x22] = *(undefined4 *)(param_1 + 0x4c);
      puVar4[0x21] = *(undefined4 *)(param_1 + 0x48);
      FUN_00509bc0();
      *(undefined4 *)((int)puVar4 + 0x92e) = 1;
      FUN_005244f0();
      if ((DAT_00689920 == 5) && (param_2 == DAT_00644520)) {
        if (param_2 == -1) {
          puVar13 = (undefined4 *)0x0;
        }
        else {
          puVar13 = &DAT_00643a80 + param_2 * 0x55;
        }
        if (DAT_006dbe9d == '\0') {
          pcVar14 = *(char **)(&DAT_005c78c4 + *(int *)(&DAT_0065b34c + puVar13[4] * 0x93a) * 4);
        }
        else {
          pcVar14 = (&PTR_s_creatures_wizard6_ani_005c78dc)
                    [*(int *)(&DAT_0065b34c + puVar13[4] * 0x93a)];
        }
        uVar11 = 0xffffffff;
        do {
          pcVar9 = pcVar14;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pcVar9 = pcVar14 + 1;
          cVar1 = *pcVar14;
          pcVar14 = pcVar9;
        } while (cVar1 != '\0');
        uVar11 = ~uVar11;
        pcVar14 = pcVar9 + -uVar11;
        pcVar9 = local_100;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar9 = *(undefined4 *)pcVar14;
          pcVar14 = pcVar14 + 4;
          pcVar9 = pcVar9 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar9 = *pcVar14;
          pcVar14 = pcVar14 + 1;
          pcVar9 = pcVar9 + 1;
        }
        uVar11 = 0xffffffff;
        pcVar14 = local_100;
        do {
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          cVar1 = *pcVar14;
          pcVar14 = pcVar14 + 1;
        } while (cVar1 != '\0');
        pcVar14 = local_100;
        while ((local_100[0] != '\\' && (~uVar11 != 1 && -1 < (int)(~uVar11 - 1)))) {
          local_100[0] = pcVar14[1];
          pcVar14 = pcVar14 + 1;
        }
        pcVar14 = pcVar14 + 1;
        pcVar9 = _strchr(local_100,0x2e);
        if (pcVar9 != (char *)0x0) {
          *pcVar9 = '\0';
        }
      }
      else {
        pcVar14 = (char *)(param_1 + 0x104);
      }
      iVar6 = __strcmpi(pcVar14,s_wizard1_005f1b74);
      if ((iVar6 == 0) || (iVar6 = __strcmpi(pcVar14,s_wizard2_005f1dac), iVar6 == 0)) {
        *(undefined4 *)((int)puVar4 + 0x95e) = 0;
      }
      else {
        iVar6 = __strcmpi(pcVar14,s_wizard3_005f1b6c);
        if ((iVar6 == 0) || (iVar6 = __strcmpi(pcVar14,s_wizard4_005f1da4), iVar6 == 0)) {
          *(undefined4 *)((int)puVar4 + 0x95e) = 1;
        }
        else {
          iVar6 = __strcmpi(pcVar14,s_wizard5_005f1d9c);
          if ((iVar6 == 0) || (iVar6 = __strcmpi(pcVar14,s_wizard6_005f1b64), iVar6 == 0)) {
            *(undefined4 *)((int)puVar4 + 0x95e) = 2;
          }
          else {
            iVar6 = __strcmpi(pcVar14,s_greek_female_005f1b44);
            if ((iVar6 == 0) || (iVar6 = __strcmpi(pcVar14,s_Ariadne_005f1b34), iVar6 == 0)) {
              *(undefined4 *)((int)puVar4 + 0x95e) = 4;
            }
            else {
              *(undefined4 *)((int)puVar4 + 0x95e) = 3;
            }
          }
        }
      }
      iVar6 = 0;
      *(undefined4 *)((int)puVar4 + 0x936) = *(undefined4 *)(param_1 + 0x60);
      *(undefined4 *)((int)puVar4 + 0x93a) = *(undefined4 *)(param_1 + 100);
      *(undefined4 *)((int)puVar4 + 0x93e) = *(undefined4 *)(param_1 + 0x68);
      *(undefined4 *)((int)puVar4 + 0x942) = *(undefined4 *)(param_1 + 0x6c);
      *(undefined4 *)((int)puVar4 + 0x946) = *(undefined4 *)(param_1 + 0x70);
      *(undefined4 *)((int)puVar4 + 0x94a) = *(undefined4 *)(param_1 + 0x74);
      *(undefined4 *)((int)puVar4 + 0x94e) = *(undefined4 *)(param_1 + 0x78);
      *(undefined4 *)((int)puVar4 + 0x952) = *(undefined4 *)(param_1 + 0x7c);
      *(undefined4 *)((int)puVar4 + 0x956) = *(undefined4 *)(param_1 + 0x80);
      if (*(int *)(param_1 + 0x92a) != 0) {
        *(undefined4 *)(param_1 + 0x926) = 0;
        if (*(int *)(param_1 + 0x92a) < 1) {
          iVar8 = -1;
        }
        else {
          iVar8 = **(int **)(param_1 + 0x932);
          *(undefined4 *)(param_1 + 0x926) = 1;
        }
        do {
          FUN_00509b60(iVar8);
          if (*(int *)(param_1 + 0x92a) <= *(int *)(param_1 + 0x926)) break;
          iVar10 = *(int *)(param_1 + 0x926) + 1;
          iVar8 = *(int *)(*(int *)(param_1 + 0x932) + -4 + iVar10 * 4);
          *(int *)(param_1 + 0x926) = iVar10;
        } while (iVar8 != -1);
      }
      if (0 < *(int *)(param_1 + 0x53a)) {
        puVar13 = (undefined4 *)(param_1 + 0x156);
        do {
          FUN_005097b0(puVar13[-3],puVar13[-2],puVar13[-1],*puVar13,puVar13[1],0);
          iVar6 = iVar6 + 1;
          puVar13 = puVar13 + 6;
        } while (iVar6 < *(int *)(param_1 + 0x53a));
      }
      puVar13 = (undefined4 *)(param_1 + 0x58a);
      iVar6 = 10;
      do {
        if (puVar13[-0x13] != 0) {
          puVar13[1] = *puVar13;
          puVar13[2] = 0;
        }
        puVar13 = puVar13 + 0x19;
        iVar6 = iVar6 + -1;
      } while (iVar6 != 0);
    }
    *(undefined4 *)(param_1 + 4) = 1;
    *(int *)(param_1 + 0x88) = param_2;
  }
  return;
}

