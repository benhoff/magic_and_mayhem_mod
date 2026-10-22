
void __fastcall FUN_00557ef0(int param_1)

{
  char cVar1;
  
  FUN_004a6a10(2);
  DAT_006de6b0 = 4;
  DAT_006ead94 = DAT_005eab0c;
  DAT_006de6ac = 0;
  FUN_005365b0();
  FUN_00537b80();
  cVar1 = FUN_005361e0();
  if (cVar1 == '\0') {
    FUN_0058a680(0x10);
    FUN_0058a680(0xd);
    FUN_0049e6a0();
    FUN_00535970();
    if (DAT_006f349c < 1) {
      MessageBoxA((HWND)0x0,s_the_screen_class_call_stack_has_u_005ec034,
                  s_SCREEN_CLASS_ERROR_005ec020,0x42030);
      return;
    }
    (**(code **)(*DAT_006f34e0 + 4))();
    DAT_006e1ef8 = DAT_006e1efc;
    DAT_006f349c = DAT_006f349c + -1;
    DAT_006f34e0 = *(int **)(&DAT_006f34a0 + DAT_006f349c * 4);
    FUN_004a4370();
    if (DAT_006f34e0 != (int *)0x0) {
      (**(code **)(*DAT_006f34e0 + 0xc))();
      return;
    }
  }
  else {
    *(undefined4 *)(param_1 + 0x26) = 2;
  }
  return;
}

