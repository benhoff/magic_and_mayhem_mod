
void __thiscall
FUN_0058d5b0(undefined4 param_1,undefined4 param_2,undefined4 param_3,undefined4 *param_4,
            undefined4 param_5,undefined4 param_6,undefined4 param_7)

{
  int *piVar1;
  undefined4 *puVar2;
  int iVar3;
  
  iVar3 = 0;
  puVar2 = &DAT_006f6b50;
  if (DAT_006f6b50 != 0) {
    do {
      if (0x7f < iVar3) {
        return;
      }
      piVar1 = puVar2 + 0xb;
      puVar2 = puVar2 + 0xb;
      iVar3 = iVar3 + 1;
    } while (*piVar1 != 0);
    if (0x7f < iVar3) {
      return;
    }
  }
  puVar2[2] = param_1;
  puVar2[3] = param_3;
  puVar2[1] = param_7;
  puVar2[4] = param_2;
  if (param_4 != (undefined4 *)0x0) {
    puVar2[5] = *param_4;
    puVar2[6] = param_4[1];
    puVar2[7] = param_4[2];
    puVar2[8] = param_4[3];
  }
  puVar2[9] = param_5;
  puVar2[10] = param_6;
  *puVar2 = 1;
  DAT_006f98d4 = DAT_006f98d4 + 1;
  return;
}

