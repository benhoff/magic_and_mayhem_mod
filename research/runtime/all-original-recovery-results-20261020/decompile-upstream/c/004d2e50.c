
void __fastcall FUN_004d2e50(int param_1)

{
  int iVar1;
  short sVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  undefined4 unaff_EBP;
  int iVar6;
  bool bVar7;
  bool bVar8;
  undefined4 local_c;
  undefined4 local_8;
  
  bVar7 = DAT_006de6d5 != 0;
  iVar6 = *(int *)(param_1 + 0x11);
  iVar1 = *(int *)(param_1 + 0x15);
  bVar8 = DAT_006de6d5 != 0;
  iVar3 = *(int *)(param_1 + 0xcf);
  if ((iVar3 == *(int *)(param_1 + 0xd7)) && (iVar3 != 0)) {
    local_c = 1;
  }
  else {
    local_c = ((iVar3 - *(int *)(param_1 + 0xd7)) * 0x80) / *(int *)(param_1 + 0xd3);
    if ((int)local_c < 0x81) {
      if ((int)local_c < 1) {
        local_c = 0;
      }
      else if (0x80 < (int)local_c) goto LAB_004d2eca;
    }
    else {
LAB_004d2eca:
      local_c = 0x80;
    }
  }
  if (iVar3 == 0) {
LAB_004d2f14:
    local_8 = 0;
  }
  else {
    local_8 = (iVar3 << 7) / *(int *)(param_1 + 0xd3);
    if ((int)local_8 < 0x81) {
      if ((int)local_8 < 1) goto LAB_004d2f14;
      if ((int)local_8 < 0x81) goto LAB_004d2f1c;
    }
    local_8 = 0x80;
  }
LAB_004d2f1c:
  iVar3 = FUN_0058b660();
  if (iVar3 != 0) {
    FUN_0057dc20(DAT_006a4656);
    FUN_0057dc60((iVar6 - ((-(uint)bVar7 & 0xa0) + 0x208)) + 0xca,
                 (iVar1 - ((-(uint)bVar8 & 0x78) + 0x154)) + 0x8c);
    for (uVar5 = *(uint *)(param_1 + 0xdb); (int)uVar5 < (int)local_c; uVar5 = uVar5 + 1) {
      uVar4 = uVar5;
      if (*(uint *)(*(int *)(param_1 + 0x5b) + 0xc) < uVar5) {
        uVar4 = 0;
      }
      iVar6 = *(int *)(*(int *)(param_1 + 99) + uVar4 * 4) + *(int *)(param_1 + 0x67);
      if (*(int *)(iVar6 + 0x1c) == -1) {
        (*DAT_006903dc)(iVar6,0x52,10);
      }
      else {
        sVar2 = (*DAT_006c4e3c)(iVar6,0x52,10,0,0);
        if (sVar2 == 1) {
          FUN_0057de00(iVar6,0x52,10,0,0);
        }
      }
    }
    for (; (int)local_c < (int)local_8; local_c = local_c + 1) {
      uVar5 = local_c;
      if (*(uint *)(*(int *)(param_1 + 0x7f) + 0xc) < local_c) {
        uVar5 = 0;
      }
      iVar6 = *(int *)(*(int *)(param_1 + 0x87) + uVar5 * 4) + *(int *)(param_1 + 0x8b);
      if (*(int *)(iVar6 + 0x1c) == -1) {
        (*DAT_006903dc)(iVar6,0x52,10);
      }
      else {
        sVar2 = (*DAT_006c4e3c)(iVar6,0x52,10,0,0);
        if (sVar2 == 1) {
          FUN_0057de00(iVar6,0x52,10,0,0);
        }
      }
    }
    uVar5 = local_8;
    if ((int)local_8 < *(int *)(param_1 + 0xdf)) {
      do {
        uVar4 = uVar5;
        if (*(uint *)(*(int *)(param_1 + 0xa3) + 0xc) < uVar5) {
          uVar4 = 0;
        }
        iVar6 = *(int *)(*(int *)(param_1 + 0xab) + uVar4 * 4) + *(int *)(param_1 + 0xaf);
        if (*(int *)(iVar6 + 0x1c) == -1) {
          (*DAT_006903dc)(iVar6,0x52,10);
        }
        else {
          sVar2 = (*DAT_006c4e3c)(iVar6,0x52,10,0,0);
          if (sVar2 == 1) {
            FUN_0057de00(iVar6,0x52,10,0,0);
          }
        }
        uVar5 = uVar5 + 1;
      } while ((int)uVar5 < *(int *)(param_1 + 0xdf));
    }
    (**(code **)(*DAT_006a4646 + 0x80))(DAT_006a4646,iVar3);
    *(uint *)(param_1 + 0xdf) = local_8;
    *(undefined4 *)(param_1 + 0xdb) = unaff_EBP;
    DAT_005e16fc = 2;
  }
  return;
}

