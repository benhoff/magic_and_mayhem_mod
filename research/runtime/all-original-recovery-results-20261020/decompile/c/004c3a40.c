
void __fastcall FUN_004c3a40(int param_1)

{
  int *piVar1;
  char cVar2;
  bool bVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  DWORD DVar6;
  int iVar7;
  byte bVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  int iVar14;
  char *pcVar15;
  char *pcVar16;
  char *pcVar17;
  int *piVar18;
  int local_750;
  int iStack_74c;
  int local_748;
  int local_744;
  int local_740;
  int local_73c;
  uint uStack_738;
  int iStack_734;
  int iStack_730;
  int local_72c [3];
  int local_720;
  undefined4 local_71c;
  undefined4 local_718;
  int local_714;
  int local_710;
  CHAR local_70c [254];
  char acStack_60e [2];
  undefined4 local_60c [64];
  CHAR local_50c [256];
  CHAR aCStack_40c [256];
  CHAR aCStack_30c [256];
  CHAR aCStack_20c [256];
  CHAR local_10c [256];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c1a3a;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_004a39a0();
  GetCurrentDirectoryA(0x100,local_10c);
  uVar9 = 0xffffffff;
  pcVar15 = local_10c;
  do {
    pcVar17 = pcVar15;
    if (uVar9 == 0) break;
    uVar9 = uVar9 - 1;
    pcVar17 = pcVar15 + 1;
    cVar2 = *pcVar15;
    pcVar15 = pcVar17;
  } while (cVar2 != '\0');
  uVar9 = ~uVar9;
  pcVar15 = pcVar17 + -uVar9;
  pcVar17 = (char *)local_60c;
  for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
    *(undefined4 *)pcVar17 = *(undefined4 *)pcVar15;
    pcVar15 = pcVar15 + 4;
    pcVar17 = pcVar17 + 4;
  }
  for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
    *pcVar17 = *pcVar15;
    pcVar15 = pcVar15 + 1;
    pcVar17 = pcVar17 + 1;
  }
  uVar9 = 0xffffffff;
  pcVar15 = (char *)local_60c;
  do {
    if (uVar9 == 0) break;
    uVar9 = uVar9 - 1;
    cVar2 = *pcVar15;
    pcVar15 = pcVar15 + 1;
  } while (cVar2 != '\0');
  if (((~uVar9 != 1) && (acStack_60e[~uVar9] != '\\')) && (acStack_60e[~uVar9] != '/')) {
    uVar9 = 0xffffffff;
    pcVar15 = &DAT_005d9058;
    do {
      pcVar17 = pcVar15;
      if (uVar9 == 0) break;
      uVar9 = uVar9 - 1;
      pcVar17 = pcVar15 + 1;
      cVar2 = *pcVar15;
      pcVar15 = pcVar17;
    } while (cVar2 != '\0');
    uVar9 = ~uVar9;
    iVar11 = -1;
    pcVar15 = (char *)local_60c;
    do {
      pcVar16 = pcVar15;
      if (iVar11 == 0) break;
      iVar11 = iVar11 + -1;
      pcVar16 = pcVar15 + 1;
      cVar2 = *pcVar15;
      pcVar15 = pcVar16;
    } while (cVar2 != '\0');
    pcVar15 = pcVar17 + -uVar9;
    pcVar17 = pcVar16 + -1;
    for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
      *(undefined4 *)pcVar17 = *(undefined4 *)pcVar15;
      pcVar15 = pcVar15 + 4;
      pcVar17 = pcVar17 + 4;
    }
    for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
      *pcVar17 = *pcVar15;
      pcVar15 = pcVar15 + 1;
      pcVar17 = pcVar17 + 1;
    }
  }
  uVar9 = 0xffffffff;
  pcVar15 = "Interface\\Grimoire\\Grimoire.cfg";
  do {
    pcVar17 = pcVar15;
    if (uVar9 == 0) break;
    uVar9 = uVar9 - 1;
    pcVar17 = pcVar15 + 1;
    cVar2 = *pcVar15;
    pcVar15 = pcVar17;
  } while (cVar2 != '\0');
  uVar9 = ~uVar9;
  iVar11 = -1;
  pcVar15 = (char *)local_60c;
  do {
    pcVar16 = pcVar15;
    if (iVar11 == 0) break;
    iVar11 = iVar11 + -1;
    pcVar16 = pcVar15 + 1;
    cVar2 = *pcVar15;
    pcVar15 = pcVar16;
  } while (cVar2 != '\0');
  pcVar15 = pcVar17 + -uVar9;
  pcVar17 = pcVar16 + -1;
  for (uVar10 = uVar9 >> 2; uVar10 != 0; uVar10 = uVar10 - 1) {
    *(undefined4 *)pcVar17 = *(undefined4 *)pcVar15;
    pcVar15 = pcVar15 + 4;
    pcVar17 = pcVar17 + 4;
  }
  for (uVar9 = uVar9 & 3; uVar9 != 0; uVar9 = uVar9 - 1) {
    *pcVar17 = *pcVar15;
    pcVar15 = pcVar15 + 1;
    pcVar17 = pcVar17 + 1;
  }
  iVar11 = FUN_00597850(0x13dc);
  local_4 = 0;
  if (iVar11 == 0) {
    uVar4 = 0;
  }
  else {
    uVar4 = FUN_004c26d0(s_Interface_grimoire_grimoire_txt_005e1edc);
  }
  local_4 = 0xffffffff;
  *(undefined4 *)(param_1 + 0x16) = uVar4;
  puVar5 = (undefined4 *)FUN_00597850(0xc);
  local_4 = 1;
  if (puVar5 == (undefined4 *)0x0) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    uVar4 = FUN_00597850(0x200);
    *puVar5 = uVar4;
    puVar5[1] = 0;
    puVar5[2] = 0;
  }
  local_4 = 0xffffffff;
  *(undefined4 **)(param_1 + 0x12) = puVar5;
  iVar11 = FUN_00597850(0x960);
  local_4 = 2;
  if (iVar11 == 0) {
    iVar11 = 0;
  }
  else {
    *(undefined4 *)(iVar11 + 0x954) = 0;
    *(undefined4 *)(iVar11 + 0x958) = 0;
    *(undefined4 *)(iVar11 + 0x95c) = 0;
    FUN_004c7960();
  }
  local_4 = 0xffffffff;
  *(int *)(param_1 + 0x1a) = iVar11;
  puVar5 = (undefined4 *)FUN_00597850(0xc);
  if (puVar5 == (undefined4 *)0x0) {
    puVar5 = (undefined4 *)0x0;
  }
  else {
    puVar5[1] = FUN_004c77c0;
    puVar5[2] = param_1;
    *puVar5 = &PTR_LAB_005c6b8c;
  }
  *(undefined4 **)(param_1 + 0xa00) = puVar5;
  FUN_004c5a60();
  FUN_004c5230();
  FUN_0059c1fd(local_10c,s__s_ix_i_Icons_spr_005e1ec8,"Interface\\Grimoire\\",
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  piVar1 = (int *)(param_1 + 0xa28);
  FUN_0057d310(local_10c,2);
  if (((((*(int *)(param_1 + 0x16) == 0) || (*(int *)(param_1 + 0x12) == 0)) ||
       ((*(int *)(param_1 + 0x22) == 0 ||
        ((*(int *)(param_1 + 0x1a) == 0 || (*(int *)(param_1 + 0x56) < 1)))))) ||
      ((*(int *)(param_1 + 0x5a) < 1 ||
       ((((((*(int *)(param_1 + 0x26) == 0 || (*(int *)(*(int *)(param_1 + 0x26) + 4) == 0)) ||
           (*(int *)(param_1 + 0x2a) == 0)) ||
          ((*(int *)(*(int *)(param_1 + 0x2a) + 4) == 0 || (*(int *)(param_1 + 0x2e) == 0)))) ||
         ((*(int *)(*(int *)(param_1 + 0x2e) + 4) == 0 ||
          ((*(int *)(param_1 + 0x32) == 0 || (*(int *)(*(int *)(param_1 + 0x32) + 4) == 0)))))) ||
        (iVar11 = FUN_0057d310("Interface\\Grimoire\\Generic\\Icons.spr",0x10), iVar11 == 0)))))) ||
     (iVar11 = FUN_0057d310("Interface\\Grimoire\\Generic\\PageTurns.spr",0x10), iVar11 == 0)) {
    MessageBoxA((HWND)0x0,s_The_Grimoire_could_not_be_loaded_005e1d98,s_Grimoire_005e1dbc,0x42030);
    FUN_00557130();
    ExceptionList = local_c;
    return;
  }
  FUN_0059c1fd(local_70c,s__s_ix_i_ClosedBook_JPG_005e1eb0,"Interface\\Grimoire\\",
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  iVar11 = FUN_0058d320(local_70c);
  if (iVar11 == 0) {
    FUN_0059c1fd(local_70c,s__s_ix_i_ClosedBook_BMP_005e1e98,"Interface\\Grimoire\\",
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
    iVar11 = FUN_0058d1a0(local_70c);
    if (iVar11 != 0) goto LAB_004c3dd9;
  }
  else {
LAB_004c3dd9:
    local_72c[2] = 0;
    local_71c = 0;
    bVar8 = -(DAT_006de6d5 != 0) & 0xa0;
    if (bVar8 == 0) {
      local_72c[2] = 0x140;
    }
    else if (bVar8 == 0xa0) {
      local_72c[2] = 400;
    }
    local_718 = 0;
    local_72c[1] = 0;
    local_720 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    local_72c[0] = 0;
    local_714 = local_72c[2];
    local_710 = local_720;
    if (DAT_006e1ee4 == 0) {
      FUN_0058c4a0(*(undefined4 *)(param_1 + 0x26),local_72c,0,0);
    }
    else {
      FUN_0058d5b0(2,*(undefined4 *)(param_1 + 0x26),local_72c,0,0,1);
    }
    local_714 = 0;
    local_71c = 0;
    local_72c[0] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    if (local_72c[0] == 0x280) {
      local_714 = 0x140;
    }
    else if (local_72c[0] == 800) {
      local_714 = 400;
    }
    local_72c[0] = local_72c[0] - local_714;
    local_720 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    local_72c[2] = local_72c[0] + local_714;
    local_718 = 0;
    local_72c[1] = 0;
    local_710 = local_720;
    FUN_0058d0e0(0,0,0);
    FUN_0058cbc0(*(undefined4 *)(param_1 + 0x2a),local_72c,&local_71c);
    FUN_004a4370();
    FUN_0058bf40(&DAT_006a4fe0,0,0);
  }
  FUN_004c5110();
  FUN_004db980(s_interface_Grimoire_Grimoiretoolt_005e1e70);
  *(undefined4 *)(*(int *)(param_1 + 0x1a) + 0x954) = *(undefined4 *)(param_1 + 0x16);
  iVar11 = 0;
  *(int *)(*(int *)(param_1 + 0x1a) + 0x958) = param_1 + 0xac6;
  *(undefined4 *)(*(int *)(param_1 + 0x1a) + 0x850) = *(undefined4 *)(param_1 + 0x1a3a);
  DVar6 = GetPrivateProfileStringA
                    (s_OPTIONS_005e1e54,s_textSpace_ModifyX_005e1e5c,&DAT_005faf28,local_50c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(local_50c,s__s_Missing_from___s__in__s_005e1d7c,s_textSpace_ModifyX_005e1e5c,
                 s_OPTIONS_005e1e54,local_60c);
    MessageBoxA((HWND)0x0,local_50c,s_Error_005e1d74,0x42030);
  }
  else {
    iVar11 = FUN_0059c5a4(local_50c);
  }
  iVar14 = 0;
  DAT_006e2058 = iVar11;
  DVar6 = GetPrivateProfileStringA
                    (s_OPTIONS_005e1e54,s_textSpace_ModifyY_005e1e40,&DAT_005faf28,local_50c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(local_50c,s__s_Missing_from___s__in__s_005e1d7c,s_textSpace_ModifyY_005e1e40,
                 s_OPTIONS_005e1e54,local_60c);
    MessageBoxA((HWND)0x0,local_50c,s_Error_005e1d74,0x42030);
  }
  else {
    iVar14 = FUN_0059c5a4(local_50c);
  }
  DAT_006e205c = iVar14;
  if (DAT_006de6d5 == 0) {
    DAT_006e2058 = (DAT_006e2058 << 3) / 10;
    DAT_006e205c = (iVar14 * 8) / 10;
  }
  FUN_0059c1fd(local_70c,s__ix_i_X_005e1e38,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  uVar4 = 0;
  DVar6 = GetPrivateProfileStringA
                    (s_CLOSE_GRIMOIRE_005e1e28,local_70c,&DAT_005faf28,local_50c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(local_50c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,s_CLOSE_GRIMOIRE_005e1e28
                 ,local_60c);
    MessageBoxA((HWND)0x0,local_50c,s_Error_005e1d74,0x42030);
  }
  else {
    uVar4 = FUN_0059c5a4(local_50c);
  }
  *(undefined4 *)(param_1 + 0x6e) = uVar4;
  FUN_0059c1fd(local_70c,s__ix_i_Y_005e1e20,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  uVar4 = 0;
  DVar6 = GetPrivateProfileStringA
                    (s_CLOSE_GRIMOIRE_005e1e28,local_70c,&DAT_005faf28,local_50c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(local_50c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,s_CLOSE_GRIMOIRE_005e1e28
                 ,local_60c);
    MessageBoxA((HWND)0x0,local_50c,s_Error_005e1d74,0x42030);
  }
  else {
    uVar4 = FUN_0059c5a4(local_50c);
  }
  *(undefined4 *)(param_1 + 0x72) = uVar4;
  FUN_004cb810(piVar1);
  FUN_004cb7d0(0,1,0xffffffff);
  FUN_004ca9b0(*(undefined4 *)(param_1 + 0x22));
  local_744 = *(int *)(param_1 + 0x72);
  local_748 = *(int *)(param_1 + 0x6e);
  iVar11 = *(int *)(param_1 + 0xa34);
  local_73c = (*(int *)(**(int **)(param_1 + 0xa30) + 8 + *(int *)(param_1 + 0xa34)) -
              *(int *)(**(int **)(param_1 + 0xa30) + 0x10 + iVar11)) + 1 + local_744;
  local_740 = (*(int *)(**(int **)(param_1 + 0xa30) + 4 + iVar11) -
              *(int *)(**(int **)(param_1 + 0xa30) + 0xc + iVar11)) + 1 + local_748;
  FUN_004cb0b0(3999,&local_748,*(undefined4 *)(param_1 + 0xa00),0,1);
  FUN_004ca930();
  iStack_74c = param_1 + 0x630;
  *(undefined4 *)(param_1 + 0xa81) = 0;
  iStack_734 = 0;
  iStack_730 = 1;
  uStack_738 = 4;
  piVar18 = (int *)(param_1 + 0x220);
  do {
    iVar11 = iStack_730;
    FUN_0059c1fd(local_70c,s__ix_i__iLX_005e1e14,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,iStack_730);
    local_750 = 0;
    DVar6 = GetPrivateProfileStringA
                      (&DAT_005e1e0c,local_70c,&DAT_005faf28,local_50c,0x100,(LPCSTR)local_60c);
    if (DVar6 == 0) {
      FUN_0059c1fd(local_50c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,&DAT_005e1e0c,local_60c
                  );
      MessageBoxA((HWND)0x0,local_50c,s_Error_005e1d74,0x42030);
    }
    else {
      local_750 = FUN_0059c5a4(local_50c);
    }
    piVar18[-0x10] = local_750;
    FUN_0059c1fd(local_70c,s__ix_i__iLY_005e1e00,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,iVar11);
    local_750 = 0;
    DVar6 = GetPrivateProfileStringA
                      (&DAT_005e1e0c,local_70c,&DAT_005faf28,aCStack_30c,0x100,(LPCSTR)local_60c);
    if (DVar6 == 0) {
      FUN_0059c1fd(aCStack_30c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,&DAT_005e1e0c,
                   local_60c);
      MessageBoxA((HWND)0x0,aCStack_30c,s_Error_005e1d74,0x42030);
    }
    else {
      local_750 = FUN_0059c5a4(aCStack_30c);
    }
    piVar18[-0xf] = local_750;
    FUN_0059c1fd(local_70c,s__ix_i__iRX_005e1df4,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,iVar11);
    local_750 = 0;
    DVar6 = GetPrivateProfileStringA
                      (&DAT_005e1e0c,local_70c,&DAT_005faf28,aCStack_20c,0x100,(LPCSTR)local_60c);
    if (DVar6 == 0) {
      FUN_0059c1fd(aCStack_20c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,&DAT_005e1e0c,
                   local_60c);
      MessageBoxA((HWND)0x0,aCStack_20c,s_Error_005e1d74,0x42030);
    }
    else {
      local_750 = FUN_0059c5a4(aCStack_20c);
    }
    *piVar18 = local_750;
    FUN_0059c1fd(local_70c,s__ix_i__iRY_005e1de8,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,iVar11);
    iVar11 = 0;
    DVar6 = GetPrivateProfileStringA
                      (&DAT_005e1e0c,local_70c,&DAT_005faf28,aCStack_40c,0x100,(LPCSTR)local_60c);
    if (DVar6 == 0) {
      FUN_0059c1fd(aCStack_40c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,&DAT_005e1e0c,
                   local_60c);
      MessageBoxA((HWND)0x0,aCStack_40c,s_Error_005e1d74,0x42030);
    }
    else {
      iVar11 = FUN_0059c5a4(aCStack_40c);
    }
    piVar18[1] = iVar11;
    FUN_004cb810(piVar1);
    FUN_004cb810(piVar1);
    uVar10 = uStack_738 - 2;
    FUN_004cb7d0(uVar10,uStack_738 - 1,0xffffffff);
    FUN_004cb7d0(uStack_738,uStack_738 + 1,0xffffffff);
    FUN_004ca9b0(&DAT_006a49e0);
    FUN_004ca9b0(&DAT_006a49e0);
    *(int *)(iStack_74c + 0x35) = iStack_730;
    *(int *)(iStack_74c + -0x39b) = iStack_730;
    local_744 = piVar18[1];
    local_748 = *piVar18;
    uVar9 = uVar10;
    if (*(uint *)(*piVar1 + 0xc) < uVar10) {
      uVar9 = 0;
    }
    uVar12 = uVar10;
    if (*(uint *)(*piVar1 + 0xc) < uVar10) {
      uVar12 = 0;
    }
    local_73c = (*(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar9 * 4) + 8 +
                         *(int *)(param_1 + 0xa34)) -
                *(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar12 * 4) + 0x10 +
                        *(int *)(param_1 + 0xa34))) + 1 + local_744;
    uVar9 = uVar10;
    if (*(uint *)(*piVar1 + 0xc) < uVar10) {
      uVar9 = 0;
    }
    if (*(uint *)(*piVar1 + 0xc) < uVar10) {
      uVar10 = 0;
    }
    local_740 = (*(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar9 * 4) + 4 +
                         *(int *)(param_1 + 0xa34)) -
                *(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar10 * 4) + 0xc +
                        *(int *)(param_1 + 0xa34))) + 1 + local_748;
    FUN_004cb0b0(iStack_734,&local_748,*(undefined4 *)(param_1 + 0xa00),0,1);
    uVar9 = uStack_738;
    local_744 = piVar18[-0xf];
    local_748 = piVar18[-0x10];
    uVar10 = uStack_738;
    if (*(uint *)(*piVar1 + 0xc) < uStack_738) {
      uVar10 = 0;
    }
    uVar12 = uStack_738;
    if (*(uint *)(*piVar1 + 0xc) < uStack_738) {
      uVar12 = 0;
    }
    local_73c = (*(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar10 * 4) + 8 +
                         *(int *)(param_1 + 0xa34)) -
                *(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar12 * 4) + 0x10 +
                        *(int *)(param_1 + 0xa34))) + 1 + local_744;
    uVar10 = uStack_738;
    if (*(uint *)(*piVar1 + 0xc) < uStack_738) {
      uVar10 = 0;
    }
    uVar12 = uStack_738;
    if (*(uint *)(*piVar1 + 0xc) < uStack_738) {
      uVar12 = 0;
    }
    local_740 = (*(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar10 * 4) + 4 +
                         *(int *)(param_1 + 0xa34)) -
                *(int *)(*(int *)(*(int *)(param_1 + 0xa30) + uVar12 * 4) + 0xc +
                        *(int *)(param_1 + 0xa34))) + 1 + local_748;
    FUN_004cb0b0(iStack_734,&local_748,*(undefined4 *)(param_1 + 0xa00),0,1);
    uStack_738 = uVar9 + 4;
    iVar11 = iStack_730 + 1;
    piVar18 = piVar18 + 2;
    iStack_734 = iStack_734 + 0x7c;
    iStack_74c = iStack_74c + 0x7a;
    bVar3 = iStack_730 < 8;
    iStack_730 = iVar11;
  } while (bVar3);
  FUN_004ca930();
  FUN_004ca950();
  iVar11 = 7;
  do {
    FUN_004ca950();
    FUN_004ca930();
    iVar11 = iVar11 + -1;
  } while (iVar11 != 0);
  FUN_0059c1fd(local_70c,s__ix_i_X_005e1e38,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  uVar4 = 0;
  DVar6 = GetPrivateProfileStringA
                    (s_PAGETURN_LEFT_005e1dd8,local_70c,&DAT_005faf28,aCStack_40c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(aCStack_40c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,
                 s_PAGETURN_LEFT_005e1dd8,local_60c);
    MessageBoxA((HWND)0x0,aCStack_40c,s_Error_005e1d74,0x42030);
  }
  else {
    uVar4 = FUN_0059c5a4(aCStack_40c);
  }
  *(undefined4 *)(param_1 + 0x5e) = uVar4;
  FUN_0059c1fd(local_70c,s__ix_i_Y_005e1e20,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  uVar4 = 0;
  DVar6 = GetPrivateProfileStringA
                    (s_PAGETURN_LEFT_005e1dd8,local_70c,&DAT_005faf28,aCStack_40c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(aCStack_40c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,
                 s_PAGETURN_LEFT_005e1dd8,local_60c);
    MessageBoxA((HWND)0x0,aCStack_40c,s_Error_005e1d74,0x42030);
  }
  else {
    uVar4 = FUN_0059c5a4(aCStack_40c);
  }
  *(undefined4 *)(param_1 + 0x62) = uVar4;
  piVar1 = (int *)(param_1 + 0xa04);
  FUN_004cb810(piVar1);
  FUN_004cb7d0(0,0xffffffff,0xffffffff);
  FUN_004ca9b0(*(undefined4 *)(param_1 + 0x26));
  local_744 = *(int *)(param_1 + 0x62);
  local_748 = *(int *)(param_1 + 0x5e);
  if ((uint)(*(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                      + 8 + *(int *)(param_1 + 0xa10)) -
            *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                     + 0x10 + *(int *)(param_1 + 0xa10))) <
      (uint)(*(int *)(**(int **)(param_1 + 0xa0c) + 8 + *(int *)(param_1 + 0xa10)) -
            *(int *)(**(int **)(param_1 + 0xa0c) + 0x10 + *(int *)(param_1 + 0xa10)))) {
    iVar11 = *(int *)(**(int **)(param_1 + 0xa0c) + 8 + *(int *)(param_1 + 0xa10));
    iVar14 = *(int *)(**(int **)(param_1 + 0xa0c) + 0x10 + *(int *)(param_1 + 0xa10));
  }
  else {
    iVar11 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                      + 8 + *(int *)(param_1 + 0xa10));
    iVar14 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                      + 0x10 + *(int *)(param_1 + 0xa10));
  }
  local_73c = (iVar11 - iVar14) + 1 + local_744;
  if ((uint)(*(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                      + 4 + *(int *)(param_1 + 0xa10)) -
            *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                     + 0xc + *(int *)(param_1 + 0xa10))) <
      (uint)(*(int *)(**(int **)(param_1 + 0xa0c) + 4 + *(int *)(param_1 + 0xa10)) -
            *(int *)(**(int **)(param_1 + 0xa0c) + 0xc + *(int *)(param_1 + 0xa10)))) {
    iVar11 = *(int *)(**(int **)(param_1 + 0xa0c) + 4 + *(int *)(param_1 + 0xa10));
    iVar14 = *(int *)(**(int **)(param_1 + 0xa0c) + 0xc + *(int *)(param_1 + 0xa10));
  }
  else {
    iVar11 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                      + 4 + *(int *)(param_1 + 0xa10));
    iVar14 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + (uint)(*(int *)(*piVar1 + 0xc) != 0) * 4)
                      + 0xc + *(int *)(param_1 + 0xa10));
  }
  local_740 = (iVar11 - iVar14) + 1 + local_748;
  FUN_004cb0b0(1999,&local_748,*(undefined4 *)(param_1 + 0xa00),0,1);
  FUN_0059c1fd(local_70c,s__ix_i_X_005e1e38,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  uVar4 = 0;
  DVar6 = GetPrivateProfileStringA
                    (s_PAGETURN_RIGHT_005e1dc8,local_70c,&DAT_005faf28,aCStack_40c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(aCStack_40c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,
                 s_PAGETURN_RIGHT_005e1dc8,local_60c);
    MessageBoxA((HWND)0x0,aCStack_40c,s_Error_005e1d74,0x42030);
  }
  else {
    uVar4 = FUN_0059c5a4(aCStack_40c);
  }
  *(undefined4 *)(param_1 + 0x66) = uVar4;
  FUN_0059c1fd(local_70c,s__ix_i_Y_005e1e20,(-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
  uVar4 = 0;
  DVar6 = GetPrivateProfileStringA
                    (s_PAGETURN_RIGHT_005e1dc8,local_70c,&DAT_005faf28,aCStack_40c,0x100,
                     (LPCSTR)local_60c);
  if (DVar6 == 0) {
    FUN_0059c1fd(aCStack_40c,s__s_Missing_from___s__in__s_005e1d7c,local_70c,
                 s_PAGETURN_RIGHT_005e1dc8,local_60c);
    MessageBoxA((HWND)0x0,aCStack_40c,s_Error_005e1d74,0x42030);
  }
  else {
    uVar4 = FUN_0059c5a4(aCStack_40c);
  }
  *(undefined4 *)(param_1 + 0x6a) = uVar4;
  FUN_004cb810(piVar1);
  FUN_004cb7d0(2,0xffffffff,0xffffffff);
  FUN_004ca9b0(*(undefined4 *)(param_1 + 0x2a));
  local_744 = *(int *)(param_1 + 0x6a);
  local_748 = *(int *)(param_1 + 0x66);
  iVar11 = 2;
  if (*(uint *)(*piVar1 + 0xc) < 2) {
    iVar11 = 0;
  }
  iVar14 = 2;
  if (*(uint *)(*piVar1 + 0xc) < 2) {
    iVar14 = 0;
  }
  iVar13 = 3;
  iVar7 = 3;
  if (*(uint *)(*piVar1 + 0xc) < 3) {
    iVar7 = 0;
  }
  if (*(uint *)(*piVar1 + 0xc) < 3) {
    iVar13 = 0;
  }
  if ((uint)(*(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar7 * 4) + 8 +
                     *(int *)(param_1 + 0xa10)) -
            *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar13 * 4) + 0x10 +
                    *(int *)(param_1 + 0xa10))) <
      (uint)(*(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar11 * 4) + 8 +
                     *(int *)(param_1 + 0xa10)) -
            *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar14 * 4) + 0x10 +
                    *(int *)(param_1 + 0xa10)))) {
    iVar11 = 2;
    if (*(uint *)(*piVar1 + 0xc) < 2) {
      iVar11 = 0;
    }
    iVar14 = 2;
    if (*(uint *)(*piVar1 + 0xc) < 2) {
      iVar14 = 0;
    }
    iVar11 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar11 * 4) + 8 +
                     *(int *)(param_1 + 0xa10)) -
             *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar14 * 4) + 0x10 +
                     *(int *)(param_1 + 0xa10));
  }
  else {
    iVar11 = 3;
    if (*(uint *)(*piVar1 + 0xc) < 3) {
      iVar11 = 0;
    }
    iVar14 = 3;
    if (*(uint *)(*piVar1 + 0xc) < 3) {
      iVar14 = 0;
    }
    iVar11 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar11 * 4) + 8 +
                     *(int *)(param_1 + 0xa10)) -
             *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar14 * 4) + 0x10 +
                     *(int *)(param_1 + 0xa10));
  }
  local_73c = iVar11 + 1 + local_744;
  iVar11 = 2;
  if (*(uint *)(*piVar1 + 0xc) < 2) {
    iVar11 = 0;
  }
  iVar14 = 2;
  if (*(uint *)(*piVar1 + 0xc) < 2) {
    iVar14 = 0;
  }
  iVar7 = 3;
  if (*(uint *)(*piVar1 + 0xc) < 3) {
    iVar7 = 0;
  }
  iVar13 = 3;
  if (*(uint *)(*piVar1 + 0xc) < 3) {
    iVar13 = 0;
  }
  if ((uint)(*(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar7 * 4) + 4 +
                     *(int *)(param_1 + 0xa10)) -
            *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar13 * 4) + 0xc +
                    *(int *)(param_1 + 0xa10))) <
      (uint)(*(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar11 * 4) + 4 +
                     *(int *)(param_1 + 0xa10)) -
            *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar14 * 4) + 0xc +
                    *(int *)(param_1 + 0xa10)))) {
    iVar14 = 2;
    iVar11 = 2;
    if (*(uint *)(*piVar1 + 0xc) < 2) {
      iVar11 = 0;
    }
    iVar11 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar11 * 4) + 4 +
                     *(int *)(param_1 + 0xa10));
    if (1 < *(uint *)(*piVar1 + 0xc)) goto LAB_004c4ef3;
  }
  else {
    iVar14 = 3;
    iVar11 = 3;
    if (*(uint *)(*piVar1 + 0xc) < 3) {
      iVar11 = 0;
    }
    iVar11 = *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar11 * 4) + 4 +
                     *(int *)(param_1 + 0xa10));
    if (2 < *(uint *)(*piVar1 + 0xc)) goto LAB_004c4ef3;
  }
  iVar14 = 0;
LAB_004c4ef3:
  local_740 = local_748 + 1 +
              (iVar11 - *(int *)(*(int *)(*(int *)(param_1 + 0xa0c) + iVar14 * 4) + 0xc +
                                *(int *)(param_1 + 0xa10)));
  FUN_004cb0b0(2999,&local_748,*(undefined4 *)(param_1 + 0xa00),0,1);
  *(undefined4 *)(param_1 + 0x3a) = 0;
  *(undefined4 *)(param_1 + 0x3e) = 0;
  *(undefined4 *)(param_1 + 0x42) = 0;
  *(undefined4 *)(param_1 + 0x46) = 0;
  *(undefined4 *)(param_1 + 0x4a) = 0;
  FUN_004c72d0(0,0,1);
  ExceptionList = local_c;
  return;
}

