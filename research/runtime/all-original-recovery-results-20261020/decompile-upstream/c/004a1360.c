
bool __thiscall FUN_004a1360(int *param_1,char *param_2,char *param_3)

{
  char cVar1;
  char *pcVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  int *piVar6;
  char *pcVar7;
  char *pcVar8;
  int *piVar9;
  
  if (*param_1 != 0) {
    FUN_0059c4c3(*param_1);
    *param_1 = 0;
  }
  param_1[1] = 0;
  if (*param_3 == 'r') {
    DAT_0064186c = FUN_0059cae2(param_2,param_3);
    if ((DAT_0064186c == 0) && (DAT_006e1ed0 != 0)) {
      uVar3 = 0xffffffff;
      pcVar2 = &DAT_005e12e4;
      do {
        pcVar8 = pcVar2;
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        pcVar8 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar8;
      } while (cVar1 != '\0');
      uVar3 = ~uVar3;
      DAT_0064186c = 0;
      pcVar2 = pcVar8 + -uVar3;
      pcVar8 = (char *)&DAT_00634758;
      for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
        *pcVar8 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar8 = pcVar8 + 1;
      }
      uVar3 = 0xffffffff;
      pcVar2 = param_2;
      do {
        pcVar8 = pcVar2;
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        pcVar8 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar8;
      } while (cVar1 != '\0');
      uVar3 = ~uVar3;
      iVar5 = -1;
      pcVar2 = (char *)&DAT_00634758;
      do {
        pcVar7 = pcVar2;
        if (iVar5 == 0) break;
        iVar5 = iVar5 + -1;
        pcVar7 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar7;
      } while (cVar1 != '\0');
      pcVar2 = pcVar8 + -uVar3;
      pcVar8 = pcVar7 + -1;
      for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
        *pcVar8 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar8 = pcVar8 + 1;
      }
      while( true ) {
        FUN_00482010(1);
        DAT_0064186c = FUN_0059cae2(&DAT_00634758,param_3);
        if (DAT_0064186c != 0) break;
        DAT_00641870._0_1_ = 0;
        DAT_00641970._0_1_ = 0;
        pcVar2 = (char *)FUN_0058a6b0();
        uVar3 = 0xffffffff;
        do {
          pcVar8 = pcVar2;
          if (uVar3 == 0) break;
          uVar3 = uVar3 - 1;
          pcVar8 = pcVar2 + 1;
          cVar1 = *pcVar2;
          pcVar2 = pcVar8;
        } while (cVar1 != '\0');
        uVar3 = ~uVar3;
        pcVar2 = pcVar8 + -uVar3;
        pcVar8 = (char *)&DAT_00641870;
        for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined4 *)pcVar8 = *(undefined4 *)pcVar2;
          pcVar2 = pcVar2 + 4;
          pcVar8 = pcVar8 + 4;
        }
        for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
          *pcVar8 = *pcVar2;
          pcVar2 = pcVar2 + 1;
          pcVar8 = pcVar8 + 1;
        }
        pcVar2 = (char *)FUN_0058a6b0();
        uVar3 = 0xffffffff;
        do {
          pcVar8 = pcVar2;
          if (uVar3 == 0) break;
          uVar3 = uVar3 - 1;
          pcVar8 = pcVar2 + 1;
          cVar1 = *pcVar2;
          pcVar2 = pcVar8;
        } while (cVar1 != '\0');
        uVar3 = ~uVar3;
        pcVar2 = pcVar8 + -uVar3;
        pcVar8 = (char *)&DAT_00641970;
        for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
          *(undefined4 *)pcVar8 = *(undefined4 *)pcVar2;
          pcVar2 = pcVar2 + 4;
          pcVar8 = pcVar8 + 4;
        }
        for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
          *pcVar8 = *pcVar2;
          pcVar2 = pcVar2 + 1;
          pcVar8 = pcVar8 + 1;
        }
        FUN_004a39a0();
        ShowCursor(1);
        iVar5 = MessageBoxA(DAT_00656614,(LPCSTR)&DAT_00641970,(LPCSTR)&DAT_00641870,0x51035);
        ShowCursor(0);
        FUN_004a39a0();
        if ((iVar5 != 4) || (DAT_0064186c != 0)) break;
      }
    }
    *param_1 = DAT_0064186c;
  }
  else {
    iVar5 = FUN_0059cae2(param_2,param_3);
    *param_1 = iVar5;
  }
  uVar3 = 0xffffffff;
  do {
    pcVar2 = param_2;
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    pcVar2 = param_2 + 1;
    cVar1 = *param_2;
    param_2 = pcVar2;
  } while (cVar1 != '\0');
  uVar3 = ~uVar3;
  piVar6 = (int *)(pcVar2 + -uVar3);
  piVar9 = param_1 + 2;
  for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *piVar9 = *piVar6;
    piVar6 = piVar6 + 1;
    piVar9 = piVar9 + 1;
  }
  for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
    *(char *)piVar9 = (char)*piVar6;
    piVar6 = (int *)((int)piVar6 + 1);
    piVar9 = (int *)((int)piVar9 + 1);
  }
  return *param_1 != 0;
}

