
void __fastcall FUN_0058f890(uint *param_1)

{
  short sVar1;
  int iVar2;
  uint uVar3;
  uint uVar4;
  uint uVar5;
  short *psVar6;
  uint uVar7;
  short *psVar8;
  int local_8;
  
  uVar3 = param_1[6];
  uVar7 = 0xff;
  if (uVar3 == 0) {
    local_8 = 0;
  }
  else {
    if (0xfe < uVar3) {
      uVar3 = 0xff;
    }
    local_8 = (int)(0xff / (ulonglong)uVar3) + 1;
  }
  uVar3 = *param_1;
  psVar6 = (short *)param_1[7];
  if (uVar3 != 0) {
    psVar8 = psVar6;
    for (uVar4 = uVar3 >> 1; uVar4 != 0; uVar4 = uVar4 - 1) {
      psVar8[0] = 0;
      psVar8[1] = 0;
      psVar8 = psVar8 + 2;
    }
    psVar6 = psVar6 + uVar3;
    for (uVar3 = (uint)((uVar3 & 1) != 0); uVar3 != 0; uVar3 = uVar3 - 1) {
      *psVar8 = 0;
      psVar8 = psVar8 + 1;
    }
  }
  uVar3 = param_1[1] - 2;
  if (param_1[6] < uVar3) {
    do {
      uVar4 = *param_1;
      if (uVar4 != 0) {
        psVar8 = psVar6;
        for (uVar5 = uVar4 >> 1; uVar5 != 0; uVar5 = uVar5 - 1) {
          psVar8[0] = 0;
          psVar8[1] = 0;
          psVar8 = psVar8 + 2;
        }
        psVar6 = psVar6 + uVar4;
        for (uVar4 = (uint)((uVar4 & 1) != 0); uVar4 != 0; uVar4 = uVar4 - 1) {
          *psVar8 = 0;
          psVar8 = psVar8 + 1;
        }
      }
      uVar3 = uVar3 - 1;
    } while (param_1[6] < uVar3);
  }
  if (0 < (int)uVar3) {
    do {
      *psVar6 = 0;
      for (iVar2 = *param_1 - 2; psVar6 = psVar6 + 1, iVar2 != 0; iVar2 = iVar2 + -1) {
        uVar4 = uVar7;
        if (0xff < (ushort)uVar7) {
          uVar4 = 0xff;
        }
        if (DAT_006e1f88 == 0) {
          sVar1 = CONCAT11(0xff,(byte)uVar4 & 0xfc) << 3;
        }
        else {
          sVar1 = CONCAT11(0x1f,(byte)uVar4 & 0xf8) << 2;
        }
        *psVar6 = sVar1;
      }
      *psVar6 = 0;
      uVar7 = ((int)(uVar7 - local_8) < 1) - 1 & uVar7 - local_8;
      psVar6 = psVar6 + 1;
      uVar3 = uVar3 - 1;
    } while (uVar3 != 0);
  }
  uVar3 = *param_1;
  if (uVar3 != 0) {
    for (uVar7 = uVar3 >> 1; uVar7 != 0; uVar7 = uVar7 - 1) {
      psVar6[0] = 0;
      psVar6[1] = 0;
      psVar6 = psVar6 + 2;
    }
    for (uVar3 = (uint)((uVar3 & 1) != 0); uVar3 != 0; uVar3 = uVar3 - 1) {
      *psVar6 = 0;
      psVar6 = psVar6 + 1;
    }
  }
  return;
}

