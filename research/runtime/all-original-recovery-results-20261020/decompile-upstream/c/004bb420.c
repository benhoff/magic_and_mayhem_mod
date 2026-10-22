
void __fastcall FUN_004bb420(int *param_1)

{
  if (param_1[2] == 0) {
    *(undefined4 *)((int)param_1 + 0x43) = 0;
    FUN_004a39a0();
    (**(code **)(*param_1 + 0x24))();
    *(undefined4 *)((int)param_1 + 0x33) = 0;
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)((int)param_1 + 0x13) + 0x20),
                 *(undefined4 *)(*(int *)((int)param_1 + 0x13) + 0x24));
    *(undefined1 *)((int)param_1 + 0x5b) = 0;
  }
  return;
}

