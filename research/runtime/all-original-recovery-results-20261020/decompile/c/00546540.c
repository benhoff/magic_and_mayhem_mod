
uint __thiscall FUN_00546540(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint uVar1;
  
  uVar1 = *(uint *)(param_1 + 0x60);
  if (((uVar1 == 0) || (*(int *)(uVar1 + 0xe4) == 0)) ||
     ((-1 < DAT_00644520 && ((DAT_00644520 < 8 && (*(int *)(uVar1 + 0x24 + DAT_00644520 * 4) != 0)))
      ))) {
    uVar1 = FUN_00545c10(param_2,param_3);
  }
  return uVar1 & 0xffff0000;
}

