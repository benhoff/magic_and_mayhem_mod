
undefined4 __fastcall FUN_0058be90(int param_1)

{
  undefined4 *puVar1;
  undefined1 *puVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined4 local_400 [252];
  int iStack_10;
  
  puVar1 = local_400;
  for (iVar3 = 0x100; iVar3 != 0; iVar3 = iVar3 + -1) {
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
  }
  if (DAT_006f68d8 == 0) {
    iVar3 = (**(code **)(**(int **)(param_1 + 0xc) + 0x10))(*(int **)(param_1 + 0xc));
    if (iVar3 != 0) {
      return 0;
    }
  }
  else {
    iVar3 = (**(code **)(**(int **)(param_1 + 0xc) + 0x10))(*(int **)(param_1 + 0xc));
    if (iVar3 != 0) {
      return 0;
    }
  }
  iVar3 = 0x100;
  puVar2 = (undefined1 *)(iStack_10 + 1);
  puVar4 = &stack0xfffffbed;
  do {
    puVar2[-1] = puVar4[-1];
    *puVar2 = *puVar4;
    puVar2[1] = puVar4[1];
    puVar2 = puVar2 + 3;
    iVar3 = iVar3 + -1;
    puVar4 = puVar4 + 4;
  } while (iVar3 != 0);
  return 1;
}

