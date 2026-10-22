
void FUN_00557130(void)

{
  if (DAT_006f349c < 1) {
    MessageBoxA((HWND)0x0,s_the_screen_class_call_stack_has_u_005ec034,s_SCREEN_CLASS_ERROR_005ec020
                ,0x42030);
  }
  else {
    (**(code **)(*DAT_006f34e0 + 4))();
    DAT_006f349c = DAT_006f349c + -1;
    DAT_006e1ef8 = DAT_006e1efc;
    DAT_006f34e0 = *(int **)(&DAT_006f34a0 + DAT_006f349c * 4);
    FUN_004a4370();
    if (DAT_006f34e0 != (int *)0x0) {
                    /* WARNING: Could not recover jumptable at 0x00557191. Too many branches */
                    /* WARNING: Treating indirect jump as call */
      (**(code **)(*DAT_006f34e0 + 0xc))();
      return;
    }
  }
  return;
}

