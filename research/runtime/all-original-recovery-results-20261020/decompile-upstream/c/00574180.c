
bool FUN_00574180(void)

{
  char cVar1;
  uint uVar2;
  uint uVar3;
  int iVar4;
  char *in_stack_00000018;
  char *in_stack_0000001c;
  int iStack0000002c;
  int iStack00000030;
  int in_stack_00000044;
  int in_stack_00000048;
  int in_stack_0000004c;
  int in_stack_00000050;
  undefined4 local_64;
  undefined4 local_60;
  undefined4 local_5c;
  undefined4 local_58;
  undefined4 local_54;
  undefined4 local_50;
  undefined4 local_4c;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined2 local_1c;
  int *local_12;
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2c98;
  pvStack_c = ExceptionList;
  local_44 = 0;
  local_40 = 0;
  local_3c = 0;
  local_30 = 0;
  local_34 = 0;
  local_38 = 0;
  local_28 = 0;
  local_24 = 0;
  local_20 = 0;
  local_1c = 0;
  DAT_006f694c = 0;
  local_4c = 0;
  local_54 = 0;
  local_50 = 0;
  local_12 = (int *)0x0;
  if (DAT_006de6d5 == 0) {
    if (DAT_006de6d9 == 0) {
      iVar4 = (in_stack_00000050 - in_stack_00000048) + 0xf0;
    }
    else {
      iVar4 = (in_stack_00000050 - in_stack_00000048) + 0xaa;
    }
    iStack0000002c = (((in_stack_0000004c - in_stack_00000044) + 0x140) * 100) / 0x50;
    iStack00000030 = (iVar4 * 100) / 0x50;
  }
  else {
    iStack0000002c = (in_stack_0000004c - in_stack_00000044) + 400;
    if (DAT_006de6d9 == 0) {
      iStack00000030 = (in_stack_00000050 - in_stack_00000048) + 300;
    }
    else {
      iStack00000030 = (in_stack_00000050 - in_stack_00000048) + 0xe6;
    }
  }
  local_64 = 0;
  local_60 = 0;
  local_5c = 800;
  local_58 = 600;
  if (DAT_006de6d9 != 0) {
    local_58 = 0x1cc;
  }
  uVar2 = 0xffffffff;
  do {
    if (uVar2 == 0) break;
    uVar2 = uVar2 - 1;
    cVar1 = *in_stack_00000018;
    in_stack_00000018 = in_stack_00000018 + 1;
  } while (cVar1 != '\0');
  uVar3 = 0xffffffff;
  do {
    if (uVar3 == 0) break;
    uVar3 = uVar3 - 1;
    cVar1 = *in_stack_0000001c;
    in_stack_0000001c = in_stack_0000001c + 1;
  } while (cVar1 != '\0');
  ExceptionList = &pvStack_c;
  iVar4 = FUN_005736e0(&local_64,&stack0x0000002c,~uVar3 - 1,~uVar2 - 1);
  local_4 = 0;
  if (local_12 != (int *)0x0) {
    (**(code **)(*local_12 + 4))(1);
    local_12 = (int *)0x0;
  }
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return iVar4 != -1;
}

