
void __fastcall FUN_00470500(int param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  char *pcVar8;
  int *piVar9;
  char *pcVar10;
  int local_4;
  
  if ((((DAT_00689920 == 2) || (DAT_00689920 == 1)) || (DAT_006e1f0c != 0)) ||
     (((pcVar8 = *(char **)(param_1 + 0x100df), pcVar8 != (char *)0x0 && (*pcVar8 != '\0')) &&
      ((pcVar8[1] == '\0' && (*(int *)(param_1 + 0x102eb) != 0)))))) {
    iVar3 = 0;
    piVar4 = &DAT_00643a80;
    iVar2 = 8;
    do {
      if (*piVar4 != 0) {
        iVar3 = iVar3 + 1;
      }
      piVar4 = piVar4 + 0x55;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    if (DAT_006e1f0c == 1) {
      iVar2 = FUN_004f7670();
      iVar5 = FUN_00466900(PTR_s_Cornelius_005c5c54);
      if (iVar5 != 0) {
        iVar5 = (iVar5 + -0x643a80) / 0x154;
        FUN_00592170(&DAT_005faf28,1,200,DAT_005e141c,DAT_00656624 + iVar5);
        uVar6 = 0xffffffff;
        pcVar8 = &DAT_005da100;
        do {
          if (uVar6 == 0) break;
          uVar6 = uVar6 - 1;
          cVar1 = *pcVar8;
          pcVar8 = pcVar8 + 1;
        } while (cVar1 != '\0');
        if (~uVar6 - 1 < 0x46) {
          uVar6 = 0xffffffff;
          pcVar8 = &DAT_005da100;
          do {
            pcVar10 = pcVar8;
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            pcVar10 = pcVar8 + 1;
            cVar1 = *pcVar8;
            pcVar8 = pcVar10;
          } while (cVar1 != '\0');
          uVar6 = ~uVar6;
          pcVar8 = pcVar10 + -uVar6;
          pcVar10 = &DAT_0065b354 + iVar5 * 0x93a;
          for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
            *(undefined4 *)pcVar10 = *(undefined4 *)pcVar8;
            pcVar8 = pcVar8 + 4;
            pcVar10 = pcVar10 + 4;
          }
          for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
            *pcVar10 = *pcVar8;
            pcVar8 = pcVar8 + 1;
            pcVar10 = pcVar10 + 1;
          }
        }
        FUN_005926f0(iVar5,3,DAT_00656624,iVar3 <= iVar2);
      }
      iVar5 = FUN_00466900(PTR_s_secondAI_005c5c58);
      if (iVar5 != 0) {
        iVar5 = (iVar5 + -0x643a80) / 0x154;
        FUN_00592170(&DAT_005faf28,1,200,DAT_005e141c,DAT_00656624 + iVar5);
        uVar6 = 0xffffffff;
        pcVar8 = s_CelticWizard_005da0f0;
        do {
          if (uVar6 == 0) break;
          uVar6 = uVar6 - 1;
          cVar1 = *pcVar8;
          pcVar8 = pcVar8 + 1;
        } while (cVar1 != '\0');
        if (~uVar6 - 1 < 0x46) {
          uVar6 = 0xffffffff;
          pcVar8 = s_CelticWizard_005da0f0;
          do {
            pcVar10 = pcVar8;
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            pcVar10 = pcVar8 + 1;
            cVar1 = *pcVar8;
            pcVar8 = pcVar10;
          } while (cVar1 != '\0');
          uVar6 = ~uVar6;
          pcVar8 = pcVar10 + -uVar6;
          pcVar10 = &DAT_0065b354 + iVar5 * 0x93a;
          for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
            *(undefined4 *)pcVar10 = *(undefined4 *)pcVar8;
            pcVar8 = pcVar8 + 4;
            pcVar10 = pcVar10 + 4;
          }
          for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
            *pcVar10 = *pcVar8;
            pcVar8 = pcVar8 + 1;
            pcVar10 = pcVar10 + 1;
          }
        }
        FUN_005926f0(iVar5,3,DAT_00656624,iVar3 <= iVar2);
        return;
      }
    }
    else {
      iVar5 = FUN_004f7670();
      iVar2 = DAT_006c3780;
      if (iVar5 <= DAT_006c3780) {
        iVar2 = FUN_004f7670();
      }
      iVar5 = 0;
      piVar9 = &DAT_00643a80;
      piVar4 = &DAT_00658f2d;
      local_4 = 8;
      do {
        if (*piVar9 != 0) {
          if (1 < DAT_006e1f0c) {
            DAT_006c375c = 10000;
          }
          FUN_00592450((int)piVar4 + -0x1d,iVar5,piVar9[1] == 0);
          FUN_005926f0(iVar5,3,DAT_00656624,iVar3 <= iVar2);
          FUN_00465570(0);
          piVar9[0x54] = *piVar4;
        }
        iVar5 = iVar5 + 1;
        piVar4 = (int *)((int)piVar4 + 0x133);
        piVar9 = piVar9 + 0x55;
        local_4 = local_4 + -1;
      } while (local_4 != 0);
    }
  }
  else {
    iVar2 = FUN_00466900(PTR_s_Cornelius_005c5c54);
    if (iVar2 != 0) {
      *(undefined4 *)(iVar2 + 4) = 1;
      iVar2 = (iVar2 + -0x643a80) / 0x154;
      FUN_004667c0(iVar2);
      iVar3 = FUN_005908e0();
      if (iVar3 == -1) {
        iVar3 = FUN_00593ba0();
        if (DAT_006e1f0c == 0) {
          FUN_00590c10(s__start_wzd_005e15fc,0,0xffffffff);
        }
        else {
          FUN_00592170(PTR_s_Cornelius_005c5c54,0,100,DAT_005e141c,DAT_00656624);
        }
      }
      if (DAT_00689920 == 4) {
        uVar6 = 0xffffffff;
        pcVar8 = &DAT_005da100;
        do {
          if (uVar6 == 0) break;
          uVar6 = uVar6 - 1;
          cVar1 = *pcVar8;
          pcVar8 = pcVar8 + 1;
        } while (cVar1 != '\0');
        if (~uVar6 - 1 < 0x46) {
          uVar6 = 0xffffffff;
          pcVar8 = &DAT_005da100;
          do {
            pcVar10 = pcVar8;
            if (uVar6 == 0) break;
            uVar6 = uVar6 - 1;
            pcVar10 = pcVar8 + 1;
            cVar1 = *pcVar8;
            pcVar8 = pcVar10;
          } while (cVar1 != '\0');
          uVar6 = ~uVar6;
          pcVar8 = pcVar10 + -uVar6;
          pcVar10 = &DAT_0065b354 + iVar3 * 0x93a;
          for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
            *(undefined4 *)pcVar10 = *(undefined4 *)pcVar8;
            pcVar8 = pcVar8 + 4;
            pcVar10 = pcVar10 + 4;
          }
          for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
            *pcVar10 = *pcVar8;
            pcVar8 = pcVar8 + 1;
            pcVar10 = pcVar10 + 1;
          }
        }
      }
      FUN_005926f0(iVar2,1,DAT_00656624,0);
    }
    iVar2 = 0;
    do {
      if (iVar2 == -1) {
        piVar4 = (int *)0x0;
      }
      else {
        piVar4 = &DAT_00643a80 + iVar2 * 0x55;
      }
      if ((*piVar4 != 0) && (piVar4[3] == -1)) {
        if (iVar2 == -1) {
          return;
        }
        FUN_004f0320(iVar2,DAT_00656624);
        return;
      }
      iVar2 = iVar2 + 1;
    } while (iVar2 < 8);
  }
  return;
}

