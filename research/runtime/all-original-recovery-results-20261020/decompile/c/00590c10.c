
undefined4 __thiscall FUN_00590c10(undefined4 *param_1,char *param_2,int param_3,undefined4 param_4)

{
  char cVar1;
  byte bVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  DWORD DVar7;
  char *pcVar8;
  undefined4 *puVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  uint *puVar13;
  int iVar14;
  int iVar15;
  byte *lpReturnedString;
  byte *pbVar16;
  undefined4 *puVar17;
  char *pcVar18;
  char *pcVar19;
  bool bVar20;
  int local_18;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2f1b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  *(undefined4 *)((int)param_1 + 0x92a) = 0;
  puVar9 = param_1 + 0x23;
  for (iVar10 = 0x19; iVar10 != 0; iVar10 = iVar10 + -1) {
    *puVar9 = 0xffffffff;
    puVar9 = puVar9 + 1;
  }
  GetCurrentDirectoryA(0x100,&DAT_006f9fe4);
  uVar11 = 0xffffffff;
  pcVar8 = &DAT_006f9fe4;
  do {
    pcVar19 = pcVar8;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar19 = pcVar8 + 1;
    cVar1 = *pcVar8;
    pcVar8 = pcVar19;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  pcVar8 = pcVar19 + -uVar11;
  pcVar19 = (char *)&DAT_006f98e0;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
    pcVar8 = pcVar8 + 4;
    pcVar19 = pcVar19 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar19 = *pcVar8;
    pcVar8 = pcVar8 + 1;
    pcVar19 = pcVar19 + 1;
  }
  uVar11 = 0xffffffff;
  pcVar8 = (char *)&DAT_006f98e0;
  do {
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    cVar1 = *pcVar8;
    pcVar8 = pcVar8 + 1;
  } while (cVar1 != '\0');
  if (((~uVar11 != 1) && (cVar1 = *(char *)((int)&DAT_006f98dc + ~uVar11 + 2), cVar1 != '\\')) &&
     (cVar1 != '/')) {
    uVar11 = 0xffffffff;
    pcVar8 = &DAT_005d9058;
    do {
      pcVar19 = pcVar8;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar19 = pcVar8 + 1;
      cVar1 = *pcVar8;
      pcVar8 = pcVar19;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    iVar10 = -1;
    pcVar8 = (char *)&DAT_006f98e0;
    do {
      pcVar18 = pcVar8;
      if (iVar10 == 0) break;
      iVar10 = iVar10 + -1;
      pcVar18 = pcVar8 + 1;
      cVar1 = *pcVar8;
      pcVar8 = pcVar18;
    } while (cVar1 != '\0');
    pcVar8 = pcVar19 + -uVar11;
    pcVar19 = pcVar18 + -1;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
      pcVar8 = pcVar8 + 4;
      pcVar19 = pcVar19 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar19 = *pcVar8;
      pcVar8 = pcVar8 + 1;
      pcVar19 = pcVar19 + 1;
    }
  }
  uVar11 = 0xffffffff;
  pcVar8 = s_wizards__005f1d90;
  do {
    pcVar19 = pcVar8;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar19 = pcVar8 + 1;
    cVar1 = *pcVar8;
    pcVar8 = pcVar19;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  iVar10 = -1;
  pcVar8 = (char *)&DAT_006f98e0;
  do {
    pcVar18 = pcVar8;
    if (iVar10 == 0) break;
    iVar10 = iVar10 + -1;
    pcVar18 = pcVar8 + 1;
    cVar1 = *pcVar8;
    pcVar8 = pcVar18;
  } while (cVar1 != '\0');
  pcVar8 = pcVar19 + -uVar11;
  pcVar19 = pcVar18 + -1;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
    pcVar8 = pcVar8 + 4;
    pcVar19 = pcVar19 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar19 = *pcVar8;
    pcVar8 = pcVar8 + 1;
    pcVar19 = pcVar19 + 1;
  }
  uVar11 = 0xffffffff;
  do {
    pcVar8 = param_2;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar8 = param_2 + 1;
    cVar1 = *param_2;
    param_2 = pcVar8;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  iVar10 = -1;
  pcVar19 = (char *)&DAT_006f98e0;
  do {
    pcVar18 = pcVar19;
    if (iVar10 == 0) break;
    iVar10 = iVar10 + -1;
    pcVar18 = pcVar19 + 1;
    cVar1 = *pcVar19;
    pcVar19 = pcVar18;
  } while (cVar1 != '\0');
  pcVar8 = pcVar8 + -uVar11;
  pcVar19 = pcVar18 + -1;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
    pcVar8 = pcVar8 + 4;
    pcVar19 = pcVar19 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar19 = *pcVar8;
    pcVar8 = pcVar8 + 1;
    pcVar19 = pcVar19 + 1;
  }
  lpReturnedString = (byte *)(param_1 + 3);
  GetPrivateProfileStringA
            (s_GENERAL_005e0158,&DAT_005e0160,&DAT_005faf28,(LPSTR)lpReturnedString,0x100,
             (LPCSTR)&DAT_006f98e0);
  pbVar16 = &DAT_005faf28;
  do {
    bVar2 = *lpReturnedString;
    bVar20 = bVar2 < *pbVar16;
    if (bVar2 != *pbVar16) {
LAB_00590d76:
      iVar10 = (1 - (uint)bVar20) - (uint)(bVar20 != 0);
      goto LAB_00590d7b;
    }
    if (bVar2 == 0) break;
    bVar2 = lpReturnedString[1];
    bVar20 = bVar2 < pbVar16[1];
    if (bVar2 != pbVar16[1]) goto LAB_00590d76;
    lpReturnedString = lpReturnedString + 2;
    pbVar16 = pbVar16 + 2;
  } while (bVar2 != 0);
  iVar10 = 0;
LAB_00590d7b:
  if (iVar10 == 0) {
    MessageBoxA((HWND)0x0,(LPCSTR)&DAT_006f98e0,s_FAILED_TO_LOAD_WIZARD_005f1d78,0x42030);
    uVar6 = 0;
  }
  else {
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,&DAT_005e0160,&DAT_005faf28,(LPSTR)(param_1 + 3),0x100,
               (LPCSTR)&DAT_006f98e0);
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_MaxMana_005f1d70,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
    param_1[0xf] = uVar6;
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_MaxHealth_005f1d64,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = FUN_0059c5a4(&DAT_006f9ae0);
    if (iVar10 != 0) {
      param_1[0x10] = iVar10;
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_ControlLimit_005e08a0,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = FUN_0059c5a4(&DAT_006f9ae0);
    if (iVar10 != 0) {
      param_1[0x11] = iVar10;
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_Intelligence_005e9a54,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = FUN_0059c5a4(&DAT_006f9ae0);
    if (iVar10 != 0) {
      param_1[0x13] = iVar10;
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_MagicResistance_005f1d54,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = FUN_0059c5a4(&DAT_006f9ae0);
    if (iVar10 != 0) {
      param_1[0x12] = iVar10;
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_CombatModifier_005f1d44,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = FUN_0059c5a4(&DAT_006f9ae0);
    if (iVar10 != 0) {
      param_1[0x14] = iVar10;
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_DifficultyModifier_005f1d30,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = FUN_0059c5a4(&DAT_006f9ae0);
    if (iVar10 != 0) {
      param_1[0x15] = iVar10;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_AggressiveAttack_005e3400,&DAT_005faf28,&DAT_006f9ae0,
                       0x100,(LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x18] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_CautiousAttack_005e33e4,&DAT_005faf28,&DAT_006f9ae0,
                       0x100,(LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x19] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_OccupyPowerPoint_005e33d0,&DAT_005faf28,&DAT_006f9ae0,
                       0x100,(LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x1a] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_CollectMana_005e33c4,&DAT_005faf28,&DAT_006f9ae0,0x100,
                       (LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x1b] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_EvadeDetection_005e33b4,&DAT_005faf28,&DAT_006f9ae0,
                       0x100,(LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x1c] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_Scout_005e33ac,&DAT_005faf28,&DAT_006f9ae0,0x100,
                       (LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x1d] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_Retreat_005e33a4,&DAT_005faf28,&DAT_006f9ae0,0x100,
                       (LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x1e] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_CollectFood_005e3398,&DAT_005faf28,&DAT_006f9ae0,0x100,
                       (LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x1f] = uVar6;
    }
    DVar7 = GetPrivateProfileStringA
                      (s_GENERAL_005e0158,s_ProtectWizard_005e3388,&DAT_005faf28,&DAT_006f9ae0,0x100
                       ,(LPCSTR)&DAT_006f98e0);
    if (DVar7 != 0) {
      uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
      param_1[0x20] = uVar6;
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_WizardAnimFile_005f1d20,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    iVar10 = __strcmpi(&DAT_006f9ae0,&DAT_005faf28);
    if (iVar10 != 0) {
      uVar11 = 0xffffffff;
      pcVar8 = &DAT_006f9ae0;
      do {
        pcVar19 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar19 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar19;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      pcVar8 = pcVar19 + -uVar11;
      pcVar19 = (char *)(param_1 + 0x41);
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar19 = pcVar19 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar19 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar19 = pcVar19 + 1;
      }
    }
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_startTalismans_Law_005f1d0c,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
    param_1[0x3c] = uVar6;
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_startTalismans_Neutral_005f1cf4,&DAT_005faf28,&DAT_006f9ae0,
               0x100,(LPCSTR)&DAT_006f98e0);
    uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
    param_1[0x3d] = uVar6;
    GetPrivateProfileStringA
              (s_GENERAL_005e0158,s_startTalismans_Chaos_005f1cdc,&DAT_005faf28,&DAT_006f9ae0,0x100,
               (LPCSTR)&DAT_006f98e0);
    uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
    param_1[0x3e] = uVar6;
    param_2 = (char *)0x0;
    do {
      if ((((DAT_006e2030 == '\0') || (param_2 == (char *)0x1)) ||
          ((param_2 == (char *)0x2 || ((param_2 == (char *)0x3 || (param_2 == (char *)0x4)))))) ||
         (param_2 == (char *)0x11)) {
        uVar11 = 0xffffffff;
        pcVar8 = s_MITEM__005f1cd4;
        do {
          pcVar19 = pcVar8;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pcVar19 = pcVar8 + 1;
          cVar1 = *pcVar8;
          pcVar8 = pcVar19;
        } while (cVar1 != '\0');
        uVar11 = ~uVar11;
        pcVar8 = pcVar19 + -uVar11;
        pcVar19 = (char *)&DAT_006f9ce0;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
          pcVar8 = pcVar8 + 4;
          pcVar19 = pcVar19 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar19 = *pcVar8;
          pcVar8 = pcVar8 + 1;
          pcVar19 = pcVar19 + 1;
        }
        FUN_0059c1fd(&DAT_006f9ae0,&DAT_005f19c0,param_2);
        uVar11 = 0xffffffff;
        pcVar8 = &DAT_006f9ae0;
        do {
          pcVar19 = pcVar8;
          if (uVar11 == 0) break;
          uVar11 = uVar11 - 1;
          pcVar19 = pcVar8 + 1;
          cVar1 = *pcVar8;
          pcVar8 = pcVar19;
        } while (cVar1 != '\0');
        uVar11 = ~uVar11;
        iVar10 = -1;
        pcVar8 = (char *)&DAT_006f9ce0;
        do {
          pcVar18 = pcVar8;
          if (iVar10 == 0) break;
          iVar10 = iVar10 + -1;
          pcVar18 = pcVar8 + 1;
          cVar1 = *pcVar8;
          pcVar8 = pcVar18;
        } while (cVar1 != '\0');
        pcVar8 = pcVar19 + -uVar11;
        pcVar19 = pcVar18 + -1;
        for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
          *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
          pcVar8 = pcVar8 + 4;
          pcVar19 = pcVar19 + 4;
        }
        for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
          *pcVar19 = *pcVar8;
          pcVar8 = pcVar8 + 1;
          pcVar19 = pcVar19 + 1;
        }
        DVar7 = GetPrivateProfileStringA
                          (s_START_MAGIC_ITEMS_005f1cc0,(LPCSTR)&DAT_006f9ce0,&DAT_005faf28,
                           &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
        if (DVar7 != 0) {
          pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
          if (pcVar8 != (char *)0x0) {
            *pcVar8 = '\0';
          }
          pcVar8 = _strchr(&DAT_006f9ae0,0x20);
          if (pcVar8 != (char *)0x0) {
            *pcVar8 = '\0';
          }
          pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005e01d0);
          if (pcVar8 != (char *)0x0) {
            iVar10 = 0;
            puVar13 = param_1 + 0x23;
            do {
              if ((char *)*puVar13 == param_2) break;
              if (*puVar13 == 0xffffffff) {
                param_1[iVar10 + 0x23] = param_2;
                break;
              }
              iVar10 = iVar10 + 1;
              puVar13 = puVar13 + 1;
            } while (iVar10 < 0x19);
          }
        }
      }
      param_2 = (char *)((int)param_2 + 1);
    } while (param_2 < 0x19);
    param_2 = (char *)0x0;
    do {
      uVar11 = 0xffffffff;
      pcVar8 = &DAT_005f1424;
      do {
        pcVar19 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar19 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar19;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      pcVar8 = pcVar19 + -uVar11;
      pcVar19 = (char *)&DAT_006f9ce0;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar19 = pcVar19 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar19 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar19 = pcVar19 + 1;
      }
      FUN_0059c1fd(&DAT_006f9ae0,&DAT_005f19c0,param_2);
      uVar11 = 0xffffffff;
      pcVar8 = &DAT_006f9ae0;
      do {
        pcVar19 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar19 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar19;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      iVar10 = -1;
      pcVar8 = (char *)&DAT_006f9ce0;
      do {
        pcVar18 = pcVar8;
        if (iVar10 == 0) break;
        iVar10 = iVar10 + -1;
        pcVar18 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar18;
      } while (cVar1 != '\0');
      pcVar8 = pcVar19 + -uVar11;
      pcVar19 = pcVar18 + -1;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar19 = pcVar19 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar19 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar19 = pcVar19 + 1;
      }
      GetPrivateProfileStringA
                (s_START_SPELLS_005f1cb0,(LPCSTR)&DAT_006f9ce0,&DAT_005faf28,&DAT_006f9ae0,0x100,
                 (LPCSTR)&DAT_006f98e0);
      pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
      if (pcVar8 != (char *)0x0) {
        *pcVar8 = '\0';
      }
      pcVar8 = _strchr(&DAT_006f9ae0,0x20);
      if (pcVar8 != (char *)0x0) {
        *pcVar8 = '\0';
      }
      pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005e01d0);
      if (pcVar8 != (char *)0x0) {
        FUN_00590310(param_2);
      }
      param_2 = (char *)((int)param_2 + 1);
    } while (param_2 < 0x68);
    *(undefined4 *)((int)param_1 + 0x53a) = 0;
    param_2 = (char *)0x0;
    if (DAT_006b1254 != 0) {
      local_18 = 0;
      do {
        if (((DAT_006e2030 == '\0') || (local_18 == 0x100)) || (local_18 == 0x340)) {
          uVar11 = 0xffffffff;
          pcVar8 = s_OBJECT__005eb18c;
          do {
            pcVar19 = pcVar8;
            if (uVar11 == 0) break;
            uVar11 = uVar11 - 1;
            pcVar19 = pcVar8 + 1;
            cVar1 = *pcVar8;
            pcVar8 = pcVar19;
          } while (cVar1 != '\0');
          uVar11 = ~uVar11;
          pcVar8 = pcVar19 + -uVar11;
          pcVar19 = (char *)&DAT_006f9ce0;
          for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
            *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
            pcVar8 = pcVar8 + 4;
            pcVar19 = pcVar19 + 4;
          }
          for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
            *pcVar19 = *pcVar8;
            pcVar8 = pcVar8 + 1;
            pcVar19 = pcVar19 + 1;
          }
          FUN_0059c1fd(&DAT_006f9ae0,&DAT_005f19c0,param_2);
          uVar11 = 0xffffffff;
          pcVar8 = &DAT_006f9ae0;
          do {
            pcVar19 = pcVar8;
            if (uVar11 == 0) break;
            uVar11 = uVar11 - 1;
            pcVar19 = pcVar8 + 1;
            cVar1 = *pcVar8;
            pcVar8 = pcVar19;
          } while (cVar1 != '\0');
          uVar11 = ~uVar11;
          iVar10 = -1;
          pcVar8 = (char *)&DAT_006f9ce0;
          do {
            pcVar18 = pcVar8;
            if (iVar10 == 0) break;
            iVar10 = iVar10 + -1;
            pcVar18 = pcVar8 + 1;
            cVar1 = *pcVar8;
            pcVar8 = pcVar18;
          } while (cVar1 != '\0');
          pcVar8 = pcVar19 + -uVar11;
          pcVar19 = pcVar18 + -1;
          for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
            *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
            pcVar8 = pcVar8 + 4;
            pcVar19 = pcVar19 + 4;
          }
          for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
            *pcVar19 = *pcVar8;
            pcVar8 = pcVar8 + 1;
            pcVar19 = pcVar19 + 1;
          }
          GetPrivateProfileStringA
                    (s_START_OBJECTS_005f1ca0,(LPCSTR)&DAT_006f9ce0,&DAT_005faf28,&DAT_006f9ae0,
                     0x100,(LPCSTR)&DAT_006f98e0);
          pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
          if (pcVar8 != (char *)0x0) {
            *pcVar8 = '\0';
          }
          pcVar8 = _strchr(&DAT_006f9ae0,0x20);
          if (pcVar8 != (char *)0x0) {
            *pcVar8 = '\0';
          }
          pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005e01d0);
          if (pcVar8 != (char *)0x0) {
            if (param_2 < DAT_006b1254) {
              puVar9 = (undefined4 *)(DAT_006b1250 + local_18);
            }
            else {
              puVar9 = (undefined4 *)0x0;
            }
            uVar6 = puVar9[3];
            uVar3 = puVar9[2];
            uVar4 = puVar9[1];
            uVar5 = *puVar9;
            if (*(uint *)((int)param_1 + 0x53a) < 0x2a) {
              *(char **)((int)param_1 + *(uint *)((int)param_1 + 0x53a) * 0x18 + 0x14a) = param_2;
              *(undefined4 *)((int)param_1 + *(int *)((int)param_1 + 0x53a) * 0x18 + 0x14e) = uVar5;
              *(undefined4 *)((int)param_1 + *(int *)((int)param_1 + 0x53a) * 0x18 + 0x152) = uVar4;
              *(undefined4 *)((int)param_1 + *(int *)((int)param_1 + 0x53a) * 0x18 + 0x156) = uVar3;
              *(undefined4 *)((int)param_1 + *(int *)((int)param_1 + 0x53a) * 0x18 + 0x15a) = uVar6;
              *(int *)((int)param_1 + 0x53a) = *(int *)((int)param_1 + 0x53a) + 1;
            }
          }
        }
        param_2 = (char *)((int)param_2 + 1);
        local_18 = local_18 + 0x20;
      } while (param_2 < DAT_006b1254);
    }
    iVar10 = 0;
    do {
      puVar9 = (undefined4 *)((int)param_1 + iVar10 * 100 + 0x53e);
      puVar17 = puVar9;
      for (iVar14 = 0x19; iVar14 != 0; iVar14 = iVar14 + -1) {
        *puVar17 = 0;
        puVar17 = puVar17 + 1;
      }
      uVar11 = 0xffffffff;
      *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x55e) = 0xffffffff;
      *puVar9 = 0;
      pcVar8 = s_ACTION__005f1c98;
      do {
        pcVar19 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar19 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar19;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      pcVar8 = pcVar19 + -uVar11;
      pcVar19 = (char *)&DAT_006f9ce0;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar19 = pcVar19 + 4;
      }
      iVar14 = iVar10 + 1;
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar19 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar19 = pcVar19 + 1;
      }
      FUN_0059c1fd(&DAT_006f9ae0,&DAT_005f19c0,iVar14);
      uVar11 = 0xffffffff;
      pcVar8 = &DAT_006f9ae0;
      do {
        pcVar19 = pcVar8;
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        pcVar19 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar19;
      } while (cVar1 != '\0');
      uVar11 = ~uVar11;
      iVar15 = -1;
      pcVar8 = (char *)&DAT_006f9ce0;
      do {
        pcVar18 = pcVar8;
        if (iVar15 == 0) break;
        iVar15 = iVar15 + -1;
        pcVar18 = pcVar8 + 1;
        cVar1 = *pcVar8;
        pcVar8 = pcVar18;
      } while (cVar1 != '\0');
      pcVar8 = pcVar19 + -uVar11;
      pcVar19 = pcVar18 + -1;
      for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *(undefined4 *)pcVar19 = *(undefined4 *)pcVar8;
        pcVar8 = pcVar8 + 4;
        pcVar19 = pcVar19 + 4;
      }
      for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *pcVar19 = *pcVar8;
        pcVar8 = pcVar8 + 1;
        pcVar19 = pcVar19 + 1;
      }
      DVar7 = GetPrivateProfileStringA
                        ((LPCSTR)&DAT_006f9ce0,s_Valid_005ecf0c,&DAT_005faf28,&DAT_006f9ae0,0x100,
                         (LPCSTR)&DAT_006f98e0);
      if (DVar7 != 0) {
        pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
        if (pcVar8 != (char *)0x0) {
          *pcVar8 = '\0';
        }
        pcVar8 = _strchr(&DAT_006f9ae0,0x20);
        if (pcVar8 != (char *)0x0) {
          *pcVar8 = '\0';
        }
        pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005e01d0);
        if (pcVar8 != (char *)0x0) {
          *puVar9 = 1;
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_Destination_005f1c8c,&DAT_005faf28,
                             &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strstr(&DAT_006f9ae0,s_Section_005f1c84);
            if (pcVar8 == (char *)0x0) {
              pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005f1c4c);
              if (pcVar8 != (char *)0x0) {
                *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x542) = 2;
                DVar7 = GetPrivateProfileStringA
                                  ((LPCSTR)&DAT_006f9ce0,s_NodeType_005f1c40,&DAT_005faf28,
                                   &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
                if (DVar7 != 0) {
                  pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005f1c38);
                  if (pcVar8 != (char *)0x0) {
                    *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x556) = 1;
                  }
                }
              }
            }
            else {
              *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x542) = 1;
              DVar7 = GetPrivateProfileStringA
                                ((LPCSTR)&DAT_006f9ce0,s_SectionID_005f1c78,&DAT_005faf28,
                                 &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
              if (DVar7 != 0) {
                pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x546) = uVar6;
              }
              DVar7 = GetPrivateProfileStringA
                                ((LPCSTR)&DAT_006f9ce0,s_Xposition_005f1c6c,&DAT_005faf28,
                                 &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
              if (DVar7 != 0) {
                pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x54a) = uVar6;
              }
              DVar7 = GetPrivateProfileStringA
                                ((LPCSTR)&DAT_006f9ce0,s_Yposition_005f1c60,&DAT_005faf28,
                                 &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
              if (DVar7 != 0) {
                pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x54e) = uVar6;
              }
              DVar7 = GetPrivateProfileStringA
                                ((LPCSTR)&DAT_006f9ce0,s_Zposition_005f1c54,&DAT_005faf28,
                                 &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
              if (DVar7 != 0) {
                pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                if (pcVar8 != (char *)0x0) {
                  *pcVar8 = '\0';
                }
                uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x552) = uVar6;
              }
            }
            DVar7 = GetPrivateProfileStringA
                              ((LPCSTR)&DAT_006f9ce0,s_WaitingTime_005f1c2c,&DAT_005faf28,
                               &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
            if (DVar7 != 0) {
              pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
              if (pcVar8 != (char *)0x0) {
                *pcVar8 = '\0';
              }
              pcVar8 = _strchr(&DAT_006f9ae0,0x20);
              if (pcVar8 != (char *)0x0) {
                *pcVar8 = '\0';
              }
              iVar15 = FUN_0059c5a4(&DAT_006f9ae0);
              *(int *)((int)param_1 + iVar10 * 100 + 0x55a) = iVar15 * 0x11;
            }
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_SpellType_005f1c20,&DAT_005faf28,&DAT_006f9ae0,
                             0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            uVar11 = FUN_0059c5a4(&DAT_006f9ae0);
            if (((int)uVar11 < 1) || (0x67 < uVar11)) {
              uVar11 = 0xffffffff;
            }
            *(uint *)((int)param_1 + iVar10 * 100 + 0x55e) = uVar11;
            DVar7 = GetPrivateProfileStringA
                              ((LPCSTR)&DAT_006f9ce0,s_SpellTarget_005ec770,&DAT_005faf28,
                               &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
            if (DVar7 != 0) {
              pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
              if (pcVar8 != (char *)0x0) {
                *pcVar8 = '\0';
              }
              pcVar8 = _strchr(&DAT_006f9ae0,0x20);
              if (pcVar8 != (char *)0x0) {
                *pcVar8 = '\0';
              }
              pcVar8 = _strstr(&DAT_006f9ae0,s_Section_005f1c84);
              if (pcVar8 == (char *)0x0) {
                pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005f1c4c);
                if (pcVar8 == (char *)0x0) {
                  pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005f1bc8);
                  if (pcVar8 == (char *)0x0) {
                    pcVar8 = _strstr(&DAT_006f9ae0,s_Enemy_005f1bc0);
                    if (pcVar8 == (char *)0x0) {
                      pcVar8 = _strstr(&DAT_006f9ae0,s_Friend_005f1bb8);
                      if (pcVar8 != (char *)0x0) {
                        *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x562) = 5;
                      }
                    }
                    else {
                      *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x562) = 4;
                    }
                  }
                  else {
                    *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x562) = 3;
                  }
                }
                else {
                  *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x562) = 2;
                  DVar7 = GetPrivateProfileStringA
                                    ((LPCSTR)&DAT_006f9ce0,s_TargetNodeType_005f1bd0,&DAT_005faf28,
                                     &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
                  if (DVar7 != 0) {
                    pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                    if (pcVar8 != (char *)0x0) {
                      *pcVar8 = '\0';
                    }
                    pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                    if (pcVar8 != (char *)0x0) {
                      *pcVar8 = '\0';
                    }
                    pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005f1c38);
                    if (pcVar8 != (char *)0x0) {
                      *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x576) = 1;
                    }
                  }
                }
              }
              else {
                *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x562) = 1;
                DVar7 = GetPrivateProfileStringA
                                  ((LPCSTR)&DAT_006f9ce0,s_SpellTargetID_005f1c10,&DAT_005faf28,
                                   &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
                if (DVar7 != 0) {
                  pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                  *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x566) = uVar6;
                }
                DVar7 = GetPrivateProfileStringA
                                  ((LPCSTR)&DAT_006f9ce0,s_XtargetPosition_005f1c00,&DAT_005faf28,
                                   &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
                if (DVar7 != 0) {
                  pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                  *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x56a) = uVar6;
                }
                DVar7 = GetPrivateProfileStringA
                                  ((LPCSTR)&DAT_006f9ce0,s_YtargetPosition_005f1bf0,&DAT_005faf28,
                                   &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
                if (DVar7 != 0) {
                  pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                  *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x56e) = uVar6;
                }
                DVar7 = GetPrivateProfileStringA
                                  ((LPCSTR)&DAT_006f9ce0,s_ZtargetPosition_005f1be0,&DAT_005faf28,
                                   &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
                if (DVar7 != 0) {
                  pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  pcVar8 = _strchr(&DAT_006f9ae0,0x20);
                  if (pcVar8 != (char *)0x0) {
                    *pcVar8 = '\0';
                  }
                  uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
                  *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x572) = uVar6;
                }
              }
            }
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,&DAT_005f1bb0,&DAT_005faf28,&DAT_006f9ae0,0x100,
                             (LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
            *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x57a) = uVar6;
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_Health_005e9a34,&DAT_005faf28,&DAT_006f9ae0,
                             0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
            *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x57e) = uVar6;
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_BeginTime_005f1ba4,&DAT_005faf28,&DAT_006f9ae0,
                             0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            iVar15 = FUN_0059c5a4(&DAT_006f9ae0);
            *(int *)((int)param_1 + iVar10 * 100 + 0x582) = iVar15 * 0x11;
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_EndTime_005f1b9c,&DAT_005faf28,&DAT_006f9ae0,
                             0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            iVar15 = FUN_0059c5a4(&DAT_006f9ae0);
            *(int *)((int)param_1 + iVar10 * 100 + 0x586) = iVar15 * 0x11;
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_NumberOfActions_005ece44,&DAT_005faf28,
                             &DAT_006f9ae0,0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strstr(&DAT_006f9ae0,s_Infinite_005f1b90);
            if (pcVar8 == (char *)0x0) {
              uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
            }
            else {
              uVar6 = 0xffffffff;
            }
            *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x58a) = uVar6;
            *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x58e) = uVar6;
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_Dependancy_005f1b84,&DAT_005faf28,&DAT_006f9ae0
                             ,0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            uVar6 = FUN_0059c5a4(&DAT_006f9ae0);
            *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x596) = uVar6;
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_Rating_005ec844,&DAT_005faf28,&DAT_006f9ae0,
                             0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            iVar15 = FUN_0059c5a4(&DAT_006f9ae0);
            *(float *)((int)param_1 + iVar10 * 100 + 0x59e) = (float)((iVar15 + -0x32) / 0x32);
          }
          DVar7 = GetPrivateProfileStringA
                            ((LPCSTR)&DAT_006f9ce0,s_Interrupt_005ec858,&DAT_005faf28,&DAT_006f9ae0,
                             0x100,(LPCSTR)&DAT_006f98e0);
          if (DVar7 != 0) {
            pcVar8 = _strchr(&DAT_006f9ae0,0x3b);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strchr(&DAT_006f9ae0,0x20);
            if (pcVar8 != (char *)0x0) {
              *pcVar8 = '\0';
            }
            pcVar8 = _strstr(&DAT_006f9ae0,&DAT_005e01d0);
            if (pcVar8 != (char *)0x0) {
              *(undefined4 *)((int)param_1 + iVar10 * 100 + 0x59a) = 1;
            }
          }
        }
      }
      iVar10 = iVar14;
    } while (iVar14 < 10);
    param_1[0x21] = param_4;
    *param_1 = 1;
    param_1[2] = param_3;
    if ((param_3 == 0) && (*(int *)((int)param_1 + 0x936) == 0)) {
      iVar10 = FUN_00597850(0x57c);
      local_4 = 0;
      if (iVar10 == 0) {
        uVar6 = 0;
      }
      else {
        uVar6 = FUN_004c3600();
      }
      *(undefined4 *)((int)param_1 + 0x936) = uVar6;
    }
    uVar6 = 1;
  }
  ExceptionList = local_c;
  return uVar6;
}

