
undefined4 __fastcall FUN_0054aa30(int param_1)

{
  int iVar1;
  
  iVar1 = FUN_005456c0();
  *(int *)(param_1 + 0x18) = *(int *)(param_1 + 0x18) + -1;
  if ((iVar1 != 0) && (0 < *(int *)(param_1 + 0xc))) {
    return 1;
  }
  return 0;
}

