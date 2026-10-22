
undefined4 __thiscall
FUN_004ce050(int param_1,undefined4 param_2,int param_3,undefined4 param_4,undefined4 param_5)

{
  uint uVar1;
  
  *(undefined4 *)(param_1 + 0x25) = param_4;
  *(undefined4 *)(param_1 + 0x2d) = param_2;
  if (DAT_006e2068 == 0) {
    FUN_0057d310(s_sprites_slider_spr_005e2c98,2);
  }
  uVar1 = *(uint *)(*DAT_006e2070 + 8 + DAT_006e2074);
  if ((uint)(*(int *)(param_3 + 0xc) - *(int *)(param_3 + 4)) < uVar1) {
    *(uint *)(param_3 + 0xc) = uVar1 + *(int *)(param_3 + 4);
  }
  FUN_004a4e20(param_3,param_5);
  *(undefined4 *)(param_1 + 0x3d) = 0;
  FUN_004cd680();
  return 1;
}

