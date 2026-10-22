
/* WARNING: Removing unreachable block (ram,0x005922f0) */
/* WARNING: Removing unreachable block (ram,0x00592358) */
/* WARNING: Removing unreachable block (ram,0x0059235a) */
/* WARNING: Removing unreachable block (ram,0x00592366) */

undefined4 __thiscall
FUN_00592170(undefined4 *param_1,char *param_2,undefined4 param_3,uint param_4,int param_5,
            undefined4 param_6)

{
  char cVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  int *piVar7;
  undefined1 *puVar8;
  char *pcVar9;
  int iVar10;
  uint local_424;
  undefined1 local_41d;
  undefined1 local_41c [4];
  undefined4 local_418;
  undefined4 local_414;
  undefined4 local_410;
  undefined1 local_40c [4];
  undefined4 local_408;
  undefined4 local_404;
  undefined4 local_400;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2f51;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_0054e170(0x499602d2);
  local_40c[0] = local_41d;
  local_408 = 0;
  local_404 = 0;
  local_400 = 0;
  local_418 = 0;
  local_41c[0] = local_41d;
  local_414 = 0;
  local_410 = 0;
  local_4 = 2;
  if (param_2 == (char *)0x0) {
    FUN_00597870(0);
    local_418 = 0;
    local_414 = 0;
    local_410 = 0;
    FUN_00597870(local_408);
    uVar3 = 0;
  }
  else {
    uVar4 = 0xffffffff;
    param_1[0x10] = 400;
    do {
      pcVar6 = param_2;
      if (uVar4 == 0) break;
      uVar4 = uVar4 - 1;
      pcVar6 = param_2 + 1;
      cVar1 = *param_2;
      param_2 = pcVar6;
    } while (cVar1 != '\0');
    uVar4 = ~uVar4;
    pcVar6 = pcVar6 + -uVar4;
    pcVar9 = (char *)(param_1 + 3);
    for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
      *(undefined4 *)pcVar9 = *(undefined4 *)pcVar6;
      pcVar6 = pcVar6 + 4;
      pcVar9 = pcVar9 + 4;
    }
    for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
      *pcVar9 = *pcVar6;
      pcVar6 = pcVar6 + 1;
      pcVar9 = pcVar9 + 1;
    }
    param_1[0xf] = param_4;
    param_1[0x21] = 0xffffffff;
    param_1[2] = param_3;
    *param_1 = 1;
    local_424 = 0;
    piVar7 = &DAT_006b1270;
    do {
      if ((((local_424 != 0) && (local_424 != 0x18)) && (local_424 != 0x19)) &&
         ((local_424 != 0x1a && (*piVar7 != 0)))) {
        uVar3 = local_414;
        if (local_424 < 0x1c) {
          uVar3 = local_404;
        }
        FUN_00429fa0(uVar3,&local_424);
      }
      local_424 = local_424 + 1;
      piVar7 = (int *)((int)piVar7 + 0x2d1);
    } while (local_424 < 0x5c);
    FUN_0054e170(param_6);
    local_424 = 0;
    if (0 < param_5) {
      do {
        uVar4 = FUN_0054e200();
        puVar8 = local_40c;
        if ((uVar4 & 1) != 0) {
          puVar8 = local_41c;
        }
        if ((*(int *)(puVar8 + 4) != 0) &&
           (iVar10 = *(int *)(puVar8 + 8) - *(int *)(puVar8 + 4) >> 2, iVar10 != 0)) {
          uVar4 = FUN_0054e200();
          piVar7 = (int *)(*(int *)(puVar8 + 4) +
                          (int)((longlong)(ulonglong)(uVar4 & 0x7fffffff) % (longlong)iVar10) * 4);
          FUN_00590310(*piVar7);
          iVar10 = *(int *)(puVar8 + 8);
          iVar2 = *piVar7;
          *piVar7 = *(int *)(iVar10 + -4);
          *(int *)(iVar10 + -4) = iVar2;
          FUN_0042a140(*(int *)(puVar8 + 8) + -4,*(int *)(puVar8 + 8));
          *(int *)(puVar8 + 8) = *(int *)(puVar8 + 8) + -4;
          uVar4 = *(uint *)((int)&DAT_006b1270 + *piVar7 * 0x2d1);
          if (uVar4 < param_4) {
            uVar4 = param_4;
          }
          param_1[0xf] = uVar4;
        }
        local_424 = local_424 + 1;
      } while ((int)local_424 < param_5);
    }
    param_1[0x18] = DAT_005e18c4;
    param_1[0x19] = DAT_005e18c8;
    param_1[0x1a] = DAT_005e18cc;
    param_1[0x1b] = DAT_005e18d0;
    param_1[0x1c] = DAT_005e18d4;
    param_1[0x1d] = DAT_005e18d8;
    param_1[0x1e] = DAT_005e18dc;
    param_1[0x1f] = DAT_005e18e0;
    param_1[0x20] = DAT_005e18e4;
    FUN_00597870(local_418);
    local_418 = 0;
    local_414 = 0;
    local_410 = 0;
    FUN_00597870(local_408);
    uVar3 = 1;
  }
  ExceptionList = local_c;
  return uVar3;
}

