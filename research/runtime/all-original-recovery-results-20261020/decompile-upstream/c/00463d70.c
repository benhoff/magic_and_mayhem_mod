
void __fastcall FUN_00463d70(int param_1)

{
  undefined4 uVar1;
  int iVar2;
  int iVar3;
  
  iVar3 = 0;
  iVar2 = param_1;
  if (0 < *(int *)(param_1 + 0x1c5)) {
    do {
      iVar3 = iVar3 + 1;
      if ((*(char *)(iVar2 + 0x92) != '\0') && (*(int *)(iVar2 + 0x57) != 0)) {
        uVar1 = *(undefined4 *)(iVar2 + 0x77);
        if (DAT_006de6dd == 0) {
          FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(iVar2 + 0x73),uVar1);
        }
        else if (DAT_006e1ee4 == 0) {
          FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(iVar2 + 0x73),uVar1);
        }
        else {
          FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(iVar2 + 0x73),uVar1,1);
        }
      }
      iVar2 = iVar2 + 0x97;
    } while (iVar3 < *(int *)(param_1 + 0x1c5));
  }
  return;
}

