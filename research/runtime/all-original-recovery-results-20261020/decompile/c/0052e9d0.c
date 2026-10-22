
undefined4 __thiscall FUN_0052e9d0(int param_1,char *param_2,int param_3,char *param_4)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  char *pcVar7;
  char *pcVar8;
  undefined4 *puVar9;
  char *pcVar10;
  undefined2 local_324;
  undefined1 local_322;
  undefined4 local_321;
  undefined2 local_31d;
  undefined1 local_31b;
  undefined1 auStack_318 [18];
  char acStack_306 [262];
  undefined1 local_200 [3];
  undefined4 local_1fd [63];
  undefined2 local_100;
  undefined1 local_fe;
  undefined4 local_fd [63];
  
  local_200._0_2_ = DAT_005ea258;
  local_200[2] = DAT_005ea25a;
  puVar9 = (undefined4 *)(local_200 + 3);
  for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar9 = 0;
    puVar9 = puVar9 + 1;
  }
  *(undefined1 *)puVar9 = 0;
  local_100 = DAT_005ea258;
  local_fe = DAT_005ea25a;
  local_324 = DAT_005ea258;
  puVar9 = local_fd;
  for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
    *puVar9 = 0;
    puVar9 = puVar9 + 1;
  }
  *(undefined1 *)puVar9 = 0;
  local_321 = 0;
  local_31d = 0;
  local_322 = DAT_005ea25a;
  local_31b = 0;
  GetCurrentDirectoryA(0x100,(LPSTR)&local_100);
  if (DAT_006e1ed0 == 0) {
    iVar4 = FUN_0059dfcf(param_2);
    if (iVar4 != 0) {
      FUN_0049e4e0();
    }
  }
  else {
    FUN_00482010(1);
    iVar4 = FUN_0059dfcf(param_2);
    while (iVar4 != 0) {
      iVar4 = FUN_004a12a0();
      if (iVar4 == 0) {
        FUN_0049e4e0();
      }
      iVar4 = FUN_0059dfcf(param_2);
    }
  }
  FUN_0059de2f(param_3,&local_324,10);
  uVar5 = 0xffffffff;
  pcVar7 = (char *)(param_1 + 0x3ee5);
  do {
    pcVar8 = pcVar7;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  pcVar7 = pcVar8 + -uVar5;
  pcVar8 = local_200;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar8 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar8 = pcVar8 + 1;
  }
  uVar5 = 0xffffffff;
  pcVar7 = (char *)&local_324;
  do {
    pcVar8 = pcVar7;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  iVar4 = -1;
  pcVar7 = local_200;
  do {
    pcVar10 = pcVar7;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar10 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar10;
  } while (cVar1 != '\0');
  pcVar7 = pcVar8 + -uVar5;
  pcVar8 = pcVar10 + -1;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar8 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar8 = pcVar8 + 1;
  }
  uVar5 = 0xffffffff;
  pcVar7 = &DAT_005ea2dc;
  do {
    pcVar8 = pcVar7;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  iVar4 = -1;
  pcVar7 = local_200;
  do {
    pcVar10 = pcVar7;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar10 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar10;
  } while (cVar1 != '\0');
  pcVar7 = pcVar8 + -uVar5;
  pcVar8 = pcVar10 + -1;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar8 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar8 = pcVar8 + 1;
  }
  iVar4 = FUN_0059dc1d(local_200,auStack_318);
  if (iVar4 == -1) {
    iVar2 = MessageBoxA((HWND)0x0,s_Could_not_find_map_section_speci_005ea27c,
                        s_Map_Cfg_Error_005ea2cc,0x40004);
    if (iVar2 == 6) {
      FUN_0059dfcf(&local_100);
      goto LAB_0052ecff;
    }
    FUN_0059ddac(0xffffffff);
    DestroyWindow(DAT_00656614);
  }
  do {
    iVar2 = 0;
    cVar1 = acStack_306[2];
    while (cVar1 != '.') {
      iVar3 = iVar2 + 3;
      iVar2 = iVar2 + 1;
      cVar1 = acStack_306[iVar3];
    }
    iVar3 = FUN_0059c6e7((int)acStack_306[iVar2 + 1]);
    while ((0 < iVar3 && (-1 < iVar2 + -1))) {
      iVar3 = FUN_0059c6e7((int)acStack_306[iVar2]);
      iVar2 = iVar2 + -1;
    }
    iVar3 = 0;
    pcVar7 = acStack_306 + iVar2 + 2;
    cVar1 = acStack_306[iVar2 + 2];
    while (cVar1 != '.') {
      *(char *)((int)&local_324 + iVar3) = cVar1;
      pcVar8 = pcVar7 + 1;
      iVar3 = iVar3 + 1;
      pcVar7 = pcVar7 + 1;
      cVar1 = *pcVar8;
    }
    *(undefined1 *)((int)&local_324 + iVar3) = 0;
    iVar2 = FUN_0059c5a4(&local_324);
    if (param_3 == iVar2) {
      uVar5 = 0xffffffff;
      goto code_r0x0052ec60;
    }
    iVar2 = FUN_0059dce7(iVar4,auStack_318);
  } while (iVar2 == 0);
  FUN_0059dfcf(&local_100);
LAB_0052ecff:
  FUN_0059ddac(iVar4);
  return 0;
  while( true ) {
    uVar5 = uVar5 - 1;
    pcVar7 = param_2 + 1;
    cVar1 = *param_2;
    param_2 = pcVar7;
    if (cVar1 == '\0') break;
code_r0x0052ec60:
    pcVar7 = param_2;
    if (uVar5 == 0) break;
  }
  uVar5 = ~uVar5;
  pcVar7 = pcVar7 + -uVar5;
  pcVar8 = param_4;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar8 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar8 = pcVar8 + 1;
  }
  uVar5 = 0xffffffff;
  pcVar7 = &DAT_005d9058;
  do {
    pcVar8 = pcVar7;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  iVar2 = -1;
  pcVar7 = param_4;
  do {
    pcVar10 = pcVar7;
    if (iVar2 == 0) break;
    iVar2 = iVar2 + -1;
    pcVar10 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar10;
  } while (cVar1 != '\0');
  pcVar7 = pcVar8 + -uVar5;
  pcVar8 = pcVar10 + -1;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
    pcVar7 = pcVar7 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar8 = *pcVar7;
    pcVar7 = pcVar7 + 1;
    pcVar8 = pcVar8 + 1;
  }
  uVar5 = 0xffffffff;
  pcVar7 = acStack_306 + 2;
  do {
    pcVar8 = pcVar7;
    if (uVar5 == 0) break;
    uVar5 = uVar5 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar5 = ~uVar5;
  iVar2 = -1;
  do {
    pcVar7 = param_4;
    if (iVar2 == 0) break;
    iVar2 = iVar2 + -1;
    pcVar7 = param_4 + 1;
    cVar1 = *param_4;
    param_4 = pcVar7;
  } while (cVar1 != '\0');
  pcVar8 = pcVar8 + -uVar5;
  pcVar7 = pcVar7 + -1;
  for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
    *(undefined4 *)pcVar7 = *(undefined4 *)pcVar8;
    pcVar8 = pcVar8 + 4;
    pcVar7 = pcVar7 + 4;
  }
  for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
    *pcVar7 = *pcVar8;
    pcVar8 = pcVar8 + 1;
    pcVar7 = pcVar7 + 1;
  }
  FUN_0059dfcf(&local_100);
  FUN_0059ddac(iVar4);
  return 1;
}

