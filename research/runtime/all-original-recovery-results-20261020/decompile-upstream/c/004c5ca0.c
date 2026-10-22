
void __fastcall FUN_004c5ca0(int param_1)

{
  int iVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  
  piVar4 = (int *)(param_1 + 0x26);
  iVar3 = 2;
  do {
    iVar2 = 2;
    do {
      iVar1 = *piVar4;
      if (iVar1 != 0) {
        FUN_0058b3e0();
        FUN_00597870(iVar1);
        *piVar4 = 0;
      }
      piVar4 = piVar4 + 1;
      iVar2 = iVar2 + -1;
    } while (iVar2 != 0);
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
  return;
}

