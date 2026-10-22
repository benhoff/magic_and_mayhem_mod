
void __fastcall FUN_005571e0(int *param_1)

{
  HWND hWnd;
  int iVar1;
  undefined4 *puVar2;
  tagMSG tStack_1c;
  
  if (DAT_006f349c < 1) {
    MessageBoxA((HWND)0x0,s_the_screen_class_call_stack_has_u_005ec034,s_SCREEN_CLASS_ERROR_005ec020
                ,0x42030);
    return;
  }
  DAT_006e1ef8 = DAT_006e1efc;
  (**(code **)(*DAT_006f34e0 + 4))();
  iVar1 = DAT_006f349c + -1;
  if (0 < iVar1) {
    puVar2 = (undefined4 *)(&DAT_006f34a0 + iVar1 * 4);
    do {
      if ((int *)*puVar2 == param_1) {
        DAT_006f34e0 = *(int **)(&DAT_006f34a0 + iVar1 * 4);
        DAT_006f349c = iVar1;
        if (DAT_006f34e0 != (int *)0x0) {
          FUN_004a4370();
          (**(code **)(*DAT_006f34e0 + 0xc))();
        }
        break;
      }
      (**(code **)(*(int *)*puVar2 + 4))();
      iVar1 = iVar1 + -1;
      puVar2 = puVar2 + -1;
    } while (0 < iVar1);
  }
  if (iVar1 < 0) {
    FUN_004a4370();
    if (0xe < DAT_006f349c) {
      MessageBoxA((HWND)0x0,s_the_screen_class_call_stack_has_o_005ebff4,
                  s_SCREEN_CLASS_ERROR_005ec020,0x42030);
      return;
    }
    if (DAT_006f34e0 != (int *)0x0) {
      (**(code **)(*DAT_006f34e0 + 8))();
    }
    hWnd = DAT_00656614;
    DAT_006e1ef8 = DAT_006e1efc;
    DAT_006f349c = DAT_006f349c + 1;
    DAT_006f34e0 = param_1;
    *(int **)(&DAT_006f34a0 + DAT_006f349c * 4) = param_1;
    iVar1 = PeekMessageA(&tStack_1c,hWnd,0x100,0x108,1);
    while (iVar1 != 0) {
      iVar1 = PeekMessageA(&tStack_1c,DAT_00656614,0x100,0x108,1);
    }
    iVar1 = PeekMessageA(&tStack_1c,DAT_00656614,0x201,0x201,1);
    while (iVar1 != 0) {
      iVar1 = PeekMessageA(&tStack_1c,DAT_00656614,0x201,0x201,1);
    }
    FUN_004a4370();
    (**(code **)*DAT_006f34e0)();
  }
  return;
}

