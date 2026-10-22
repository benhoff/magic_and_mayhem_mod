
int __thiscall FUN_004713a0(int param_1,int param_2)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined4 *puVar9;
  undefined4 *puVar10;
  int *piVar11;
  undefined4 uVar12;
  undefined4 local_14;
  undefined4 local_10;
  undefined4 local_c;
  undefined4 local_4;
  
  FUN_004dc4a0(((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x2a) / 2,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1bc,7);
  FUN_00477060();
  uVar1 = param_2;
  iVar2 = FUN_0059c24f(param_1 + 0xfc5a,1,0x100,param_2);
  if (iVar2 != 0x100) {
    return -1;
  }
  iVar2 = FUN_0059c24f(&DAT_006e980c,4,4,uVar1);
  if (iVar2 != 4) {
    return -1;
  }
  uVar12 = uVar1;
  FUN_00473890(uVar1);
  iVar2 = FUN_004a0ea0(uVar12);
  if (iVar2 == -1) {
    return -1;
  }
  param_2 = DAT_006c4830;
  iVar3 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  iVar3 = FUN_00501a60(uVar1);
  if (iVar3 == -1) {
    return -1;
  }
  iVar4 = FUN_004ffc50(uVar1);
  if (iVar4 == -1) {
    return -1;
  }
  param_2 = *(undefined4 *)(param_1 + 0xfc52);
  iVar5 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar5 != 1) {
    return -1;
  }
  param_2 = iVar2 + iVar3 + 0x118 + iVar4;
  if (*(int *)(param_1 + 0xfc52) != 0) {
    puVar9 = *(undefined4 **)(param_1 + 0xfc2e);
    local_10 = *(undefined4 *)(param_1 + 0xfc3a);
    puVar10 = *(undefined4 **)(param_1 + 0xfc32);
    local_c = *(undefined4 *)(param_1 + 0xfc3e);
    iVar2 = *(int *)(param_1 + 0xfc36);
    local_4 = *(undefined4 *)(param_1 + 0xfc46);
    if (puVar10 != *(undefined4 **)(param_1 + 0xfc42)) {
      do {
        local_14 = FUN_00501940(*puVar10);
        iVar3 = FUN_0059c24f(&local_14,4,1,uVar1);
        if (iVar3 != 1) {
          return -1;
        }
        puVar10 = puVar10 + 1;
        param_2 = param_2 + 4;
        if (puVar10 == puVar9) {
          puVar10 = *(undefined4 **)(iVar2 + 4);
          iVar2 = iVar2 + 4;
          puVar9 = puVar10 + 0x400;
        }
        local_10 = *(undefined4 *)(param_1 + 0xfc3a);
        local_c = *(undefined4 *)(param_1 + 0xfc3e);
        local_4 = *(undefined4 *)(param_1 + 0xfc46);
      } while (puVar10 != *(undefined4 **)(param_1 + 0xfc42));
    }
  }
  iVar2 = FUN_004e0fd0(uVar1);
  if (iVar2 == -1) {
    return -1;
  }
  iVar2 = param_2 + iVar2;
  iVar3 = FUN_004669c0(uVar1);
  if (iVar3 == -1) {
    return -1;
  }
  iVar4 = FUN_0052bcb0(uVar1);
  if (iVar4 == -1) {
    return -1;
  }
  FUN_004dc5e0();
  param_2 = DAT_006e1fa8;
  iVar5 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar5 != 1) {
    return -1;
  }
  param_2 = DAT_006e1f94;
  iVar5 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar5 != 1) {
    return -1;
  }
  param_2 = DAT_006e1f98;
  iVar5 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar5 != 1) {
    return -1;
  }
  iVar5 = FUN_0049d990(uVar1);
  if (iVar5 == -1) {
    return -1;
  }
  iVar6 = FUN_00544ee0(uVar1);
  if (iVar6 == -1) {
    return -1;
  }
  iVar7 = FUN_0057b670();
  if (iVar7 == -1) {
    return -1;
  }
  param_2 = DAT_006e1f3c;
  iVar8 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar8 != 1) {
    return -1;
  }
  iVar7 = iVar2 + iVar3 + iVar4 + iVar5 + iVar6 + 0x10 + iVar7;
  if (DAT_006e1f3c == 0) {
    FUN_0053aa30();
  }
  else {
    iVar2 = FUN_00542cd0(uVar1);
    if (iVar2 == -1) {
      return -1;
    }
    iVar7 = iVar7 + iVar2;
  }
  iVar2 = FUN_005346f0(uVar1);
  if (iVar2 == -1) {
    return -1;
  }
  iVar7 = iVar7 + iVar2;
  piVar11 = &DAT_006581a4;
  do {
    param_2 = *piVar11;
    iVar2 = FUN_0059c24f(&param_2,4,1,uVar1);
    if (iVar2 != 1) {
      return -1;
    }
    iVar2 = 4;
    if (*piVar11 != 0) {
      param_2 = piVar11[1];
      iVar2 = FUN_0059c24f(&param_2,4,1,uVar1);
      if (iVar2 != 1) {
        return -1;
      }
      param_2 = piVar11[2];
      iVar2 = FUN_0059c24f(&param_2,4,1,uVar1);
      if (iVar2 != 1) {
        return -1;
      }
      param_2 = piVar11[3];
      iVar2 = FUN_0059c24f(&param_2,4,1,uVar1);
      if (iVar2 != 1) {
        return -1;
      }
      iVar2 = 0x10;
    }
    piVar11 = piVar11 + 5;
    iVar7 = iVar7 + iVar2;
  } while ((int)piVar11 < 0x658974);
  iVar2 = FUN_0056cbe0(uVar1);
  if (iVar2 == -1) {
    return -1;
  }
  param_2 = *(undefined4 *)(param_1 + 0xfdea);
  iVar3 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100a0);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100a1);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100a3);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100a5);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100a7);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100ad);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2._0_1_ = *(undefined1 *)(param_1 + 0x100ae);
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2 = CONCAT31(param_2._1_3_,*(undefined1 *)(param_1 + 0x100af));
  iVar3 = FUN_0059c24f(&param_2,1,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2 = *(undefined4 *)(param_1 + 0x100a8);
  iVar3 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  param_2 = DAT_006e202c;
  iVar3 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar3 != 1) {
    return -1;
  }
  uVar12 = uVar1;
  FUN_00463cb0(uVar1);
  iVar3 = FUN_00464120(uVar12);
  if (iVar3 == -1) {
    return -1;
  }
  param_2 = DAT_006a2890;
  iVar4 = FUN_0059c24f(&param_2,4,1,uVar1);
  if (iVar4 == 1) {
    param_2 = DAT_00657cdc;
    iVar4 = FUN_0059c24f(&param_2,4,1,uVar1);
    if (iVar4 != 1) {
      return -1;
    }
    FUN_004dc5e0();
    return iVar3 + iVar7 + 0x1c + iVar2;
  }
  return -1;
}

