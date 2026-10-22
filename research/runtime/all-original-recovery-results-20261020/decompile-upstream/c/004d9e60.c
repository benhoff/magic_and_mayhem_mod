
void __fastcall FUN_004d9e60(int param_1)

{
  undefined4 uVar1;
  
  uVar1 = FUN_0058a680(*(undefined4 *)(param_1 + 0x139));
  FUN_004d9f30(uVar1);
  FUN_0059cb60(param_1 + 0x139,param_1 + 0x13d,*(int *)(param_1 + 0x161) * 4 + -4);
  *(int *)(param_1 + 0x161) = *(int *)(param_1 + 0x161) + -1;
  return;
}

