
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_004e8480(char *param_1)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  int iVar4;
  DWORD DVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  int *piVar9;
  char *pcVar10;
  int *piVar11;
  int local_a18 [75];
  char local_8ec [2272];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2206;
  local_c = ExceptionList;
  uVar7 = 0xffffffff;
  do {
    pcVar2 = param_1;
    if (uVar7 == 0) break;
    uVar7 = uVar7 - 1;
    pcVar2 = param_1 + 1;
    cVar1 = *param_1;
    param_1 = pcVar2;
  } while (cVar1 != '\0');
  uVar7 = ~uVar7;
  piVar11 = (int *)(pcVar2 + -uVar7);
  piVar9 = local_a18 + 0xb;
  for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
    *piVar9 = *piVar11;
    piVar11 = piVar11 + 1;
    piVar9 = piVar9 + 1;
  }
  for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(char *)piVar9 = (char)*piVar11;
    piVar11 = (int *)((int)piVar11 + 1);
    piVar9 = (int *)((int)piVar9 + 1);
  }
  uVar7 = 0xffffffff;
  pcVar2 = &DAT_005e3f9c;
  do {
    pcVar3 = pcVar2;
    if (uVar7 == 0) break;
    uVar7 = uVar7 - 1;
    pcVar3 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar3;
  } while (cVar1 != '\0');
  uVar7 = ~uVar7;
  pcVar2 = pcVar3 + -uVar7;
  pcVar3 = local_8ec;
  for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
    *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar3 = pcVar3 + 4;
  }
  for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
    *pcVar3 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar3 = pcVar3 + 1;
  }
  uVar7 = 0xffffffff;
  pcVar2 = s_realms_celtic_celtic_cfg_005e3f80;
  do {
    pcVar3 = pcVar2;
    if (uVar7 == 0) break;
    uVar7 = uVar7 - 1;
    pcVar3 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar3;
  } while (cVar1 != '\0');
  uVar7 = ~uVar7;
  local_a18[10] = 1;
  pcVar2 = pcVar3 + -uVar7;
  pcVar3 = (char *)&DAT_006ddd58;
  ExceptionList = &local_c;
  for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
    *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar3 = pcVar3 + 4;
  }
  for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
    *pcVar3 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar3 = pcVar3 + 1;
  }
  FUN_004ca370();
  do {
    DAT_00656624 = timeGetTime();
    DAT_006e1f9c = 1;
LAB_004e8534:
    while( true ) {
      while( true ) {
        if (local_a18[10] == 0) {
          piVar11 = (int *)0x0;
        }
        else {
          piVar11 = local_a18 + 0xb;
          local_a18[10] = 0;
        }
        pcVar2 = (char *)FUN_0059c99c(piVar11,local_8ec);
        if (pcVar2 == (char *)0x0) {
          ExceptionList = local_c;
          return;
        }
        pcVar3 = _strchr(pcVar2,0x3d);
        if (pcVar3 == (char *)0x0) {
          pcVar3 = _strchr(pcVar2,0);
        }
        else {
          *pcVar3 = '\0';
          pcVar3 = pcVar3 + 1;
        }
        iVar4 = __strcmpi(pcVar2,s_profile_005e3f78);
        if (iVar4 == 0) {
          FUN_0059dfcf(s__projects_chaos_005e3f68);
        }
        iVar4 = __strcmpi(pcVar2,(char *)&PTR_DAT_005e3f64);
        if (iVar4 == 0) {
          DAT_006a2920 = 1;
        }
        iVar4 = __strcmpi(pcVar2,s_direct_005e3f5c);
        if (iVar4 != 0) break;
        DAT_006e1f08 = 1;
      }
      iVar4 = __strcmpi(pcVar2,s_directAI_005e3f50);
      if (((iVar4 != 0) && (iVar4 = __strcmpi(pcVar2,s_directAIhost_005e3f40), iVar4 != 0)) &&
         (iVar4 = __strcmpi(pcVar2,s_directAIjoin_005e3f30), iVar4 != 0)) break;
      iVar4 = __strcmpi(pcVar2,s_directAIhost_005e3f40);
      if (iVar4 == 0) {
        DAT_006e1f0c = 2;
      }
      else {
        iVar4 = __strcmpi(pcVar2,s_directAIjoin_005e3f30);
        DAT_006e1f0c = (-(uint)(iVar4 != 0) & 0xfffffffe) + 3;
      }
      uVar7 = 0xffffffff;
      pcVar2 = s_AI_network_battle_005e3e38;
      do {
        pcVar10 = pcVar2;
        if (uVar7 == 0) break;
        uVar7 = uVar7 - 1;
        pcVar10 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar10;
      } while (cVar1 != '\0');
      uVar7 = ~uVar7;
      pcVar2 = pcVar10 + -uVar7;
      pcVar10 = (char *)&DAT_006a4a14;
      for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
        *(undefined4 *)pcVar10 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar10 = pcVar10 + 4;
      }
      DAT_00689924 = 3;
      DAT_006e1f18 = 2;
      for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar10 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar10 = pcVar10 + 1;
      }
      FUN_0059bc22(pcVar3,&DAT_005da898,&DAT_006e1f18);
    }
    iVar4 = __strcmpi(pcVar2,s_bgt56yHNmju7_005e3f20);
    if (iVar4 == 0) {
      DAT_006e2032 = 1;
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,s_train_005e3f18);
    if (iVar4 == 0) {
      local_a18[0] = 1;
      local_a18[1] = 4;
      local_a18[2] = 5;
      local_a18[3] = 6;
      local_a18[4] = 7;
      local_a18[5] = 8;
      local_a18[6] = 9;
      local_a18[7] = 10;
      local_a18[8] = 0xf;
      local_a18[9] = 0x11;
      FUN_0054e170(0x499602d2);
      local_4 = 0;
      uVar7 = 0xffffffff;
      pcVar2 = s_realms_multiplayer_training_cfg_005e3ef8;
      do {
        pcVar10 = pcVar2;
        if (uVar7 == 0) break;
        uVar7 = uVar7 - 1;
        pcVar10 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar10;
      } while (cVar1 != '\0');
      uVar7 = ~uVar7;
      DAT_006e1f0c = 1;
      _DAT_006e1f10 = 1;
      pcVar2 = pcVar10 + -uVar7;
      pcVar10 = (char *)&DAT_006ddd58;
      for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
        *(undefined4 *)pcVar10 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar10 = pcVar10 + 4;
      }
      for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar10 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar10 = pcVar10 + 1;
      }
      FUN_004ca370();
      DVar5 = timeGetTime();
      FUN_0054e170(DVar5);
      uVar7 = FUN_0054e200();
      DAT_00656620 = local_a18[(uVar7 & 0x7fffffff) % 10];
      DAT_00656624 = timeGetTime();
      DAT_00689924 = 0;
      iVar4 = __strcmpi(pcVar3,s_random_005e3ef0);
      if (iVar4 == 0) {
        DAT_005d8650 = 2;
      }
      local_4 = 0xffffffff;
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,s_bugsearch_005e3ee4);
    if (((iVar4 == 0) || (iVar4 = __strcmpi(pcVar2,s_bughost_005e3edc), iVar4 == 0)) ||
       (iVar4 = __strcmpi(pcVar2,s_bugjoin_005e3ed4), iVar4 == 0)) {
      FUN_0054e170(0x499602d2);
      local_4 = 1;
      iVar4 = __strcmpi(pcVar2,s_bughost_005e3edc);
      if (iVar4 == 0) {
        DAT_006e1f0c = 2;
      }
      else {
        iVar4 = __strcmpi(pcVar2,s_bugjoin_005e3ed4);
        DAT_006e1f0c = (-(uint)(iVar4 != 0) & 0xfffffffe) + 3;
      }
      uVar7 = 0xffffffff;
      pcVar2 = s_Network_bug_search_005e3e5c;
      do {
        pcVar10 = pcVar2;
        if (uVar7 == 0) break;
        uVar7 = uVar7 - 1;
        pcVar10 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar10;
      } while (cVar1 != '\0');
      uVar7 = ~uVar7;
      _DAT_006e1f10 = 1;
      DAT_006e1f14 = 0x1d4c;
      pcVar2 = pcVar10 + -uVar7;
      pcVar10 = (char *)&DAT_006a4a14;
      for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
        *(undefined4 *)pcVar10 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar10 = pcVar10 + 4;
      }
      DAT_006e1f18 = 2;
      for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar10 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar10 = pcVar10 + 1;
      }
      SetFileAttributesA(PTR_s_AI_BRAIN_DAT_005c6fa8,1);
      uVar7 = 0xffffffff;
      pcVar2 = s_realms_multiplayer_training_cfg_005e3ef8;
      do {
        pcVar10 = pcVar2;
        if (uVar7 == 0) break;
        uVar7 = uVar7 - 1;
        pcVar10 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar10;
      } while (cVar1 != '\0');
      uVar7 = ~uVar7;
      pcVar2 = pcVar10 + -uVar7;
      pcVar10 = (char *)&DAT_006ddd58;
      for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
        *(undefined4 *)pcVar10 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar10 = pcVar10 + 4;
      }
      for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar10 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar10 = pcVar10 + 1;
      }
      FUN_004ca370();
      if (DAT_006e1f0c < 3) {
        iVar4 = 0;
        iVar6 = FUN_004b8090();
        while (iVar6 != 0) {
          iVar4 = iVar4 + 1;
          iVar6 = FUN_004b8090();
        }
        DVar5 = timeGetTime();
        FUN_0054e170(DVar5);
        uVar7 = FUN_0054e200();
        DAT_00656620 = (int)((longlong)(ulonglong)(uVar7 & 0x7fffffff) % (longlong)iVar4) + 1;
        DAT_00656624 = timeGetTime();
      }
      DAT_00689924 = 3;
      FUN_0059bc22(pcVar3,s__d__________d_005e3e4c,&DAT_006e1f14,&DAT_006a4a14,&DAT_006e1f18);
      local_4 = 0xffffffff;
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,s_playback_005e3ec8);
    if (iVar4 == 0) {
      DAT_006e1f61 = 1;
      DAT_006e1f60 = 0;
      DAT_006e1f08 = 1;
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,&DAT_005d9fe4);
    if (iVar4 == 0) {
      DAT_006e1f60 = 1;
      DAT_006e1f61 = 0;
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,s_realm_005e3ec0);
    if (iVar4 == 0) {
      uVar7 = 0xffffffff;
      do {
        pcVar2 = pcVar3;
        if (uVar7 == 0) break;
        uVar7 = uVar7 - 1;
        pcVar2 = pcVar3 + 1;
        cVar1 = *pcVar3;
        pcVar3 = pcVar2;
      } while (cVar1 != '\0');
      uVar7 = ~uVar7;
      pcVar2 = pcVar2 + -uVar7;
      pcVar3 = (char *)&DAT_006ddd58;
      for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
        *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar3 = pcVar3 + 4;
      }
      for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
        *pcVar3 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar3 = pcVar3 + 1;
      }
      cVar1 = FUN_004ca370();
      if (cVar1 == '\0') {
        FUN_0049e680();
      }
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,s_region_005e3ea4);
    if (iVar4 == 0) {
      DAT_00656620 = FUN_0059c5a4(pcVar3);
      goto LAB_004e8534;
    }
    iVar4 = __strcmpi(pcVar2,s_mapseed_005e3e9c);
    if (iVar4 != 0) {
      iVar4 = __strcmpi(pcVar2,s_wizard_005e3e94);
      if (iVar4 == 0) {
        uVar7 = 0xffffffff;
        do {
          pcVar2 = pcVar3;
          if (uVar7 == 0) break;
          uVar7 = uVar7 - 1;
          pcVar2 = pcVar3 + 1;
          cVar1 = *pcVar3;
          pcVar3 = pcVar2;
        } while (cVar1 != '\0');
        uVar7 = ~uVar7;
        pcVar2 = pcVar2 + -uVar7;
        pcVar3 = s__start_wzd_005e15fc;
        for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
          *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
          pcVar2 = pcVar2 + 4;
          pcVar3 = pcVar3 + 4;
        }
        for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
          *pcVar3 = *pcVar2;
          pcVar2 = pcVar2 + 1;
          pcVar3 = pcVar3 + 1;
        }
        uVar7 = 0xffffffff;
        pcVar2 = &DAT_005e3e8c;
        do {
          pcVar3 = pcVar2;
          if (uVar7 == 0) break;
          uVar7 = uVar7 - 1;
          pcVar3 = pcVar2 + 1;
          cVar1 = *pcVar2;
          pcVar2 = pcVar3;
        } while (cVar1 != '\0');
        uVar7 = ~uVar7;
        iVar4 = -1;
        pcVar2 = s__start_wzd_005e15fc;
        do {
          pcVar10 = pcVar2;
          if (iVar4 == 0) break;
          iVar4 = iVar4 + -1;
          pcVar10 = pcVar2 + 1;
          cVar1 = *pcVar2;
          pcVar2 = pcVar10;
        } while (cVar1 != '\0');
        pcVar2 = pcVar3 + -uVar7;
        pcVar3 = pcVar10 + -1;
        for (uVar8 = uVar7 >> 2; uVar8 != 0; uVar8 = uVar8 - 1) {
          *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
          pcVar2 = pcVar2 + 4;
          pcVar3 = pcVar3 + 4;
        }
        for (uVar7 = uVar7 & 3; uVar7 != 0; uVar7 = uVar7 - 1) {
          *pcVar3 = *pcVar2;
          pcVar2 = pcVar2 + 1;
          pcVar3 = pcVar3 + 1;
        }
      }
      else {
        iVar4 = __strcmpi(pcVar2,s_difficulty_005e3e80);
        if (iVar4 == 0) {
          DAT_00689924 = FUN_0059c5a4(pcVar3);
        }
        else {
          iVar4 = __strcmpi(pcVar2,s_remote_005e3e78);
          if (iVar4 == 0) {
            DAT_006de6c4 = 1;
          }
          else {
            iVar4 = __strcmpi(pcVar2,s_netsim_005e3e70);
            if (iVar4 == 0) {
              DAT_006ead90 = 1;
            }
          }
        }
      }
      goto LAB_004e8534;
    }
    DAT_00656624 = FUN_0059c5a4(pcVar3);
    DAT_006e1f9c = 0;
    if (DAT_00656624 != 0) goto LAB_004e8534;
    DAT_00656624 = 0;
  } while( true );
}

