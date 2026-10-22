
void FUN_004cf200(undefined4 param_1,undefined4 param_2,undefined4 param_3)

{
  char cVar1;
  char *pcVar2;
  uint uVar3;
  uint uVar4;
  char *pcVar5;
  
  switch(param_3) {
  case 0:
    pcVar5 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar2 = pcVar5;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar2 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar2;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar2 + -uVar3;
    pcVar2 = (char *)&DAT_006e2440;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar2 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar2 = pcVar2 + 4;
    }
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *pcVar2 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar2 = pcVar2 + 1;
    }
    pcVar2 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar5 = pcVar2;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar5 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar5;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar5 + -uVar3;
    pcVar2 = &DAT_006e2540;
    break;
  case 1:
    pcVar5 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar2 = pcVar5;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar2 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar2;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar2 + -uVar3;
    pcVar2 = (char *)&DAT_006e2440;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar2 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar2 = pcVar2 + 4;
    }
    goto LAB_004cf360;
  case 2:
    goto LAB_004cf33e;
  case 3:
    pcVar5 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar2 = pcVar5;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar2 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar2;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar2 + -uVar3;
    pcVar2 = (char *)&DAT_006e2440;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar2 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar2 = pcVar2 + 4;
    }
    for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
      *pcVar2 = *pcVar5;
      pcVar5 = pcVar5 + 1;
      pcVar2 = pcVar2 + 1;
    }
    pcVar2 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar5 = pcVar2;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar5 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar5;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar5 + -uVar3;
    pcVar2 = &DAT_006e2540;
    break;
  case 4:
    pcVar5 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar2 = pcVar5;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar2 = pcVar5 + 1;
      cVar1 = *pcVar5;
      pcVar5 = pcVar2;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar2 + -uVar3;
    pcVar2 = (char *)&DAT_006e2440;
    for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
      *(undefined4 *)pcVar2 = *(undefined4 *)pcVar5;
      pcVar5 = pcVar5 + 4;
      pcVar2 = pcVar2 + 4;
    }
    goto LAB_004cf360;
  case 5:
LAB_004cf33e:
    pcVar2 = (char *)FUN_0058a6b0();
    uVar3 = 0xffffffff;
    do {
      pcVar5 = pcVar2;
      if (uVar3 == 0) break;
      uVar3 = uVar3 - 1;
      pcVar5 = pcVar2 + 1;
      cVar1 = *pcVar2;
      pcVar2 = pcVar5;
    } while (cVar1 != '\0');
    uVar3 = ~uVar3;
    pcVar5 = pcVar5 + -uVar3;
    pcVar2 = (char *)&DAT_006e2440;
    break;
  default:
    goto switchD_004cf212_default;
  }
  for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
    *(undefined4 *)pcVar2 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar2 = pcVar2 + 4;
  }
LAB_004cf360:
  for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
    *pcVar2 = *pcVar5;
    pcVar5 = pcVar5 + 1;
    pcVar2 = pcVar2 + 1;
  }
switchD_004cf212_default:
  FUN_004cf3b0(param_1,param_2,&DAT_006e2440,-(uint)(DAT_006e2540 != '\0') & 0x6e2540);
  return;
}

