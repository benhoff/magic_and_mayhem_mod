
void __fastcall FUN_004d2800(int *param_1)

{
  int iVar1;
  int local_10;
  int local_c;
  int local_8;
  int local_4;
  
  iVar1 = *param_1;
  if (iVar1 != 0) {
    local_10 = param_1[3] - *(int *)(iVar1 + 0x20);
    local_c = param_1[4] - *(int *)(iVar1 + 0x24);
    local_8 = (param_1[5] - *(int *)(iVar1 + 0x20)) + 1;
    local_4 = (param_1[6] - *(int *)(iVar1 + 0x24)) + 1;
    iVar1 = FUN_0058b660();
    if (iVar1 != 0) {
      if (param_1[2] != 0) {
        FUN_0057dc20(*(undefined4 *)(*param_1 + 0x14));
        FUN_0057dc60(local_8,local_4);
        FUN_004a5ce0(param_1[2],&local_10,*(undefined4 *)((int)param_1 + 0x1d),
                     *(undefined4 *)((int)param_1 + 0x21),*(undefined4 *)((int)param_1 + 0x25),
                     (char)param_1[7],0);
      }
      (**(code **)(**(int **)(*param_1 + 4) + 0x80))(*(int **)(*param_1 + 4),iVar1);
    }
  }
  return;
}

