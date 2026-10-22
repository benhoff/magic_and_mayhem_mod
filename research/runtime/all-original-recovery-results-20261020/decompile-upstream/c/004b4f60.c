
void __fastcall FUN_004b4f60(int param_1)

{
  int iVar1;
  int iVar2;
  
  if (*(int *)(param_1 + 8) != 0) {
    FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)(param_1 + 0x13) + 0x20),
                 *(undefined4 *)(*(int *)(param_1 + 0x13) + 0x24));
    iVar2 = 0;
    do {
      (**(code **)(*(int *)(*(int *)(param_1 + 0x73) + iVar2) + 0x10))();
      iVar2 = iVar2 + 0x59;
    } while (iVar2 < 0xb2);
    iVar2 = 0;
    do {
      iVar1 = *(int *)(param_1 + 0x77) + iVar2;
      if (iVar1 == *(int *)(param_1 + 0x8f)) {
        if (*(int *)(*(int *)(param_1 + 0x7f) + 0x28) < *(int *)(*(int *)(param_1 + 0x7f) + 0x20))
        goto LAB_004b504c;
      }
      else if (iVar1 == *(int *)(param_1 + 0x93)) {
        if (*(int *)(*(int *)(param_1 + 0x7f) + 0x54) < *(int *)(*(int *)(param_1 + 0x7f) + 0x4c))
        goto LAB_004b504c;
      }
      else if (iVar1 == *(int *)(param_1 + 0x97)) {
        if (*(int *)(*(int *)(param_1 + 0x7f) + 0x80) < *(int *)(*(int *)(param_1 + 0x7f) + 0x78))
        goto LAB_004b504c;
      }
      else if (iVar1 == *(int *)(param_1 + 0x9b)) {
        if (*(int *)(*(int *)(param_1 + 0x83) + 0x1c) < *(int *)(*(int *)(param_1 + 0x83) + 0x14))
        goto LAB_004b504c;
      }
      else if (iVar1 == *(int *)(param_1 + 0x9f)) {
        if (*(int *)(*(int *)(param_1 + 0x83) + 0x48) < *(int *)(*(int *)(param_1 + 0x83) + 0x40)) {
LAB_004b504c:
          FUN_004d2800();
        }
      }
      else if ((iVar1 != *(int *)(param_1 + 0xa3)) ||
              (*(int *)(*(int *)(param_1 + 0x83) + 0x74) < *(int *)(*(int *)(param_1 + 0x83) + 0x6c)
              )) goto LAB_004b504c;
      iVar2 = iVar2 + 0x29;
    } while (iVar2 < 0x2b9);
    iVar2 = 0;
    do {
      FUN_004dd9b0(*(undefined4 *)(iVar2 + *(int *)(param_1 + 0x7f)));
      iVar2 = iVar2 + 0x2c;
    } while (iVar2 < 0x84);
    iVar2 = 0;
    do {
      FUN_004dd6c0(*(undefined4 *)(iVar2 + *(int *)(param_1 + 0x83)));
      iVar2 = iVar2 + 0x2c;
    } while (iVar2 < 0x84);
    iVar2 = 0;
    do {
      (**(code **)(*(int *)(iVar2 + *(int *)(param_1 + 0x87)) + 0x10))();
      iVar2 = iVar2 + 0x7a;
    } while (iVar2 < 0x5b8);
    if (*(int *)(param_1 + 0x11b) != 0) {
      FUN_00572e90();
    }
  }
  return;
}

