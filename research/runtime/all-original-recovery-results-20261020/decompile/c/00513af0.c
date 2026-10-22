
undefined4 __fastcall FUN_00513af0(int param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  
  iVar1 = **(int **)(param_1 + 0xac);
  if ((((iVar1 != 0) && (iVar1 != 0x18)) && (iVar1 != 0x19)) &&
     ((iVar1 != 0x1a && ((*(int *)(param_1 + 0xa0) == 1 || (*(int *)(param_1 + 0xa0) == 2)))))) {
    iVar1 = *(int *)(param_1 + 0x10);
    iVar3 = 0;
    if (DAT_006c5c80 < iVar1) {
      uVar2 = (uint)*(ushort *)
                     (DAT_006c54dc +
                     ((&DAT_006cb8c2)[iVar1] + *(int *)(param_1 + 8) +
                     (&DAT_006cb942)[*(int *)(param_1 + 0xc)]) * 0xc);
      if (uVar2 == 0) {
        uVar2 = (uint)*(ushort *)
                       (DAT_006c54dc +
                       ((&DAT_006cb8be)[iVar1] + *(int *)(param_1 + 8) +
                       (&DAT_006cb942)[*(int *)(param_1 + 0xc)]) * 0xc);
      }
      iVar3 = (*(int *)(param_1 + 0xa4) - iVar1) * 10;
      if (*(int *)(DAT_0065660c + 0xf0 + uVar2 * 0x164) == 1) {
        iVar3 = iVar3 / 2;
      }
    }
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x25bb);
    }
    if ((int)((uVar2 & 0x7fffffff) % 99 + 1) < iVar3) {
      if (*(int *)(param_1 + 0xa0) == 2) {
        FUN_00513e40(DAT_006c305e);
        return 1;
      }
      FUN_00513e40(DAT_006b9577);
      return 1;
    }
  }
  return 0;
}

