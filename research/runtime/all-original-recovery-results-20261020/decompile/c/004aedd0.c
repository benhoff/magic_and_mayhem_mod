
void __fastcall FUN_004aedd0(int *param_1)

{
  undefined1 *puVar1;
  
  if (param_1[2] == 0) {
    *(undefined4 *)((int)param_1 + 0x43) = 0;
    *(undefined4 *)((int)param_1 + 0x33) = 0;
    *(undefined1 *)((int)param_1 + 0x6f) = 0;
    DAT_0068991c = 1;
    DAT_00689920 = 2;
    (**(code **)(*param_1 + 0x24))();
    puVar1 = DAT_006de715;
    if (DAT_006de715 == (undefined1 *)0x0) {
      puVar1 = &DAT_005c6160;
    }
    FUN_004cd0e0(puVar1);
    if (DAT_006e1f0c != 0) {
      FUN_00557510();
      *(undefined1 *)((int)param_1 + 0x6f) = 1;
    }
  }
  return;
}

