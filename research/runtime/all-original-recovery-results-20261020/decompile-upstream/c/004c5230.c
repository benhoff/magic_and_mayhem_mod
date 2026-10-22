
void __fastcall FUN_004c5230(int param_1)

{
  int iVar1;
  int *piVar2;
  int local_16c [26];
  undefined4 local_104;
  undefined1 local_100 [256];
  
  iVar1 = FUN_00597850(0x32);
  if (iVar1 == 0) {
    iVar1 = 0;
  }
  else {
    *(undefined4 *)(iVar1 + 4) = 0;
    *(undefined4 *)(iVar1 + 8) = 0;
    *(undefined4 *)(iVar1 + 0xc) = 0;
    *(undefined4 *)(iVar1 + 0x18) = 0;
    *(undefined4 *)(iVar1 + 0x14) = 0;
    *(undefined4 *)(iVar1 + 0x10) = 0;
    *(undefined4 *)(iVar1 + 0x20) = 0;
    *(undefined4 *)(iVar1 + 0x24) = 0;
    *(undefined4 *)(iVar1 + 0x28) = 0;
    *(undefined2 *)(iVar1 + 0x2c) = 0;
    DAT_006f694c = 0;
  }
  *(int *)(param_1 + 0x22) = iVar1;
  if (iVar1 != 0) {
    piVar2 = local_16c;
    for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
      *piVar2 = 0;
      piVar2 = piVar2 + 1;
    }
    local_16c[0] = 0x6c;
    local_16c[1] = 7;
    local_104 = 0x840;
    local_16c[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    local_16c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    FUN_0058ad90(local_16c,s_c__Projects_Chaos_Grimoire_CPP_005e1efc,0x431);
    FUN_0059c1fd(local_100,s__s_ix_i__s_JPG_005e1f2c,"Interface\\Grimoire\\",
                 (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,"BACKDROP");
    iVar1 = FUN_0058d320(local_100);
    if (iVar1 == 0) {
      FUN_0059c1fd(local_100,s__s_ix_i__s_BMP_005e1f1c,"Interface\\Grimoire\\",
                   (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                   (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,"BACKDROP");
      iVar1 = FUN_0058d1a0(local_100);
      if (iVar1 == 0) {
        iVar1 = *(int *)(param_1 + 0x22);
        if (iVar1 != 0) {
          FUN_0058b3e0();
          FUN_00597870(iVar1);
        }
        *(undefined4 *)(param_1 + 0x22) = 0;
      }
    }
  }
  return;
}

