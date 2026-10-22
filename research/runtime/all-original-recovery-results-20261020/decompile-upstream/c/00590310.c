
bool __thiscall FUN_00590310(int param_1,int param_2)

{
  int iVar1;
  bool bVar2;
  
  if (((((((DAT_006e2030 != '\0') && (param_2 != 1)) && (param_2 != 0xf)) &&
        ((param_2 != 10 && (param_2 != 3)))) &&
       (((param_2 != 0x13 && ((param_2 != 0xe && (param_2 != 0xb)))) && (param_2 != 8)))) &&
      ((((param_2 != 0x29 && (param_2 != 0x47)) && (param_2 != 0x34)) &&
       (((param_2 != 0x4a && (param_2 != 0x49)) &&
        ((param_2 != 0x2e && ((param_2 != 0x22 && (param_2 != 0x44)))))))))) &&
     ((param_2 != 0x21 && ((param_2 != 0x57 && (param_2 != 0x4e)))))) {
    return false;
  }
  iVar1 = *(int *)(param_1 + 4);
  bVar2 = iVar1 < *(int *)(param_1 + 8);
  if (bVar2) {
    *(int *)(*(int *)(param_1 + 0xc) + iVar1 * 4) = param_2;
    *(int *)(param_1 + 4) = *(int *)(param_1 + 4) + 1;
  }
  return bVar2;
}

