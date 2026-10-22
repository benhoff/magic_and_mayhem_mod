
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

uint __fastcall FUN_0058d540(int param_1)

{
  uint *puVar1;
  int iVar2;
  uint local_20 [4];
  uint uStack_10;
  
  puVar1 = local_20;
  for (iVar2 = 8; iVar2 != 0; iVar2 = iVar2 + -1) {
    *puVar1 = 0;
    puVar1 = puVar1 + 1;
  }
  local_20[0] = 0x20;
  iVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x54))(*(int **)(param_1 + 8),local_20);
  if (iVar2 != 0) {
    FUN_00484c00(iVar2);
  }
  _DAT_006e1f80 = (short)uStack_10;
  DAT_006e1f7c = (short)local_20[2];
  DAT_006e1f7e = (short)local_20[3];
  return ~(uStack_10 | local_20[3] | local_20[2]) >> 0xf & 1;
}

