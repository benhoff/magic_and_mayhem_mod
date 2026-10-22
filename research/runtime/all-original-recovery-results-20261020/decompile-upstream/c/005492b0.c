
uint __thiscall FUN_005492b0(int param_1,undefined4 param_2,undefined4 param_3)

{
  uint in_EAX;
  uint uVar1;
  int iVar2;
  
  if ((((DAT_006f2150 != 0) && (in_EAX = DAT_00644520, -1 < (int)DAT_00644520)) &&
      ((int)DAT_00644520 < 8)) &&
     ((*(int *)(*(int *)(param_1 + 0x1c) + 0x24 + DAT_00644520 * 4) != 0 &&
      (iVar2 = *(int *)(DAT_006f2150 + 0xc), iVar2 != 0)))) {
    uVar1 = FUN_0054e200();
    uVar1 = (uint)((longlong)(ulonglong)(uVar1 & 0x7fffffff) % (longlong)iVar2);
    if (*(uint *)(DAT_006f2150 + 0xc) < uVar1) {
      uVar1 = 0;
    }
    iVar2 = *(int *)(DAT_006f2158 + uVar1 * 4) + DAT_006f215c;
    if (*(int *)(iVar2 + 0x1c) == -1) {
      uVar1 = (*DAT_006903dc)(iVar2,param_2,param_3);
      return uVar1 & 0xffff0000;
    }
    in_EAX = (*DAT_006c4e3c)(iVar2,param_2,param_3,0,0);
    if ((short)in_EAX == 1) {
      in_EAX = FUN_0057de00(iVar2,param_2,param_3,0,0);
    }
  }
  return in_EAX & 0xffff0000;
}

