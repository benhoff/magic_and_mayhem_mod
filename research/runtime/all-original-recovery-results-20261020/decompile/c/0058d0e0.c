
/* WARNING: Removing unreachable block (ram,0x0058d10b) */
/* WARNING: Removing unreachable block (ram,0x0058d0ff) */
/* WARNING: Removing unreachable block (ram,0x0058d117) */

bool __thiscall FUN_0058d0e0(int param_1,byte param_2,byte param_3,byte param_4)

{
  uint uVar1;
  int iVar2;
  uint local_8;
  uint local_4;
  
  if (DAT_006e1f88 == 0) {
    uVar1 = ((param_2 & 0xfff8) << 5 | param_3 & 0xfc) << 3;
  }
  else {
    uVar1 = ((param_2 & 0xf8) << 5 | param_3 & 0xf8) << 2;
  }
  uVar1 = uVar1 | param_4 >> 3;
  local_8 = uVar1;
  local_4 = uVar1;
  iVar2 = (**(code **)(**(int **)(param_1 + 8) + 0x74))(*(int **)(param_1 + 8),8,&local_8);
  if (iVar2 != 0) {
    FUN_00484c00(iVar2);
  }
  *(short *)(param_1 + 0x2c) = (short)uVar1;
  return uVar1 == 0;
}

