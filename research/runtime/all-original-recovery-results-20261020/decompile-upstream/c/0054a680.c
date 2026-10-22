
void FUN_0054a680(void)

{
  int iVar1;
  
  if (DAT_006f2218 == 0) {
    iVar1 = FUN_0057d310(s_creatures_bodyparts_offal_spr_005eb4ec,8);
    if (iVar1 != 0) {
      DAT_006f220c = *(uint *)(DAT_006f2218 + 0xc) / 9;
      FUN_0057d690(1,8);
      FUN_0057d690(2,8);
      FUN_0057d690(6,8);
      FUN_0057d690(4,8);
      FUN_0057d690(5,8);
      FUN_0057d690(7,8);
    }
  }
  if (DAT_006f2240 == 0) {
    iVar1 = FUN_0057d310(s_creatures_bodyparts_offalsml_spr_005eb4c8,8);
    if (iVar1 != 0) {
      DAT_006f2148 = *(uint *)(DAT_006f2240 + 0xc) / 9;
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

