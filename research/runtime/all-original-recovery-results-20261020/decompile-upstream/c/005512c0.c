
void __thiscall FUN_005512c0(int param_1,int param_2)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  int iVar4;
  
  FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x8d),*(undefined4 *)(param_1 + 0x91));
  if (param_2 != 0) {
    if ((DAT_006e0080 == 0) && (*(int *)(&DAT_0065b1d9 + param_2 * 4) != DAT_00659f51)) {
      iVar1 = 1;
      iVar4 = *(int *)(param_1 + 0x823) + 1;
      if (1 < iVar4) {
        piVar2 = &DAT_0065b1dd;
        piVar3 = (int *)(param_1 + 0x1df + param_2 * 4);
        do {
          if ((*piVar2 == DAT_00659f51) && (*piVar3 != 0)) goto LAB_0055133b;
          iVar1 = iVar1 + 1;
          piVar2 = piVar2 + 1;
          piVar3 = piVar3 + 0x14;
        } while (iVar1 < iVar4);
      }
    }
    else {
LAB_0055133b:
      if (*(int *)(param_1 + 0xa44) == 0) {
        if (*(int *)(param_1 + 0x54) != param_2) {
          FUN_0054b2d0(param_1 + 0x22);
          *(int *)(param_1 + 0x54) = param_2;
        }
        iVar1 = *(int *)(param_1 + 0x1e) + param_2 * 0x10;
        if (DAT_006e1ee4 == 0) {
          FUN_0058c8a0(&DAT_006a49e0,iVar1 + -0x10,*(undefined4 *)(iVar1 + -0x10),
                       *(undefined4 *)(iVar1 + -0xc));
        }
        else {
          FUN_0058d5b0(0,&DAT_006a49e0,iVar1 + -0x10,*(undefined4 *)(iVar1 + -0x10),
                       *(undefined4 *)(iVar1 + -0xc),1);
        }
      }
    }
  }
  if (((&DAT_0065ab9d)[DAT_00659f51] != 0) &&
     ((*(int *)(&DAT_0065b1d9 + (&DAT_0065ae1d)[DAT_00659f51] * 4) == DAT_00659f51 ||
      ((&DAT_0065a065)[DAT_00659f51 * 5] != 0)))) {
    FUN_004a2d70(&DAT_006a49e0);
  }
  FUN_005506a0(&DAT_006a49e0);
  FUN_00550970(&DAT_006a49e0);
  (**(code **)(*(int *)(param_1 + 0x8a1) + 0x10))();
  (**(code **)(*(int *)(param_1 + 0x827) + 0x10))();
  (**(code **)(*(int *)(param_1 + 0x91b) + 0x10))();
  (**(code **)(*(int *)(param_1 + 0x995) + 0x10))();
  if (*(int *)(param_1 + 0xa44) != 0) {
    FUN_00572e90();
  }
  FUN_004db7e0();
  FUN_0056dab0();
  FUN_0056da70();
  return;
}

