
void __thiscall FUN_004d7f30(int param_1,uint param_2)

{
  int iVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  
  iVar3 = FUN_0058b660();
  if (iVar3 != 0) {
    FUN_0057dc20(*(undefined4 *)(param_1 + 0x40));
    FUN_0057dc60(0x31,0x31);
    uVar4 = param_2;
    if (*(uint *)(*(int *)(param_1 + 8) + 0xc) < param_2) {
      uVar4 = 0;
    }
    iVar1 = *(int *)(*(int *)(param_1 + 0x10) + uVar4 * 4);
    iVar5 = iVar1 + *(int *)(param_1 + 0x14);
    if (*(int *)(iVar1 + 0x1c + *(int *)(param_1 + 0x14)) == -1) {
      (*DAT_006903dc)(iVar5);
    }
    else {
      sVar2 = (*DAT_006c4e3c)(iVar5,0,0,0,0);
      if (sVar2 == 1) {
        FUN_0057de00(iVar5,0,0,0,0);
      }
    }
    (**(code **)(**(int **)(param_1 + 0x30) + 0x80))(*(int **)(param_1 + 0x30),iVar3);
    *(uint *)(param_1 + 0x62) = param_2;
  }
  return;
}

