
void FUN_004cf010(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  char cVar1;
  char *pcVar2;
  char *pcVar3;
  uint uVar4;
  uint uVar5;
  
  DAT_006e2240 = '\0';
  pcVar2 = (char *)FUN_0058a6b0();
  uVar4 = 0xffffffff;
  do {
    pcVar3 = pcVar2;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar3 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar3;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  pcVar2 = pcVar3 + -uVar4;
  pcVar3 = (char *)&DAT_006e2340;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar3 = pcVar3 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar3 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar3 = pcVar3 + 1;
  }
  switch(param_3) {
  case 0:
    pcVar2 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar3 = pcVar2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar3 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar3;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar3 + -uVar4;
    pcVar3 = (char *)&DAT_006e2140;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar3 = pcVar3 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar3 = *pcVar2;
      pcVar2 = pcVar2 + 1;
      pcVar3 = pcVar3 + 1;
    }
    pcVar3 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar2 = pcVar3;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar2 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar2;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar2 + -uVar4;
    pcVar3 = &DAT_006e2240;
    break;
  case 1:
    pcVar2 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar3 = pcVar2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar3 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar3;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar3 + -uVar4;
    pcVar3 = (char *)&DAT_006e2140;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar3 = pcVar3 + 4;
    }
    goto LAB_004cf1a7;
  case 2:
    goto LAB_004cf185;
  case 3:
    pcVar2 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar3 = pcVar2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar3 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar3;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar3 + -uVar4;
    pcVar3 = (char *)&DAT_006e2140;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar3 = pcVar3 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar3 = *pcVar2;
      pcVar2 = pcVar2 + 1;
      pcVar3 = pcVar3 + 1;
    }
    pcVar3 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar2 = pcVar3;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar2 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar2;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar2 + -uVar4;
    pcVar3 = &DAT_006e2240;
    break;
  case 4:
    pcVar2 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar3 = pcVar2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar3 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar3;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar3 + -uVar4;
    pcVar3 = (char *)&DAT_006e2140;
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
      pcVar2 = pcVar2 + 4;
      pcVar3 = pcVar3 + 4;
    }
    goto LAB_004cf1a7;
  case 5:
LAB_004cf185:
    pcVar3 = (char *)FUN_0058a6b0();
    uVar4 = 0xffffffff;
    do {
      pcVar2 = pcVar3;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar2 = pcVar3 + 1;
      cVar1 = *pcVar3;
      pcVar3 = pcVar2;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar2 = pcVar2 + -uVar4;
    pcVar3 = (char *)&DAT_006e2140;
    break;
  default:
    goto switchD_004cf059_default;
  }
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar3 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar3 = pcVar3 + 4;
  }
LAB_004cf1a7:
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar3 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar3 = pcVar3 + 1;
  }
switchD_004cf059_default:
  FUN_004cf3b0(param_1,&DAT_006e2340,&DAT_006e2140,-(uint)(DAT_006e2240 != '\0') & 0x6e2240);
  return;
}

