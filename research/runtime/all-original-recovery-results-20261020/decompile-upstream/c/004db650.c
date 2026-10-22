
void __fastcall FUN_004db650(int param_1)

{
  int iVar1;
  int *piVar2;
  int local_6c [26];
  undefined4 local_4;
  
  piVar2 = local_6c;
  for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  local_6c[3] = *(undefined4 *)(param_1 + 0x10);
  local_6c[2] = *(int *)(param_1 + 0xc) * 8 + 8;
  local_6c[0] = 0x6c;
  local_6c[1] = 7;
  local_4 = 0x40;
  FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x20df);
  FUN_0058bc10(0x1f);
  FUN_0058d0e0(0,0,0xff);
  return;
}

