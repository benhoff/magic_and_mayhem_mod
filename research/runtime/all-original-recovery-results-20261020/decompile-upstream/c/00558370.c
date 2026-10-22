
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00558370(int param_1)

{
  int iVar1;
  undefined1 local_10 [16];
  
  iVar1 = FUN_00536270();
  if (iVar1 == 0) {
    FUN_0058a680(0x10);
    FUN_0058a680(0xb);
    FUN_0049e6a0();
    FUN_00535970();
  }
  else {
    FUN_00535910(local_10,0x10);
    iVar1 = FUN_00537da0(local_10);
    if (iVar1 != 0) {
      DAT_006de6b4 = 1;
      _DAT_006903e4 = &DAT_006a4b18;
      DAT_006a4ba7 = &DAT_006903e0;
      *(undefined **)(param_1 + 0x22) = &DAT_006a4b1c;
      return;
    }
    FUN_0058a680(0x10);
    FUN_0058a680(10);
    FUN_0049e6a0();
    FUN_00535970();
  }
  if (0 < DAT_006f349c) {
    (**(code **)(*DAT_006f34e0 + 4))();
    DAT_006f349c = DAT_006f349c + -1;
    DAT_006e1ef8 = DAT_006e1efc;
    DAT_006f34e0 = *(int **)(&DAT_006f34a0 + DAT_006f349c * 4);
    FUN_004a4370();
    if (DAT_006f34e0 == (int *)0x0) {
      return;
    }
    (**(code **)(*DAT_006f34e0 + 0xc))();
    return;
  }
  MessageBoxA((HWND)0x0,s_the_screen_class_call_stack_has_u_005ec034,s_SCREEN_CLASS_ERROR_005ec020,
              0x42030);
  return;
}

