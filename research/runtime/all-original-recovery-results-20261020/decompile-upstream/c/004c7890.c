
void __fastcall FUN_004c7890(int param_1)

{
  int iVar1;
  
  if (*(int *)(param_1 + 0x1e) == 0) {
    if (*(char *)(param_1 + 0xc) == '\0') {
      FUN_004c6030();
      FUN_004c76c0();
      goto LAB_004c78d7;
    }
  }
  else if (*(char *)(param_1 + 0xc) == '\0') {
    FUN_00557130();
    return;
  }
  FUN_0058ed80();
  iVar1 = *(int *)(param_1 + 0xd);
  *(int *)(param_1 + 0xd) = iVar1 + 1;
  if (4 < iVar1) {
    *(undefined1 *)(param_1 + 0xc) = 0;
  }
LAB_004c78d7:
  DAT_00642c60 = 0;
  if (*(char *)(param_1 + 0xc) == '\0') {
    FUN_004a45b0();
  }
  FUN_0058bf40(&DAT_006a4fe0,0,0);
  DAT_00642c60 = 1;
  FUN_004a5070();
  return;
}

