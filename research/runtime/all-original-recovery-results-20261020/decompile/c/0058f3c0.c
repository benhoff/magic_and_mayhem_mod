
undefined4 __thiscall FUN_0058f3c0(int *param_1,LPCSTR param_2)

{
  char cVar1;
  DWORD DVar2;
  char *pcVar3;
  char *pcVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  char *pcVar8;
  CHAR local_404 [256];
  CHAR local_304 [256];
  int local_204;
  char local_200 [256];
  char local_100 [256];
  
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  GetCurrentDirectoryA(0x100,local_404);
  uVar5 = 0xffffffff;
  pcVar3 = local_404;
  do {
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    cVar1 = *pcVar3;
    pcVar3 = pcVar3 + 1;
  } while (cVar1 != '\0');
  if (((~uVar5 != 1) && (local_404[~uVar5 - 2] != '\\')) && (local_404[~uVar5 - 2] != '/')) {
    uVar5 = 0xffffffff;
    pcVar3 = &DAT_005d9058;
    do {
      pcVar4 = pcVar3;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar4 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar4;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    iVar6 = -1;
    pcVar3 = local_404;
    do {
      pcVar8 = pcVar3;
      if (iVar6 == 0) break;
      iVar6 = iVar6 + -1;
      pcVar8 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar8;
    } while (cVar1 != '\0');
    pcVar3 = pcVar4 + -uVar5;
    pcVar4 = pcVar8 + -1;
    for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar4 = *(undefined4 *)pcVar3;
      pcVar3 = pcVar3 + 4;
      pcVar4 = pcVar4 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar4 = *pcVar3;
      pcVar3 = pcVar3 + 1;
      pcVar4 = pcVar4 + 1;
    }
  }
  uVar5 = 0xffffffff;
  pcVar3 = s_cfg_music_cfg_005f1a94;
  do {
    pcVar4 = pcVar3;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar4 = pcVar3 + 1;
    cVar1 = *pcVar3;
    pcVar3 = pcVar4;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  iVar6 = -1;
  pcVar3 = local_404;
  do {
    pcVar8 = pcVar3;
    if (iVar6 == 0) break;
    iVar6 = iVar6 + -1;
    pcVar8 = pcVar3 + 1;
    cVar1 = *pcVar3;
    pcVar3 = pcVar8;
  } while (cVar1 != '\0');
  pcVar3 = pcVar4 + -uVar5;
  pcVar4 = pcVar8 + -1;
  for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar4 = *(undefined4 *)pcVar3;
    pcVar3 = pcVar3 + 4;
    pcVar4 = pcVar4 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar4 = *pcVar3;
    pcVar3 = pcVar3 + 1;
    pcVar4 = pcVar4 + 1;
  }
  DVar2 = GetPrivateProfileStringA
                    (s_GENERAL_005e0158,s_mainTrack_005f1ad0,&DAT_005faf28,local_304,0x100,local_404
                    );
  if (DVar2 != 0) {
    uVar5 = FUN_0059c5a4(local_304);
    param_1[2] = uVar5;
    uVar7 = DAT_005f1a90 - 1;
    if ((int)(((int)uVar5 < 1) - 1 & uVar5) < (int)uVar7) {
      uVar7 = ((int)uVar5 < 1) - 1 & uVar5;
    }
    param_1[2] = uVar7;
  }
  DVar2 = GetPrivateProfileStringA
                    (s_GENERAL_005e0158,s_realmTrack_005f1ac4,&DAT_005faf28,local_304,0x100,
                     local_404);
  if (DVar2 != 0) {
    uVar5 = FUN_0059c5a4(local_304);
    param_1[3] = uVar5;
    uVar7 = DAT_005f1a90 - 1;
    if ((int)(((int)uVar5 < 1) - 1 & uVar5) < (int)uVar7) {
      uVar7 = ((int)uVar5 < 1) - 1 & uVar5;
    }
    param_1[3] = uVar7;
  }
  DVar2 = GetPrivateProfileStringA
                    (s_GENERAL_005e0158,s_battleEndTrack_005f1ab4,&DAT_005faf28,local_304,0x100,
                     local_404);
  if (DVar2 != 0) {
    uVar5 = FUN_0059c5a4(local_304);
    param_1[4] = uVar5;
    uVar7 = DAT_005f1a90 - 1;
    if ((int)(((int)uVar5 < 1) - 1 & uVar5) < (int)uVar7) {
      uVar7 = ((int)uVar5 < 1) - 1 & uVar5;
    }
    param_1[4] = uVar7;
  }
  if (param_2 != (LPCSTR)0x0) {
    DVar2 = GetPrivateProfileStringA
                      (s_BATTLE_TRACKS_005f1aa4,param_2,&DAT_005faf28,local_304,0x100,local_404);
    if (DVar2 == 0) {
      return 0;
    }
    uVar5 = 0xffffffff;
    pcVar3 = local_304;
    do {
      pcVar4 = pcVar3;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar4 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar4;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    pcVar3 = pcVar4 + -uVar5;
    pcVar4 = local_200;
    for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar4 = *(undefined4 *)pcVar3;
      pcVar3 = pcVar3 + 4;
      pcVar4 = pcVar4 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar4 = *pcVar3;
      pcVar3 = pcVar3 + 1;
      pcVar4 = pcVar4 + 1;
    }
    uVar5 = 0xffffffff;
    pcVar3 = &DAT_005da820;
    do {
      pcVar4 = pcVar3;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar4 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar4;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    pcVar3 = pcVar4 + -uVar5;
    pcVar4 = local_100;
    for (uVar7 = uVar5 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar4 = *(undefined4 *)pcVar3;
      pcVar3 = pcVar3 + 4;
      pcVar4 = pcVar4 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar4 = *pcVar3;
      pcVar3 = pcVar3 + 1;
      pcVar4 = pcVar4 + 1;
    }
    local_204 = 1;
    while( true ) {
      if (local_204 == 0) {
        pcVar3 = (char *)0x0;
      }
      else {
        pcVar3 = local_200;
        local_204 = 0;
      }
      pcVar3 = (char *)FUN_0059c99c(pcVar3,local_100);
      if (pcVar3 == (char *)0x0) break;
      pcVar4 = _strchr(pcVar3,0x3d);
      if (pcVar4 == (char *)0x0) {
        _strchr(pcVar3,0);
      }
      else {
        *pcVar4 = '\0';
      }
      if (DAT_005f1a90 <= param_1[1]) {
        return 1;
      }
      uVar5 = FUN_0059c5a4(pcVar3);
      *(uint *)(*param_1 + param_1[1] * 4) = ((int)uVar5 < 1) - 1 & uVar5;
      param_1[1] = param_1[1] + 1;
    }
  }
  return 1;
}

