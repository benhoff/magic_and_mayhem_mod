
void __fastcall FUN_005458b0(int *param_1)

{
  int iVar1;
  int *piVar2;
  
  iVar1 = 0;
  piVar2 = param_1;
  if (param_1[0x700] < 1) {
    param_1[0x700] = 0;
    param_1[0x701] = 0;
    return;
  }
  do {
    if ((undefined4 *)*piVar2 != (undefined4 *)0x0) {
      (*(code *)**(undefined4 **)*piVar2)(1);
    }
    *piVar2 = 0;
    iVar1 = iVar1 + 1;
    piVar2 = piVar2 + 1;
  } while (iVar1 < param_1[0x700]);
  param_1[0x700] = 0;
  param_1[0x701] = 0;
  return;
}

