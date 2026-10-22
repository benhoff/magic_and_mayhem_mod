
/* WARNING: Removing unreachable block (ram,0x004e7543) */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_004e4270(void)

{
  char cVar1;
  bool bVar2;
  DWORD DVar3;
  int iVar4;
  char *pcVar5;
  char *pcVar6;
  undefined4 uVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  uint uVar12;
  uint uVar13;
  int iVar14;
  char *pcVar15;
  undefined4 *puVar16;
  int *piVar17;
  int *piVar18;
  float10 fVar19;
  undefined8 local_520;
  char local_514 [256];
  char local_414 [256];
  char local_314 [4];
  char local_310 [4];
  char local_30c [4];
  char local_308 [2];
  char local_306;
  undefined4 local_305 [60];
  undefined4 local_214;
  undefined4 local_210;
  undefined4 local_20c;
  undefined2 local_208;
  undefined4 local_206 [60];
  char local_114 [21];
  undefined4 local_ff;
  void *local_14;
  undefined1 *puStack_10;
  undefined4 local_c;
  
  local_c = 0xffffffff;
  puStack_10 = &LAB_005c21de;
  local_14 = ExceptionList;
  local_314 = (char  [4])s_CDRomDrive_cfg_005e3e28._0_4_;
  local_310 = (char  [4])s_CDRomDrive_cfg_005e3e28._4_4_;
  local_308 = (char  [2])s_CDRomDrive_cfg_005e3e28._12_2_;
  local_306 = s_CDRomDrive_cfg_005e3e28[0xe];
  local_30c = (char  [4])s_CDRomDrive_cfg_005e3e28._8_4_;
  puVar16 = local_305;
  for (iVar11 = 0x3c; iVar11 != 0; iVar11 = iVar11 + -1) {
    *puVar16 = 0;
    puVar16 = puVar16 + 1;
  }
  *(undefined1 *)puVar16 = 0;
  pcVar5 = s_cfg_Install_Type_cfg_005e3e10;
  pcVar6 = local_114;
  for (iVar11 = 5; iVar11 != 0; iVar11 = iVar11 + -1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  *pcVar6 = *pcVar5;
  puVar16 = &local_ff;
  for (iVar11 = 0x3a; iVar11 != 0; iVar11 = iVar11 + -1) {
    *puVar16 = 0;
    puVar16 = puVar16 + 1;
  }
  local_214 = DAT_005e3e00;
  local_20c = DAT_005e3e08;
  *(undefined2 *)puVar16 = 0;
  *(undefined1 *)((int)puVar16 + 2) = 0;
  local_210 = DAT_005e3e04;
  local_208 = DAT_005e3e0c;
  puVar16 = local_206;
  for (iVar11 = 0x3c; iVar11 != 0; iVar11 = iVar11 + -1) {
    *puVar16 = 0;
    puVar16 = puVar16 + 1;
  }
  *(undefined2 *)puVar16 = 0;
  ExceptionList = &local_14;
  GetCurrentDirectoryA(0x100,&DAT_006e2988);
  uVar12 = 0xffffffff;
  pcVar5 = &DAT_006e2988;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = (char *)&DAT_006e2780;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  uVar12 = 0xffffffff;
  pcVar5 = (char *)&DAT_006e2780;
  do {
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  if (((~uVar12 != 1) && ((&DAT_006e277e)[~uVar12] != '\\')) && ((&DAT_006e277e)[~uVar12] != '/')) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005d9058;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    iVar11 = -1;
    pcVar5 = (char *)&DAT_006e2780;
    do {
      pcVar15 = pcVar5;
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      pcVar15 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar15;
    } while (cVar1 != '\0');
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = pcVar15 + -1;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
  }
  uVar12 = 0xffffffff;
  pcVar5 = local_314;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  iVar11 = -1;
  pcVar5 = (char *)&DAT_006e2780;
  do {
    pcVar15 = pcVar5;
    if (iVar11 == 0) break;
    iVar11 = iVar11 + -1;
    pcVar15 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar15;
  } while (cVar1 != '\0');
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = pcVar15 + -1;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_CDROM_Drive_Letter_With_Colon_005e3dd0,s_CDROMDrive_005e3df0,&DAT_005e3dfc,
                     &DAT_005e12e4,0x14,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005e12e4;
    do {
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar5 + 1;
    } while (cVar1 != '\0');
    if (((~uVar12 != 1) && (cVar1 = *(char *)((int)&DAT_005e12e0 + ~uVar12 + 2), cVar1 != '\\')) &&
       (cVar1 != '/')) {
      uVar12 = 0xffffffff;
      pcVar5 = &DAT_005d9058;
      do {
        pcVar6 = pcVar5;
        if (uVar12 == 0) break;
        uVar12 = uVar12 - 1;
        pcVar6 = pcVar5 + 1;
        cVar1 = *pcVar5;
        pcVar5 = pcVar6;
      } while (cVar1 != '\0');
      uVar12 = ~uVar12;
      iVar11 = -1;
      pcVar5 = &DAT_005e12e4;
      do {
        pcVar15 = pcVar5;
        if (iVar11 == 0) break;
        iVar11 = iVar11 + -1;
        pcVar15 = pcVar5 + 1;
        cVar1 = *pcVar5;
        pcVar5 = pcVar15;
      } while (cVar1 != '\0');
      pcVar5 = pcVar6 + -uVar12;
      pcVar6 = pcVar15 + -1;
      for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
        *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
        pcVar5 = pcVar5 + 4;
        pcVar6 = pcVar6 + 4;
      }
      for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
        *pcVar6 = *pcVar5;
        pcVar5 = pcVar5 + 1;
        pcVar6 = pcVar6 + 1;
      }
    }
  }
  DVar3 = timeGetTime();
  FUN_0054e170(DVar3);
  uVar12 = 0xffffffff;
  pcVar5 = &DAT_006e2988;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = (char *)&DAT_006e2780;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  uVar12 = 0xffffffff;
  pcVar5 = (char *)&DAT_006e2780;
  do {
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  if (((~uVar12 != 1) && ((&DAT_006e277e)[~uVar12] != '\\')) && ((&DAT_006e277e)[~uVar12] != '/')) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005d9058;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    iVar11 = -1;
    pcVar5 = (char *)&DAT_006e2780;
    do {
      pcVar15 = pcVar5;
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      pcVar15 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar15;
    } while (cVar1 != '\0');
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = pcVar15 + -1;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
  }
  uVar12 = 0xffffffff;
  pcVar5 = local_114;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  iVar11 = -1;
  pcVar5 = (char *)&DAT_006e2780;
  do {
    pcVar15 = pcVar5;
    if (iVar11 == 0) break;
    iVar11 = iVar11 + -1;
    pcVar15 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar15;
  } while (cVar1 != '\0');
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = pcVar15 + -1;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_Installation_Type_005e3dac,&DAT_005e3dc0,&DAT_005e3dc8,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e3dc8);
    if (iVar11 == 0) {
      DAT_006e1ed0 = 0;
    }
    else {
      iVar11 = __strcmpi(&DAT_006e2a88,s_Standard_005e3da0);
      DAT_006e1ed0 = (iVar11 != 0) + 1;
    }
  }
  uVar12 = 0xffffffff;
  pcVar5 = &DAT_006e2988;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = (char *)&DAT_006e2780;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  uVar12 = 0xffffffff;
  pcVar5 = (char *)&DAT_006e2780;
  do {
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  if (((~uVar12 != 1) && ((&DAT_006e277e)[~uVar12] != '\\')) && ((&DAT_006e277e)[~uVar12] != '/')) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005d9058;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    iVar11 = -1;
    pcVar5 = (char *)&DAT_006e2780;
    do {
      pcVar15 = pcVar5;
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      pcVar15 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar15;
    } while (cVar1 != '\0');
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = pcVar15 + -1;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
  }
  uVar12 = 0xffffffff;
  pcVar5 = (char *)&local_214;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  iVar11 = -1;
  pcVar5 = (char *)&DAT_006e2780;
  do {
    pcVar15 = pcVar5;
    if (iVar11 == 0) break;
    iVar11 = iVar11 + -1;
    pcVar15 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar15;
  } while (cVar1 != '\0');
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = pcVar15 + -1;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UseWinCursor_005e3d90,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    DAT_00642c64 = (uint)(iVar11 != 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_SameThreadCursor_005e3d6c,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) &&
     ((iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0 || (DAT_006a46fc != 0)))) {
    _DAT_006e1edc = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_LightFadeUp_005e3d60,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    _DAT_006e201c = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AmbientLight_005e3d50,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -0x7e) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x7f)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -0x7e) {
        iVar11 = -0x7f;
      }
      else {
        iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      iVar11 = 0x7f;
    }
    DAT_005e18e8 = -iVar11;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_LightRamp_005e3d44,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -0x7e) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x7f)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -0x7e) {
        DAT_005e18ec = 0xffffff81;
      }
      else {
        DAT_005e18ec = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e18ec = 0x7f;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_IndicatorLightLevel_005e3d30,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -0x7e) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -0x7e) {
        DAT_005e18f4 = 0xffffff81;
      }
      else {
        DAT_005e18f4 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e18f4 = 0;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_ObjectLightLevel_005e3d1c,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -0x7e) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -0x7e) {
        DAT_005e18f8 = 0xffffff81;
      }
      else {
        DAT_005e18f8 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e18f8 = 0;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_NoLightSources_005e3d0c,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_005e1404 = 0;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AllCreaturesLit_005e3cfc,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    _DAT_00689494 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AllowUnitTracking_005e3ce8,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,s_FALSE_005e3ce0), iVar11 == 0)) {
    _DAT_005e1788 = 0;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AlignDrift_005e3cd4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10000)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        _DAT_005e1750 = 1;
      }
      else {
        _DAT_005e1750 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      _DAT_005e1750 = 10000;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_autounlock_005e3cc8,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    DAT_006e1f1c = (uint)(iVar11 == 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_lightwrite_005e3cb4,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    _DAT_005e1400 = (uint)(iVar11 == 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_SpoolMapGenInfo_005e3ca4,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1ee8 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_ShowInfoBar_005e3c98,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    _DAT_006de0d6 = (uint)(iVar11 == 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_ShowVersionAtStartup_005e3c80,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    _DAT_006e1ed8 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_g_showMapSeed_005e3c70,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1ef4 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_ShowFrameRate_005e3c60,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1ee0 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_PageFlipping_005e3c50,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    _DAT_005e1408 = (uint)(iVar11 == 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_UseMallocedBackBuffer_005e3c38,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    DAT_006e1f78 = (uint)(iVar11 == 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_ShowSpriteLog_005e3c28,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    DAT_006e1f24 = (uint)(iVar11 == 0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_UseCSpritePrinter_005e3c14,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    if (iVar11 == 0) {
      DAT_006e1f38 = 1;
    }
    iVar11 = __strcmpi(&DAT_006e2a88,s_FALSE_005e3ce0);
    if (iVar11 == 0) {
      DAT_006e1f38 = 0;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_SkipFrameEvery_005e3c04,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_006e1ef8 = 0;
        DAT_006e1efc = DAT_006e1ef8;
      }
      else {
        DAT_006e1ef8 = FUN_0059c5a4(&DAT_006e2a88);
        DAT_006e1efc = DAT_006e1ef8;
      }
    }
    else {
      DAT_006e1ef8 = 10;
      DAT_006e1efc = DAT_006e1ef8;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_SkipXFrames_005e3bf8,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar4 = FUN_0059c5a4(&DAT_006e2a88);
    iVar11 = DAT_006e1efc;
    if (DAT_006e1efc < iVar4) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    }
    if (iVar11 < 0xb) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (DAT_006e1efc < iVar11) {
        DAT_005e1418 = FUN_0059c5a4(&DAT_006e2a88);
      }
      else {
        DAT_005e1418 = DAT_006e1efc;
      }
    }
    else {
      DAT_005e1418 = 0xb;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_MaxSkipXFrames_005e3be8,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005e1414 = 0;
      }
      else {
        DAT_005e1414 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1414 = 0x10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_ActiveAI_005e3bdc,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,s_FALSE_005e3ce0), iVar11 == 0)) {
    DAT_005e1410 = 0;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_CreatureAnimChecks_005e3bc8,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e9808 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_SeeAllGrimoire_005e3bb8,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1f20 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DEBUG_005e3cc0,s_ValidateScripts_005e3ba8,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1f40 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_LightCurve_005e3b9c,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000)) &&
       (iVar11 = FUN_0059c5a4(&DAT_006e2a88), 1 < iVar11)) {
      FUN_0059c5a4(&DAT_006e2a88);
    }
    FUN_00582210();
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_ColourFactor_005e3b8c,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000)) &&
       (iVar11 = FUN_0059c5a4(&DAT_006e2a88), 1 < iVar11)) {
      FUN_0059c5a4(&DAT_006e2a88);
    }
    FUN_00582220();
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_LightPower_005e3b80,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    fVar19 = (float10)FUN_0059d4be(&DAT_006e2a88);
    local_520 = (double)fVar19;
    if (fVar19 <= (float10)_DAT_005c5380) {
      local_520 = 0.0;
    }
    else if (_DAT_005c6fd0 <= local_520) {
      local_520 = 1000.0;
    }
    FUN_00582230((undefined4)local_520,local_520._4_4_);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_ColourPower_005e3b74,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    fVar19 = (float10)FUN_0059d4be(&DAT_006e2a88);
    local_520 = (double)fVar19;
    if (fVar19 <= (float10)_DAT_005c5380) {
      local_520 = 0.0;
    }
    else if (_DAT_005c6fd0 <= local_520) {
      local_520 = 1000.0;
    }
    FUN_00582250((undefined4)local_520,local_520._4_4_);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UWLightCurve_005e3b64,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000)) &&
       (iVar11 = FUN_0059c5a4(&DAT_006e2a88), 1 < iVar11)) {
      FUN_0059c5a4(&DAT_006e2a88);
    }
    FUN_00582270();
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UWColourFactor_005e3b54,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000)) &&
       (iVar11 = FUN_0059c5a4(&DAT_006e2a88), 1 < iVar11)) {
      FUN_0059c5a4(&DAT_006e2a88);
    }
    FUN_00582280();
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UWLightPower_005e3b44,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    fVar19 = (float10)FUN_0059d4be(&DAT_006e2a88);
    local_520 = (double)fVar19;
    if (fVar19 <= (float10)_DAT_005c5380) {
      local_520 = 0.0;
    }
    else if (_DAT_005c6fd0 <= local_520) {
      local_520 = 1000.0;
    }
    FUN_00582290((undefined4)local_520,local_520._4_4_);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UWColourPower_005e3b34,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    fVar19 = (float10)FUN_0059d4be(&DAT_006e2a88);
    local_520 = (double)fVar19;
    if (fVar19 <= (float10)_DAT_005c5380) {
      local_520 = 0.0;
    }
    else if (_DAT_005c6fd0 <= local_520) {
      local_520 = 1000.0;
    }
    FUN_005822b0((undefined4)local_520,local_520._4_4_);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_TestLineOfSight_005e3b24,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    _DAT_006e1ef0 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_CreatureDieTime_005e3b14,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (iVar11 < 2) {
      DAT_005e4e6c = 1;
    }
    else {
      DAT_005e4e6c = FUN_0059c5a4(&DAT_006e2a88);
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UWLightGradient_005e3b04,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x14)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        _DAT_00657fe4 = 0;
      }
      else {
        _DAT_00657fe4 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      _DAT_00657fe4 = 0x14;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AllowLawChaos_005e3af4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    _DAT_006e1eec = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_RateOfDrowning_005e3ae4,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (iVar11 < 1) {
      DAT_005e1748 = 0;
    }
    else {
      DAT_005e1748 = FUN_0059c5a4(&DAT_006e2a88);
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_MaxNodesForRouteFinding_005e3acc,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (iVar11 < 1) {
      DAT_005e174c = 0;
    }
    else {
      DAT_005e174c = FUN_0059c5a4(&DAT_006e2a88);
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_timesystem_005e3abc,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,s_NOTIONAL_TIME_005e3aac);
    if (iVar11 == 0) {
      DAT_006e1f64 = 0;
    }
    else {
      iVar11 = __strcmpi(&DAT_006e2a88,s_REAL_TIME_005e3aa0);
      if (iVar11 == 0) {
        DAT_006e1f64 = 1;
      }
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_timelimit_005e3a90,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (iVar11 < 1) {
      DAT_005e15f4 = 0;
    }
    else {
      DAT_005e15f4 = FUN_0059c5a4(&DAT_006e2a88);
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_interrupt1_005e3a80,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    fVar19 = (float10)FUN_0059d4be(&DAT_006e2a88);
    _DAT_005d8654 = (float)fVar19;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_interrupt2_005e3a70,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    fVar19 = (float10)FUN_0059d4be(&DAT_006e2a88);
    _DAT_005d8658 = (float)fVar19;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_div_g_limit_005e3a60,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005d865c = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_div_q_limit_005e3a50,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005d8660 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_AI_danger_excess_limit_005e3a38,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005faf1c = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_UseNodeMap_005e3a2c,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1f3c = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_COMBAT_MODIFIERS_005e3a10,s_Impetus_005e3a24,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 500)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        iVar11 = 0;
      }
      else {
        iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      iVar11 = 500;
    }
    DAT_005e177c = (iVar11 * 5) / 2;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_COMBAT_MODIFIERS_005e3a10,s_RelativeHeight_005e3a00,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 500)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        iVar11 = 0;
      }
      else {
        iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      iVar11 = 500;
    }
    DAT_005e1778 = (iVar11 * 5) / 2;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_COMBAT_MODIFIERS_005e3a10,s_RelativeFacing_005e39f0,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    piVar17 = &DAT_005e1758;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 == (char *)0x0) break;
      pcVar6 = _strchr(pcVar5,0x3d);
      if (pcVar6 == (char *)0x0) {
        _strchr(pcVar5,0);
      }
      else {
        *pcVar6 = '\0';
      }
      FUN_0059c5a4(pcVar5);
      iVar11 = FUN_0059c5a4(pcVar5);
      if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(pcVar5), iVar11 < 500)) {
        iVar11 = FUN_0059c5a4(pcVar5);
        if (iVar11 < 1) {
          iVar11 = 0;
        }
        else {
          iVar11 = FUN_0059c5a4(pcVar5);
        }
      }
      else {
        iVar11 = 500;
      }
      piVar18 = piVar17 + 1;
      *piVar17 = (iVar11 * 5) / 2;
      piVar17 = piVar18;
    } while (piVar18 != &DAT_005e1778);
  }
  GetPrivateProfileStringA
            (s_GLOBAL_OPTIONS_005e3d80,s_SelectTolerence_005e39e0,&DAT_005faf28,&DAT_006e2a88,0x100,
             (LPCSTR)&DAT_006e2780);
  _DAT_006899f1 = FUN_0059c5a4(&DAT_006e2a88);
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_ManaGeneration_005e39d0,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1900 = 1;
      }
      else {
        DAT_005e1900 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1900 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_ManaRecoveryDelay_005e39b0,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1904 = 1;
      }
      else {
        DAT_005e1904 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1904 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_ManaRecovery_005e39a0,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1908 = 1;
      }
      else {
        DAT_005e1908 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1908 = 1000;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_HealthReduction_005e3990,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        _DAT_005e190c = 1;
      }
      else {
        _DAT_005e190c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      _DAT_005e190c = 1000;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_LordBooster_005e3984,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1910 = 1;
      }
      else {
        DAT_005e1910 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1910 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_KingBooster_005e3978,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1914 = 1;
      }
      else {
        DAT_005e1914 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1914 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_DamagePoints_005e3968,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        _DAT_005e1918 = 1;
      }
      else {
        _DAT_005e1918 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      _DAT_005e1918 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_WIZARD_LEVEL_005e3948,s_SPUpgradeLevel_005e3958,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    puVar16 = &DAT_006c4764;
    while( true ) {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 == (char *)0x0) break;
      pcVar6 = _strchr(pcVar5,0x3d);
      if (pcVar6 == (char *)0x0) {
        _strchr(pcVar5,0);
        uVar7 = FUN_0059c5a4(pcVar5);
        *puVar16 = uVar7;
        puVar16 = puVar16 + 1;
      }
      else {
        *pcVar6 = '\0';
        uVar7 = FUN_0059c5a4(pcVar5);
        *puVar16 = uVar7;
        puVar16 = puVar16 + 1;
      }
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_WIZARD_LEVEL_005e3948,s_MPUpgradeLevel_005e3938,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    puVar16 = &DAT_006e000c;
    while( true ) {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 == (char *)0x0) break;
      pcVar6 = _strchr(pcVar5,0x3d);
      if (pcVar6 == (char *)0x0) {
        _strchr(pcVar5,0);
        uVar7 = FUN_0059c5a4(pcVar5);
        *puVar16 = uVar7;
        puVar16 = puVar16 + 1;
      }
      else {
        *pcVar6 = '\0';
        uVar7 = FUN_0059c5a4(pcVar5);
        *puVar16 = uVar7;
        puVar16 = puVar16 + 1;
      }
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_DamagePoints_005e3968,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        _DAT_005e1918 = 1;
      }
      else {
        _DAT_005e1918 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      _DAT_005e1918 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MANA_SYSTEM_005e39c4,s_DamagePoints_005e3968,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        _DAT_005e1918 = 1;
      }
      else {
        _DAT_005e1918 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      _DAT_005e1918 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_BloodLevel_005e392c,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 200)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1420 = 1;
      }
      else {
        DAT_005e1420 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1420 = 200;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_PercentAsGore_005e390c,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1424 = 1;
      }
      else {
        DAT_005e1424 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1424 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_GoreTimeOnFloor_005e38fc,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x4b0)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1428 = 1;
      }
      else {
        DAT_005e1428 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1428 = 0x4b0;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_TexturedGore_005e38ec,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,s_FALSE_005e3ce0), iVar11 == 0)) {
    DAT_005e1430 = 0;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_TexturedGoreUpper_005e38d8,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -199) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 200)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -199) {
        iVar11 = -200;
      }
      else {
        iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      iVar11 = 200;
    }
    _DAT_005e1434 = (float)iVar11 * _DAT_005c53b4;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_TexturedGoreLower_005e38c4,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -199) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 200)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -199) {
        iVar11 = -200;
      }
      else {
        iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      iVar11 = 200;
    }
    _DAT_005e1438 = (float)iVar11 * _DAT_005c53b4;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,&DAT_005e38bc,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e142c = 1;
      }
      else {
        DAT_005e142c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e142c = 10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BLOOD_N_GORE_005e391c,s_Trail_005e38b4,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 5)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_006e1f5c = 0;
      }
      else {
        DAT_006e1f5c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_006e1f5c = 5;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PEST_FLY_005e38a0,s_MinDist_005e38ac,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x80)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e14b8 = 1;
      }
      else {
        DAT_005e14b8 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e14b8 = 0x80;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PEST_FLY_005e38a0,s_TopSpeed_005e3894,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x20)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e14bc = 1;
      }
      else {
        DAT_005e14bc = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e14bc = 0x20;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PEST_FLY_005e38a0,s_Random_005e388c,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  iVar11 = DAT_005e14c0;
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (iVar11 < 1) {
      iVar4 = 0;
    }
    else {
      iVar4 = FUN_0059c5a4(&DAT_006e2a88);
    }
    iVar11 = DAT_005e14bc / 2;
    if (iVar4 < DAT_005e14bc / 2) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        iVar11 = 0;
      }
      else {
        iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
  }
  DAT_005e14c0 = iVar11;
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_Colour_005e3884,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    puVar16 = &DAT_005e143c;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 != (char *)0x0) {
        pcVar6 = _strchr(pcVar5,0x3d);
        if (pcVar6 == (char *)0x0) {
          _strchr(pcVar5,0);
        }
        else {
          *pcVar6 = '\0';
        }
      }
      uVar7 = FUN_0059c5a4(pcVar5);
      *puVar16 = uVar7;
      puVar16 = puVar16 + 1;
    } while ((int)puVar16 < 0x5e1460);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_ParticlesPerFrame_005e385c,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005e1460 = 0;
      }
      else {
        DAT_005e1460 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1460 = 10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_Lives_005e3854,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005e1464 = 0;
      }
      else {
        DAT_005e1464 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1464 = 10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_Speed_005e384c,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x20)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1468 = 1;
      }
      else {
        DAT_005e1468 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1468 = 0x20;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_Delay_005e0504,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0xc)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e146c = 1;
      }
      else {
        DAT_005e146c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e146c = 0xc;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_Decay_005e3844,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -99) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -99) {
        DAT_005e1470 = 0xffffff9c;
      }
      else {
        DAT_005e1470 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1470 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_Boost_005e383c,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < -99) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < -99) {
        DAT_005e1474 = 0xffffff9c;
      }
      else {
        DAT_005e1474 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1474 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_IndirectSpellParticleLevel_005e3820,
                     &DAT_005faf28,&DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 200)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005e1478 = 0;
      }
      else {
        DAT_005e1478 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e1478 = 200;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPELL_CAST_EFFECT_005e3870,s_IndirectSpellParticlePeriod_005e3804,
                     &DAT_005faf28,&DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e147c = 1;
      }
      else {
        DAT_005e147c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e147c = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_EARTH_BIND_EFFECT_005e37f0,s_Colour_005e3884,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    puVar16 = &DAT_005e1480;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 != (char *)0x0) {
        pcVar6 = _strchr(pcVar5,0x3d);
        if (pcVar6 == (char *)0x0) {
          _strchr(pcVar5,0);
        }
        else {
          *pcVar6 = '\0';
        }
      }
      uVar7 = FUN_0059c5a4(pcVar5);
      *puVar16 = uVar7;
      puVar16 = puVar16 + 1;
    } while ((int)puVar16 < 0x5e1490);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_EARTH_BIND_EFFECT_005e37f0,&DAT_005e38bc,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    piVar17 = &DAT_005e1494;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 != (char *)0x0) {
        pcVar6 = _strchr(pcVar5,0x3d);
        if (pcVar6 == (char *)0x0) {
          _strchr(pcVar5,0);
        }
        else {
          *pcVar6 = '\0';
        }
      }
      iVar11 = FUN_0059c5a4(pcVar5);
      if (iVar11 < 1) {
        iVar11 = 1;
      }
      else if (5 < iVar11) {
        iVar11 = 5;
      }
      *piVar17 = iVar11;
      piVar17 = piVar17 + 1;
    } while ((int)piVar17 < 0x5e149c);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_EARTH_BIND_EFFECT_005e37f0,s_DensityMod_005e37e4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e1490 = 1;
      }
      else {
        DAT_005e1490 = FUN_0059c5a4(&DAT_006e2a88);
        if (DAT_005e1490 < 0xc9) {
          if (DAT_005e1490 < 0) {
            DAT_005e1490 = 0;
          }
          else if (200 < DAT_005e1490) goto LAB_004e6cff;
        }
        else {
LAB_004e6cff:
          DAT_005e1490 = 200;
        }
      }
    }
    else {
      DAT_005e1490 = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_LEVITATE_EFFECT_005e37d4,s_Colour_005e3884,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    puVar16 = &DAT_005e149c;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 != (char *)0x0) {
        pcVar6 = _strchr(pcVar5,0x3d);
        if (pcVar6 == (char *)0x0) {
          _strchr(pcVar5,0);
        }
        else {
          *pcVar6 = '\0';
        }
      }
      uVar7 = FUN_0059c5a4(pcVar5);
      *puVar16 = uVar7;
      puVar16 = puVar16 + 1;
    } while ((int)puVar16 < 0x5e14ac);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_LEVITATE_EFFECT_005e37d4,&DAT_005e38bc,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    piVar17 = &DAT_005e14b0;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 != (char *)0x0) {
        pcVar6 = _strchr(pcVar5,0x3d);
        if (pcVar6 == (char *)0x0) {
          _strchr(pcVar5,0);
        }
        else {
          *pcVar6 = '\0';
        }
      }
      iVar11 = FUN_0059c5a4(pcVar5);
      if (iVar11 < 1) {
        iVar11 = 1;
      }
      else if (5 < iVar11) {
        iVar11 = 5;
      }
      *piVar17 = iVar11;
      piVar17 = piVar17 + 1;
    } while ((int)piVar17 < 0x5e14b8);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_LEVITATE_EFFECT_005e37d4,s_DensityMod_005e37e4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e14ac = 1;
      }
      else {
        DAT_005e14ac = FUN_0059c5a4(&DAT_006e2a88);
        if (DAT_005e14ac < 0xc9) {
          if (DAT_005e14ac < 0) {
            DAT_005e14ac = 0;
          }
          else if (200 < DAT_005e14ac) goto LAB_004e6fb3;
        }
        else {
LAB_004e6fb3:
          DAT_005e14ac = 200;
        }
      }
    }
    else {
      DAT_005e14ac = 100;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SUBVERT_EFFECT_005e37b8,s_FailColour_005e37c8,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    bVar2 = true;
    puVar16 = &DAT_005e14c8;
    do {
      if (bVar2) {
        pcVar5 = local_514;
        bVar2 = false;
      }
      else {
        pcVar5 = (char *)0x0;
      }
      pcVar5 = (char *)FUN_0059c99c(pcVar5,local_414);
      if (pcVar5 != (char *)0x0) {
        pcVar6 = _strchr(pcVar5,0x3d);
        if (pcVar6 == (char *)0x0) {
          _strchr(pcVar5,0);
        }
        else {
          *pcVar6 = '\0';
        }
      }
      uVar7 = FUN_0059c5a4(pcVar5);
      *puVar16 = uVar7;
      puVar16 = puVar16 + 1;
    } while ((int)puVar16 < 0x5e14d4);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SUBVERT_EFFECT_005e37b8,s_FailParticleCount_005e37a4,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 100)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005e14c4 = 1;
      }
      else {
        DAT_005e14c4 = FUN_0059c5a4(&DAT_006e2a88);
        if (DAT_005e14c4 < 0x65) {
          if (DAT_005e14c4 < 0) {
            DAT_005e14c4 = 0;
            goto LAB_004e70c0;
          }
          if (DAT_005e14c4 < 0x65) goto LAB_004e70c0;
        }
        DAT_005e14c4 = 100;
      }
    }
    else {
      DAT_005e14c4 = 100;
    }
  }
LAB_004e70c0:
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_NetRandomLog_005e3794,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_006e1f30 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_NetworkLogging_005e377c,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    _DAT_006e1f34 = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_NetEffectsLog_005e376c,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_005fed7c = 1;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_NumRandomSpells_005e375c,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x19)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005e141c = 0;
      }
      else {
        DAT_005e141c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005e141c = 0x19;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_messageDelayFrames_LAN_005e3744,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 2) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 2) {
        DAT_005eab08 = 1;
      }
      else {
        DAT_005eab08 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005eab08 = 10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_messageDelayFrames_NET_005e372c,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005eab0c = 0;
      }
      else {
        DAT_005eab0c = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005eab0c = 10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_messageDelayFrames_SERIAL_005e3710,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 10)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_005eab10 = 0;
      }
      else {
        DAT_005eab10 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_005eab10 = 10;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_messageResendDelay_005e36fc,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 1000000)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        uVar12 = 0;
      }
      else {
        uVar12 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      uVar12 = 1000000;
    }
    DAT_005eab14 = uVar12 & ((int)uVar12 < 1) - 1;
    if ((int)DAT_005c72e0 <= (int)DAT_005eab14) {
      DAT_005eab14 = DAT_005c72e0;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_NETWORK_005e378c,s_validationMessages_005e36e8,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0);
    DAT_005eab18 = iVar11 == 0;
  }
  DVar3 = GetPrivateProfileStringA
                    (s_GLOBAL_OPTIONS_005e3d80,s_EffectRadius_005e36d8,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_006e2a88;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_514;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    uVar12 = 0xffffffff;
    pcVar5 = &DAT_005da820;
    do {
      pcVar6 = pcVar5;
      if (uVar12 == 0) break;
      uVar12 = uVar12 - 1;
      pcVar6 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar6;
    } while (cVar1 != '\0');
    uVar12 = ~uVar12;
    pcVar5 = pcVar6 + -uVar12;
    pcVar6 = local_414;
    for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
      *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar6 = pcVar6 + 4;
    }
    for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
      *pcVar6 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar6 = pcVar6 + 1;
    }
    pcVar5 = (char *)FUN_0059c99c(local_514,local_414);
    if (pcVar5 != (char *)0x0) {
      pcVar6 = _strchr(pcVar5,0x3d);
      if (pcVar6 == (char *)0x0) {
        _strchr(pcVar5,0);
      }
      else {
        *pcVar6 = '\0';
      }
    }
    iVar11 = FUN_0059c5a4(pcVar5);
    piVar17 = (int *)FUN_00597850(iVar11 * 0x10 + 4);
    local_c = 0;
    if (piVar17 == (int *)0x0) {
      piVar18 = (int *)0x0;
    }
    else {
      piVar18 = piVar17 + 1;
      *piVar17 = iVar11;
      FUN_0059c441(piVar18,0x10,iVar11,&LAB_004e8470,&LAB_0046f2e0);
    }
    local_c = 0xffffffff;
    DAT_006db79a = piVar18;
    if ((piVar18 != (int *)0x0) && (0 < iVar11)) {
      do {
        pcVar5 = (char *)FUN_0059c99c(0,local_414);
        if (pcVar5 != (char *)0x0) {
          pcVar6 = _strchr(pcVar5,0x3d);
          if (pcVar6 == (char *)0x0) {
            _strchr(pcVar5,0);
          }
          else {
            *pcVar6 = '\0';
          }
        }
        uVar7 = FUN_0059c5a4(pcVar5);
        FUN_00484150(uVar7);
        iVar11 = iVar11 + -1;
      } while (iVar11 != 0);
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_BATTLE_LOG_005e36bc,s_logBufferSize_005e36c8,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if (iVar11 < 1) {
      DAT_005e14d4 = 0;
    }
    else {
      DAT_005e14d4 = FUN_0059c5a4(&DAT_006e2a88);
    }
  }
  GetPrivateProfileStringA
            (s_BATTLE_LOG_005e36bc,s_logFileName_005e36b0,&DAT_005faf28,s_save_event_log_005e14d8,
             0x100,(LPCSTR)&DAT_006e2780);
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_HealthModify0_005e36a0,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fac = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ManaModify0_005e3688,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fbc = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ControlModify0_005e3678,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fcc = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_MagicResistModify0_005e3664,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fdc = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_IntelligenceModify0_005e3650,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fec = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_CombatModify0_005e3640,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1ffc = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_AIRandomness0_005e3630,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e200c = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_HealthModify1_005e3620,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fb0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ManaModify1_005e3614,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fc0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ControlModify1_005e3604,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fd0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_MagicResistModify1_005e35f0,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fe0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_IntelligenceModify1_005e35dc,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1ff0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_CombatModify1_005e35cc,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e2000 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_AIRandomness1_005e35bc,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e2010 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_HealthModify2_005e35ac,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fb4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ManaModify2_005e35a0,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fc4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ControlModify2_005e3590,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fd4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_MagicResistModify2_005e357c,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fe4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_IntelligenceModify2_005e3568,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1ff4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_CombatModify2_005e3558,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e2004 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_AIRandomness2_005e3548,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e2014 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_HealthModify3_005e3538,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fb8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ManaModify3_005e352c,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fc8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_ControlModify3_005e351c,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fd8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_MagicResistModify3_005e3508,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1fe8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_IntelligenceModify3_005e34f4,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e1ff8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_CombatModify3_005e34e4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e2008 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_DIFFICULTY_005e3694,s_AIRandomness3_005e34d4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    _DAT_006e2018 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_POISON_POINTS_005e34b8,s_PoisonDelay_005e34c8,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 == 0) {
    DAT_006de830 = 5;
  }
  else {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x19)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_006de830 = 0;
      }
      else {
        DAT_006de830 = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_006de830 = 0x19;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    (s_MR_RECOVERY_005e34a4,s_MRDelay_005e34b0,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 == 0) {
    DAT_006a49dc = 0x1e;
  }
  else {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
    if ((iVar11 < 1) || (iVar11 = FUN_0059c5a4(&DAT_006e2a88), iVar11 < 0x19)) {
      iVar11 = FUN_0059c5a4(&DAT_006e2a88);
      if (iVar11 < 1) {
        DAT_006a49dc = 0;
      }
      else {
        DAT_006a49dc = FUN_0059c5a4(&DAT_006e2a88);
      }
    }
    else {
      DAT_006a49dc = 0x19;
    }
  }
  iVar4 = 2;
  iVar11 = 0x20;
  iVar14 = 2;
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Maxspeed_0_005e3498,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Startspeed_0_005e347c,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Accelspeed_0_005e346c,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar14 = FUN_0059c5a4(&DAT_006e2a88);
  }
  iVar8 = iVar11 * 2;
  iVar9 = iVar4 * 2;
  iVar10 = iVar14 * 2;
  _DAT_005f08d0 = iVar11;
  _DAT_005f08dc = iVar4;
  _DAT_005f08e8 = iVar14;
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Maxspeed_1_005e3460,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Startspeed_1_005e3450,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar9 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Accelspeed_1_005e3440,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar10 = FUN_0059c5a4(&DAT_006e2a88);
  }
  iVar11 = iVar8 * 2;
  iVar4 = iVar9 * 2;
  iVar14 = iVar10 * 2;
  _DAT_005f08d4 = iVar8;
  _DAT_005f08e0 = iVar9;
  _DAT_005f08ec = iVar10;
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Maxspeed_2_005e3434,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar11 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Startspeed_2_005e3424,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SPEECH_BOX_005e348c,s_Accelspeed_2_005e3414,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    iVar14 = FUN_0059c5a4(&DAT_006e2a88);
  }
  _DAT_005f08d8 = iVar11;
  _DAT_005f08e4 = iVar4;
  _DAT_005f08f0 = iVar14;
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_AggressiveAttack_005e3400,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18c4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_CautiousAttack_005e33e4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18c8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_OccupyPowerPoint_005e33d0,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18cc = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_CollectMana_005e33c4,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18d0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_EvadeDetection_005e33b4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18d4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_Scout_005e33ac,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18d8 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_Retreat_005e33a4,&DAT_005faf28,&DAT_006e2a88,0x100,
                     (LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18dc = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_CollectFood_005e3398,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18e0 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_PERSONALITY_005e33f4,s_ProtectWizard_005e3388,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    DAT_005e18e4 = FUN_0059c5a4(&DAT_006e2a88);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_AggressiveAttack_005e3400,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c4944,&DAT_006c4948,&DAT_006c494c);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_CautiousAttack_005e33e4,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c4950,&DAT_006c4954,&DAT_006c4958);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_OccupyPowerPoint_005e33d0,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c495c,&DAT_006c4960,&DAT_006c4964);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_CollectMana_005e33c4,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c4968,&DAT_006c496c,&DAT_006c4970);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_EvadeDetection_005e33b4,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c4974,&DAT_006c4978,&DAT_006c497c);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_Scout_005e33ac,&DAT_005faf28,&DAT_006e2a88,0x100
                     ,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c4980,&DAT_006c4984,&DAT_006c4988);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_Retreat_005e33a4,&DAT_005faf28,&DAT_006e2a88,
                     0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c498c,&DAT_006c4990,&DAT_006c4994);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_CollectFood_005e3398,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c4998,&DAT_006c499c,&DAT_006c49a0);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_OUTCOME_FRACTIONS_005e3374,s_ProtectWizard_005e3388,&DAT_005faf28,
                     &DAT_006e2a88,0x100,(LPCSTR)&DAT_006e2780);
  if (DVar3 != 0) {
    FUN_0059bc22(&DAT_006e2a88,s__f___f___f_005e3368,&DAT_006c49a4,&DAT_006c49a8,&DAT_006c49ac);
  }
  DVar3 = GetPrivateProfileStringA
                    (s_SINGLEBATTLE_005e3344,s_GiveAISameSpells_005e3354,&DAT_005faf28,&DAT_006e2a88
                     ,0x100,(LPCSTR)&DAT_006e2780);
  if ((DVar3 != 0) && (iVar11 = __strcmpi(&DAT_006e2a88,&DAT_005e01d0), iVar11 == 0)) {
    DAT_00658f08 = 1;
  }
  pcVar5 = (char *)FUN_0058a6b0();
  uVar12 = 0xffffffff;
  do {
    pcVar6 = pcVar5;
    if (uVar12 == 0) break;
    uVar12 = uVar12 - 1;
    pcVar6 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar6;
  } while (cVar1 != '\0');
  uVar12 = ~uVar12;
  pcVar5 = pcVar6 + -uVar12;
  pcVar6 = (char *)&DAT_005e12f8;
  for (uVar13 = uVar12 >> 2; uVar13 != 0; uVar13 = uVar13 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar12 = uVar12 & 3; uVar12 != 0; uVar12 = uVar12 - 1) {
    *pcVar6 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar6 = pcVar6 + 1;
  }
  ExceptionList = local_14;
  return 1;
}

