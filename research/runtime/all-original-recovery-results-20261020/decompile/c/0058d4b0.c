
uint __fastcall FUN_0058d4b0(int param_1)

{
  uint uVar1;
  int *piVar2;
  undefined1 local_8 [8];
  
  piVar2 = *(int **)(param_1 + 4);
  uVar1 = 0;
  if (piVar2 != (int *)0x0) {
    uVar1 = (**(code **)(*piVar2 + 0x40))(piVar2,8,local_8);
    if (uVar1 == 0) {
      return (uint)piVar2 & 0xffff;
    }
  }
  return uVar1 & 0xffff0000;
}

