
void __fastcall FUN_004db710(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int *piVar3;
  int local_6c [26];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0x128) == 0) {
    piVar3 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *piVar3 = 0;
      piVar3 = piVar3 + 1;
    }
    local_6c[3] = *(undefined4 *)(param_1 + 0x10);
    local_6c[2] = *(int *)(param_1 + 0xc) * 8 + 8;
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x2102);
    FUN_0058bc10(0x1f);
    FUN_0058d0e0(0,0,0xff);
  }
  if (*(int *)(param_1 + 0x186) == 0) {
    puVar1 = (undefined4 *)FUN_00597850(0xc);
    if (puVar1 != (undefined4 *)0x0) {
      puVar1[1] = FUN_004db500;
      puVar1[2] = param_1;
      *puVar1 = &PTR_LAB_005c6f34;
      *(undefined4 **)(param_1 + 0x186) = puVar1;
      *(undefined4 **)(param_1 + 0x14c) = puVar1;
      return;
    }
    *(undefined4 *)(param_1 + 0x186) = 0;
    *(undefined4 *)(param_1 + 0x14c) = 0;
  }
  return;
}

