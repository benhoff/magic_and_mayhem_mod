
void __fastcall FUN_004a9cb0(int *param_1)

{
  if (param_1[2] == 0) {
    *(undefined4 *)((int)param_1 + 0x43) = 0;
    *(undefined1 *)(param_1 + 0x19) = 0;
    (**(code **)(*param_1 + 0x24))();
    *(undefined4 *)((int)param_1 + 0x33) = 0;
    *(undefined1 *)((int)param_1 + 0x5f) = 0;
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)((int)param_1 + 0x13) + 0x20),
                 *(undefined4 *)(*(int *)((int)param_1 + 0x13) + 0x24));
  }
  return;
}

