
void __thiscall FUN_00463eb0(int param_1,int param_2,char *param_3)

{
  char cVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  uint uVar6;
  char *pcVar7;
  char *pcVar8;
  undefined4 local_110;
  int local_10c;
  undefined4 local_108;
  int local_104;
  char local_100 [256];
  
  if ((param_2 != 0) && (iVar2 = *(int *)(param_1 + 0x1c5), iVar2 < 3)) {
    iVar3 = 0;
    if (0 < iVar2) {
      piVar4 = (int *)(param_1 + 0x89);
      do {
        if (*piVar4 == param_2) {
          return;
        }
        iVar3 = iVar3 + 1;
        piVar4 = (int *)((int)piVar4 + 0x97);
      } while (iVar3 < iVar2);
    }
    uVar5 = 0xffffffff;
    do {
      pcVar7 = param_3;
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      pcVar7 = param_3 + 1;
      cVar1 = *param_3;
      param_3 = pcVar7;
    } while (cVar1 != '\0');
    uVar5 = ~uVar5;
    pcVar7 = pcVar7 + -uVar5;
    pcVar8 = local_100;
    for (uVar6 = uVar5 >> 2; uVar6 != 0; uVar6 = uVar6 - 1) {
      *(undefined4 *)pcVar8 = *(undefined4 *)pcVar7;
      pcVar7 = pcVar7 + 4;
      pcVar8 = pcVar8 + 4;
    }
    for (uVar5 = uVar5 & 3; uVar5 != 0; uVar5 = uVar5 - 1) {
      *pcVar8 = *pcVar7;
      pcVar7 = pcVar7 + 1;
      pcVar8 = pcVar8 + 1;
    }
    FUN_00463650(param_2,local_100);
    iVar3 = *(int *)(param_1 + 0x1c5);
    iVar2 = (-(uint)(iVar3 != 0) & 5) + 0x50 + iVar3 * 0x30;
    iVar3 = iVar3 * 0x97 + param_1;
    *(undefined4 *)(iVar3 + 0x73) = 10;
    *(int *)(iVar3 + 0x77) = iVar2;
    if (*(int *)(iVar3 + 4) == -1) {
      local_104 = iVar2 + 0x30;
      local_110 = 10;
      local_108 = 0x3a;
      local_10c = iVar2;
      FUN_004a4e20(&local_110,1);
      *(undefined1 *)(iVar3 + 0x92) = 1;
    }
    if (*(char *)(param_1 + 0x1ca) != '\0') {
      iVar3 = *(int *)(param_1 + 0x1c5);
      iVar2 = iVar3 + param_1 + iVar3 * 0x96;
      if (*(char *)(iVar3 + param_1 + 0x8d + iVar3 * 0x96) != '\x01') {
        *(undefined1 *)(iVar2 + 0x8d) = 1;
        FUN_004a5000();
        if (*(char *)(iVar2 + 0x8d) == '\0') {
          FUN_004638a0();
        }
        else {
          FUN_0058eac0(0,0,0xff,0);
        }
      }
    }
    *(undefined1 *)(param_1 + 0x1c9) = 1;
    *(int *)(param_1 + 0x1c5) = *(int *)(param_1 + 0x1c5) + 1;
  }
  return;
}

