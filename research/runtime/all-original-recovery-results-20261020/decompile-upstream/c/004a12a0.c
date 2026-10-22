
bool FUN_004a12a0(void)

{
  char cVar1;
  char *pcVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  
  DAT_00641870._0_1_ = 0;
  DAT_00641970._0_1_ = 0;
  pcVar2 = (char *)FUN_0058a6b0();
  uVar4 = 0xffffffff;
  do {
    pcVar6 = pcVar2;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar6 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar6;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  pcVar2 = pcVar6 + -uVar4;
  pcVar6 = (char *)&DAT_00641870;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar6 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar6 = pcVar6 + 1;
  }
  pcVar2 = (char *)FUN_0058a6b0();
  uVar4 = 0xffffffff;
  do {
    pcVar6 = pcVar2;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar6 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar6;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  pcVar2 = pcVar6 + -uVar4;
  pcVar6 = (char *)&DAT_00641970;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar6 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar6 = pcVar6 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar6 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar6 = pcVar6 + 1;
  }
  FUN_004a39a0();
  ShowCursor(1);
  iVar3 = MessageBoxA(DAT_00656614,(LPCSTR)&DAT_00641970,(LPCSTR)&DAT_00641870,0x51035);
  ShowCursor(0);
  FUN_004a39a0();
  return iVar3 == 4;
}

