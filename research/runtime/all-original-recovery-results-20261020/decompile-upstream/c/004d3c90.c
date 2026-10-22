
void __fastcall FUN_004d3c90(int param_1)

{
  short sVar1;
  int iVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  int iVar6;
  
  if (DAT_006a4646 != (int *)0x0) {
    iVar2 = FUN_0058b660();
    (**(code **)(*DAT_006a4646 + 0x80))(DAT_006a4646,iVar2);
    if (iVar2 != 0) {
      iVar2 = *(int *)(param_1 + 0x19) - DAT_006a4662;
      iVar3 = *(int *)(param_1 + 0x11) - DAT_006a4662;
      iVar4 = *(int *)(param_1 + 0x15) - DAT_006a4666;
      iVar6 = *(int *)(param_1 + 0x1d) - DAT_006a4666;
      FUN_0057dc20(DAT_006a4656);
      FUN_0057dc60(iVar2 + 1,iVar6 + 1);
      iVar2 = iVar3 + 0x24;
      iVar6 = iVar4 + 0x28;
      iVar5 = *DAT_006593e8 + DAT_006593ec;
      if (*(int *)(*DAT_006593e8 + 0x1c + DAT_006593ec) == -1) {
        (*DAT_006903dc)(iVar5,iVar2,iVar6);
      }
      else {
        sVar1 = (*DAT_006c4e3c)(iVar5,iVar2,iVar6,0,0);
        if (sVar1 == 1) {
          FUN_0057de00(iVar5,iVar2,iVar6,0,0);
        }
      }
      iVar3 = iVar3 + 0x50;
      iVar5 = DAT_006593e8[*(int *)(DAT_006593e0 + 0xc) != 0] + DAT_006593ec;
      if (*(int *)(DAT_006593e8[*(int *)(DAT_006593e0 + 0xc) != 0] + 0x1c + DAT_006593ec) == -1) {
        (*DAT_006903dc)(iVar5,iVar3,iVar6);
      }
      else {
        sVar1 = (*DAT_006c4e3c)(iVar5,iVar3,iVar6,0,0);
        if (sVar1 == 1) {
          FUN_0057de00(iVar5,iVar3,iVar6,0,0);
        }
      }
      iVar6 = 2;
      if (*(uint *)(DAT_006593e0 + 0xc) < 2) {
        iVar6 = 0;
      }
      iVar4 = iVar4 + 0x62;
      iVar5 = DAT_006593e8[iVar6] + DAT_006593ec;
      if (*(int *)(DAT_006593e8[iVar6] + 0x1c + DAT_006593ec) == -1) {
        (*DAT_006903dc)(iVar5,iVar2,iVar4);
      }
      else {
        sVar1 = (*DAT_006c4e3c)(iVar5,iVar2,iVar4,0,0);
        if (sVar1 == 1) {
          FUN_0057de00(iVar5,iVar2,iVar4,0,0);
        }
      }
      iVar2 = 3;
      if (*(uint *)(DAT_006593e0 + 0xc) < 3) {
        iVar2 = 0;
      }
      iVar6 = DAT_006593e8[iVar2] + DAT_006593ec;
      if (*(int *)(DAT_006593e8[iVar2] + 0x1c + DAT_006593ec) == -1) {
        (*DAT_006903dc)(iVar6,iVar3,iVar4);
        return;
      }
      sVar1 = (*DAT_006c4e3c)(iVar6,iVar3,iVar4,0,0);
      if (sVar1 == 1) {
        FUN_0057de00(iVar6,iVar3,iVar4,0,0);
      }
    }
  }
  return;
}

