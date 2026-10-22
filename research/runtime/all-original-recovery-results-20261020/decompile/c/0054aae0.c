
int __fastcall FUN_0054aae0(int param_1)

{
  int iVar1;
  
  iVar1 = *(int *)(param_1 + 0x2c) * 0x400;
  return ((int)(iVar1 + (iVar1 >> 0x1f & 3U)) >> 2) + *(int *)(param_1 + 0x28);
}

