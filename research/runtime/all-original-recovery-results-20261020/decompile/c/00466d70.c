
undefined4 __fastcall FUN_00466d70(int param_1,int param_2)

{
  int iVar1;
  uint uVar2;
  uint uVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30 [4];
  undefined1 local_20;
  undefined4 local_1f;
  undefined1 local_1b;
  undefined4 local_18 [6];
  
  uVar4 = *(int *)(param_1 + 8) - param_2;
  uVar2 = *(int *)(param_1 + 8) + param_2;
  uVar4 = ((int)uVar4 < 0) - 1 & uVar4;
  uVar3 = DAT_006c5494;
  if ((int)uVar2 <= (int)DAT_006c5494) {
    uVar3 = uVar2;
  }
  if (uVar4 != uVar3) {
    uVar2 = FUN_0054e200();
    uVar4 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)(int)(uVar3 - uVar4)) +
            uVar4;
  }
  local_38 = FUN_0040e650(uVar4);
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_artifact_cpp_005d9a14,0x8d);
  }
  uVar4 = *(int *)(param_1 + 0xc) - param_2;
  uVar2 = *(int *)(param_1 + 0xc) + param_2;
  uVar4 = ((int)uVar4 < 0) - 1 & uVar4;
  uVar3 = DAT_006c5498;
  if ((int)uVar2 <= (int)DAT_006c5498) {
    uVar3 = uVar2;
  }
  if (uVar4 != uVar3) {
    uVar2 = FUN_0054e200();
    uVar4 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)(int)(uVar3 - uVar4)) +
            uVar4;
  }
  local_3c = FUN_004132e0(uVar4);
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_artifact_cpp_005d9a14,0x8e);
  }
  iVar5 = DAT_006c549c;
  if (DAT_006c549c == 0) {
    local_34 = 0;
  }
  else {
    uVar3 = FUN_0054e200();
    local_34 = (undefined4)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar5);
  }
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_artifact_cpp_005d9a14,0x8f);
  }
  iVar5 = *(int *)(param_1 + 0xa8);
  local_20 = 0;
  local_1b = 0;
  local_30[0] = *(undefined4 *)((int)&DAT_006a5f8c + iVar5 * 0x5c9);
  iVar1 = iVar5 * 0x5c9;
  local_30[2] = *(undefined4 *)(&DAT_006a5fbc + iVar5 * 0x5c9);
  local_30[1] = *(undefined4 *)((int)&DAT_006a5f88 + iVar1);
  local_30[3] = *(undefined4 *)((int)&DAT_006a5fc4 + iVar1);
  local_1f = *(undefined4 *)((int)&DAT_006a5f80 + iVar1);
  puVar6 = local_30;
  puVar7 = local_18;
  for (iVar5 = 5; iVar5 != 0; iVar5 = iVar5 + -1) {
    *puVar7 = *puVar6;
    puVar6 = puVar6 + 1;
    puVar7 = puVar7 + 1;
  }
  *(undefined2 *)puVar7 = *(undefined2 *)puVar6;
  iVar5 = FUN_004f5230(0xffffffff,local_18,&local_38,&local_3c,&local_34);
  if (iVar5 != 0) {
    FUN_0051e850(local_38,local_3c,local_34,0);
    return 1;
  }
  return 0;
}

