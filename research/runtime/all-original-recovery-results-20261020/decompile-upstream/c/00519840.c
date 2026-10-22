
undefined4 __fastcall FUN_00519840(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  
  iVar2 = FUN_005198b0(*(undefined4 *)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),
                       *(undefined4 *)(param_1 + 0x10));
  if (iVar2 != 0) {
    iVar1 = *(int *)(param_1 + 0x88);
    uVar3 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x315b);
    }
    if (0x2d < (int)((uVar3 & 0x7fffffff) % 0x23 + iVar1 + iVar2 * 2)) {
      return 1;
    }
  }
  return 0;
}

