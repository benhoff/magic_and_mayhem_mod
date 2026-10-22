
void __fastcall FUN_004c5110(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int local_84 [3];
  undefined4 local_78 [26];
  undefined4 local_10;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c1a5b;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00486950();
  local_4 = 0;
  iVar1 = FUN_00597850(0x32);
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    *(undefined4 *)(iVar1 + 4) = 0;
    *(undefined4 *)(iVar1 + 8) = 0;
    *(undefined4 *)(iVar1 + 0xc) = 0;
    *(undefined4 *)(iVar1 + 0x18) = 0;
    *(undefined4 *)(iVar1 + 0x14) = 0;
    *(undefined4 *)(iVar1 + 0x10) = 0;
    *(undefined4 *)(iVar1 + 0x20) = 0;
    *(undefined4 *)(iVar1 + 0x24) = 0;
    *(undefined4 *)(iVar1 + 0x28) = 0;
    *(undefined2 *)(iVar1 + 0x2c) = 0;
    DAT_006f694c = 0;
  }
  *(int *)(param_1 + 0x1a3a) = iVar1;
  if (iVar1 != 0) {
    iVar1 = FUN_00486ae0("Interface\\Grimoire\\Generic\\statbar.bmp");
    if (iVar1 != 0) {
      puVar2 = local_78;
      for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
        *puVar2 = 0;
        puVar2 = puVar2 + 1;
      }
      local_78[0] = 0x6c;
      local_78[1] = 7;
      local_10 = 0x840;
      local_78[3] = *(undefined4 *)(local_84[0] + 4);
      local_78[2] = *(undefined4 *)(local_84[0] + 8);
      FUN_0058ad90(local_78,s_c__Projects_Chaos_Grimoire_CPP_005e1efc,0x41a);
      FUN_0058d240(local_84);
    }
  }
  local_4 = 0xffffffff;
  FUN_00486960();
  ExceptionList = local_c;
  return;
}

