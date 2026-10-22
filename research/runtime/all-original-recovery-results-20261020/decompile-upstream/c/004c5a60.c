
void __fastcall FUN_004c5a60(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  byte bVar4;
  undefined4 uVar5;
  int iVar6;
  int *piVar7;
  int local_78;
  int local_74;
  int *local_70;
  int local_6c [26];
  undefined4 local_4;
  
  local_70 = (int *)(param_1 + 0x2a);
  local_74 = 0;
  do {
    local_78 = 0;
    do {
      iVar2 = FUN_00597850(0x32);
      if (iVar2 == 0) {
        iVar2 = 0;
      }
      else {
        *(undefined4 *)(iVar2 + 4) = 0;
        *(undefined4 *)(iVar2 + 8) = 0;
        *(undefined4 *)(iVar2 + 0xc) = 0;
        *(undefined4 *)(iVar2 + 0x18) = 0;
        *(undefined4 *)(iVar2 + 0x14) = 0;
        *(undefined4 *)(iVar2 + 0x10) = 0;
        *(undefined4 *)(iVar2 + 0x20) = 0;
        *(undefined4 *)(iVar2 + 0x24) = 0;
        *(undefined4 *)(iVar2 + 0x28) = 0;
        *(undefined2 *)(iVar2 + 0x2c) = 0;
        DAT_006f694c = 0;
      }
      *(int *)(param_1 + 0x26 + (local_74 + local_78) * 4) = iVar2;
      if (iVar2 != 0) {
        uVar5 = 0;
        piVar7 = local_6c;
        for (iVar2 = 0x1b; iVar2 != 0; iVar2 = iVar2 + -1) {
          *piVar7 = 0;
          piVar7 = piVar7 + 1;
        }
        local_6c[0] = 0x6c;
        local_6c[1] = 7;
        bVar4 = -(DAT_006de6d5 != 0) & 0xa0;
        local_4 = 0x840;
        if (bVar4 == 0) {
          if ((local_78 == 0) || (local_78 == 1)) {
            uVar5 = 0x140;
          }
        }
        else if ((bVar4 == 0xa0) && ((local_78 == 0 || (local_78 == 1)))) {
          uVar5 = 400;
        }
        local_6c[2] = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
        local_6c[3] = uVar5;
        FUN_0058ad90(local_6c,s_c__Projects_Chaos_Grimoire_CPP_005e1efc,0x506);
        FUN_0058d0e0(0,0,0xff);
        FUN_0058bc10(0x1f);
        if (local_78 == 1) {
          iVar6 = 0;
          uVar3 = -(uint)(DAT_006de6d5 != 0) & 0xa0;
          iVar2 = uVar3 + 0x280;
          if (iVar2 == 0x280) {
            iVar6 = 0x140;
          }
          else if (iVar2 == 800) {
            iVar2 = *local_70;
            *(uint *)(iVar2 + 0x20) = uVar3 + 0xf0;
            *(undefined4 *)(iVar2 + 0x24) = 0;
            goto LAB_004c5be5;
          }
          iVar1 = *local_70;
          *(int *)(iVar1 + 0x20) = iVar2 - iVar6;
          *(undefined4 *)(iVar1 + 0x24) = 0;
        }
        else {
          iVar2 = *(int *)(param_1 + 0x26 + (local_74 + local_78) * 4);
          *(undefined4 *)(iVar2 + 0x20) = 0;
          *(undefined4 *)(iVar2 + 0x24) = 0;
        }
      }
LAB_004c5be5:
      local_78 = local_78 + 1;
    } while (local_78 < 2);
    local_74 = local_74 + 2;
    local_70 = local_70 + 2;
    if (3 < local_74) {
      return;
    }
  } while( true );
}

