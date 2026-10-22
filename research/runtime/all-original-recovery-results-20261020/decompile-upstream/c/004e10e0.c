
bool __thiscall FUN_004e10e0(int param_1,byte param_2,int param_3,int param_4)

{
  return (*(byte *)(param_3 + param_4 * 8 + param_1) & 0x80U >> (param_2 & 0x1f)) != 0;
}

