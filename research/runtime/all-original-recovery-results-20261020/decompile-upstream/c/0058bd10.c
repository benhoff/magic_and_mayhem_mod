
bool __thiscall FUN_0058bd10(int param_1,undefined1 *param_2)

{
  int *piVar1;
  undefined1 *puVar2;
  undefined1 *puVar3;
  int iVar4;
  int *unaff_EDI;
  int iVar5;
  undefined1 local_404 [4];
  undefined1 local_400 [1024];
  
  piVar1 = *(int **)(param_1 + 8);
  if (piVar1 == (int *)0x0) {
    return false;
  }
  if (DAT_006f8174 != 0) {
    if (DAT_006f68d8 == 0) {
      iVar5 = 0xec;
      param_2 = param_2 + 0x1e;
    }
    else {
      iVar5 = 0x100;
    }
    if (iVar5 != 0) {
      puVar3 = local_400 + 1;
      iVar4 = iVar5;
      do {
        puVar3[-1] = *param_2;
        puVar2 = param_2 + 2;
        *puVar3 = param_2[1];
        param_2 = param_2 + 3;
        puVar3[1] = *puVar2;
        puVar3[2] = 4;
        puVar3 = puVar3 + 4;
        iVar4 = iVar4 + -1;
      } while (iVar4 != 0);
    }
    iVar4 = (**(code **)(*piVar1 + 0x50))(piVar1,local_404);
    if (iVar4 == 0) {
      iVar5 = (**(code **)(*unaff_EDI + 0x18))(unaff_EDI,0,0,iVar5,&stack0xfffffbf8);
      return iVar5 == 0;
    }
    return false;
  }
  return false;
}

