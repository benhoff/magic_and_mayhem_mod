
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004e9a30(void)

{
  char cVar1;
  int iVar2;
  char *pcVar3;
  char *pcVar4;
  LPCSTR lpText;
  HANDLE hFindFile;
  undefined4 uVar5;
  undefined4 uVar6;
  BOOL BVar7;
  uint uVar8;
  uint uVar9;
  CHAR *pCVar10;
  char *pcVar11;
  CHAR *pCVar12;
  UINT uType;
  char local_454 [256];
  undefined4 local_354;
  undefined4 local_350;
  _WIN32_FIND_DATAA local_14c;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c221b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  if (DAT_006578b8 == 0) {
    ExceptionList = &local_c;
    _DAT_006e2028 = FindWindowA(s_SJE_CdPlayerClass_005e4070,(LPCSTR)0x0);
    if (_DAT_006e2028 != (HWND)0x0) {
      PostMessageA(_DAT_006e2028,0x10,0,0);
    }
  }
  if (DAT_006de705 != 0) {
    FUN_00481770();
  }
  if (DAT_006de701 != 0) {
    iVar2 = FUN_0056e640();
    if (iVar2 < 0) {
      pcVar3 = (char *)FUN_00597850(0x100);
      *pcVar3 = '\0';
      pcVar4 = (char *)FUN_0058a6b0();
      uVar8 = 0xffffffff;
      uType = 0x40030;
      do {
        pcVar11 = pcVar4;
        if (uVar8 == 0) break;
        uVar8 = uVar8 - 1;
        pcVar11 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar11;
      } while (cVar1 != '\0');
      uVar8 = ~uVar8;
      pcVar4 = pcVar11 + -uVar8;
      pcVar11 = pcVar3;
      for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar11 = *(undefined4 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        pcVar11 = pcVar11 + 4;
      }
      for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
        *pcVar11 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        pcVar11 = pcVar11 + 1;
      }
      pcVar4 = pcVar3;
      lpText = (LPCSTR)FUN_0056ef00();
      MessageBoxA((HWND)0x0,lpText,pcVar4,uType);
      FUN_00597870(pcVar3);
      DestroyWindow(DAT_00656614);
    }
  }
  FUN_00535970();
  CoUninitialize();
  hFindFile = FindFirstFileA(s_Cfg_Encrypted___cfg_005e4140,&local_14c);
  if (hFindFile != (HANDLE)0xffffffff) {
    do {
      local_354 = 0;
      local_350 = 0;
      uVar8 = 0xffffffff;
      pcVar4 = &DAT_005e4128;
      do {
        pcVar3 = pcVar4;
        if (uVar8 == 0) break;
        uVar8 = uVar8 - 1;
        pcVar3 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar3;
      } while (cVar1 != '\0');
      uVar8 = ~uVar8;
      pcVar4 = pcVar3 + -uVar8;
      pcVar3 = local_454;
      for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar3 = *(undefined4 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        pcVar3 = pcVar3 + 4;
      }
      local_4 = 0;
      for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
        *pcVar3 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        pcVar3 = pcVar3 + 1;
      }
      uVar8 = 0xffffffff;
      pCVar10 = local_14c.cFileName;
      do {
        pCVar12 = pCVar10;
        if (uVar8 == 0) break;
        uVar8 = uVar8 - 1;
        pCVar12 = pCVar10 + 1;
        cVar1 = *pCVar10;
        pCVar10 = pCVar12;
      } while (cVar1 != '\0');
      uVar8 = ~uVar8;
      iVar2 = -1;
      pcVar4 = local_454;
      do {
        pcVar3 = pcVar4;
        if (iVar2 == 0) break;
        iVar2 = iVar2 + -1;
        pcVar3 = pcVar4 + 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar3;
      } while (cVar1 != '\0');
      pCVar10 = pCVar12 + -uVar8;
      pcVar4 = pcVar3 + -1;
      for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
        *(undefined4 *)pcVar4 = *(undefined4 *)pCVar10;
        pCVar10 = pCVar10 + 4;
        pcVar4 = pcVar4 + 4;
      }
      for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
        *pcVar4 = *pCVar10;
        pCVar10 = pCVar10 + 1;
        pcVar4 = pcVar4 + 1;
      }
      iVar2 = FUN_004a1360(local_454,&DAT_005d96e0);
      if (iVar2 != 0) {
        uVar5 = FUN_004a1590(0);
        uVar6 = FUN_00597850(uVar5);
        FUN_004a1760(uVar6,uVar5);
        FUN_004a1550();
        uVar8 = 0xffffffff;
        pcVar4 = s_Cfg_Encrypted__005e4130;
        do {
          pcVar3 = pcVar4;
          if (uVar8 == 0) break;
          uVar8 = uVar8 - 1;
          pcVar3 = pcVar4 + 1;
          cVar1 = *pcVar4;
          pcVar4 = pcVar3;
        } while (cVar1 != '\0');
        uVar8 = ~uVar8;
        pcVar4 = pcVar3 + -uVar8;
        pcVar3 = local_454;
        for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
          *(undefined4 *)pcVar3 = *(undefined4 *)pcVar4;
          pcVar4 = pcVar4 + 4;
          pcVar3 = pcVar3 + 4;
        }
        for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
          *pcVar3 = *pcVar4;
          pcVar4 = pcVar4 + 1;
          pcVar3 = pcVar3 + 1;
        }
        uVar8 = 0xffffffff;
        pCVar10 = local_14c.cFileName;
        do {
          pCVar12 = pCVar10;
          if (uVar8 == 0) break;
          uVar8 = uVar8 - 1;
          pCVar12 = pCVar10 + 1;
          cVar1 = *pCVar10;
          pCVar10 = pCVar12;
        } while (cVar1 != '\0');
        uVar8 = ~uVar8;
        iVar2 = -1;
        pcVar4 = local_454;
        do {
          pcVar3 = pcVar4;
          if (iVar2 == 0) break;
          iVar2 = iVar2 + -1;
          pcVar3 = pcVar4 + 1;
          cVar1 = *pcVar4;
          pcVar4 = pcVar3;
        } while (cVar1 != '\0');
        pCVar10 = pCVar12 + -uVar8;
        pcVar4 = pcVar3 + -1;
        for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
          *(undefined4 *)pcVar4 = *(undefined4 *)pCVar10;
          pCVar10 = pCVar10 + 4;
          pcVar4 = pcVar4 + 4;
        }
        for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
          *pcVar4 = *pCVar10;
          pCVar10 = pCVar10 + 1;
          pcVar4 = pcVar4 + 1;
        }
        iVar2 = FUN_004a1360(local_454,&DAT_005d9e54);
        if (iVar2 != 0) {
          iVar2 = FUN_004a17c0(uVar6,uVar5);
          FUN_004a1550();
          if (iVar2 != 0) {
            uVar8 = 0xffffffff;
            pcVar4 = &DAT_005e4128;
            do {
              pcVar3 = pcVar4;
              if (uVar8 == 0) break;
              uVar8 = uVar8 - 1;
              pcVar3 = pcVar4 + 1;
              cVar1 = *pcVar4;
              pcVar4 = pcVar3;
            } while (cVar1 != '\0');
            uVar8 = ~uVar8;
            pcVar4 = pcVar3 + -uVar8;
            pcVar3 = local_454;
            for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
              *(undefined4 *)pcVar3 = *(undefined4 *)pcVar4;
              pcVar4 = pcVar4 + 4;
              pcVar3 = pcVar3 + 4;
            }
            for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
              *pcVar3 = *pcVar4;
              pcVar4 = pcVar4 + 1;
              pcVar3 = pcVar3 + 1;
            }
            uVar8 = 0xffffffff;
            pCVar10 = local_14c.cFileName;
            do {
              pCVar12 = pCVar10;
              if (uVar8 == 0) break;
              uVar8 = uVar8 - 1;
              pCVar12 = pCVar10 + 1;
              cVar1 = *pCVar10;
              pCVar10 = pCVar12;
            } while (cVar1 != '\0');
            uVar8 = ~uVar8;
            iVar2 = -1;
            pcVar4 = local_454;
            do {
              pcVar3 = pcVar4;
              if (iVar2 == 0) break;
              iVar2 = iVar2 + -1;
              pcVar3 = pcVar4 + 1;
              cVar1 = *pcVar4;
              pcVar4 = pcVar3;
            } while (cVar1 != '\0');
            pCVar10 = pCVar12 + -uVar8;
            pcVar4 = pcVar3 + -1;
            for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
              *(undefined4 *)pcVar4 = *(undefined4 *)pCVar10;
              pCVar10 = pCVar10 + 4;
              pcVar4 = pcVar4 + 4;
            }
            for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
              *pcVar4 = *pCVar10;
              pCVar10 = pCVar10 + 1;
              pcVar4 = pcVar4 + 1;
            }
            DeleteFileA(local_454);
          }
        }
        FUN_00597870(uVar6);
      }
      local_4 = 0xffffffff;
      FUN_004a1550();
      BVar7 = FindNextFileA(hFindFile,&local_14c);
    } while (BVar7 != 0);
    FindClose(hFindFile);
  }
  ExceptionList = local_c;
  return;
}

