
/* WARNING: Removing unreachable block (ram,0x0048241e) */

bool __thiscall FUN_004822a0(int param_1,LPCSTR param_2,undefined4 param_3)

{
  char cVar1;
  bool bVar2;
  int iVar3;
  HANDLE hFile;
  MCIERROR MVar4;
  uint uVar5;
  uint uVar6;
  char *pcVar7;
  char *pcVar8;
  char *pcVar9;
  char local_100 [256];
  
  if (DAT_006e2030 != '\0') {
    return true;
  }
  bVar2 = false;
  if (*(int *)(param_1 + 0x32c) == 0) {
    iVar3 = FUN_0059c669((int)DAT_005e12e4);
    if ((iVar3 != 0) && (DAT_005e12e5 == ':')) {
      _strncpy((char *)(param_1 + 0x388),&DAT_005e12e4,2);
    }
    FUN_00481490(param_3);
    bVar2 = true;
  }
  else {
    while( true ) {
      *(undefined4 *)(param_1 + 0x380) = 4;
      MVar4 = mciSendCommandA(*(MCIDEVICEID *)(param_1 + 0x33c),0x814,0x102,param_1 + 0x378);
      if ((MVar4 == 0) &&
         (((iVar3 = *(int *)(param_1 + 0x37c), iVar3 == 0x212 || (iVar3 == 0x20d)) ||
          (iVar3 == 0x20c)))) break;
      FUN_00482010(1);
    }
  }
  GetDriveTypeA(param_2);
  *(undefined4 *)(param_1 + 0x380) = 1;
  mciSendCommandA(*(MCIDEVICEID *)(param_1 + 0x33c),0x814,0x102,param_1 + 0x378);
  uVar5 = 0xffffffff;
  do {
    pcVar7 = param_2;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar7 = param_2 + 1;
    cVar1 = *param_2;
    param_2 = pcVar7;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  pcVar7 = pcVar7 + -uVar5;
  pcVar9 = local_100;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar9 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar9 = pcVar9 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar9 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar9 = pcVar9 + 1;
  }
  uVar5 = FUN_0054e200();
  switch(uVar5 & 3) {
  case 0:
    uVar5 = 0xffffffff;
    pcVar7 = &DAT_005da7d4;
    do {
      pcVar9 = pcVar7;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar9 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar9;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    iVar3 = -1;
    pcVar7 = local_100;
    do {
      pcVar8 = pcVar7;
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      pcVar8 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar8;
    } while (cVar1 != '\0');
    pcVar7 = pcVar9 + -uVar5;
    pcVar9 = pcVar8 + -1;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar9 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcVar9 = pcVar9 + 1;
    }
    switch(DAT_006e1ed4) {
    case 0:
    case 1:
      break;
    case 2:
    case 3:
      break;
    case 4:
      break;
    case 5:
    }
    break;
  case 1:
    uVar5 = 0xffffffff;
    pcVar7 = &DAT_005da7c4;
    do {
      pcVar9 = pcVar7;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar9 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar9;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    iVar3 = -1;
    pcVar7 = local_100;
    do {
      pcVar8 = pcVar7;
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      pcVar8 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar8;
    } while (cVar1 != '\0');
    pcVar7 = pcVar9 + -uVar5;
    pcVar9 = pcVar8 + -1;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar9 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcVar9 = pcVar9 + 1;
    }
    switch(DAT_006e1ed4) {
    case 0:
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      break;
    case 4:
      break;
    case 5:
    }
    break;
  case 2:
    uVar5 = 0xffffffff;
    pcVar7 = s_fmv_Medieval_avi_005da7b0;
    do {
      pcVar9 = pcVar7;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar9 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar9;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    iVar3 = -1;
    pcVar7 = local_100;
    do {
      pcVar8 = pcVar7;
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      pcVar8 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar8;
    } while (cVar1 != '\0');
    pcVar7 = pcVar9 + -uVar5;
    pcVar9 = pcVar8 + -1;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar9 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcVar9 = pcVar9 + 1;
    }
    switch(DAT_006e1ed4) {
    case 0:
      break;
    case 1:
      break;
    case 2:
    case 3:
      break;
    case 4:
      break;
    case 5:
    }
    break;
  case 3:
    uVar5 = 0xffffffff;
    pcVar7 = &DAT_005da7a0;
    do {
      pcVar9 = pcVar7;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar9 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar9;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    iVar3 = -1;
    pcVar7 = local_100;
    do {
      pcVar8 = pcVar7;
      if (iVar3 == 0) break;
      iVar3 = iVar3 + -1;
      pcVar8 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar8;
    } while (cVar1 != '\0');
    pcVar7 = pcVar9 + -uVar5;
    pcVar9 = pcVar8 + -1;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar9 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcVar9 = pcVar9 + 1;
    }
    switch(DAT_006e1ed4) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      break;
    case 5:
    }
  }
  hFile = CreateFileA(local_100,0x80000000,0,(LPSECURITY_ATTRIBUTES)0x0,3,0x80,(HANDLE)0x0);
  if (hFile != (HANDLE)0x0) {
    GetFileSize(hFile,(LPDWORD)0x0);
  }
  CloseHandle(hFile);
  if ((bVar2) && (*(int *)(param_1 + 0x32c) != 0)) {
    if (*(int *)(param_1 + 0x328) != 0) {
      FUN_00482010(1);
      *(undefined4 *)(param_1 + 0x328) = 0;
    }
    iVar3 = *(int *)(param_1 + 0x344);
    if (iVar3 != 0) {
      FUN_0059bf07(iVar3,0x46c,*(undefined4 *)(iVar3 + -4),FUN_00481d90);
      FUN_00597870(iVar3 + -4);
      *(undefined4 *)(param_1 + 0x344) = 0;
    }
    MVar4 = mciSendCommandA(*(MCIDEVICEID *)(param_1 + 0x33c),0x804,2,0);
    if (MVar4 == 0) {
      *(undefined4 *)(param_1 + 0x32c) = 0;
    }
  }
  return hFile != (HANDLE)0x0;
}

