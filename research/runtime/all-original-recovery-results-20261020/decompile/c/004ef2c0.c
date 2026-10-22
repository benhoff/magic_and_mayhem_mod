
void __thiscall
FUN_004ef2c0(int param_1,int param_2,undefined4 param_3,int param_4,int param_5,int param_6)

{
  bool bVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int *piVar7;
  int *piVar8;
  int local_18;
  int local_14;
  int local_10;
  int local_c;
  int local_4;
  
  local_c = 0;
  if (DAT_006e1ee8 != 0) {
    if (DAT_005e437c == 0) {
      local_c = FUN_0059cae2(s_events_plc_005e4920,&DAT_005e4888);
    }
    else {
      local_c = FUN_0059cae2(s_events_plc_005e4920,&DAT_005dc824);
      DAT_005e437c = 0;
      if (local_c != 0) {
        FUN_0059d50b(local_c,s_Events_Placement_log_005e4908);
        FUN_0059d50b(local_c,s____________________005e47c0);
      }
    }
    if (DAT_006e1ee8 != 0) {
      FUN_0059d50b(local_c,s____New_Event_List____005e48f0);
    }
  }
  if (*(int *)(param_2 + 0x14) == 0) {
    if (DAT_006e1ee8 != 0) {
      FUN_0059c4c3(local_c);
      return;
    }
  }
  else {
    piVar7 = *(int **)(param_2 + 0x10);
    local_4 = 0;
    if (0 < *(int *)(param_2 + 0x14)) {
LAB_004ef38e:
      bVar1 = true;
      piVar6 = piVar7 + 1;
      piVar8 = piVar7;
      do {
        iVar4 = *piVar8;
        iVar2 = *(int *)(param_1 + 0x50) / 2;
        iVar3 = iVar2;
        if (iVar4 < iVar2) {
          if (*piVar6 < iVar2) {
            local_10 = 1;
            iVar3 = iVar2 + -1;
            iVar2 = iVar2 + -1;
          }
          else {
            if (iVar2 <= iVar4) goto LAB_004ef3b9;
LAB_004ef3cf:
            if (*piVar6 < iVar2) goto LAB_004ef3df;
            local_10 = 3;
            iVar3 = iVar2 + -1;
          }
        }
        else {
LAB_004ef3b9:
          if (*piVar6 < iVar2) {
            local_10 = 2;
            iVar2 = iVar2 + -1;
          }
          else {
            if (iVar4 < iVar2) goto LAB_004ef3cf;
LAB_004ef3df:
            local_10 = 4;
          }
        }
        *piVar8 = iVar4 - iVar3;
        iVar4 = *piVar6 - iVar2;
        *piVar6 = iVar4;
        switch(param_6) {
        case 0:
          local_18 = *piVar8;
          local_14 = iVar4;
          break;
        case 1:
          local_18 = -iVar4;
          local_14 = *piVar8;
          break;
        case 2:
          local_18 = -*piVar8;
          iVar5 = iVar4;
          goto LAB_004ef425;
        case 3:
          iVar5 = *piVar8;
          local_18 = iVar4;
LAB_004ef425:
          local_14 = -iVar5;
        }
        iVar4 = param_6 + local_10 * 4;
        local_18 = local_18 + *(int *)(&DAT_005e42dc + iVar4 * 8) + iVar3;
        local_14 = local_14 + *(int *)(&DAT_005e42e0 + iVar4 * 8) + iVar2;
        *piVar8 = local_18 + param_4;
        piVar8 = piVar7 + 3;
        *piVar6 = local_14 + param_5;
        if (((*piVar8 == -1) || (piVar6 = piVar7 + 4, piVar7[4] == -1)) || (!bVar1))
        goto LAB_004ef496;
        bVar1 = false;
      } while( true );
    }
LAB_004ef5b2:
    if (DAT_006e1ee8 != 0) {
      FUN_0059c4c3(local_c);
    }
  }
  return;
LAB_004ef496:
  iVar4 = piVar7[3];
  if (iVar4 < *piVar7) {
    piVar7[3] = *piVar7;
    *piVar7 = iVar4;
  }
  iVar4 = piVar7[1];
  if (piVar7[4] < iVar4) {
    piVar7[1] = piVar7[4];
    piVar7[4] = iVar4;
  }
  iVar4 = piVar7[5];
  if (iVar4 < piVar7[2]) {
    piVar7[5] = piVar7[2];
    piVar7[2] = iVar4;
  }
  if (*piVar7 < 0) {
    *piVar7 = *(int *)(param_1 + 4) + *piVar7;
  }
  if (*(int *)(param_1 + 4) <= *piVar7) {
    *piVar7 = *piVar7 - *(int *)(param_1 + 4);
  }
  if (piVar7[1] < 0) {
    piVar7[1] = *(int *)(param_1 + 8) + piVar7[1];
  }
  if (*(int *)(param_1 + 8) <= piVar7[1]) {
    piVar7[1] = piVar7[1] - *(int *)(param_1 + 8);
  }
  if (piVar7[3] < 0) {
    piVar7[3] = piVar7[3] + *(int *)(param_1 + 4);
  }
  if (*(int *)(param_1 + 4) <= piVar7[3]) {
    piVar7[3] = piVar7[3] - *(int *)(param_1 + 4);
  }
  if (piVar7[4] < 0) {
    piVar7[4] = *(int *)(param_1 + 8) + piVar7[4];
  }
  if (*(int *)(param_1 + 8) <= piVar7[4]) {
    piVar7[4] = piVar7[4] - *(int *)(param_1 + 8);
  }
  FUN_0049f020(piVar7);
  if (DAT_006e1ee8 != 0) {
    FUN_0059d50b(local_c,s_Point_1_X1____i_Y1____i_Z1__i_005e48d0,*piVar7,piVar7[1],piVar7[2]);
    FUN_0059d50b(local_c,s_Point_2_X2____i_Y2____i_Z2__i_005e48b0,piVar7[3],piVar7[4],piVar7[5]);
  }
  piVar7 = piVar7 + 0x12;
  local_4 = local_4 + 1;
  if (*(int *)(param_2 + 0x14) <= local_4) goto LAB_004ef5b2;
  goto LAB_004ef38e;
}

