
undefined4 __fastcall FUN_004d96d0(int param_1)

{
  int iVar1;
  uint uVar2;
  int *piVar3;
  
  if (*(int *)(param_1 + 0x5a4) != 0) {
    if (DAT_006de6dd == 0) {
      FUN_0058bc10(0);
    }
    else {
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
    }
    iVar1 = DAT_006de6d5;
    *(undefined4 *)(param_1 + 0x5c4) = 100;
    uVar2 = 0;
    *(uint *)(param_1 + 0x5c0) = (-(uint)(iVar1 != 0) & 0xa0) + 0x238;
    if (*(int *)(param_1 + 0x594) != 0) {
      piVar3 = (int *)(param_1 + 0xc);
      do {
        if (*(int *)((int)piVar3 + 0x82) != 3) {
          (**(code **)(*piVar3 + 0x10))();
        }
        uVar2 = uVar2 + 1;
        piVar3 = (int *)((int)piVar3 + 0x8a);
      } while (uVar2 < *(uint *)(param_1 + 0x594));
    }
  }
  return 0;
}

