
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined4 FUN_004a30b0(void)

{
  char cVar1;
  DWORD DVar2;
  undefined4 uVar3;
  char *pcVar4;
  char *pcVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  char *pcVar10;
  undefined4 *puVar11;
  int *piVar12;
  undefined4 *local_680;
  int local_67c;
  int local_678;
  int local_674;
  int local_670 [26];
  undefined4 local_608;
  CHAR local_604 [254];
  char acStack_506 [2];
  undefined4 local_504 [64];
  char local_404 [256];
  int local_304;
  char local_300 [256];
  char local_200 [256];
  CHAR local_100 [256];
  
  GetCurrentDirectoryA(0x100,local_100);
  uVar6 = 0xffffffff;
  pcVar4 = local_100;
  do {
    pcVar5 = pcVar4;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar5 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar5;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar4 = pcVar5 + -uVar6;
  pcVar5 = (char *)local_504;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar5 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar4 = (char *)local_504;
  do {
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar4 + 1;
  } while (cVar1 != '\0');
  if (((~uVar6 != 1) && (acStack_506[~uVar6] != '\\')) && (acStack_506[~uVar6] != '/')) {
    uVar6 = 0xffffffff;
    pcVar4 = &DAT_005d9058;
    do {
      pcVar5 = pcVar4;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    iVar8 = -1;
    pcVar4 = (char *)local_504;
    do {
      pcVar10 = pcVar4;
      if (iVar8 == 0) break;
      iVar8 = iVar8 + -1;
      pcVar10 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar10;
    } while (cVar1 != '\0');
    pcVar4 = pcVar5 + -uVar6;
    pcVar5 = pcVar10 + -1;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
  }
  uVar6 = 0xffffffff;
  pcVar4 = s_cfg_cursors_cfg_005e0550;
  do {
    pcVar5 = pcVar4;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar5 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar5;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  iVar8 = -1;
  pcVar4 = (char *)local_504;
  do {
    pcVar10 = pcVar4;
    if (iVar8 == 0) break;
    iVar8 = iVar8 + -1;
    pcVar10 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar10;
  } while (cVar1 != '\0');
  pcVar4 = pcVar5 + -uVar6;
  pcVar5 = pcVar10 + -1;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar5 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  }
  GetPrivateProfileStringA
            (s_HEADER_005da478,s_BitmapWidth_005e0544,&DAT_005faf28,local_604,0x100,
             (LPCSTR)local_504);
  iVar8 = FUN_0059c5a4(local_604);
  if (iVar8 < 1) {
LAB_004a31da:
    iVar8 = FUN_0059c5a4(local_604);
    if (iVar8 < 1) {
      local_67c = 0;
    }
    else {
      local_67c = FUN_0059c5a4(local_604);
    }
  }
  else {
    iVar8 = FUN_0059c5a4(local_604);
    if (iVar8 < 1000) goto LAB_004a31da;
    local_67c = 1000;
  }
  GetPrivateProfileStringA
            (s_HEADER_005da478,s_BitmapHeight_005e0534,&DAT_005faf28,local_604,0x100,
             (LPCSTR)local_504);
  iVar8 = FUN_0059c5a4(local_604);
  if (0 < iVar8) {
    iVar8 = FUN_0059c5a4(local_604);
    if (999 < iVar8) {
      local_678 = 1000;
      goto LAB_004a32a3;
    }
  }
  iVar8 = FUN_0059c5a4(local_604);
  if (iVar8 < 1) {
    local_678 = 0;
  }
  else {
    local_678 = FUN_0059c5a4(local_604);
  }
LAB_004a32a3:
  local_680 = (undefined4 *)&DAT_00642684;
  local_674 = 0;
  do {
    uVar6 = 0xffffffff;
    pcVar4 = s_CURSOR__005e052c;
    do {
      pcVar5 = pcVar4;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    pcVar4 = pcVar5 + -uVar6;
    pcVar5 = local_404;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    FUN_0059de2f(local_674,local_604,10);
    uVar6 = 0xffffffff;
    pcVar4 = local_604;
    do {
      pcVar5 = pcVar4;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    iVar8 = -1;
    pcVar4 = local_404;
    do {
      pcVar10 = pcVar4;
      if (iVar8 == 0) break;
      iVar8 = iVar8 + -1;
      pcVar10 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar10;
    } while (cVar1 != '\0');
    pcVar4 = pcVar5 + -uVar6;
    pcVar5 = pcVar10 + -1;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    DVar2 = GetPrivateProfileStringA
                      (local_404,s_Hotspotx_005e0520,&DAT_005faf28,local_604,0x100,(LPCSTR)local_504
                      );
    if (DVar2 != 0) {
      iVar8 = FUN_0059c5a4(local_604);
      if (iVar8 < 1) {
LAB_004a3376:
        iVar8 = FUN_0059c5a4(local_604);
        if (iVar8 < 1) {
          uVar3 = 0;
        }
        else {
          uVar3 = FUN_0059c5a4(local_604);
        }
      }
      else {
        iVar8 = FUN_0059c5a4(local_604);
        if (iVar8 < 0x28) goto LAB_004a3376;
        uVar3 = 0x28;
      }
      local_680[-3] = uVar3;
    }
    DVar2 = GetPrivateProfileStringA
                      (local_404,s_Hotspoty_005e0514,&DAT_005faf28,local_604,0x100,(LPCSTR)local_504
                      );
    if (DVar2 != 0) {
      iVar8 = FUN_0059c5a4(local_604);
      if (iVar8 < 1) {
LAB_004a3402:
        iVar8 = FUN_0059c5a4(local_604);
        if (iVar8 < 1) {
          uVar3 = 0;
        }
        else {
          uVar3 = FUN_0059c5a4(local_604);
        }
      }
      else {
        iVar8 = FUN_0059c5a4(local_604);
        if (iVar8 < 0x28) goto LAB_004a3402;
        uVar3 = 0x28;
      }
      local_680[-2] = uVar3;
    }
    DVar2 = GetPrivateProfileStringA
                      (local_404,s_Frames_005e050c,&DAT_005faf28,local_604,0x100,(LPCSTR)local_504);
    if (DVar2 != 0) {
      uVar6 = 0xffffffff;
      uVar7 = 0;
      pcVar4 = local_604;
      do {
        pcVar5 = pcVar4;
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        pcVar5 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar5;
      } while (cVar1 != '\0');
      uVar6 = ~uVar6;
      pcVar4 = pcVar5 + -uVar6;
      pcVar5 = local_300;
      for (uVar9 = uVar6 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        pcVar5 = pcVar5 + 4;
      }
      for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *pcVar5 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        pcVar5 = pcVar5 + 1;
      }
      uVar6 = 0xffffffff;
      pcVar4 = &DAT_005da820;
      do {
        pcVar5 = pcVar4;
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        pcVar5 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar5;
      } while (cVar1 != '\0');
      uVar6 = ~uVar6;
      pcVar4 = pcVar5 + -uVar6;
      pcVar5 = local_200;
      for (uVar9 = uVar6 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        pcVar5 = pcVar5 + 4;
      }
      for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *pcVar5 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        pcVar5 = pcVar5 + 1;
      }
      local_304 = 1;
      puVar11 = local_680;
      do {
        if (local_304 == 0) {
          pcVar4 = (char *)0x0;
        }
        else {
          pcVar4 = local_300;
          local_304 = 0;
        }
        pcVar4 = (char *)FUN_0059c99c(pcVar4,local_200);
        if (pcVar4 == (char *)0x0) break;
        pcVar5 = _strchr(pcVar4,0x3d);
        if (pcVar5 == (char *)0x0) {
          _strchr(pcVar4,0);
        }
        else {
          *pcVar5 = '\0';
        }
        uVar3 = FUN_0059c5a4(pcVar4);
        *puVar11 = uVar3;
        uVar7 = uVar7 + 1;
        puVar11 = puVar11 + 1;
      } while (uVar7 < 0xc);
      local_680[-1] = uVar7;
    }
    DVar2 = GetPrivateProfileStringA
                      (local_404,s_Delay_005e0504,&DAT_005faf28,local_604,0x100,(LPCSTR)local_504);
    if (DVar2 != 0) {
      iVar8 = FUN_0059c5a4(local_604);
      if (iVar8 < 1) {
LAB_004a35a3:
        iVar8 = FUN_0059c5a4(local_604);
        if (iVar8 < 1) {
          uVar3 = 0;
        }
        else {
          uVar3 = FUN_0059c5a4(local_604);
        }
      }
      else {
        iVar8 = FUN_0059c5a4(local_604);
        if (iVar8 < 1000) goto LAB_004a35a3;
        uVar3 = 1000;
      }
      local_680[0xd] = uVar3;
    }
    local_674 = local_674 + 1;
    local_680 = local_680 + 0x11;
    if (0x15 < local_674) {
      piVar12 = local_670;
      for (iVar8 = 0x1b; iVar8 != 0; iVar8 = iVar8 + -1) {
        *piVar12 = 0;
        piVar12 = piVar12 + 1;
      }
      local_670[0] = 0x6c;
      local_670[1] = 7;
      local_608 = 0x40;
      local_670[3] = 0x28;
      local_670[2] = 0x28;
      FUN_0058ad90(local_670,s_c__Projects_Chaos_Flatwin_cpp_005e04e4,0xb8);
      FUN_0058bc10(0x1f);
      piVar12 = local_670;
      for (iVar8 = 0x1b; iVar8 != 0; iVar8 = iVar8 + -1) {
        *piVar12 = 0;
        piVar12 = piVar12 + 1;
      }
      local_608 = 0x40;
      local_670[0] = 0x6c;
      local_670[1] = 7;
      local_670[3] = local_67c;
      local_670[2] = local_678;
      FUN_0058ad90(local_670,s_c__Projects_Chaos_Flatwin_cpp_005e04e4,0xc2);
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
      FUN_0058d1a0(s_bitmaps_cursors_bmp_005e04d0);
      _DAT_00642028 = 0xffffffff;
      DAT_00641a8c = local_67c / 0x28;
      DAT_00642c50 = local_678 / 0x28;
      DAT_00642c58 = CreateMutexA((LPSECURITY_ATTRIBUTES)0x0,0,(LPCSTR)0x0);
      return 1;
    }
  } while( true );
}

