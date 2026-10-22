
void __thiscall
FUN_004f0e70(int param_1,undefined4 param_2,uint *param_3,uint *param_4,int *param_5)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  int *piVar5;
  
  iVar1 = *(int *)(param_1 + 0x7f4);
  iVar4 = 0;
  if (iVar1 != 0) {
    uVar2 = FUN_0054e200();
    iVar4 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar1);
  }
  if (DAT_006e1f30 != 0) {
    FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0xdf4);
  }
  iVar1 = *(int *)(param_1 + 0x7f4);
  iVar3 = 0;
  if (0 < iVar1) {
    piVar5 = (int *)(param_1 + 0x90);
    do {
      if (*piVar5 == 0) break;
      iVar3 = iVar3 + 1;
      piVar5 = piVar5 + 5;
    } while (iVar3 < iVar1);
  }
  if (iVar3 == iVar1) {
    FUN_0049e680();
    DestroyWindow(DAT_00656614);
  }
  iVar1 = *(int *)(param_1 + 0x90 + iVar4 * 0x14);
  while (iVar1 != 0) {
    iVar1 = *(int *)(param_1 + 0x7f4);
    iVar4 = 0;
    if (iVar1 != 0) {
      uVar2 = FUN_0054e200();
      iVar4 = (int)((longlong)(ulonglong)(uVar2 & 0x7fffffff) % (longlong)iVar1);
    }
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0xe04);
    }
    iVar1 = *(int *)(param_1 + 0x90 + iVar4 * 0x14);
  }
  iVar1 = param_1 + iVar4 * 0x14;
  uVar2 = *(uint *)(param_1 + 0x84 + iVar4 * 0x14);
  if ((int)uVar2 < 0) {
    do {
      uVar2 = uVar2 + DAT_006c5494;
    } while ((int)uVar2 < 0);
  }
  else {
    for (; DAT_006c5494 <= uVar2; uVar2 = uVar2 - DAT_006c5494) {
    }
  }
  *param_3 = uVar2;
  uVar2 = *(uint *)(iVar1 + 0x88);
  if ((int)uVar2 < 0) {
    do {
      uVar2 = uVar2 + DAT_006c5498;
    } while ((int)uVar2 < 0);
  }
  else {
    for (; DAT_006c5498 <= uVar2; uVar2 = uVar2 - DAT_006c5498) {
    }
  }
  *param_4 = uVar2;
  *param_5 = *(int *)(param_1 + (iVar4 * 5 + 0x23) * 4);
  *(undefined4 *)(iVar1 + 0x90) = 1;
  if (*(int *)(DAT_0065660c + 0x181c8) == 0x10) {
    *param_5 = *param_5 + 1;
  }
  return;
}

