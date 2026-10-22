
void __fastcall FUN_004b5ac0(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  uint *puVar3;
  uint local_178 [26];
  undefined4 local_110;
  undefined1 local_10c [256];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c0d4b;
  pvStack_c = ExceptionList;
  DAT_006f694c = 0;
  puVar3 = local_178;
  for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar3 = 0;
    puVar3 = puVar3 + 1;
  }
  local_4 = 0;
  local_178[0] = 0x6c;
  local_178[1] = 7;
  local_110 = 0x840;
  local_178[3] = ((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280) / 2;
  local_178[2] = ((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0) / 2;
  ExceptionList = &pvStack_c;
  FUN_0058ad90(local_178,s_c__Projects_Chaos_game_cpp_005e05c0,0x1ef1);
  FUN_005908e0();
  uVar1 = FUN_00593a80();
  FUN_0059c1fd(local_10c,s_Interface_CharacterScreen__ix_i__005e0f94,
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,uVar1);
  iVar2 = FUN_0058d320(local_10c);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  else {
    FUN_0058d0e0(0,0,0xff);
    FUN_0058db80(0,0,0xff,0xf);
    if (DAT_006e1ee4 == 0) {
      FUN_0058c140(*(undefined4 *)(param_1 + 0x13),0,0);
    }
    else {
      FUN_0058d5b0(1,*(undefined4 *)(param_1 + 0x13),0,0,0,1);
    }
  }
  FUN_0058b3e0();
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return;
}

