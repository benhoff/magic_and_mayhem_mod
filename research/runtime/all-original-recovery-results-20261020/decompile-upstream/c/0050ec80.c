
void __fastcall FUN_0050ec80(int param_1)

{
  ushort uVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  
  iVar2 = (&DAT_006cb942)[*(int *)(param_1 + 0x96f)] + (&DAT_006cb8c2)[*(int *)(param_1 + 0x973)] +
          *(int *)(param_1 + 0x96b);
  uVar1 = *(ushort *)(DAT_006c54dc + 10 + iVar2 * 0xc);
  if ((((uVar1 & 0x30) != 0) && ((uVar1 & 0x10) == 0)) &&
     (uVar1 = *(ushort *)(DAT_006c54dc + iVar2 * 0xc + 2), uVar1 != 0xffff)) {
    if (uVar1 < DAT_0068949c) {
      iVar2 = DAT_00689498 + (uint)uVar1 * 0x22e;
    }
    else {
      iVar2 = 0;
    }
    if (*(int *)(iVar2 + 0x28) == 0x1a) {
      iVar4 = *(int *)(*(int *)(param_1 + 0xac) + 0x1f8);
      uVar3 = FUN_0054e200();
      if (DAT_006e1f30 != 0) {
        FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x1be6);
      }
      iVar4 = *(int *)(iVar2 + 0x1d2) -
              ((int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar4) + iVar4);
      *(int *)(iVar2 + 0x1d2) = iVar4;
      if (iVar4 < 0xb) {
        *(int *)(iVar2 + 0x1d2) = iVar4 + -10;
        *(undefined4 *)(param_1 + 0x688) = 1;
        return;
      }
      FUN_0049a560();
    }
  }
  return;
}

