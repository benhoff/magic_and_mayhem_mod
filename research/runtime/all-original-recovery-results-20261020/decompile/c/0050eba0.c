
void __fastcall FUN_0050eba0(int param_1)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = FUN_00464ec0();
  if (*(int *)(param_1 + 0x7cd) == 1) {
    FUN_00464ec0();
  }
  if (iVar1 == 2) {
    FUN_0050ec80();
    return;
  }
  if (iVar1 == 3) {
    FUN_00570480(param_1);
    FUN_00571310(0x2bf,*(undefined4 *)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),
                 *(undefined4 *)(param_1 + 0x10),0,0,0);
    return;
  }
  if (iVar1 == 1) {
    FUN_0050db60(0xf);
    if (*(int *)(param_1 + 0x688) != 0) {
      *(undefined4 *)(param_1 + 0x66c) = 10;
      return;
    }
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x1bc9);
    }
    *(uint *)(param_1 + 0x66c) = (uVar2 & 0x7fffffff) % 3 + 2;
  }
  return;
}

