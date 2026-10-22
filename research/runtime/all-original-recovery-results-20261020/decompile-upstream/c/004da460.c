
void __fastcall FUN_004da460(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  uint uVar3;
  undefined4 uVar4;
  
  if ((0 < *(int *)(param_1 + 0x165)) && (*(int *)(param_1 + 0x16d) != 0)) {
    if (*(int *)(param_1 + 0x1b3) < *(int *)(param_1 + 0x1bb)) {
      uVar4 = *(undefined4 *)(param_1 + 0x18d);
      uVar1 = *(undefined4 *)(param_1 + 0x189);
      iVar2 = param_1 + 0x1ab;
      if (DAT_006de6dd == 0) {
        if (DAT_006e1ee4 == 0) {
          FUN_0058c4a0(&DAT_006a49e0,iVar2,uVar1,uVar4);
        }
        else {
          FUN_0058d5b0(2,&DAT_006a49e0,iVar2,uVar1,uVar4,1);
        }
      }
      else if (DAT_006e1ee4 == 0) {
        FUN_0058c8a0(&DAT_006a49e0,iVar2,uVar1,uVar4);
      }
      else {
        FUN_0058d5b0(0,&DAT_006a49e0,iVar2,uVar1,uVar4,1);
      }
      uVar4 = *(undefined4 *)(param_1 + 0x18d);
      if (DAT_006de6dd == 0) {
        iVar2 = *(int *)(param_1 + 0x1b3) + *(int *)(param_1 + 0x189);
        if (DAT_006e1ee4 == 0) {
          FUN_0058c4a0(&DAT_006a49e0,param_1 + 0x19b,iVar2,uVar4);
        }
        else {
          FUN_0058d5b0(2,&DAT_006a49e0,param_1 + 0x19b,iVar2,uVar4,1);
        }
      }
      else {
        iVar2 = *(int *)(param_1 + 0x1b3) + *(int *)(param_1 + 0x189);
        if (DAT_006e1ee4 == 0) {
          FUN_0058c8a0(&DAT_006a49e0,param_1 + 0x19b,iVar2,uVar4);
        }
        else {
          FUN_0058d5b0(0,&DAT_006a49e0,param_1 + 0x19b,iVar2,uVar4,1);
        }
      }
      uVar3 = *(int *)(param_1 + 0x1b3) + 0x2c;
      if (*(uint *)(param_1 + 0x179) <= uVar3) {
        uVar3 = *(uint *)(param_1 + 0x179);
      }
      *(uint *)(param_1 + 0x1b3) = uVar3;
    }
    else if (DAT_006de6dd == 0) {
      FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x189),*(undefined4 *)(param_1 + 0x18d));
    }
    else if (DAT_006e1ee4 == 0) {
      FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x189),*(undefined4 *)(param_1 + 0x18d));
    }
    else {
      FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(param_1 + 0x189),
                   *(undefined4 *)(param_1 + 0x18d),1);
    }
    iVar2 = *(int *)(param_1 + 0x165) + -1;
    *(int *)(param_1 + 0x165) = iVar2;
    if (iVar2 < 1) {
      FUN_004a5000();
      if (*(int *)(param_1 + 0x161) != 0) {
        uVar4 = FUN_0058a680(*(undefined4 *)(param_1 + 0x139));
        FUN_004d9f30(uVar4);
        FUN_0059cb60(param_1 + 0x139,param_1 + 0x13d,*(int *)(param_1 + 0x161) * 4 + -4);
        *(int *)(param_1 + 0x161) = *(int *)(param_1 + 0x161) + -1;
      }
    }
  }
  return;
}

