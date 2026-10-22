
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

undefined2 FUN_004e8d80(HINSTANCE param_1,int param_2)

{
  LONG LVar1;
  char cVar2;
  undefined2 uVar3;
  undefined4 uVar4;
  BOOL BVar5;
  HWND pHVar6;
  LPCSTR pCVar7;
  char *pcVar8;
  char *pcVar9;
  HDC hdc;
  int iVar10;
  uint uVar11;
  uint uVar12;
  int iVar13;
  CHAR *pCVar14;
  DWORD DVar15;
  CHAR *pCVar16;
  char *pcVar17;
  UINT UVar18;
  tagPOINT local_4b4;
  HWND local_4ac;
  tagRECT tStack_4a8;
  DWORD DStack_498;
  DWORD DStack_494;
  DWORD DStack_490;
  DWORD DStack_48c;
  uint uStack_488;
  undefined4 uStack_484;
  undefined4 uStack_480;
  undefined4 uStack_47c;
  undefined4 uStack_478;
  WNDCLASSA WStack_474;
  char local_448 [256];
  undefined4 local_348;
  undefined4 local_344;
  _WIN32_FIND_DATAA local_140;
  
  DAT_006e80e8 = FUN_00597d00(&LAB_004e3cb0);
  DAT_006cbb68 = param_1;
  local_4b4.x = (LONG)FindFirstFileA(s_Cfg_Encrypted___cfg_005e4140,&local_140);
  if ((HANDLE)local_4b4.x != (HANDLE)0xffffffff) {
    do {
      uVar11 = 0xffffffff;
      pcVar8 = s_Cfg_Encrypted__005e4130;
      do {
        pcVar9 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar9 = pcVar8 + 1;
        cVar2 = *pcVar8;
        pcVar8 = pcVar9;
      } while (cVar2 != '\0');
      uVar11 = ~uVar11;
      pcVar8 = pcVar9 + -uVar11;
      pcVar9 = local_448;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar9 = pcVar9 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar9 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar9 = pcVar9 + 1;
      }
      uVar11 = 0xffffffff;
      pCVar14 = local_140.cFileName;
      do {
        pCVar16 = pCVar14;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pCVar16 = pCVar14 + 1;
        cVar2 = *pCVar14;
        pCVar14 = pCVar16;
      } while (cVar2 != '\0');
      uVar11 = ~uVar11;
      local_348 = 0;
      iVar13 = -1;
      pcVar8 = local_448;
      do {
        pcVar9 = pcVar8;
        if (iVar13 == 0) break;
        iVar13 = iVar13 + -1;
        pcVar9 = pcVar8 + 1;
        cVar2 = *pcVar8;
        pcVar8 = pcVar9;
      } while (cVar2 != '\0');
      pCVar14 = pCVar16 + -uVar11;
      pcVar8 = pcVar9 + -1;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pCVar14;
        pCVar14 = pCVar14 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar8 = *pCVar14;
        pCVar14 = pCVar14 + 1;
        pcVar8 = pcVar8 + 1;
      }
      local_344 = 0;
      iVar13 = FUN_004a1360(local_448,&DAT_005d96e0);
      if (iVar13 != 0) {
        uVar4 = FUN_004a15d0();
        local_4ac = (HWND)FUN_00597850(uVar4);
        FUN_004a1820(local_4ac,uVar4);
        FUN_004a1550();
        uVar11 = 0xffffffff;
        pcVar8 = &DAT_005e4128;
        do {
          pcVar9 = pcVar8;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pcVar9 = pcVar8 + 1;
          cVar2 = *pcVar8;
          pcVar8 = pcVar9;
        } while (cVar2 != '\0');
        uVar11 = ~uVar11;
        pcVar8 = pcVar9 + -uVar11;
        pcVar9 = local_448;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
          pcVar8 = pcVar8 + 4;
          pcVar9 = pcVar9 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar9 = *pcVar8;
          pcVar8 = pcVar8 + 1;
          pcVar9 = pcVar9 + 1;
        }
        uVar11 = 0xffffffff;
        pCVar14 = local_140.cFileName;
        do {
          pCVar16 = pCVar14;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pCVar16 = pCVar14 + 1;
          cVar2 = *pCVar14;
          pCVar14 = pCVar16;
        } while (cVar2 != '\0');
        uVar11 = ~uVar11;
        iVar13 = -1;
        pcVar8 = local_448;
        do {
          pcVar9 = pcVar8;
          if (iVar13 == 0) break;
          iVar13 = iVar13 + -1;
          pcVar9 = pcVar8 + 1;
          cVar2 = *pcVar8;
          pcVar8 = pcVar9;
        } while (cVar2 != '\0');
        pCVar14 = pCVar16 + -uVar11;
        pcVar8 = pcVar9 + -1;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar8 = *(undefined4 *)pCVar14;
          pCVar14 = pCVar14 + 4;
          pcVar8 = pcVar8 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar8 = *pCVar14;
          pCVar14 = pCVar14 + 1;
          pcVar8 = pcVar8 + 1;
        }
        iVar13 = FUN_004a1360(local_448,&DAT_005d9e54);
        if (iVar13 != 0) {
          FUN_004a1790(local_4ac,uVar4);
          FUN_004a1550();
        }
        FUN_00597870(local_4ac);
      }
      FUN_004a1550();
      BVar5 = FindNextFileA((HANDLE)local_4b4.x,&local_140);
    } while (BVar5 != 0);
    FindClose((HANDLE)local_4b4.x);
  }
  FUN_004e8480();
  DAT_006e1ec0 = CreateMutexA((LPSECURITY_ATTRIBUTES)0x0,0,(LPCSTR)0x0);
  DAT_00689928 = RegisterWindowMessageA(s_QueryCancelAutoPlay_005e4114);
  CoInitialize((LPVOID)0x0);
  FUN_004eab00();
  iVar13 = FUN_004e4270();
  if (iVar13 == 0) {
    MessageBoxA((HWND)0x0,s_The_configuration_file__CHAOS_CF_005e40a8,
                s_NO_CONFIGURATION_FILE_005e40fc,0x42030);
  }
  FUN_00484470();
  FUN_0054be80();
  DAT_006e0080 = 0;
  _DAT_00656610 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
  _DAT_00657cd8 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
  if (param_2 == 0) {
    WStack_474.style = 3;
    WStack_474.lpfnWndProc = (WNDPROC)&LAB_004e9d60;
    WStack_474.cbClsExtra = 0;
    WStack_474.cbWndExtra = 0;
    WStack_474.hInstance = param_1;
    WStack_474.hIcon = LoadIconA(param_1,(LPCSTR)0x65);
    WStack_474.hCursor = (HCURSOR)0x0;
    WStack_474.hbrBackground = GetStockObject(4);
    WStack_474.lpszMenuName = (LPCSTR)0x0;
    WStack_474.lpszClassName = PTR_s_CHAOS_005e13f8;
    RegisterClassA(&WStack_474);
  }
  FUN_00485f00();
  if (DAT_006a2920 == 0) {
    DVar15 = 0x92080008;
    tStack_4a8.left = 0;
    tStack_4a8.top = 0;
    tStack_4a8.right = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    tStack_4a8.bottom = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
  }
  else {
    DVar15 = 0x12ca0000;
    tStack_4a8.right = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    tStack_4a8.left = 0;
    tStack_4a8.top = 0;
    tStack_4a8.bottom = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    AdjustWindowRect(&tStack_4a8,0x12ca0000,0);
    tStack_4a8.right = tStack_4a8.right - tStack_4a8.left;
    tStack_4a8.bottom = tStack_4a8.bottom - tStack_4a8.top;
    tStack_4a8.top = 0;
    tStack_4a8.left = 0;
    iVar13 = GetSystemMetrics(0);
    tStack_4a8.left = tStack_4a8.left + (iVar13 - tStack_4a8.right) / 2;
    iVar13 = GetSystemMetrics(1);
    tStack_4a8.top = tStack_4a8.top + (iVar13 - tStack_4a8.bottom) / 2;
  }
  pHVar6 = CreateWindowExA(0,PTR_s_CHAOS_005e13f8,(LPCSTR)&DAT_005e12f8,DVar15,tStack_4a8.left,
                           tStack_4a8.top,tStack_4a8.right,tStack_4a8.bottom,(HWND)0x0,(HMENU)0x0,
                           param_1,(LPVOID)0x0);
  DAT_00656614 = pHVar6;
  if (pHVar6 == (HWND)0x0) {
    return 0;
  }
  uStack_484 = 10;
  uStack_480 = 10;
  uStack_47c = 0xa0;
  uStack_478 = 0xa0;
  local_4ac = pHVar6;
  FUN_00485ef0(0x6e5,&uStack_484,pHVar6,param_1,1,s_Debug_Window_005e4098);
  FUN_00485ef0(0x6e6,&uStack_484,pHVar6,param_1,0,s_AI_Window_005e408c);
  DAT_006e2031 = '\x01';
  pCVar7 = (LPCSTR)FUN_00597850(0x100);
  pcVar8 = (char *)FUN_00597850(0x100);
  *pCVar7 = '\0';
  *pcVar8 = '\0';
  local_4b4.x = (LONG)pcVar8;
  pcVar9 = (char *)FUN_0058a6b0();
  uVar11 = 0xffffffff;
  do {
    pcVar17 = pcVar9;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar17 = pcVar9 + 1;
    cVar2 = *pcVar9;
    pcVar9 = pcVar17;
  } while (cVar2 != '\0');
  uVar11 = ~uVar11;
  pcVar9 = pcVar17 + -uVar11;
  pcVar17 = pCVar7;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar17 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar17 = pcVar17 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar17 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar17 = pcVar17 + 1;
  }
  pcVar9 = (char *)FUN_0058a6b0();
  uVar11 = 0xffffffff;
  do {
    pcVar17 = pcVar9;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar17 = pcVar9 + 1;
    cVar2 = *pcVar9;
    pcVar9 = pcVar17;
  } while (cVar2 != '\0');
  uVar11 = ~uVar11;
  pcVar9 = pcVar17 + -uVar11;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar8 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar8 = pcVar8 + 1;
  }
  cVar2 = FUN_004822a0(&DAT_005e12e4,DAT_00656614);
  DVar15 = DStack_498;
  while (cVar2 == '\0') {
    ShowCursor(1);
    DVar15 = MessageBoxA((HWND)0x0,(LPCSTR)local_4b4.x,pCVar7,0x42032);
    ShowCursor(0);
    if (DVar15 != 4) goto LAB_004e92d4;
    cVar2 = FUN_004822a0(&DAT_005e12e4,DAT_00656614);
  }
  DAT_006e2031 = '\0';
LAB_004e92d4:
  FUN_00597870(local_4b4.x);
  FUN_00597870(pCVar7);
  if (DVar15 == 3) {
    return 0;
  }
  if (DAT_006de701 != 0) {
    local_4b4.x = FUN_0056df60(DAT_00656614,DAT_006cbb68,s_Sounds_005e4084);
    if (local_4b4.x < 0) {
      pcVar9 = (char *)FUN_00597850(0x100);
      *pcVar9 = '\0';
      pcVar8 = (char *)FUN_0058a6b0();
      uVar11 = 0xffffffff;
      UVar18 = 0x40030;
      do {
        pcVar17 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar17 = pcVar8 + 1;
        cVar2 = *pcVar8;
        pcVar8 = pcVar17;
      } while (cVar2 != '\0');
      uVar11 = ~uVar11;
      pcVar8 = pcVar17 + -uVar11;
      pcVar17 = pcVar9;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar17 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar17 = pcVar17 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar17 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar17 = pcVar17 + 1;
      }
      pcVar8 = pcVar9;
      pCVar7 = (LPCSTR)FUN_0056ef00();
      MessageBoxA((HWND)0x0,pCVar7,pcVar8,UVar18);
      FUN_00597870(pcVar9);
      DAT_006de701 = 0;
    }
    else {
      local_4b4.x = FUN_0056e910(1);
      if (local_4b4.x < 0) {
        pcVar9 = (char *)FUN_00597850(0x100);
        *pcVar9 = '\0';
        pcVar8 = (char *)FUN_0058a6b0();
        uVar11 = 0xffffffff;
        UVar18 = 0x40030;
        do {
          pcVar17 = pcVar8;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pcVar17 = pcVar8 + 1;
          cVar2 = *pcVar8;
          pcVar8 = pcVar17;
        } while (cVar2 != '\0');
        uVar11 = ~uVar11;
        pcVar8 = pcVar17 + -uVar11;
        pcVar17 = pcVar9;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar17 = *(undefined4 *)pcVar8;
          pcVar8 = pcVar8 + 4;
          pcVar17 = pcVar17 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar17 = *pcVar8;
          pcVar8 = pcVar8 + 1;
          pcVar17 = pcVar17 + 1;
        }
        pcVar8 = pcVar9;
        pCVar7 = (LPCSTR)FUN_0056ef00();
        MessageBoxA((HWND)0x0,pCVar7,pcVar8,UVar18);
        FUN_00597870(pcVar9);
        DAT_006de701 = 0;
      }
      else {
        FUN_0056fd30(DAT_006de70d);
        FUN_005703c0();
      }
    }
  }
  pHVar6 = FindWindowA(s_SJE_CdPlayerClass_005e4070,(LPCSTR)0x0);
  DAT_006578b8 = (uint)(pHVar6 != (HWND)0x0);
  if (DAT_006a2920 != 0) {
    hdc = GetDC(local_4ac);
    iVar13 = GetDeviceCaps(hdc,0xc);
    iVar10 = GetDeviceCaps(hdc,0xe);
    if ((iVar13 != 0x10) || (iVar10 != 1)) {
      ReleaseDC(local_4ac,hdc);
      ShowCursor(1);
      MessageBoxA((HWND)0x0,s_Must_be_in_high_colour_mode_to_r_005e4040,s_ERROR_005d96d8,0x42030);
      ShowCursor(0);
      return 0;
    }
    ReleaseDC(local_4ac,hdc);
  }
  UpdateWindow(local_4ac);
  FUN_004bd810();
  if (uStack_488 < 0x600) {
    pCVar7 = (LPCSTR)FUN_00597850(0x100);
    local_4b4.x = FUN_00597850(0x100);
    *pCVar7 = '\0';
    *(char *)local_4b4.x = '\0';
    pcVar8 = (char *)FUN_0058a6b0();
    uVar11 = 0xffffffff;
    do {
      pcVar9 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar8 + 1;
      cVar2 = *pcVar8;
      pcVar8 = pcVar9;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    pcVar8 = pcVar9 + -uVar11;
    pcVar9 = pCVar7;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar9 = pcVar9 + 1;
    }
    pcVar8 = (char *)FUN_0058a6b0();
    uVar11 = 0xffffffff;
    do {
      pcVar9 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar8 + 1;
      cVar2 = *pcVar8;
      pcVar8 = pcVar9;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    pcVar8 = pcVar9 + -uVar11;
    pcVar9 = (char *)local_4b4.x;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar9 = pcVar9 + 1;
    }
    ShowCursor(1);
    LVar1 = local_4b4.x;
    iVar13 = MessageBoxA((HWND)0x0,(LPCSTR)local_4b4.x,pCVar7,0x42034);
    ShowCursor(0);
    FUN_00597870(LVar1);
    FUN_00597870(pCVar7);
    if (iVar13 == 6) {
      return 0;
    }
  }
  if (((DAT_006e2030 == '\0') && (DAT_006e2031 == '\0')) && (iVar13 = FUN_00483f30(), iVar13 == 0))
  {
    pCVar7 = (LPCSTR)FUN_00597850(0x100);
    local_4b4.x = FUN_00597850(0x100);
    *pCVar7 = '\0';
    *(char *)local_4b4.x = '\0';
    pcVar8 = (char *)FUN_0058a6b0();
    uVar11 = 0xffffffff;
    do {
      pcVar9 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar8 + 1;
      cVar2 = *pcVar8;
      pcVar8 = pcVar9;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    pcVar8 = pcVar9 + -uVar11;
    pcVar9 = pCVar7;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar9 = pcVar9 + 1;
    }
    pcVar8 = (char *)FUN_0058a6b0();
    uVar11 = 0xffffffff;
    do {
      pcVar9 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar8 + 1;
      cVar2 = *pcVar8;
      pcVar8 = pcVar9;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    pcVar8 = pcVar9 + -uVar11;
    pcVar9 = (char *)local_4b4.x;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar9 = pcVar9 + 1;
    }
    ShowCursor(1);
    LVar1 = local_4b4.x;
    iVar13 = MessageBoxA((HWND)0x0,(LPCSTR)local_4b4.x,pCVar7,0x42034);
    ShowCursor(0);
    FUN_00597870(LVar1);
    FUN_00597870(pCVar7);
    if (iVar13 == 6) {
      return 0;
    }
  }
  DStack_48c = 0;
  DStack_494 = 0;
  DStack_498 = 0;
  DStack_490 = 0;
  GetDiskFreeSpaceA((LPCSTR)0x0,&DStack_48c,&DStack_494,&DStack_498,&DStack_490);
  if (DStack_490 * DStack_494 * DStack_48c < 0x3200000) {
    pCVar7 = (LPCSTR)FUN_00597850(0x100);
    local_4b4.x = FUN_00597850(0x100);
    *pCVar7 = '\0';
    *(char *)local_4b4.x = '\0';
    pcVar8 = (char *)FUN_0058a6b0();
    uVar11 = 0xffffffff;
    do {
      pcVar9 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar8 + 1;
      cVar2 = *pcVar8;
      pcVar8 = pcVar9;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    pcVar8 = pcVar9 + -uVar11;
    pcVar9 = pCVar7;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar9 = pcVar9 + 1;
    }
    pcVar8 = (char *)FUN_0058a6b0();
    uVar11 = 0xffffffff;
    do {
      pcVar9 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar9 = pcVar8 + 1;
      cVar2 = *pcVar8;
      pcVar8 = pcVar9;
    } while (cVar2 != '\0');
    uVar11 = ~uVar11;
    pcVar8 = pcVar9 + -uVar11;
    pcVar9 = (char *)local_4b4.x;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar9 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar9 = pcVar9 + 1;
    }
    ShowCursor(1);
    LVar1 = local_4b4.x;
    iVar13 = MessageBoxA((HWND)0x0,(LPCSTR)local_4b4.x,pCVar7,0x42034);
    ShowCursor(0);
    FUN_00597870(LVar1);
    FUN_00597870(pCVar7);
    if (iVar13 == 6) {
      return 0;
    }
  }
  if (DAT_006de705 != 0) {
    iVar13 = FUN_0059c669((int)DAT_005e12e4);
    if ((iVar13 != 0) && (DAT_005e12e5 == ':')) {
      FUN_00482280(&DAT_005e12e4);
    }
    iVar13 = FUN_00481490(DAT_00656614);
    if (iVar13 != 0) {
      pcVar9 = (char *)FUN_00597850(0x100);
      *pcVar9 = '\0';
      pcVar8 = (char *)FUN_0058a6b0();
      uVar11 = 0xffffffff;
      UVar18 = 0x40030;
      do {
        pcVar17 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar17 = pcVar8 + 1;
        cVar2 = *pcVar8;
        pcVar8 = pcVar17;
      } while (cVar2 != '\0');
      uVar11 = ~uVar11;
      pcVar8 = pcVar17 + -uVar11;
      pcVar17 = pcVar9;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar17 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar17 = pcVar17 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar17 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar17 = pcVar17 + 1;
      }
      pcVar8 = pcVar9;
      pCVar7 = (LPCSTR)FUN_004813b0();
      MessageBoxA((HWND)0x0,pCVar7,pcVar8,UVar18);
      FUN_00597870(pcVar9);
    }
    FUN_00482190(DAT_006de709);
  }
  FUN_004e40d0();
  FUN_005357c0();
  if ((DAT_006de6e9 != 0) && (DAT_006de6c5 == '\0')) {
    local_4b4.x = 0;
    local_4b4.y = 0;
    ClientToScreen(DAT_00656614,&local_4b4);
    iVar13 = DAT_005c6fc4;
    iVar10 = DAT_005c6fc0;
    if (DAT_006de6d5 != 0) {
      iVar13 = DAT_005c6fbc;
      iVar10 = DAT_005c6fb8;
    }
    local_4b4.x = local_4b4.x + iVar10;
    local_4b4.y = local_4b4.y + iVar13;
    FUN_0059c1fd(local_448,s__sfmv_intro0_avi_005e402c,&DAT_005e12e4);
    cVar2 = FUN_00469a00(DAT_006a4fe4,local_4b4.x,local_4b4.y,0);
    if (cVar2 != '\0') {
      FUN_0059c1fd(local_448,s__sfmv_intro1_avi_005e4018,&DAT_005e12e4);
      FUN_00469a00(DAT_006a4fe4,local_4b4.x,local_4b4.y,0);
    }
    FUN_0058bc10(0);
  }
  iVar13 = FUN_0057a550();
  if (iVar13 == 0) {
    FUN_0049e4e0();
  }
  iVar13 = FUN_005446a0(s_cfg_objects_cfg_005e3ff8);
  if (iVar13 == 0) {
    FUN_0049e4e0();
  }
  iVar13 = FUN_00407f60();
  if (iVar13 == 0) {
    FUN_0049e4e0();
  }
  iVar13 = FUN_00483940();
  if (iVar13 == 0) {
    FUN_0049e4e0();
  }
  iVar13 = FUN_00483f00();
  if (iVar13 == 0) {
    FUN_0049e4e0();
  }
  FUN_00557040();
  uVar3 = FUN_00557370();
  FUN_004e9a30();
  return uVar3;
}

