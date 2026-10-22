
bool __thiscall FUN_0048ec30(int param_1,int param_2,int param_3)

{
  int iVar1;
  uint uVar2;
  
  iVar1 = *(int *)(param_1 + 0x4c) * 0x2d1;
  if (((*(int *)((int)&DAT_006b13d5 + iVar1) == 0) && (*(int *)((int)&DAT_006b13d9 + iVar1) == 0))
     && (*(int *)(&DAT_006b13dd + iVar1) == 0)) {
    return true;
  }
  uVar2 = FUN_0054e200();
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0xf2a);
  }
  return (uVar2 & 0x7fffffff) % 100 < (uint)(param_2 * 100) / (uint)(param_2 + param_3);
}

