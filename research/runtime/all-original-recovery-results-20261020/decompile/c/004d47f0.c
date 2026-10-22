
void __fastcall FUN_004d47f0(int param_1)

{
  int iVar1;
  undefined4 *puVar2;
  int iVar3;
  int *piVar4;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c [26];
  undefined4 local_4;
  
  if (*(int *)(param_1 + 0x82) == 0) {
    iVar1 = *(int *)(param_1 + 0x15);
    piVar4 = local_6c;
    for (iVar3 = 0x1b; iVar3 != 0; iVar3 = iVar3 + -1) {
      *piVar4 = 0;
      piVar4 = piVar4 + 1;
    }
    local_6c[2] = (*(int *)(param_1 + 0x1d) - iVar1) + 4;
    local_6c[3] = (*(int *)(param_1 + 0x19) - *(int *)(param_1 + 0x11)) + 8;
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x13d6);
    if (*(int *)(param_1 + 0xb1) == 0) {
      puVar2 = (undefined4 *)FUN_00597850(0xc);
      if (puVar2 == (undefined4 *)0x0) {
        puVar2 = (undefined4 *)0x0;
      }
      else {
        puVar2[1] = &LAB_004d4140;
        puVar2[2] = param_1;
        *puVar2 = &PTR_LAB_005c6e80;
      }
      *(undefined4 **)(param_1 + 0xb1) = puVar2;
      *(undefined4 **)(param_1 + 0xa6) = puVar2;
    }
  }
  iVar1 = *(int *)(*(int *)(param_1 + 0x31) + 0x20);
  iVar3 = *(int *)(*(int *)(param_1 + 0x31) + 0x24);
  local_7c = (*(int *)(param_1 + 0x11) - iVar1) + -4;
  local_78 = *(int *)(param_1 + 0x15) - iVar3;
  local_70 = (*(int *)(param_1 + 0x1d) - iVar3) + 4;
  local_74 = (*(int *)(param_1 + 0x19) - iVar1) + 4;
  if (DAT_006e1ee4 != 0) {
    FUN_0058d5b0(2,param_1 + 0x7e,&local_7c,0,0,1);
    *(undefined1 *)(param_1 + 0xb0) = 1;
    return;
  }
  FUN_0058c4a0(param_1 + 0x7e,&local_7c,0,0);
  *(undefined1 *)(param_1 + 0xb0) = 1;
  return;
}

