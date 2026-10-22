
void __fastcall FUN_004de6c0(int param_1)

{
  int iVar1;
  undefined4 local_30;
  undefined4 local_2c;
  undefined4 local_28;
  undefined4 local_24;
  undefined4 local_20;
  undefined1 local_1c [28];
  
  local_2c = 0;
  local_28 = 0;
  local_24 = 0x30;
  local_20 = 0x16;
  if (*(int *)(param_1 + 0x14) != 0) {
    FUN_0058d1a0(s_bitmaps_Control_Limit_bmp_005e329c);
    iVar1 = FUN_0058b660();
    if (iVar1 != 0) {
      FUN_0057dc20(*(undefined4 *)(param_1 + 0x24));
      FUN_0057dc60(local_24,local_20);
      FUN_004a5a90(&local_30,8);
      FUN_004a5a90(&local_30,4);
      FUN_0059c1fd(local_1c,s__i__i_005e3294,*(undefined4 *)(param_1 + 8),
                   *(undefined4 *)(param_1 + 0xc));
      FUN_004a5ce0(local_1c,&local_2c,0,0,0,local_30,0);
      (**(code **)(**(int **)(param_1 + 0x14) + 0x80))(*(int **)(param_1 + 0x14),iVar1);
    }
  }
  return;
}

