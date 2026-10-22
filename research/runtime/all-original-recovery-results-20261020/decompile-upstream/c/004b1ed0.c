
void __fastcall FUN_004b1ed0(int *param_1)

{
  FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)((int)param_1 + 0x13) + 0x20),
               *(undefined4 *)(*(int *)((int)param_1 + 0x13) + 0x24));
  FUN_0058bf40(&DAT_006a4fe0,0,0);
  FUN_004a4370();
  FUN_004a39a0();
  (**(code **)(*param_1 + 0x20))();
  *(undefined4 *)((int)param_1 + 0x33) = 0;
  return;
}

