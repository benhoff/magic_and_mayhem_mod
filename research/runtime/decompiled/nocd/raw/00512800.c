/* Ghidra inferred pseudocode; not original or buildable source.
 * Entry VA: 00512800; see manifest.json for executable hash. */

bool __thiscall
FUN_00512800(int param_1,undefined4 param_2,undefined4 param_3,undefined4 param_4,undefined4 param_5
            )

{
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  int iVar1;
  undefined4 *puVar2;
  undefined4 *puVar3;
  undefined4 uVar4;
  undefined4 uVar5;
  undefined4 local_4;
  
  puVar2 = &local_4;
  local_4 = DAT_005e174c;
  DAT_00690354 = 1;
  puVar3 = puVar2;
  FUN_0040eeb0(param_4);
  uVar5 = extraout_ECX;
  FUN_0040e8a0(param_3);
  uVar4 = extraout_ECX_00;
  FUN_0040e290(param_2);
  FUN_0054b800(param_1,param_5,uVar4,uVar5,puVar2,puVar3);
  puVar2 = &DAT_00690148;
  puVar3 = (undefined4 *)(param_1 + 0x96b);
  for (iVar1 = 0x83; iVar1 != 0; iVar1 = iVar1 + -1) {
    *puVar3 = *puVar2;
    puVar2 = puVar2 + 1;
    puVar3 = puVar3 + 1;
  }
  *(undefined4 *)(param_1 + 0xb8b) = 1;
  *(undefined4 *)(param_1 + 0xd03) = 0;
  if (*(int *)(param_1 + 0x97b) == 0) {
    *(undefined4 *)(param_1 + 0xb8b) = 0;
  }
  return *(int *)(param_1 + 0x97b) != 0;
}

