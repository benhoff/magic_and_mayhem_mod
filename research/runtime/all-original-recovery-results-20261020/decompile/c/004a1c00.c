
undefined4 FUN_004a1c00(undefined4 *param_1,int param_2)

{
  byte bVar1;
  undefined4 uVar2;
  int iVar3;
  uint uVar4;
  uint *puVar5;
  uint *puVar6;
  uint uVar7;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c047b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_0054e170(0x499602d2);
  local_4 = 0;
  if (param_2 < 0xd) {
    uVar2 = 0;
  }
  else {
    puVar5 = param_1 + 1;
    FUN_0054e170(*param_1);
    uVar7 = param_2 - 4;
    iVar3 = (int)(uVar7 + ((int)uVar7 >> 0x1f & 3U)) >> 2;
    puVar6 = puVar5;
    if (0 < iVar3) {
      do {
        uVar4 = FUN_0054e200();
        puVar6 = puVar5 + 1;
        iVar3 = iVar3 + -1;
        *puVar5 = *puVar5 ^ uVar4;
        puVar5 = puVar6;
      } while (iVar3 != 0);
    }
    uVar7 = uVar7 & 0x80000003;
    if ((int)uVar7 < 0) {
      uVar7 = (uVar7 - 1 | 0xfffffffc) + 1;
    }
    if (0 < (int)uVar7) {
      do {
        bVar1 = FUN_0054e200();
        uVar7 = uVar7 - 1;
        *(byte *)puVar6 = (byte)*puVar6 ^ bVar1;
      } while (uVar7 != 0);
    }
    uVar2 = 1;
  }
  ExceptionList = local_c;
  return uVar2;
}

