
undefined4 __fastcall FUN_004b0790(int param_1)

{
  bool bVar1;
  char cVar2;
  DWORD DVar3;
  char *pcVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  char *pcVar8;
  char local_101;
  char local_100 [256];
  
  bVar1 = false;
  if (*(char *)(param_1 + 0x5f) != '\0') {
    DVar3 = timeGetTime();
    if (*(uint *)(param_1 + 0x5b) <= DVar3) {
      pcVar4 = (char *)FUN_004d1df0();
      uVar6 = 0xffffffff;
      do {
        pcVar8 = pcVar4;
        if (uVar6 == 0) break;
        uVar6 = uVar6 - 1;
        pcVar8 = pcVar4 + 1;
        cVar2 = *pcVar4;
        pcVar4 = pcVar8;
      } while (cVar2 != '\0');
      uVar6 = ~uVar6;
      pcVar4 = pcVar8 + -uVar6;
      pcVar8 = local_100;
      for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pcVar4;
        pcVar4 = pcVar4 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
        *pcVar8 = *pcVar4;
        pcVar4 = pcVar4 + 1;
        pcVar8 = pcVar8 + 1;
      }
      FUN_004d1d80();
      if ((*(char *)(param_1 + 0x65) != '\0') &&
         (((DAT_006de6b0 == 2 && (DAT_006de6c4 != '\0')) || (DAT_006de6b0 == 3)))) {
        bVar1 = true;
        ShowCursor(1);
        FUN_004a39a0();
        FUN_0058bc10(0);
        FUN_0058bc10(0);
      }
      cVar2 = FUN_00535fa0(&local_101);
      if ((cVar2 == '\0') && (local_101 == '\0')) {
        FUN_0058a680(9);
        FUN_0049e680();
        local_101 = '\x01';
      }
      if (bVar1) {
        ShowCursor(0);
        FUN_00556fb0();
      }
      if (*(char *)(param_1 + 0x65) != '\0') {
        if (local_101 != '\0') {
          *(undefined **)(param_1 + 0x37) = &DAT_006e0020;
          *(undefined4 *)(param_1 + 0x43) = 1;
          return 1;
        }
        *(undefined1 *)(param_1 + 0x65) = 0;
        *(undefined1 *)(param_1 + 0x12) = 0;
      }
      iVar5 = FUN_004d1ed0(local_100);
      if (iVar5 != -1) {
        FUN_004d1060(iVar5);
      }
      *(int *)(param_1 + 0x5b) = *(int *)(param_1 + 0x5b) + 500;
    }
  }
  return 1;
}

