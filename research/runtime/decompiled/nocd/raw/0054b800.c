/* Ghidra inferred pseudocode; not original or buildable source.
 * Entry VA: 0054b800; see manifest.json for executable hash. */

void __thiscall
FUN_0054b800(int *param_1,int param_2,undefined4 param_3,int param_4,int param_5,int param_6,
            int *param_7)

{
  undefined4 *puVar1;
  undefined4 uVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  int iVar6;
  undefined4 extraout_ECX;
  int *piVar7;
  undefined4 *puVar8;
  int local_d4;
  int local_d0;
  int *local_cc;
  int local_c8;
  int local_c4;
  int local_c0;
  int local_bc;
  int local_b8;
  int local_b4;
  int local_b0;
  int local_ac;
  int local_a8;
  undefined1 local_a4 [4];
  undefined1 local_a0 [8];
  int local_98;
  undefined1 local_94 [8];
  undefined1 local_8c [4];
  undefined1 local_88 [4];
  undefined1 local_84 [4];
  undefined1 local_80 [4];
  undefined4 local_7c;
  int local_78;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  undefined4 local_68;
  int local_4e;
  int local_4a;
  int local_46;
  undefined4 local_42;
  undefined4 local_32;
  undefined4 local_2e;
  int local_20;
  undefined4 local_1c [7];
  
  local_70 = 0;
  local_6c = 0;
  local_68 = 0;
  FUN_0040e250(0,0,0,0,0,0,0);
  local_74 = param_3;
  local_78 = param_2;
  piVar3 = (int *)(param_2 + 0x96b);
  piVar7 = param_1;
  for (iVar6 = 0x83; iVar6 != 0; iVar6 = iVar6 + -1) {
    *piVar7 = *piVar3;
    piVar3 = piVar3 + 1;
    piVar7 = piVar7 + 1;
  }
  local_4a = param_5;
  local_4e = param_4;
  local_46 = param_6;
  local_42 = 0;
  local_32 = 0;
  local_b8 = DAT_006c54dc + ((&DAT_006cb942)[param_5] + (&DAT_006cb8c2)[param_6] + param_4) * 0xc;
  local_2e = 0;
  local_b0 = 0;
  local_c0 = 0;
  local_c8 = 0;
  local_b4 = 0;
  local_a8 = 0;
  local_7c = 0;
  local_c4 = DAT_006c54dc +
             ((&DAT_006cb942)[*(int *)(param_2 + 0xc)] + (&DAT_006cb8c2)[*(int *)(param_2 + 0x10)] +
             *(int *)(param_2 + 8)) * 0xc;
  if ((char)param_1[0x83] != '\0') {
    *(undefined1 *)(param_1 + 0x83) = 0;
    uVar2 = *(undefined4 *)((int)param_1 + 0x225);
    puVar1 = (undefined4 *)FUN_0043ff50(&local_d0);
    FUN_00440060(&local_ac,*puVar1,uVar2);
    local_cc = *(int **)((int)param_1 + 0x235);
    puVar1 = (undefined4 *)FUN_0043ff60(&local_d0);
    FUN_004400f0(&local_ac,*puVar1,local_cc);
    local_cc = (int *)0x0;
    uVar2 = FUN_004401d0(&local_cc,&local_c4);
    FUN_00440470(uVar2);
    FUN_00440040(&local_d0,local_a0);
    puVar1 = (undefined4 *)FUN_00440080(&local_c4);
    *puVar1 = 0;
    uVar2 = FUN_004ec780(local_b8);
    *(undefined4 *)((int)param_1 + 0x211) = uVar2;
    *(int *)((int)param_1 + 0x215) = local_c4;
  }
  iVar6 = *(int *)((int)param_1 + 0x22d);
  while (iVar6 != 0) {
    piVar3 = (int *)FUN_0043ff50(local_88);
    iVar6 = *piVar3;
    piVar3 = (int *)FUN_00440080(&local_b8);
    if (*piVar3 <= *(int *)(iVar6 + 0xc)) break;
    piVar3 = (int *)FUN_0043ff50(local_84);
    local_ac = *(int *)(*piVar3 + 0xc);
    piVar3 = (int *)FUN_0043ff50(local_80);
    local_d4 = *(int *)(*piVar3 + 0x10);
    FUN_00440140(*(undefined4 *)((int)param_1 + 0x245),*(undefined4 *)((int)param_1 + 0x249));
    piVar3 = param_7;
    iVar6 = FUN_00440080(&local_d4);
    FUN_004ebae0((int)param_1 + 0x241,&local_7c,iVar6 + 8,piVar3);
    if (*param_7 < 1) goto LAB_0054bb95;
    puVar1 = (undefined4 *)FUN_0043ff50(local_8c);
    FUN_00462ac0(local_a4,*puVar1);
    local_98 = FUN_004ec780(local_b8);
    piVar3 = *(int **)((int)param_1 + 0x245);
    local_cc = piVar3;
    if (piVar3 != *(int **)((int)param_1 + 0x249)) {
      piVar7 = piVar3 + 1;
      do {
        local_cc = piVar3;
        local_d0 = FUN_004ec780(local_b8);
        local_bc = (*piVar3 - local_98) + local_d0 + local_ac;
        piVar4 = (int *)FUN_00440080(piVar7);
        if (local_bc < *piVar4) {
          *piVar4 = local_bc;
          piVar4[1] = local_d4;
          piVar4 = piVar4 + 2;
          piVar3 = piVar7;
          for (iVar6 = 7; piVar3 = piVar3 + 1, iVar6 != 0; iVar6 = iVar6 + -1) {
            *piVar4 = *piVar3;
            piVar4 = piVar4 + 1;
          }
          uVar2 = FUN_004401d0(&local_bc,piVar7);
          FUN_00440470(uVar2);
          FUN_00440040(local_a0,local_94);
          piVar3 = local_cc;
          if (local_d0 < *(int *)((int)param_1 + 0x211)) {
            *(int *)((int)param_1 + 0x211) = local_d0;
            *(int *)((int)param_1 + 0x215) = *piVar7;
          }
        }
        piVar3 = piVar3 + 9;
        piVar7 = piVar7 + 9;
        local_cc = piVar3;
      } while (piVar3 != *(int **)((int)param_1 + 0x249));
    }
    iVar6 = *(int *)((int)param_1 + 0x22d);
  }
  *(undefined1 *)(param_1 + 0x83) = 1;
LAB_0054bb95:
  FUN_00440190(*(undefined4 *)((int)param_1 + 0x255),*(undefined4 *)((int)param_1 + 0x259));
  local_d4 = *(int *)((int)param_1 + 0x215);
  while( true ) {
    local_d0 = FUN_00440080(&local_d4);
    local_20 = local_d4;
    uVar2 = *(undefined4 *)((int)param_1 + 0x259);
    puVar1 = (undefined4 *)(local_d0 + 8);
    puVar8 = local_1c;
    for (iVar6 = 7; iVar6 != 0; iVar6 = iVar6 + -1) {
      *puVar8 = *puVar1;
      puVar1 = puVar1 + 1;
      puVar8 = puVar8 + 1;
    }
    FUN_00440260(uVar2,&local_20);
    if (local_d4 == local_c4) break;
    local_d4 = *(int *)(local_d0 + 4);
  }
  puVar1 = (undefined4 *)FUN_00440180(local_a0);
  *(undefined4 *)((int)param_1 + 0x261) = *puVar1;
  puVar1 = (undefined4 *)FUN_00440080((int)param_1 + 0x215);
  *(undefined4 *)((int)param_1 + 0x265) = *puVar1;
  *param_1 = param_4;
  param_1[1] = param_5;
  param_1[2] = param_6;
  local_d0 = *(int *)((int)param_1 + 0x261);
  piVar7 = (int *)0x0;
  piVar3 = (int *)FUN_0054be50(local_a0);
  if (local_d0 != *piVar3) {
    piVar7 = (int *)(*(int *)((int)param_1 + 0x261) + -0x20);
    FUN_0042fd90(local_a4,0);
  }
  iVar6 = (*piVar7 - DAT_006c54dc) / 0xc;
  iVar5 = iVar6 / DAT_006c54a0;
  iVar6 = iVar6 - (&DAT_006cb8c2)[iVar5];
  local_b4 = FUN_004132e0(iVar6 / DAT_006c5494);
  local_c0 = FUN_0040e650(iVar6 - (&DAT_006cb942)[local_b4]);
  piVar3 = param_1 + 5;
  local_bc = *(int *)(param_2 + 0x608);
  param_1[4] = 0;
  while (iVar6 = *(int *)((int)param_1 + 0x261), piVar7 = (int *)FUN_0054be50(local_a0),
        iVar6 != *piVar7) {
    iVar6 = *(int *)((int)param_1 + 0x261);
    puVar1 = (undefined4 *)(iVar6 + -0x20);
    FUN_0042fd90(local_a4,0);
    if ((puVar1 == (undefined4 *)0x0) || (0xf < (uint)param_1[4])) break;
    FUN_0043ff90(*puVar1,&local_b0,&local_c8,&local_a8);
    piVar3[1] = local_c8;
    *piVar3 = local_b0;
    piVar3[2] = local_a8;
    if ((local_b0 == local_c0) && (local_c8 == local_b4)) {
      iVar5 = local_a8 - iVar5;
    }
    else {
      FUN_0040eb70(local_b4);
      uVar2 = extraout_ECX;
      FUN_0040e2c0(&local_c0);
      FUN_0040e4e0(uVar2);
      local_bc = FUN_004eade0();
      iVar5 = 0;
    }
    piVar3[3] = local_bc;
    piVar3[4] = iVar5;
    iVar6 = *(int *)(iVar6 + -0x1c);
    piVar3[6] = iVar6;
    if (((iVar6 == 1) || (iVar6 == 2)) || (iVar6 == 3)) {
      iVar6 = 1;
    }
    else {
      iVar6 = 0;
    }
    piVar3[5] = iVar6;
    local_c0 = local_b0;
    local_b4 = local_c8;
    param_1[4] = param_1[4] + 1;
    piVar3 = piVar3 + 7;
    iVar5 = local_a8;
  }
  param_1[3] = 0;
  return;
}

