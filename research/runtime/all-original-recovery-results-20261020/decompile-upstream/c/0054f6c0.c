
void __fastcall FUN_0054f6c0(int param_1)

{
  int iVar1;
  int *piVar2;
  int local_16c [26];
  undefined4 local_104;
  undefined1 local_100 [256];
  
  piVar2 = local_16c;
  for (iVar1 = 0x1b; iVar1 != 0; iVar1 = iVar1 + -1) {
    *piVar2 = 0;
    piVar2 = piVar2 + 1;
  }
  local_16c[3] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
  local_16c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
  local_16c[0] = 0x6c;
  local_16c[1] = 7;
  local_104 = 0x840;
  FUN_0058ad90(local_16c,s_c__Projects_Chaos_RealmView_cpp_005ebe4c,0x2f9);
  FUN_0059c1fd(local_100,s__s_ix_i__s_Map_BMP_005ebe88,"Interface\\RealmViewer\\",
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,*(undefined4 *)(param_1 + 0x12));
  FUN_0058d1a0(local_100);
  return;
}

