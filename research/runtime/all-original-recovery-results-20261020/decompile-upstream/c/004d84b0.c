
void __fastcall FUN_004d84b0(int param_1)

{
  int iVar1;
  int iVar2;
  undefined4 uVar3;
  uint uVar4;
  int iVar5;
  
  uVar4 = 0;
  if (*(int *)(param_1 + 0x15a) != 0) {
    iVar5 = 0;
    do {
      if (*(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) < 1) {
        *(undefined4 *)(*(int *)(param_1 + 0xc) + uVar4 * 4) = 0;
      }
      else if (DAT_006de6dd == 0) {
        iVar1 = *(int *)(param_1 + 0x10) + iVar5;
        iVar2 = *(int *)(iVar1 + 4) + DAT_006de6d9 * 0xc;
        uVar3 = *(undefined4 *)(*(int *)(param_1 + 0x14) + uVar4 * 4);
        if (DAT_006e1ee4 == 0) {
          FUN_0058c4a0(&DAT_006a49e0,iVar1,uVar3,iVar2);
          *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) =
               *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) + -1;
        }
        else {
          FUN_0058d5b0(2,&DAT_006a49e0,iVar1,uVar3,iVar2,1);
          *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) =
               *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) + -1;
        }
      }
      else {
        iVar1 = *(int *)(param_1 + 0x10) + iVar5;
        iVar2 = *(int *)(iVar1 + 4) + DAT_006de6d9 * 0xc;
        uVar3 = *(undefined4 *)(*(int *)(param_1 + 0x14) + uVar4 * 4);
        if (DAT_006e1ee4 == 0) {
          FUN_0058c8a0(&DAT_006a49e0,iVar1,uVar3,iVar2);
          *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) =
               *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) + -1;
        }
        else {
          FUN_0058d5b0(0,&DAT_006a49e0,iVar1,uVar3,iVar2,1);
          *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) =
               *(int *)(*(int *)(param_1 + 0xc) + uVar4 * 4) + -1;
        }
      }
      uVar4 = uVar4 + 1;
      iVar5 = iVar5 + 0x10;
    } while (uVar4 < *(uint *)(param_1 + 0x15a));
    *(undefined4 *)(param_1 + 0x166) = 1;
    return;
  }
  *(undefined4 *)(param_1 + 0x166) = 1;
  return;
}

