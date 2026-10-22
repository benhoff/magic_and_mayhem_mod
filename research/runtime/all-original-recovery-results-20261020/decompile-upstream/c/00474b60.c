
undefined * FUN_00474b60(void)

{
  char cVar1;
  char *pcVar2;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  char *pcVar8;
  char local_100 [256];
  
  iVar6 = (int)(DAT_006e2034 + (DAT_006e2034 >> 0x1f & 3U)) >> 2;
  iVar7 = 0;
  pcVar2 = (char *)FUN_0058a6b0();
  uVar4 = 0xffffffff;
  do {
    pcVar8 = pcVar2;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar8 = pcVar2 + 1;
    cVar1 = *pcVar2;
    pcVar2 = pcVar8;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  pcVar2 = pcVar8 + -uVar4;
  pcVar8 = local_100;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar8 = *(undefined4 *)pcVar2;
    pcVar2 = pcVar2 + 4;
    pcVar8 = pcVar8 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar8 = *pcVar2;
    pcVar2 = pcVar2 + 1;
    pcVar8 = pcVar8 + 1;
  }
  if (DAT_006e2034 != 0) {
    if (iVar6 < DAT_006e2038) {
      if (iVar6 * 2 < DAT_006e2038) {
        if (iVar6 * 3 < DAT_006e2038) {
          iVar7 = (DAT_006e2038 * 100) / DAT_006e2034;
        }
        else {
          iVar7 = (DAT_006e2038 * 100) / (iVar6 * 3);
        }
      }
      else {
        iVar7 = (DAT_006e2038 * 100) / (iVar6 * 2);
      }
    }
    else {
      iVar7 = (DAT_006e2038 * 100) / iVar6;
    }
  }
  uVar3 = FUN_0058a6b0(iVar7);
  FUN_0059c1fd(&DAT_005fcb7c,s__s__s___i____005da4e0,local_100,uVar3);
  return &DAT_005fcb7c;
}

