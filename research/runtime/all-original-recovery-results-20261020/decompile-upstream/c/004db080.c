
void FUN_004db080(void)

{
  undefined4 *puVar1;
  int extraout_ECX;
  int iVar2;
  char *pcVar3;
  char *pcVar4;
  char *pcVar5;
  undefined4 in_stack_00000014;
  undefined4 in_stack_00000038;
  undefined *in_stack_0000003c;
  void *in_stack_00001298;
  uint in_stack_000012a8;
  void *local_c;
  undefined1 *puStack_8;
  int local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2139;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_0059c000();
  in_stack_00000038 = DAT_005e30c4;
  in_stack_0000003c = PTR_DAT_005e30c8;
  puVar1 = (undefined4 *)&stack0x00000040;
  for (iVar2 = 0x3e; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
  }
  pcVar5 = s_Scroll_not_defined_in_Scroll_txt_005e30a0;
  pcVar3 = &stack0x00000a98;
  for (iVar2 = 8; iVar2 != 0; iVar2 = iVar2 + -1) {
    *(undefined4 *)pcVar3 = *(undefined4 *)pcVar5;
    pcVar5 = pcVar5 + 4;
    pcVar3 = pcVar3 + 4;
  }
  *pcVar3 = *pcVar5;
  puVar1 = (undefined4 *)&stack0x00000ab9;
  for (iVar2 = 0x1f7; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
  }
  *(undefined2 *)puVar1 = 0;
  *(undefined1 *)((int)puVar1 + 2) = 0;
  FUN_004c1d40();
  FUN_004c7960();
  local_4 = extraout_ECX + 0x29;
  *(undefined4 *)(extraout_ECX + 0x25) = 1;
  *(uint *)(extraout_ECX + 0x213) = in_stack_000012a8;
  FUN_0058d1a0();
  if (*(int *)(extraout_ECX + 4) == -1) {
    FUN_004a4e20();
  }
  local_c = *(void **)(extraout_ECX + 0x3d);
  puVar1 = (undefined4 *)FUN_00597850();
  if (puVar1 == (undefined4 *)0x0) {
    puVar1 = (undefined4 *)0x0;
  }
  else {
    FUN_0056d4e0();
    *puVar1 = &PTR_LAB_005c6efc;
    puVar1[0xc] = local_4;
  }
  *(undefined4 **)(extraout_ECX + 0x20f) = puVar1;
  puVar1[6] = 0xffffffff;
  *(undefined4 *)(*(int *)(extraout_ECX + 0x20f) + 0x20) = 2;
  iVar2 = FUN_0056d820();
  *(int *)(extraout_ECX + 0x73) = (int)(0x15c / (longlong)iVar2);
  FUN_0056f000();
  *(undefined4 *)(*(int *)(extraout_ECX + 0x20f) + 0x24) = in_stack_00000014;
  FUN_004a5a90();
  if (in_stack_000012a8 < *(uint *)(extraout_ECX + 0x207)) {
    pcVar5 = &stack0x00000a98;
    local_c = (void *)0x0;
    pcVar3 = *(char **)(extraout_ECX + 0x77 + in_stack_000012a8 * 4);
    do {
      if (*pcVar3 == '~') {
        pcVar4 = pcVar3 + 1;
        if (pcVar3[1] == 'B') {
          pcVar4 = pcVar3 + 2;
          if (pcVar3[2] == 'S') {
            pcVar4 = pcVar3 + 3;
            iVar2 = FUN_0059c73c();
            while (iVar2 != 0) {
              pcVar4 = pcVar4 + 1;
              iVar2 = FUN_0059c73c();
            }
            pcVar5 = &stack0x00000a98;
          }
          else if (pcVar3[2] == 'E') {
            *pcVar5 = '\0';
            local_c = (void *)0x1;
          }
        }
        else if (pcVar3[1] == 'H') {
          pcVar4 = pcVar3 + 2;
          if (pcVar3[2] == 'S') {
            pcVar4 = pcVar3 + 3;
            iVar2 = FUN_0059c73c();
            while (iVar2 != 0) {
              pcVar4 = pcVar4 + 1;
              iVar2 = FUN_0059c73c();
            }
            pcVar5 = (char *)&stack0x00000038;
          }
          else if (pcVar3[2] == 'E') {
            pcVar4 = pcVar3 + 3;
            iVar2 = FUN_0059c73c();
            while (iVar2 != 0) {
              pcVar4 = pcVar4 + 1;
              iVar2 = FUN_0059c73c();
            }
            *pcVar5 = '\0';
          }
        }
      }
      else {
        *pcVar5 = *pcVar3;
        pcVar5 = pcVar5 + 1;
        pcVar4 = pcVar3 + 1;
      }
      if (pcVar4 == *(char **)(extraout_ECX + 0x7b + in_stack_000012a8 * 4)) {
        *pcVar5 = '\0';
        break;
      }
      pcVar3 = pcVar4;
    } while (local_c == (void *)0x0);
  }
  FUN_004c1d70();
  FUN_004c1e10();
  FUN_004c79f0();
  FUN_004c1d80();
  FUN_004ca0c0();
  FUN_004c9fd0();
  FUN_0046ee20();
  FUN_004c1d60();
  ExceptionList = in_stack_00001298;
  return;
}

