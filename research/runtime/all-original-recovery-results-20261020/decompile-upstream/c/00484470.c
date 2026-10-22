
/* WARNING: Removing unreachable block (ram,0x004844d8) */
/* WARNING: Restarted to delay deadcode elimination for space: stack */

void __fastcall FUN_00484470(undefined4 *param_1)

{
  int iVar1;
  uint uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  byte in_AF;
  byte in_TF;
  byte in_IF;
  byte in_NT;
  byte in_AC;
  byte in_VIF;
  byte in_VIP;
  byte in_ID;
  uint uVar5;
  
  param_1[0xb] = 5;
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0;
  param_1[4] = 0;
  uVar5 = (uint)(in_NT & 1) * 0x4000 | (uint)(in_IF & 1) * 0x200 | (uint)(in_TF & 1) * 0x100 | 0x40
          | (uint)(in_AF & 1) * 0x10 | 4 | (uint)(in_ID & 1) * 0x200000 |
          (uint)(in_VIP & 1) * 0x100000 | (uint)(in_VIF & 1) * 0x80000 | (uint)(in_AC & 1) * 0x40000
  ;
  uVar2 = uVar5 ^ 0x200000;
  if (((uint)((uVar2 & 0x4000) != 0) * 0x4000 | (uint)((uVar2 & 0x200) != 0) * 0x200 |
       (uint)((uVar2 & 0x100) != 0) * 0x100 | (uint)((uVar2 & 0x40) != 0) * 0x40 |
       (uint)((uVar2 & 0x10) != 0) * 0x10 | (uint)((uVar2 & 4) != 0) * 4 |
       (uint)((uVar2 & 0x200000) != 0) * 0x200000 | (uint)((uVar2 & 0x40000) != 0) * 0x40000) !=
      uVar5) {
    param_1[6] = 0;
    param_1[7] = 0;
    param_1[8] = 0;
    param_1[9] = 0;
    iVar1 = cpuid_basic_info(0);
    uVar4 = *(undefined4 *)(iVar1 + 8);
    uVar3 = *(undefined4 *)(iVar1 + 0xc);
    param_1[6] = *(undefined4 *)(iVar1 + 4);
    param_1[7] = uVar4;
    param_1[8] = uVar3;
    param_1[9] = 0;
    FUN_004845f0();
  }
  if (param_1[0xb] == 6) {
    DAT_006c4e3c = FUN_0059521a;
    DAT_00689b7c = FUN_0059603e;
    DAT_00689b78 = FUN_00596490;
    DAT_006903dc = FUN_00596cb8;
  }
  else {
    DAT_006c4e3c = FUN_005947b2;
    DAT_00689b7c = FUN_00595b47;
    DAT_00689b78 = FUN_005968a4;
    DAT_006903dc = FUN_00597086;
  }
  if (DAT_006e1f38 == 1) {
    DAT_006c4e3c = FUN_0057de00;
  }
  return;
}

