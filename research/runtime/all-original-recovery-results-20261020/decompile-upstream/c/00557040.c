
void __fastcall FUN_00557040(undefined4 *param_1)

{
  HWND hWnd;
  int iVar1;
  tagMSG local_1c;
  
  if (DAT_006f349c < 0xf) {
    if (DAT_006f34e0 != (int *)0x0) {
      (**(code **)(*DAT_006f34e0 + 8))();
    }
    DAT_006e1ef8 = DAT_006e1efc;
    hWnd = DAT_00656614;
    DAT_006f349c = DAT_006f349c + 1;
    *(undefined4 **)(&DAT_006f34a0 + DAT_006f349c * 4) = param_1;
    DAT_006f34e0 = param_1;
    iVar1 = PeekMessageA(&local_1c,hWnd,0x100,0x108,1);
    while (iVar1 != 0) {
      iVar1 = PeekMessageA(&local_1c,DAT_00656614,0x100,0x108,1);
    }
    iVar1 = PeekMessageA(&local_1c,DAT_00656614,0x201,0x201,1);
    while (iVar1 != 0) {
      iVar1 = PeekMessageA(&local_1c,DAT_00656614,0x201,0x201,1);
    }
    FUN_004a4370();
    (**(code **)*DAT_006f34e0)();
    return;
  }
  MessageBoxA((HWND)0x0,s_the_screen_class_call_stack_has_o_005ebff4,s_SCREEN_CLASS_ERROR_005ec020,
              0x42030);
  return;
}

