
/* WARNING: Removing unreachable block (ram,0x004f7569) */

undefined4 __fastcall FUN_004f7540(int param_1)

{
  int *piVar1;
  ushort uVar2;
  uint uVar3;
  int iVar4;
  
  if ((*(int *)(param_1 + 0x7f8) < DAT_006a2890) && (iVar4 = *(int *)(param_1 + 0x80), iVar4 != 0))
  {
    uVar3 = FUN_0054e200();
    iVar4 = (int)((longlong)(ulonglong)(uVar3 & 0x7fffffff) % (longlong)iVar4);
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Map_cpp_005e49ac,0x1c91);
    }
    if (((-1 < iVar4) && (iVar4 < *(int *)(param_1 + 0x80))) &&
       (piVar1 = (int *)(*(int *)(param_1 + 0x7c) + iVar4 * 0x28), piVar1 != (int *)0x0)) {
      iVar4 = (&DAT_006cb942)[piVar1[1]] + (&DAT_006cb8c2)[piVar1[2]] + *piVar1;
      if (*(short *)(DAT_006c54dc + 6 + iVar4 * 0xc) != -1) {
        return 0;
      }
      uVar2 = *(ushort *)(DAT_006c54dc + iVar4 * 0xc);
      if ((uVar2 != 0) && (*(int *)(DAT_0065660c + 0x94 + (uint)uVar2 * 0x164) == 0x10)) {
        return 0;
      }
      iVar4 = FUN_00544cf0();
      if (iVar4 == 0) {
        return 0;
      }
      FUN_00502aa0(*piVar1,piVar1[1],piVar1[2]);
      FUN_00543970(0x13);
      *(int *)(param_1 + 0x7f8) = *(int *)(param_1 + 0x7f8) + 1;
    }
  }
  return 1;
}

