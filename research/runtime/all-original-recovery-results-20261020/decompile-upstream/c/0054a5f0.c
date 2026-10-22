
void __fastcall FUN_0054a5f0(int param_1)

{
  int iVar1;
  
  if ((&DAT_006f1d58)[param_1 * 9] == 0) {
    iVar1 = FUN_0057d310((&PTR_s_creatures_bodyparts_elf_spr_005c7338)[param_1],8);
    if (iVar1 != 0) {
      *(uint *)(&DAT_006f2174 + param_1 * 4) = *(uint *)((&DAT_006f1d58)[param_1 * 9] + 0xc) / 9;
      FUN_0057d690(1,8);
      FUN_0057d690(2,8);
      FUN_0057d690(6,8);
      FUN_0057d690(4,8);
      FUN_0057d690(5,8);
      FUN_0057d690(7,8);
    }
  }
  return;
}

