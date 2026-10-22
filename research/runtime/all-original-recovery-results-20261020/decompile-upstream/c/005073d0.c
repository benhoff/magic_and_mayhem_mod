
void __fastcall FUN_005073d0(int param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  
  iVar1 = *(int *)(param_1 + 0xa8);
  if ((iVar1 != 0x1b) && (*(int *)(param_1 + 0xe4) != 0)) {
    if ((*(int *)(param_1 + 0x68) == 0) &&
       (((*(int *)(param_1 + 0x174) == DAT_00644520 || (DAT_006e1f0c != 0)) && (iVar1 != 0x16)))) {
      *(undefined4 *)(param_1 + 0x68) = 1;
      DAT_006e984c = DAT_006e984c + 1;
      DAT_006903e8 = (uint)(DAT_006e984c != 0);
      if (*(int *)(param_1 + 0x6e8) != 0) {
        FUN_004943f0();
        *(undefined4 *)(param_1 + 0x6e8) = 0;
      }
      if ((*(int *)(*(int *)(param_1 + 0x8f9) + 8) != 0) &&
         (piVar2 = *(int **)(*(int *)(param_1 + 0x8f9) + 4), piVar3 = (int *)*piVar2,
         piVar3 != piVar2)) {
        do {
          if ((piVar3[2] == 2) && (piVar3[10] != 0)) {
            FUN_004943f0();
            piVar3[10] = 0;
          }
          piVar3 = (int *)*piVar3;
        } while (piVar3 != (int *)*(int *)(*(int *)(param_1 + 0x8f9) + 4));
      }
      FUN_005120c0();
      iVar1 = **(int **)(param_1 + 0xac);
      if (((((iVar1 == 0) || (iVar1 == 0x18)) || (iVar1 == 0x19)) || (iVar1 == 0x1a)) &&
         (*(int *)(param_1 + 0x7c) == 0)) {
        FUN_004d3c90();
        return;
      }
    }
    else if ((*(int *)(param_1 + 0x174) != DAT_00644520) || (iVar1 == 0x16)) {
      *(undefined4 *)(param_1 + 0x70) = 1;
    }
  }
  return;
}

