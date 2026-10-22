
void __fastcall FUN_004a1120(char *param_1,undefined4 param_2)

{
  char cVar1;
  char *pcVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  char *pcVar6;
  char *pcVar7;
  
  DAT_0064186c = FUN_0059cae2(param_1,param_2);
  if ((DAT_0064186c == 0) && (DAT_006e1ed0 != 0)) {
    uVar3 = 0xffffffff;
    pcVar2 = &DAT_005e12e4;
    do {
      pcVar7 = pcVar2;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar7 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar7;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    DAT_0064186c = 0;
    pcVar2 = pcVar7 + -uVar3;
    pcVar7 = (char *)&DAT_00634758;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar7 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar7 = pcVar7 + 4;
    }
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *pcVar7 = *pcVar2;
      pcVar2 = pcVar2 + 1;
      pcVar7 = pcVar7 + 1;
    }
    uVar3 = 0xffffffff;
    do {
      pcVar2 = param_1;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar2 = param_1 + 1;
      cVar1 = *param_1;
      param_1 = pcVar2;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    iVar5 = -1;
    pcVar7 = (char *)&DAT_00634758;
    do {
      pcVar6 = pcVar7;
      if (iVar5 == 0) break;
      iVar5 = iVar5 + -1;
      pcVar6 = pcVar7 + 1;
      cVar1 = *pcVar7;
      pcVar7 = pcVar6;
    } while (cVar1 != '\0');
    pcVar2 = pcVar2 + -uVar3;
    pcVar7 = pcVar6 + -1;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar7 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar7 = pcVar7 + 4;
    }
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *pcVar7 = *pcVar2;
      pcVar2 = pcVar2 + 1;
      pcVar7 = pcVar7 + 1;
    }
    while( true ) {
      FUN_00482010(1);
      DAT_0064186c = FUN_0059cae2(&DAT_00634758,param_2);
      if (DAT_0064186c != 0) break;
      DAT_00641870._0_1_ = 0;
      DAT_00641970._0_1_ = 0;
      pcVar2 = (char *)FUN_0058a6b0();
      uVar3 = 0xffffffff;
      do {
        pcVar7 = pcVar2;
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        pcVar7 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar7;
      } while (cVar1 != '\0');
      uVar3 = ~uVar3;
      pcVar2 = pcVar7 + -uVar3;
      pcVar7 = (char *)&DAT_00641870;
      for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 *)pcVar7 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar7 = pcVar7 + 4;
      }
      for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
        *pcVar7 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar7 = pcVar7 + 1;
      }
      pcVar2 = (char *)FUN_0058a6b0();
      uVar3 = 0xffffffff;
      do {
        pcVar7 = pcVar2;
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        pcVar7 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar7;
      } while (cVar1 != '\0');
      uVar3 = ~uVar3;
      pcVar2 = pcVar7 + -uVar3;
      pcVar7 = (char *)&DAT_00641970;
      for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 *)pcVar7 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar7 = pcVar7 + 4;
      }
      for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
        *pcVar7 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar7 = pcVar7 + 1;
      }
      FUN_004a39a0();
      ShowCursor(1);
      iVar5 = MessageBoxA(DAT_00656614,(LPCSTR)&DAT_00641970,(LPCSTR)&DAT_00641870,0x51035);
      ShowCursor(0);
      FUN_004a39a0();
      if (iVar5 != 4) {
        return;
      }
      if (DAT_0064186c != 0) {
        return;
      }
    }
  }
  return;
}

