
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_004dde90(int param_1,undefined4 param_2)

{
  int *piVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  int *piVar5;
  
  *(undefined4 *)(param_1 + 0x3a) = param_2;
  *(undefined4 *)(param_1 + 0x3e) = param_2;
  *(undefined4 *)(param_1 + 0x42) = 0xffffffff;
  iVar4 = FUN_0058d320(s_bitmaps_timer_background_jpg_005e3244);
  if (iVar4 == 0) {
    MessageBoxA((HWND)0x0,s_Could_not_load_timer_background_005e3224,s_INITIALISATION_ERROR_005da35c
                ,0x42030);
    DestroyWindow(DAT_00656614);
  }
  FUN_0058db80(0,0,0xff,0x14);
  piVar1 = *(int **)(param_1 + 0x4f);
  if ((((piVar1 != (int *)0x0) && (*(int *)(DAT_006f437c + 0x1c) != 0)) &&
      (*(int *)(DAT_006f437c + 0x18) != 0)) && (*piVar1 != 0)) {
    _DAT_006fa794 = 0;
    iVar4 = (**(code **)(*(int *)*piVar1 + 0x24))((int *)*piVar1,&DAT_006fa794);
    if ((iVar4 != 0) || ((_DAT_006fa794 & 1) != 0)) {
      DAT_006fa798 = (**(code **)(*(int *)*piVar1 + 0x48))((int *)*piVar1);
      piVar2 = *(int **)(DAT_006f437c + 0x260);
      piVar3 = (int *)piVar2[4];
      piVar5 = piVar2;
      while (piVar3 != piVar1) {
        piVar5 = (int *)*piVar5;
        if (piVar5 == piVar2) {
          piVar5 = (int *)0x0;
          break;
        }
        piVar3 = (int *)piVar5[4];
      }
      FUN_00571d20(piVar5);
      if (DAT_006fa798 == 0) {
        DAT_006fa798 = (**(code **)(*(int *)*piVar1 + 0x34))((int *)*piVar1,0);
      }
    }
  }
  *(undefined4 *)(param_1 + 0x4f) = 0;
  *(undefined1 *)(param_1 + 0x4e) = 0;
  return;
}

