
ushort __fastcall
FUN_0057e8e0(int *param_1,undefined4 param_2,undefined4 param_3,int param_4,undefined4 param_5)

{
  ushort uVar1;
  ushort uVar2;
  ushort uVar3;
  ushort uVar4;
  short sVar5;
  int iVar6;
  bool bVar7;
  
  if (param_1[7] == -1) {
    uVar1 = (*DAT_006903dc)(param_1,param_2,param_3);
  }
  else {
    if (param_4 == -0x7f) {
      sVar5 = FUN_00595677(param_1,param_2,param_3);
      bVar7 = sVar5 == 1;
      uVar1 = (ushort)bVar7;
    }
    else {
      uVar1 = (*DAT_006c4e3c)(param_1,param_2,param_3,param_4,param_5);
      bVar7 = uVar1 == 1;
    }
    if (bVar7) {
      uVar1 = FUN_0057de00(param_1,param_2,param_3,param_4,param_5);
    }
  }
  if (DAT_006de6e1 != 0) {
    uVar2 = FUN_0057f0f0(param_3,param_4,param_5);
    uVar3 = FUN_0057ec90(param_3,param_4,param_5);
    uVar4 = FUN_0057f5f0(param_3,param_4,param_5);
    return uVar1 | uVar2 | uVar3 | uVar4;
  }
  iVar6 = *param_1 + (int)param_1;
  if (*(int *)(*param_1 + 0x1c + (int)param_1) == -1) {
    uVar2 = (*DAT_006903dc)(iVar6,param_2,param_3);
  }
  else {
    if (param_4 == -0x7f) {
      sVar5 = FUN_00595677(iVar6,param_2,param_3);
      uVar2 = (ushort)(sVar5 == 1);
      if (sVar5 != 1) goto LAB_0057ea43;
    }
    else {
      uVar2 = (*DAT_006c4e3c)(iVar6,param_2,param_3,param_4,param_5);
      if (uVar2 != 1) goto LAB_0057ea43;
    }
    uVar2 = FUN_0057de00(iVar6,param_2,param_3,param_4,param_5);
  }
LAB_0057ea43:
  return uVar1 | uVar2;
}

