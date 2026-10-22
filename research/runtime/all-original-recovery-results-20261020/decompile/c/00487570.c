
undefined4 __fastcall FUN_00487570(char *param_1)

{
  char cVar1;
  int iVar2;
  DWORD DVar3;
  undefined4 uVar4;
  char *pcVar5;
  uint uVar6;
  uint uVar7;
  int iVar8;
  uint uVar9;
  uint uVar10;
  ushort *puVar11;
  char *pcVar12;
  char *pcVar13;
  undefined4 *local_8;
  
  GetCurrentDirectoryA(0x100,&DAT_005fdf18);
  uVar6 = 0xffffffff;
  pcVar5 = &DAT_005fdf18;
  do {
    pcVar13 = pcVar5;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar13 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar13;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar5 = pcVar13 + -uVar6;
  pcVar13 = (char *)&DAT_005fec74;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar13 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar13 = pcVar13 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar13 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar13 = pcVar13 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar5 = (char *)&DAT_005fec74;
  do {
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar5 + 1;
  } while (cVar1 != '\0');
  if (((~uVar6 != 1) && ((&DAT_005fec72)[~uVar6] != '\\')) && ((&DAT_005fec72)[~uVar6] != '/')) {
    uVar6 = 0xffffffff;
    pcVar5 = &DAT_005d9058;
    do {
      pcVar13 = pcVar5;
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      pcVar13 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar13;
    } while (cVar1 != '\0');
    uVar6 = ~uVar6;
    iVar8 = -1;
    pcVar5 = (char *)&DAT_005fec74;
    do {
      pcVar12 = pcVar5;
      if (iVar8 == 0) break;
      iVar8 = iVar8 + -1;
      pcVar12 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar12;
    } while (cVar1 != '\0');
    pcVar5 = pcVar13 + -uVar6;
    pcVar13 = pcVar12 + -1;
    for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
      *(undefined4 *)pcVar13 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar13 = pcVar13 + 4;
    }
    for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
      *pcVar13 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar13 = pcVar13 + 1;
    }
  }
  uVar6 = 0xffffffff;
  do {
    pcVar5 = param_1;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar5 = param_1 + 1;
    cVar1 = *param_1;
    param_1 = pcVar5;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  iVar8 = -1;
  pcVar13 = (char *)&DAT_005fec74;
  do {
    pcVar12 = pcVar13;
    if (iVar8 == 0) break;
    iVar8 = iVar8 + -1;
    pcVar12 = pcVar13 + 1;
    cVar1 = *pcVar13;
    pcVar13 = pcVar12;
  } while (cVar1 != '\0');
  pcVar5 = pcVar5 + -uVar6;
  pcVar13 = pcVar12 + -1;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar13 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar13 = pcVar13 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar13 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar13 = pcVar13 + 1;
  }
  GetPrivateProfileStringA
            (s_HEADER_005da478,s_NumberOfDoors_005e00d8,&DAT_005faf28,&DAT_005fe96c,0x100,
             (LPCSTR)&DAT_005fec74);
  iVar8 = FUN_0059c5a4(&DAT_005fe96c);
  if ((iVar8 < 1) || (iVar8 = FUN_0059c5a4(&DAT_005fe96c), iVar8 < 500)) {
    iVar8 = FUN_0059c5a4(&DAT_005fe96c);
    if (iVar8 < 1) {
      uVar6 = 0;
    }
    else {
      uVar6 = FUN_0059c5a4(&DAT_005fe96c);
    }
  }
  else {
    uVar6 = 500;
  }
  if (DAT_006e1f84 != (undefined4 *)0x0) {
    FUN_00597870(DAT_006e1f84);
  }
  DAT_006e1f84 = (undefined4 *)FUN_00597850(uVar6 << 4);
  uVar7 = 0;
  local_8 = DAT_006e1f84;
  if (uVar6 != 0) {
    do {
      uVar9 = 0xffffffff;
      pcVar5 = &DAT_005e00d0;
      do {
        pcVar13 = pcVar5;
        if (uVar9 == 0) break;
        uVar9 = uVar9 - 1;
        pcVar13 = pcVar5 + 1;
        cVar1 = *pcVar5;
        pcVar5 = pcVar13;
      } while (cVar1 != '\0');
      uVar9 = ~uVar9;
      pcVar5 = pcVar13 + -uVar9;
      pcVar13 = (char *)&DAT_005fe568;
      for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
        *(undefined4 *)pcVar13 = *(undefined4 *)pcVar5;
        pcVar5 = pcVar5 + 4;
        pcVar13 = pcVar13 + 4;
      }
      for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
        *pcVar13 = *pcVar5;
        pcVar5 = pcVar5 + 1;
        pcVar13 = pcVar13 + 1;
      }
      FUN_0059de2f(uVar7,&DAT_005fe96c,10);
      uVar9 = 0xffffffff;
      pcVar5 = &DAT_005fe96c;
      do {
        pcVar13 = pcVar5;
        if (uVar9 == 0) break;
        uVar9 = uVar9 - 1;
        pcVar13 = pcVar5 + 1;
        cVar1 = *pcVar5;
        pcVar5 = pcVar13;
      } while (cVar1 != '\0');
      uVar9 = ~uVar9;
      iVar8 = -1;
      pcVar5 = (char *)&DAT_005fe568;
      do {
        pcVar12 = pcVar5;
        if (iVar8 == 0) break;
        iVar8 = iVar8 + -1;
        pcVar12 = pcVar5 + 1;
        cVar1 = *pcVar5;
        pcVar5 = pcVar12;
      } while (cVar1 != '\0');
      pcVar5 = pcVar13 + -uVar9;
      pcVar13 = pcVar12 + -1;
      for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
        *(undefined4 *)pcVar13 = *(undefined4 *)pcVar5;
        pcVar5 = pcVar5 + 4;
        pcVar13 = pcVar13 + 4;
      }
      for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
        *pcVar13 = *pcVar5;
        pcVar5 = pcVar5 + 1;
        pcVar13 = pcVar13 + 1;
      }
      DVar3 = GetPrivateProfileStringA
                        ((LPCSTR)&DAT_005fe568,s_Height_005e00c8,&DAT_005faf28,&DAT_005fe96c,0x100,
                         (LPCSTR)&DAT_005fec74);
      if (DVar3 != 0) {
        iVar8 = FUN_0059c5a4(&DAT_005fe96c);
        if ((iVar8 < 1) || (iVar8 = FUN_0059c5a4(&DAT_005fe96c), iVar8 < 10)) {
          iVar8 = FUN_0059c5a4(&DAT_005fe96c);
          if (iVar8 < 1) {
            *local_8 = 0;
          }
          else {
            uVar4 = FUN_0059c5a4(&DAT_005fe96c);
            *local_8 = uVar4;
          }
        }
        else {
          *local_8 = 10;
        }
      }
      DVar3 = GetPrivateProfileStringA
                        ((LPCSTR)&DAT_005fe568,s_OpenAnim_005e00bc,&DAT_005faf28,&DAT_005fe96c,0x100
                         ,(LPCSTR)&DAT_005fec74);
      if (DVar3 != 0) {
        iVar8 = FUN_0059c5a4(&DAT_005fe96c);
        if ((iVar8 < 1) || (iVar8 = FUN_0059c5a4(&DAT_005fe96c), iVar8 < 9999)) {
          iVar8 = FUN_0059c5a4(&DAT_005fe96c);
          if (iVar8 < 1) {
            uVar4 = 0;
          }
          else {
            uVar4 = FUN_0059c5a4(&DAT_005fe96c);
          }
        }
        else {
          uVar4 = 9999;
        }
        local_8[1] = uVar4;
      }
      DVar3 = GetPrivateProfileStringA
                        ((LPCSTR)&DAT_005fe568,s_SoundNum_005e00b0,&DAT_005faf28,&DAT_005fe96c,0x100
                         ,(LPCSTR)&DAT_005fec74);
      if (DVar3 != 0) {
        iVar8 = FUN_0059c5a4(&DAT_005fe96c);
        if ((iVar8 < 1) || (iVar8 = FUN_0059c5a4(&DAT_005fe96c), iVar8 < 9999)) {
          iVar8 = FUN_0059c5a4(&DAT_005fe96c);
          if (iVar8 < 1) {
            uVar4 = 0;
          }
          else {
            uVar4 = FUN_0059c5a4(&DAT_005fe96c);
          }
        }
        else {
          uVar4 = 9999;
        }
        local_8[2] = uVar4;
      }
      DVar3 = GetPrivateProfileStringA
                        ((LPCSTR)&DAT_005fe568,s_ObjectNum_005e00a4,&DAT_005faf28,&DAT_005fe96c,
                         0x100,(LPCSTR)&DAT_005fec74);
      if (DVar3 != 0) {
        pcVar5 = _strchr(&DAT_005fe96c,0x3b);
        if (pcVar5 != (char *)0x0) {
          *pcVar5 = '\0';
        }
        pcVar5 = _strchr(&DAT_005fe96c,0x20);
        if (pcVar5 != (char *)0x0) {
          *pcVar5 = '\0';
        }
        iVar8 = __strcmpi(&DAT_005fe96c,&DAT_005e009c);
        if (iVar8 == 0) {
          local_8[3] = 0xffffffff;
        }
        else {
          iVar8 = FUN_0059c5a4(&DAT_005fe96c);
          if ((iVar8 < 1) || (iVar8 = FUN_0059c5a4(&DAT_005fe96c), iVar8 < 9999)) {
            iVar8 = FUN_0059c5a4(&DAT_005fe96c);
            if (iVar8 < 1) {
              uVar4 = 0;
            }
            else {
              uVar4 = FUN_0059c5a4(&DAT_005fe96c);
            }
          }
          else {
            uVar4 = 9999;
          }
          local_8[3] = uVar4;
        }
      }
      uVar7 = uVar7 + 1;
      local_8 = local_8 + 4;
    } while (uVar7 < uVar6);
  }
  puVar11 = (ushort *)(DAT_006c54dc + (DAT_006cb942 + DAT_006cb8c2) * 0xc);
  for (iVar8 = DAT_006c54a4; iVar8 != 0; iVar8 = iVar8 + -1) {
    iVar2 = *(int *)(DAT_0065660c + (uint)*puVar11 * 0x164 + 0xac);
    if ((iVar2 != 0xff) && (DAT_006e1f84[iVar2 * 4 + 3] == -1)) {
      puVar11[5] = puVar11[5] | 0x80;
    }
    puVar11 = puVar11 + 6;
  }
  return 1;
}

