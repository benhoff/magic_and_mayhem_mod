
/* WARNING: Removing unreachable block (ram,0x005590d8) */
/* WARNING: Removing unreachable block (ram,0x00558bae) */
/* WARNING: Removing unreachable block (ram,0x00558af6) */
/* WARNING: Removing unreachable block (ram,0x0055897a) */
/* WARNING: Removing unreachable block (ram,0x0055891e) */
/* WARNING: Removing unreachable block (ram,0x005589d6) */
/* WARNING: Removing unreachable block (ram,0x00558b52) */
/* WARNING: Removing unreachable block (ram,0x00559071) */
/* WARNING: Removing unreachable block (ram,0x00559140) */

undefined4 __fastcall
FUN_00558580(char *param_1,undefined4 *param_2,undefined4 param_3,undefined4 param_4)

{
  char cVar1;
  bool bVar2;
  DWORD DVar3;
  char *pcVar4;
  char *pcVar5;
  undefined4 uVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  int iVar10;
  uint uVar11;
  uint uVar12;
  int iVar13;
  code *pcVar14;
  undefined4 *puVar15;
  char *pcVar16;
  undefined4 *puVar17;
  char local_200 [256];
  char local_100 [256];
  
  GetCurrentDirectoryA(0x100,&DAT_006f336c);
  uVar11 = 0xffffffff;
  pcVar4 = &DAT_006f336c;
  do {
    pcVar5 = pcVar4;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar5 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar5;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  pcVar4 = pcVar5 + -uVar11;
  pcVar5 = (char *)&DAT_006f326c;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar5 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  }
  uVar11 = 0xffffffff;
  pcVar4 = (char *)&DAT_006f326c;
  do {
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar4 + 1;
  } while (cVar1 != '\0');
  if (((~uVar11 != 1) && ((&DAT_006f326a)[~uVar11] != '\\')) && ((&DAT_006f326a)[~uVar11] != '/')) {
    uVar11 = 0xffffffff;
    pcVar4 = &DAT_005d9058;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    iVar13 = -1;
    pcVar4 = (char *)&DAT_006f326c;
    do {
      pcVar16 = pcVar4;
      if (iVar13 == 0) break;
      iVar13 = iVar13 + -1;
      pcVar16 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar16;
    } while (cVar1 != '\0');
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = pcVar16 + -1;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
  }
  uVar11 = 0xffffffff;
  do {
    pcVar4 = param_1;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar4 = param_1 + 1;
    cVar1 = *param_1;
    param_1 = pcVar4;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  iVar13 = -1;
  pcVar5 = (char *)&DAT_006f326c;
  do {
    pcVar16 = pcVar5;
    if (iVar13 == 0) break;
    iVar13 = iVar13 + -1;
    pcVar16 = pcVar5 + 1;
    cVar1 = *pcVar5;
    pcVar5 = pcVar16;
  } while (cVar1 != '\0');
  pcVar4 = pcVar4 + -uVar11;
  pcVar5 = pcVar16 + -1;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar5 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  }
  switch(param_3) {
  case 0:
    pcVar4 = s_TEXTBUTTON__005ec200;
    break;
  case 1:
    uVar11 = 0xffffffff;
    pcVar4 = &DAT_005ec1f4;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = (char *)&DAT_006f3044;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    goto LAB_00558749;
  case 2:
    pcVar4 = s_SLIDERBAR__005ec1e8;
    break;
  case 3:
    uVar11 = 0xffffffff;
    pcVar4 = s_STANDARDBUTTON__005ec1d8;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = (char *)&DAT_006f3044;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    goto LAB_00558749;
  case 4:
    pcVar4 = s_RADIOBUTTON__005ec1c8;
    break;
  case 5:
    uVar11 = 0xffffffff;
    pcVar4 = s_LISTBOX__005ec1bc;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = (char *)&DAT_006f3044;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    goto LAB_00558749;
  case 6:
    pcVar4 = s_TEXT__005ec1b4;
    break;
  case 7:
    uVar11 = 0xffffffff;
    pcVar4 = s_STATBAR__005ec1a8;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = (char *)&DAT_006f3044;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    goto LAB_00558749;
  case 8:
    pcVar4 = s_SPRITEBAR__005ec19c;
    break;
  case 9:
    uVar11 = 0xffffffff;
    pcVar4 = s_MESSAGELISTBOX__005ec18c;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = (char *)&DAT_006f3044;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    goto LAB_00558749;
  case 10:
    pcVar4 = s_PICTUREBOX__005ec180;
    break;
  default:
    goto switchD_00558649_default;
  }
  uVar11 = 0xffffffff;
  do {
    pcVar5 = pcVar4;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar5 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar5;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  pcVar4 = pcVar5 + -uVar11;
  pcVar5 = (char *)&DAT_006f3044;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    pcVar5 = pcVar5 + 4;
  }
LAB_00558749:
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar5 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  }
switchD_00558649_default:
  FUN_0059de2f(param_4,&DAT_006f2d40,10);
  uVar11 = 0xffffffff;
  pcVar4 = (char *)&DAT_006f2d40;
  do {
    pcVar5 = pcVar4;
    if (uVar11 == 0) break;
    uVar11 = uVar11 - 1;
    pcVar5 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar5;
  } while (cVar1 != '\0');
  uVar11 = ~uVar11;
  iVar13 = -1;
  pcVar4 = (char *)&DAT_006f3044;
  do {
    pcVar16 = pcVar4;
    if (iVar13 == 0) break;
    iVar13 = iVar13 + -1;
    pcVar16 = pcVar4 + 1;
    cVar1 = *pcVar4;
    pcVar4 = pcVar16;
  } while (cVar1 != '\0');
  pcVar4 = pcVar5 + -uVar11;
  pcVar5 = pcVar16 + -1;
  for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
    pcVar4 = pcVar4 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar5 = *pcVar4;
    pcVar4 = pcVar4 + 1;
    pcVar5 = pcVar5 + 1;
  }
  *param_2 = 0;
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,&DAT_005ec178,&DAT_005faf28,(LPSTR)&DAT_006f2d40,0x100,
                     (LPCSTR)&DAT_006f326c);
  if (DVar3 == 0) {
    *param_2 = 0;
  }
  else {
    pcVar4 = _strchr((char *)&DAT_006f2d40,0x3b);
    if (pcVar4 != (char *)0x0) {
      *pcVar4 = '\0';
    }
    pcVar4 = _strchr((char *)&DAT_006f2d40,0x20);
    if (pcVar4 != (char *)0x0) {
      *pcVar4 = '\0';
    }
    iVar13 = __strcmpi((char *)&DAT_006f2d40,s_LARGE_005ec170);
    if (iVar13 == 0) {
      *param_2 = &DAT_006b0400;
      FUN_004a6970();
    }
    else {
      iVar13 = __strcmpi((char *)&DAT_006f2d40,&DAT_005ec168);
      if (iVar13 == 0) {
        *param_2 = &DAT_006df1c0;
        FUN_004a6970();
      }
      else {
        *param_2 = &DAT_00656680;
        FUN_004a6970();
      }
    }
  }
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_Rect2_005ec160,&DAT_005faf28,(LPSTR)&DAT_006f2d40,0x100
                     ,(LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    uVar11 = 0xffffffff;
    pcVar4 = (char *)&DAT_006f2d40;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_200;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    uVar11 = 0xffffffff;
    pcVar4 = &DAT_005da820;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_100;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    pcVar4 = (char *)FUN_0059c99c(local_200,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[5] = uVar6;
    pcVar4 = (char *)FUN_0059c99c(0,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[6] = uVar6;
    pcVar4 = (char *)FUN_0059c99c(0,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[7] = uVar6;
    pcVar4 = (char *)FUN_0059c99c(0,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[8] = uVar6;
  }
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_Rect1_005ec158,&DAT_005faf28,(LPSTR)&DAT_006f2d40,0x100
                     ,(LPCSTR)&DAT_006f326c);
  if (DVar3 == 0) {
    uVar6 = __ftol();
    param_2[1] = uVar6;
    uVar7 = __ftol();
    param_2[2] = uVar7;
    uVar8 = __ftol();
    param_2[3] = uVar8;
    uVar9 = __ftol();
    param_2[4] = uVar9;
    FUN_0059c1fd(&DAT_006f2d40,s__i__i__i__i_005ec14c,uVar6,uVar7,uVar8,uVar9);
    WritePrivateProfileStringA
              ((LPCSTR)&DAT_006f3044,s_Rect1_005ec158,(LPCSTR)&DAT_006f2d40,(LPCSTR)&DAT_006f326c);
  }
  else {
    uVar11 = 0xffffffff;
    pcVar4 = (char *)&DAT_006f2d40;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_200;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    uVar11 = 0xffffffff;
    pcVar4 = &DAT_005da820;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_100;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    pcVar4 = (char *)FUN_0059c99c(local_200,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[1] = uVar6;
    pcVar4 = (char *)FUN_0059c99c(0,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[2] = uVar6;
    pcVar4 = (char *)FUN_0059c99c(0,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[3] = uVar6;
    pcVar4 = (char *)FUN_0059c99c(0,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
        uVar6 = FUN_0059c5a4(pcVar4);
        param_2[4] = uVar6;
        goto LAB_00558c8f;
      }
      *pcVar5 = '\0';
    }
    uVar6 = FUN_0059c5a4(pcVar4);
    param_2[4] = uVar6;
  }
LAB_00558c8f:
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,&DAT_005ec144,&DAT_005faf28,(LPSTR)&DAT_006f2d40,0x100,
                     (LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    pcVar4 = _strchr((char *)&DAT_006f2d40,0x3b);
    if (pcVar4 != (char *)0x0) {
      *pcVar4 = '\0';
    }
    if ((char)DAT_006f2d40 == '\"') {
      uVar11 = 0xffffffff;
      pcVar4 = (char *)&DAT_006f2d40;
      do {
        if (uVar11 == 0) break;
        uVar11 = uVar11 - 1;
        cVar1 = *pcVar4;
        pcVar4 = pcVar4 + 1;
      } while (cVar1 != '\0');
      puVar15 = (undefined4 *)((int)&DAT_006f2d40 + 1);
      puVar17 = &DAT_006f2d40;
      for (uVar12 = ~uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
        *puVar17 = *puVar15;
        puVar15 = puVar15 + 1;
        puVar17 = puVar17 + 1;
      }
      for (uVar11 = ~uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
        *(undefined1 *)puVar17 = *(undefined1 *)puVar15;
        puVar15 = (undefined4 *)((int)puVar15 + 1);
        puVar17 = (undefined4 *)((int)puVar17 + 1);
      }
      pcVar4 = _strchr((char *)&DAT_006f2d40,0x22);
      if (pcVar4 != (char *)0x0) {
        *pcVar4 = '\0';
      }
    }
    iVar13 = FUN_0059c6e7((int)(char)DAT_006f2d40);
    if (iVar13 == 0) {
      pcVar4 = (char *)&DAT_006f2d40;
    }
    else {
      FUN_0059c5a4(&DAT_006f2d40);
      pcVar4 = (char *)FUN_0058a6b0();
    }
    uVar11 = 0xffffffff;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = (char *)(param_2 + 9);
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
  }
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_SpriteIndexes_005ec134,&DAT_005faf28,
                     (LPSTR)&DAT_006f2d40,0x100,(LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    uVar11 = 0xffffffff;
    pcVar4 = (char *)&DAT_006f2d40;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    iVar13 = 3;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_200;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    uVar11 = 0xffffffff;
    pcVar4 = &DAT_005da820;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_100;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    bVar2 = true;
    puVar15 = param_2 + 0x49;
    do {
      if (bVar2) {
        pcVar4 = local_200;
        bVar2 = false;
      }
      else {
        pcVar4 = (char *)0x0;
      }
      pcVar4 = (char *)FUN_0059c99c(pcVar4,local_100);
      if (pcVar4 != (char *)0x0) {
        pcVar5 = _strchr(pcVar4,0x3d);
        if (pcVar5 == (char *)0x0) {
          _strchr(pcVar4,0);
        }
        else {
          *pcVar5 = '\0';
        }
        uVar6 = FUN_0059c5a4(pcVar4);
        *puVar15 = uVar6;
      }
      puVar15 = puVar15 + 1;
      iVar13 = iVar13 + -1;
    } while (iVar13 != 0);
  }
  pcVar14 = GetPrivateProfileStringA_exref;
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_minValue_005ec128,&DAT_005faf28,(LPSTR)&DAT_006f2d40,
                     0x100,(LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    uVar6 = FUN_0059c5a4(&DAT_006f2d40);
    param_2[0x4c] = uVar6;
  }
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_maxValue_005ec11c,&DAT_005faf28,(LPSTR)&DAT_006f2d40,
                     0x100,(LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    uVar6 = FUN_0059c5a4(&DAT_006f2d40);
    param_2[0x4d] = uVar6;
  }
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_TextFlags_005ec110,&DAT_005faf28,(LPSTR)&DAT_006f2d40,
                     0x100,(LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    pcVar4 = _strchr((char *)&DAT_006f2d40,0x3b);
    if (pcVar4 != (char *)0x0) {
      *pcVar4 = '\0';
    }
    pcVar4 = _strchr((char *)&DAT_006f2d40,0x20);
    if (pcVar4 != (char *)0x0) {
      *pcVar4 = '\0';
    }
    iVar13 = __strcmpi((char *)&DAT_006f2d40,&DAT_005eb674);
    if (iVar13 == 0) {
      uVar6 = 1;
    }
    else {
      iVar13 = __strcmpi((char *)&DAT_006f2d40,s_RIGHT_005eb66c);
      if (iVar13 == 0) {
        uVar6 = 2;
      }
      else {
        uVar6 = 4;
      }
    }
    FUN_004a5a90(param_2 + 0x4e,uVar6);
  }
  DVar3 = GetPrivateProfileStringA
                    ((LPCSTR)&DAT_006f3044,s_Colours_005ec108,&DAT_005faf28,(LPSTR)&DAT_006f2d40,
                     0x100,(LPCSTR)&DAT_006f326c);
  if (DVar3 != 0) {
    uVar11 = 0xffffffff;
    pcVar4 = (char *)&DAT_006f2d40;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_200;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    uVar11 = 0xffffffff;
    pcVar4 = &DAT_005da820;
    do {
      pcVar5 = pcVar4;
      if (uVar11 == 0) break;
      uVar11 = uVar11 - 1;
      pcVar5 = pcVar4 + 1;
      cVar1 = *pcVar4;
      pcVar4 = pcVar5;
    } while (cVar1 != '\0');
    uVar11 = ~uVar11;
    pcVar4 = pcVar5 + -uVar11;
    pcVar5 = local_100;
    for (uVar12 = uVar11 >> 2; uVar12 != 0; uVar12 = uVar12 - 1) {
      *(undefined4 *)pcVar5 = *(undefined4 *)pcVar4;
      pcVar4 = pcVar4 + 4;
      pcVar5 = pcVar5 + 4;
    }
    for (uVar11 = uVar11 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
      *pcVar5 = *pcVar4;
      pcVar4 = pcVar4 + 1;
      pcVar5 = pcVar5 + 1;
    }
    pcVar4 = (char *)FUN_0059c99c(local_200,local_100);
    if (pcVar4 != (char *)0x0) {
      pcVar5 = _strchr(pcVar4,0x3d);
      if (pcVar5 == (char *)0x0) {
        _strchr(pcVar4,0);
      }
      else {
        *pcVar5 = '\0';
      }
    }
    iVar13 = FUN_0059c5a4(pcVar4);
    iVar10 = FUN_00597850(iVar13 * 0xc);
    *(int *)((int)param_2 + 0x139) = iVar10;
    if ((iVar10 != 0) && (0 < iVar13)) {
      iVar10 = 0;
      do {
        pcVar4 = (char *)FUN_0059c99c(0,local_100);
        if (pcVar4 != (char *)0x0) {
          pcVar5 = _strchr(pcVar4,0x3d);
          if (pcVar5 == (char *)0x0) {
            _strchr(pcVar4,0);
          }
          else {
            *pcVar5 = '\0';
          }
        }
        uVar6 = FUN_0059c5a4(pcVar4);
        *(undefined4 *)(*(int *)((int)param_2 + 0x139) + iVar10) = uVar6;
        pcVar4 = (char *)FUN_0059c99c(0,local_100);
        if (pcVar4 != (char *)0x0) {
          pcVar5 = _strchr(pcVar4,0x3d);
          if (pcVar5 == (char *)0x0) {
            _strchr(pcVar4,0);
          }
          else {
            *pcVar5 = '\0';
          }
        }
        uVar6 = FUN_0059c5a4(pcVar4);
        *(undefined4 *)(*(int *)((int)param_2 + 0x139) + 4 + iVar10) = uVar6;
        pcVar4 = (char *)FUN_0059c99c(0,local_100);
        if (pcVar4 != (char *)0x0) {
          pcVar5 = _strchr(pcVar4,0x3d);
          if (pcVar5 == (char *)0x0) {
            _strchr(pcVar4,0);
          }
          else {
            *pcVar5 = '\0';
          }
        }
        uVar6 = FUN_0059c5a4(pcVar4);
        *(undefined4 *)(*(int *)((int)param_2 + 0x139) + 8 + iVar10) = uVar6;
        iVar10 = iVar10 + 0xc;
        iVar13 = iVar13 + -1;
        pcVar14 = GetPrivateProfileStringA_exref;
      } while (iVar13 != 0);
    }
  }
  iVar13 = (*pcVar14)(&DAT_006f3044,&DAT_005ec100,&DAT_005faf28,&DAT_006f2d40,0x100,&DAT_006f326c);
  if (iVar13 != 0) {
    uVar6 = FUN_0059c5a4(&DAT_006f2d40);
    *(undefined4 *)((int)param_2 + 0x13d) = uVar6;
  }
  return 1;
}

