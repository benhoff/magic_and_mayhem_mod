/* Ghidra inferred pseudocode; not original or buildable source.
 * Entry VA: 004ec780; see manifest.json for executable hash. */

int __thiscall FUN_004ec780(int param_1,int param_2)

{
  int iVar1;
  int iVar2;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  int iVar3;
  undefined4 uVar4;
  int local_10;
  int local_c;
  undefined4 local_8;
  uint local_4;
  
  local_4 = 0;
  local_10 = 0;
  local_8 = 0;
  local_c = 0;
  iVar3 = (param_1 - DAT_006c54dc) / 0xc;
  iVar1 = iVar3 / DAT_006c54a0;
  iVar3 = iVar3 - (&DAT_006cb8c2)[iVar1];
  local_10 = FUN_004132e0(iVar3 / (int)DAT_006c5494);
  local_4 = iVar3 - (&DAT_006cb942)[local_10];
  if ((int)local_4 < 0) {
    do {
      local_4 = local_4 + DAT_006c5494;
    } while ((int)local_4 < 0);
  }
  else {
    for (; DAT_006c5494 <= local_4; local_4 = local_4 - DAT_006c5494) {
    }
  }
  iVar3 = (param_2 - DAT_006c54dc) / 0xc;
  iVar2 = iVar3 / DAT_006c54a0;
  iVar3 = iVar3 - (&DAT_006cb8c2)[iVar2];
  local_c = FUN_004132e0(iVar3 / (int)DAT_006c5494);
  local_8 = FUN_0040e650(iVar3 - (&DAT_006cb942)[local_c]);
  uVar4 = extraout_ECX;
  FUN_0040e8d0(&local_10);
  FUN_0040eb70(uVar4);
  uVar4 = extraout_ECX_00;
  FUN_0040e2c0(&local_4);
  FUN_0040e4e0(uVar4);
  iVar3 = FUN_004eac10(iVar2 - iVar1);
  return iVar3 * 0x14;
}

