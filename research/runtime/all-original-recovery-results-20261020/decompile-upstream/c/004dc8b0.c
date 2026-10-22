
void __fastcall FUN_004dc8b0(int param_1)

{
  undefined4 *puVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int local_7c;
  int local_78;
  int local_74;
  int local_70;
  int local_6c [26];
  undefined4 local_4;
  
  iVar5 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x1b4;
  if (*(int *)(param_1 + 0x6d) == 0) {
    iVar3 = 400;
    iVar4 = 100;
  }
  else {
    iVar3 = *(int *)(param_1 + 0x79);
    iVar4 = *(int *)(param_1 + 0x7d);
  }
  if (*(int *)(param_1 + 0x85) == 0) {
    piVar6 = local_6c;
    for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
      *piVar6 = 0;
      piVar6 = piVar6 + 1;
    }
    local_6c[0] = 0x6c;
    local_6c[1] = 7;
    local_4 = 0x40;
    local_6c[2] = iVar4;
    local_6c[3] = iVar3;
    FUN_0058ad90(local_6c,s_c__Projects_Chaos_Intrface_cpp_005e2e58,0x232d);
    FUN_0058d0e0(0,0,0xff);
    *(uint *)(param_1 + 0xa5) = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x154 + (0x8c - iVar4) / 2;
    *(int *)(param_1 + 0xa1) = (iVar5 - iVar3) / 2;
  }
  if (*(int *)(param_1 + 0x130) == 0) {
    puVar1 = (undefined4 *)FUN_00597850(0x10);
    if (puVar1 == (undefined4 *)0x0) {
      puVar1 = (undefined4 *)0x0;
    }
    else {
      puVar1[1] = &LAB_004dc710;
      puVar1[2] = 0;
      puVar1[3] = param_1;
      *puVar1 = &PTR_LAB_005c6f60;
    }
    *(undefined4 **)(param_1 + 0x130) = puVar1;
    *(undefined4 **)(param_1 + 0xa9) = puVar1;
  }
  *(undefined1 *)(param_1 + 0x29) = 0;
  FUN_004dcab0();
  local_7c = (iVar5 - iVar3) / 2;
  local_74 = local_7c + iVar3;
  local_78 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x154 + (0x8c - iVar4) / 2;
  local_70 = local_78 + iVar4;
  (**(code **)(*(int *)(param_1 + 8) + 8))(&local_7c,1);
  return;
}

