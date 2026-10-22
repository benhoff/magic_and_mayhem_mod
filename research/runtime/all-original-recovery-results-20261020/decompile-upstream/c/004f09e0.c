
void __thiscall
FUN_004f09e0(int param_1,int param_2,undefined4 param_3,uint *param_4,uint *param_5,uint *param_6,
            int param_7)

{
  bool bVar1;
  bool bVar2;
  int *piVar3;
  uint uVar4;
  int *piVar5;
  uint uVar6;
  int iVar7;
  uint *puVar8;
  int iVar9;
  int iVar10;
  int iVar11;
  char *pcVar12;
  int local_340;
  int local_33c;
  int local_338;
  int local_330;
  int local_32c;
  int *local_324;
  int local_320 [200];
  
  iVar11 = 0;
  bVar1 = false;
  local_340 = 0;
  if (DAT_006e1ee8 == 0) goto LAB_004f0a91;
  if (DAT_005e43a0 == 0) {
    local_340 = FUN_0059cae2(s_Wizard_plc_005e4dc4,&DAT_005e4888);
  }
  else {
    local_340 = FUN_0059cae2(s_Wizard_plc_005e4dc4,&DAT_005dc824);
    DAT_005e43a0 = 0;
    if (local_340 != 0) {
      FUN_0059d50b(local_340,s_WizardPlacement_log_005e4dac);
      FUN_0059d50b(local_340,s____________________005e47c0);
    }
  }
  iVar11 = local_340;
  if (param_2 == 1) {
    pcVar12 = s____Trying_to_deploy_Friendly_Wiz_005e4d84;
  }
  else if (param_2 == 2) {
    pcVar12 = s____Trying_to_deploy_enemy_Wizard_005e4d58;
  }
  else {
    pcVar12 = s____Error_in_placementment_type___005e4d30;
  }
  do {
    FUN_0059d50b(iVar11,pcVar12);
LAB_004f0a91:
    do {
      iVar9 = 0;
      iVar10 = 0;
      piVar5 = *(int **)(param_1 + 100);
      local_324 = local_320 + 1;
      local_32c = 0;
      local_338 = 0;
      local_33c = 0;
      for (local_330 = 0; local_330 < *(int *)(param_1 + 0x68); local_330 = local_330 + 1) {
        iVar7 = DAT_006c54dc +
                ((&DAT_006cb8c2)[piVar5[2]] + (&DAT_006cb942)[piVar5[1]] + *piVar5) * 0xc;
        if ((iVar7 == 0) || (*(short *)(iVar7 + 4) == -1)) {
          if ((param_2 == 1) && ((piVar5[3] == 1 && (piVar5[5] == 100)))) {
            local_320[1] = local_330;
            iVar10 = 0;
            goto LAB_004f0da1;
          }
          if (((piVar5[3] == param_2) && (local_33c = local_33c + 1, piVar5[3] == param_2)) &&
             (((param_2 == 2 && (piVar5[6] == param_7)) || (param_7 == -1)))) {
            bVar2 = false;
            if ((param_2 != 2) && (!bVar1)) {
              DAT_00644524 = 0;
              while (iVar9 = DAT_00644524, DAT_00644524 < 8) {
                while( true ) {
                  DAT_00644524 = iVar9 + 1;
                  if (iVar9 == -1) {
                    piVar3 = (int *)0x0;
                  }
                  else {
                    piVar3 = &DAT_00643a80 + iVar9 * 0x55;
                  }
                  if (*piVar3 != 0) break;
                  iVar9 = DAT_00644524;
                  if (7 < DAT_00644524) goto LAB_004f0c61;
                }
                if (piVar3 == (int *)0x0) break;
                uVar4 = piVar3[3];
                if (uVar4 != 0xffffffff) {
                  if (uVar4 < DAT_006def5c) {
                    iVar9 = uVar4 * 0xe4b + DAT_006def58;
                  }
                  else {
                    iVar9 = 0;
                  }
                  uVar4 = *(int *)(iVar9 + 8) - *piVar5;
                  uVar6 = (int)uVar4 >> 0x1f;
                  iVar10 = *(int *)(param_1 + 0x50) / 2;
                  iVar11 = local_340;
                  if ((((int)((uVar4 ^ uVar6) - uVar6) < iVar10) ||
                      (uVar4 = *(int *)(iVar9 + 0xc) - piVar5[1], uVar6 = (int)uVar4 >> 0x1f,
                      (int)((uVar4 ^ uVar6) - uVar6) < iVar10)) && (bVar2 = true, DAT_006e1ee8 != 0)
                     ) {
                    FUN_0059d50b(local_340,s_Position__i___i_to_close_to__i____005e4d0c,
                                 *(int *)(iVar9 + 8),*(undefined4 *)(iVar9 + 0xc),*piVar5,piVar5[1])
                    ;
                  }
                }
              }
LAB_004f0c61:
              iVar9 = local_32c;
              iVar10 = local_338;
              if (bVar2) goto LAB_004f0c93;
            }
            iVar10 = iVar10 + piVar5[5];
            iVar9 = iVar9 + 1;
            local_324[-1] = iVar10;
            *local_324 = local_330;
            local_324 = local_324 + 2;
            local_338 = iVar10;
            local_32c = iVar9;
          }
        }
LAB_004f0c93:
        piVar5 = piVar5 + 10;
      }
      if (DAT_006e1ee8 != 0) {
        FUN_0059d50b(iVar11,s_Total_number_in_deployement_List_005e4cc4,local_33c,iVar9);
      }
      if (0 < iVar9) {
        iVar7 = 0;
        if (iVar10 != 0) {
          uVar4 = FUN_0054e200();
          iVar7 = (int)((longlong)(ulonglong)(uVar4 & 0x7fffffff) % (longlong)iVar10);
        }
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0xdd5);
        }
        iVar10 = 0;
        if (iVar9 < 1) goto LAB_004f0da1;
        piVar5 = local_320;
        goto LAB_004f0d95;
      }
      if (bVar1) {
        MessageBoxA((HWND)0x0,s_No_place_to_put_wizard__see_depl_005e4c80,
                    s_DEPLOYMENT_ERROR_005d95c0,0x42030);
        if (DAT_006e1ee8 != 0) {
          FUN_0059d50b(iVar11,s_____POSSIBLE_CAUSE_OF_ERROR______005e4c5c);
          if (local_33c == 0) {
            pcVar12 = s_Too_many_wizards_being_deployed__005e4c18;
          }
          else if (iVar9 == 0) {
            pcVar12 = s_Failed_to_deploy_them_far_enough_005e4bc8;
          }
          else {
            pcVar12 = s_Hmmm__its_not_an_obvious_error__i_005e4b8c;
          }
          FUN_0059d50b(iVar11,pcVar12);
        }
        DestroyWindow(DAT_00656614);
      }
      bVar1 = true;
    } while (DAT_006e1ee8 == 0);
    pcVar12 = s_Failed_to_place_wizards_apart__d_005e4b58;
  } while( true );
  while( true ) {
    iVar10 = iVar10 + 1;
    piVar5 = piVar5 + 2;
    if (iVar9 <= iVar10) break;
LAB_004f0d95:
    if (iVar7 < *piVar5) break;
  }
LAB_004f0da1:
  iVar9 = local_320[iVar10 * 2 + 1];
  if ((iVar9 < 0) || (*(int *)(param_1 + 0x68) <= iVar9)) {
    puVar8 = (uint *)0x0;
  }
  else {
    puVar8 = (uint *)(*(int *)(param_1 + 100) + iVar9 * 0x28);
  }
  uVar4 = *puVar8;
  if ((int)uVar4 < 0) {
    do {
      uVar4 = uVar4 + DAT_006c5494;
    } while ((int)uVar4 < 0);
  }
  else {
    for (; DAT_006c5494 <= uVar4; uVar4 = uVar4 - DAT_006c5494) {
    }
  }
  *param_4 = uVar4;
  uVar4 = puVar8[1];
  if ((int)uVar4 < 0) {
    do {
      uVar4 = uVar4 + DAT_006c5498;
    } while ((int)uVar4 < 0);
  }
  else {
    for (; DAT_006c5498 <= uVar4; uVar4 = uVar4 - DAT_006c5498) {
    }
  }
  *param_5 = uVar4;
  uVar4 = puVar8[2];
  *param_6 = uVar4;
  if (((DAT_006e1ee8 != 0) &&
      (FUN_0059d50b(iVar11,s_Wizard_deployed_at__i___i___i_005e4b38,*param_4,*param_5,uVar4),
      DAT_006e1ee8 != 0)) &&
     (FUN_0059d50b(iVar11,s_Item_removed_from_list_005e4b20), DAT_006e1ee8 != 0)) {
    FUN_0059c4c3(iVar11);
  }
  return;
}

