
void __fastcall FUN_00545910(int param_1)

{
  int iVar1;
  int iVar2;
  int *piVar3;
  
  *(undefined4 *)(param_1 + 0x1c04) = 0;
  iVar2 = *(int *)(param_1 + 0x1c00) + -1;
  if (-1 < iVar2) {
    piVar3 = (int *)(param_1 + iVar2 * 4);
    do {
      iVar1 = (**(code **)(*(int *)*piVar3 + 0x10))();
      if (iVar1 == 0) {
        if ((undefined4 *)*piVar3 != (undefined4 *)0x0) {
          (*(code *)**(undefined4 **)*piVar3)(1);
        }
        iVar1 = *(int *)(param_1 + 0x1c00) + -1;
        if (iVar2 < iVar1) {
          *(int *)(param_1 + 0x1c00) = iVar1;
          *piVar3 = *(int *)(param_1 + iVar1 * 4);
          *(undefined4 *)(param_1 + *(int *)(param_1 + 0x1c00) * 4) = 0;
        }
        else {
          *piVar3 = 0;
          *(int *)(param_1 + 0x1c00) = *(int *)(param_1 + 0x1c00) + -1;
        }
      }
      iVar2 = iVar2 + -1;
      piVar3 = piVar3 + -1;
    } while (-1 < iVar2);
  }
  return;
}

