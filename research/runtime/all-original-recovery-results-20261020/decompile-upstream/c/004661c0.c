
undefined4 __fastcall FUN_004661c0(int *param_1)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  char *pcVar7;
  char *pcVar8;
  int *piVar9;
  int *piVar10;
  char local_100 [256];
  
  iVar6 = 8;
  do {
    DAT_005fbf28 = param_1;
    FUN_00465440();
    iVar6 = iVar6 + -1;
  } while (iVar6 != 0);
  param_1[0x2a8] = -1;
  param_1[0x2a9] = 0;
  if (DAT_006e1f0c != 1) {
    if ((((DAT_00656630 < 1) || (7 < DAT_00656630)) || (DAT_00656634 < 0)) ||
       (DAT_00656630 <= DAT_00656634)) {
      return 0;
    }
    iVar6 = 0;
    if (0 < DAT_00656630) {
      do {
        if (iVar6 == -1) {
          piVar4 = (int *)0x0;
        }
        else {
          piVar4 = param_1 + iVar6 * 0x55;
        }
        *piVar4 = 1;
        FUN_0059c1fd(local_100,&DAT_005d98c4,PTR_DAT_005c5c5c,iVar6);
        uVar2 = 0xffffffff;
        pcVar7 = local_100;
        do {
          pcVar8 = pcVar7;
          if (uVar2 == 0) break;
          uVar2 = uVar2 - 1;
          pcVar8 = pcVar7 + 1;
          cVar1 = *pcVar7;
          pcVar7 = pcVar8;
        } while (cVar1 != '\0');
        uVar2 = ~uVar2;
        piVar10 = (int *)(pcVar8 + -uVar2);
        piVar9 = piVar4 + 0x14;
        for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
          *piVar9 = *piVar10;
          piVar10 = piVar10 + 1;
          piVar9 = piVar9 + 1;
        }
        for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
          *(char *)piVar9 = (char)*piVar10;
          piVar10 = (int *)((int)piVar10 + 1);
          piVar9 = (int *)((int)piVar9 + 1);
        }
        iVar5 = 0;
        do {
          if (((int)piVar4 - (int)DAT_005fbf28) / 0x154 != iVar5) {
            *(undefined1 *)(iVar5 + 0x18 + (int)piVar4) = 0;
            if (iVar5 == -1) {
              piVar10 = (int *)0x0;
            }
            else {
              piVar10 = DAT_005fbf28 + iVar5 * 0x55;
            }
            *(undefined1 *)(((int)piVar4 - (int)DAT_005fbf28) / 0x154 + 0x18 + (int)piVar10) = 0;
          }
          iVar5 = iVar5 + 1;
        } while (iVar5 < 8);
        piVar4[0x54] = iVar6;
        iVar6 = iVar6 + 1;
      } while (iVar6 < DAT_00656630);
    }
    FUN_004667c0(DAT_00656634);
    return 1;
  }
  iVar6 = 0;
  piVar4 = param_1;
  do {
    if (*piVar4 == 0) {
      *piVar4 = 1;
      goto LAB_0046622b;
    }
    iVar6 = iVar6 + 1;
    piVar4 = piVar4 + 0x55;
  } while (iVar6 < 8);
  piVar4 = (int *)0x0;
LAB_0046622b:
  piVar4[2] = 1;
  uVar2 = 0xffffffff;
  pcVar7 = PTR_s_Cornelius_005c5c54;
  do {
    pcVar8 = pcVar7;
    if (uVar2 == 0) break;
    uVar2 = uVar2 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar2 = ~uVar2;
  piVar10 = (int *)(pcVar8 + -uVar2);
  piVar9 = piVar4 + 0x14;
  for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
    *piVar9 = *piVar10;
    piVar10 = piVar10 + 1;
    piVar9 = piVar9 + 1;
  }
  for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
    *(char *)piVar9 = (char)*piVar10;
    piVar10 = (int *)((int)piVar10 + 1);
    piVar9 = (int *)((int)piVar9 + 1);
  }
  iVar6 = 0;
  do {
    FUN_00465500(iVar6,0);
    iVar6 = iVar6 + 1;
  } while (iVar6 < 8);
  iVar6 = 0;
  piVar4[0x54] = 1;
  do {
    if (*param_1 == 0) {
      *param_1 = 1;
      goto LAB_00466295;
    }
    iVar6 = iVar6 + 1;
    param_1 = param_1 + 0x55;
  } while (iVar6 < 8);
  param_1 = (int *)0x0;
LAB_00466295:
  param_1[2] = 1;
  uVar2 = 0xffffffff;
  pcVar7 = PTR_s_secondAI_005c5c58;
  do {
    pcVar8 = pcVar7;
    if (uVar2 == 0) break;
    uVar2 = uVar2 - 1;
    pcVar8 = pcVar7 + 1;
    cVar1 = *pcVar7;
    pcVar7 = pcVar8;
  } while (cVar1 != '\0');
  uVar2 = ~uVar2;
  piVar4 = (int *)(pcVar8 + -uVar2);
  piVar10 = param_1 + 0x14;
  for (uVar3 = uVar2 >> 2; uVar3 != 0; uVar3 = uVar3 - 1) {
    *piVar10 = *piVar4;
    piVar4 = piVar4 + 1;
    piVar10 = piVar10 + 1;
  }
  for (uVar2 = uVar2 & 3; uVar2 != 0; uVar2 = uVar2 - 1) {
    *(char *)piVar10 = (char)*piVar4;
    piVar4 = (int *)((int)piVar4 + 1);
    piVar10 = (int *)((int)piVar10 + 1);
  }
  iVar6 = 0;
  do {
    FUN_00465500(iVar6,0);
    iVar6 = iVar6 + 1;
  } while (iVar6 < 8);
  param_1[0x54] = 2;
  return 1;
}

