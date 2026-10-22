
void FUN_004ad240(void)

{
  char cVar1;
  char *pcVar2;
  uint *puVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  uint *puVar8;
  uint uVar9;
  char *pcVar10;
  int local_4;
  
  pcVar2 = (char *)FUN_0058a680(0);
  uVar4 = 0xffffffff;
  do {
    pcVar10 = pcVar2;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar10 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar10;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  pcVar2 = pcVar10 + -uVar4;
  pcVar10 = (char *)&DAT_00658f10;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar10 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar10 = pcVar10 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar10 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar10 = pcVar10 + 1;
  }
  DAT_00658f25 = 0;
  DAT_00658f29 = 0;
  DAT_00658f2d = 0;
  FUN_004ad120(0);
  local_4 = 1;
  puVar8 = &DAT_00659058;
  do {
    uVar4 = FUN_0054e200();
    uVar5 = (uVar4 & 0x7fffffff) % 0xc;
    iVar6 = 0;
    uVar4 = uVar5;
    do {
      iVar7 = iVar6 + 1;
      uVar9 = uVar5;
      if (iVar6 == 0xc) break;
      uVar4 = uVar4 + 1;
      if (0xb < (int)uVar4) {
        uVar4 = 0;
      }
      puVar3 = &DAT_00658f25;
      do {
        if (*puVar3 == uVar4) {
          uVar9 = 0xffffffff;
          break;
        }
        puVar3 = (uint *)((int)puVar3 + 0x133);
        uVar9 = uVar4;
      } while ((int)puVar3 < 0x6593f1);
      iVar6 = iVar7;
    } while (((((DAT_006e2030 != '\0') && (uVar9 != 0)) && (uVar9 != 3)) &&
             ((uVar9 != 4 && (uVar9 != 10)))) || (uVar9 == 0xffffffff));
    pcVar2 = (char *)FUN_0058a680(uVar9);
    uVar4 = 0xffffffff;
    do {
      pcVar10 = pcVar2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar10 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar10;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar10 + -uVar4;
    pcVar10 = (char *)((int)puVar8 + -0x15);
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar10 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar10 = pcVar10 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar10 = *pcVar2;
      pcVar2 = pcVar2 + 1;
      pcVar10 = pcVar10 + 1;
    }
    *puVar8 = uVar9;
    uVar4 = 0;
    puVar8[1] = 0;
    iVar6 = 0;
LAB_004ad34f:
    do {
      iVar7 = iVar6 + 1;
      if (iVar6 == 8) {
        uVar4 = 0;
        break;
      }
      uVar4 = uVar4 + 1;
      if (7 < (int)uVar4) {
        uVar4 = 0;
      }
      puVar3 = &DAT_00658f2d;
      do {
        iVar6 = iVar7;
        if (*puVar3 == uVar4) goto LAB_004ad34f;
        puVar3 = (uint *)((int)puVar3 + 0x133);
      } while ((int)puVar3 < 0x6593f9);
    } while (uVar4 == 0xffffffff);
    puVar8[2] = uVar4;
    FUN_004ad120(local_4);
    puVar8 = (uint *)((int)puVar8 + 0x133);
    local_4 = local_4 + 1;
    if (0x6593f0 < (int)puVar8) {
      return;
    }
  } while( true );
}

