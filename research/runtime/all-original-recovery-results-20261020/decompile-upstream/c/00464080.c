
void __thiscall FUN_00464080(int param_1,char param_2)

{
  int iVar1;
  int iVar2;
  
  iVar1 = 0;
  iVar2 = param_1;
  if (*(int *)(param_1 + 0x1c5) < 1) {
    *(char *)(param_1 + 0x1ca) = param_2;
    return;
  }
  do {
    iVar1 = iVar1 + 1;
    if (*(char *)(iVar2 + 0x8d) != param_2) {
      *(char *)(iVar2 + 0x8d) = param_2;
      if (param_2 == '\0') {
        *(undefined4 *)(iVar2 + 8) = 1;
      }
      else {
        FUN_004a5000();
      }
      if (*(char *)(iVar2 + 0x8d) == '\0') {
        FUN_004638a0();
      }
      else {
        FUN_0058eac0(0,0,0xff,0);
      }
    }
    iVar2 = iVar2 + 0x97;
  } while (iVar1 < *(int *)(param_1 + 0x1c5));
  *(char *)(param_1 + 0x1ca) = param_2;
  return;
}

