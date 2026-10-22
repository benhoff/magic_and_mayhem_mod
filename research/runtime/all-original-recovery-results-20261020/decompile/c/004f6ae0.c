
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004f6ae0(int param_1,int param_2,int param_3,float param_4,float *param_5,float *param_6,
                 float *param_7)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  undefined4 extraout_ECX;
  int extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 extraout_ECX_02;
  undefined4 extraout_ECX_03;
  undefined4 extraout_ECX_04;
  undefined4 extraout_ECX_05;
  undefined4 extraout_ECX_06;
  undefined4 extraout_ECX_07;
  undefined4 extraout_ECX_08;
  undefined4 extraout_ECX_09;
  undefined4 extraout_ECX_10;
  undefined4 extraout_ECX_11;
  undefined4 extraout_ECX_12;
  undefined4 extraout_ECX_13;
  undefined4 extraout_ECX_14;
  undefined4 extraout_ECX_15;
  undefined4 uVar4;
  undefined4 extraout_ECX_16;
  undefined4 extraout_ECX_17;
  undefined4 extraout_ECX_18;
  undefined4 extraout_ECX_19;
  undefined4 extraout_ECX_20;
  uint uVar5;
  int iVar6;
  uint uVar7;
  float fVar8;
  uint uVar9;
  uint uVar10;
  int iVar11;
  uint uVar12;
  int iVar13;
  float10 extraout_ST0;
  float10 extraout_ST0_00;
  undefined8 uVar14;
  ulonglong uVar15;
  uint local_168;
  uint local_164;
  uint local_160;
  uint local_15c;
  float local_158;
  int local_154;
  float local_150;
  float local_14c;
  float local_148;
  float local_144;
  uint local_140;
  int local_138 [26];
  int local_d0 [26];
  int local_68 [26];
  
  local_160 = 0;
  local_168 = 0;
  local_164 = 0;
  local_15c = 0;
  local_140 = 0;
  uVar2 = __ftol();
  iVar11 = (uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f);
  uVar2 = __ftol();
  iVar6 = (uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f);
  local_148 = (float)iVar11;
  if (iVar11 <= iVar6) {
    local_148 = (float)iVar6;
  }
  uVar2 = __ftol();
  local_158 = (float)((uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f));
  if (((int)local_158 < (int)local_148) && (local_158 = (float)iVar6, iVar6 < iVar11)) {
    local_158 = (float)iVar11;
  }
  local_150 = (float)(int)local_158;
  if (local_150 == (float)_DAT_005c5380) {
    return;
  }
  local_154 = 0;
  local_148 = (*param_5 * _DAT_005c5bd4) / local_150;
  local_144 = (*param_6 * _DAT_005c5bd4) / local_150;
  local_150 = (*param_7 * _DAT_005c5bd4) / local_150;
  local_14c = 0.0;
  while( true ) {
    local_158 = local_14c * local_144;
    uVar2 = __ftol();
    if ((uint)(DAT_006c5498 * 0x20) < (uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f)) {
      return;
    }
    FUN_0059d53d((double)(local_14c * local_150 + (float)_DAT_005c5a88));
    iVar6 = __ftol();
    uVar2 = iVar6 + param_3;
    if ((int)uVar2 < 0) {
      return;
    }
    if (DAT_005e1784 <= (int)uVar2) {
      return;
    }
    FUN_0059d53d((double)(local_14c * local_148 + (float)_DAT_005c5a88));
    iVar6 = __ftol();
    local_160 = (iVar6 + param_1) % (DAT_006c5494 * 0x20);
    if ((int)local_160 < 0) {
      local_160 = local_160 + DAT_006c5494 * 0x20;
    }
    FUN_0059d53d((double)(local_158 + (float)_DAT_005c5a88));
    iVar6 = __ftol();
    iVar11 = DAT_006c5498 * 0x20;
    local_164 = (iVar6 + param_2) % iVar11;
    if ((int)local_164 < 0) {
      local_164 = local_164 + iVar11;
    }
    uVar14 = CONCAT44(uVar2,iVar11);
    FUN_004e3c00(&local_164);
    uVar4 = extraout_ECX;
    FUN_004e3b90(&local_160);
    iVar11 = FUN_004f6940(uVar4,uVar14);
    iVar6 = extraout_ECX_00;
    uVar9 = local_140;
    if (iVar11 == 0) break;
    local_154 = local_154 + -1;
    local_14c = (float)local_154;
    uVar2 = __ftol();
    if ((uint)(DAT_006c5494 * 0x20) < (uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f)) {
      return;
    }
  }
  while( true ) {
    uVar7 = uVar2;
    uVar14 = CONCAT44(uVar7,iVar6);
    FUN_004e3c00(&local_164);
    uVar4 = extraout_ECX_01;
    FUN_004e3b90(&local_160);
    iVar6 = FUN_004f6940(uVar4,uVar14);
    if (iVar6 != 0) break;
    local_154 = local_154 + 1;
    local_14c = (float)local_154 * local_148;
    uVar2 = __ftol();
    if ((uint)(DAT_006c5494 * 0x20) < (uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f)) {
      return;
    }
    local_158 = (float)(extraout_ST0 * (float10)local_144);
    uVar2 = __ftol();
    if ((uint)(DAT_006c5498 * 0x20) < (uVar2 ^ (int)uVar2 >> 0x1f) - ((int)uVar2 >> 0x1f)) {
      return;
    }
    FUN_0059d53d((double)(extraout_ST0_00 * (float10)local_150 + (float10)_DAT_005c5a88));
    iVar6 = __ftol();
    uVar2 = iVar6 + param_3;
    if ((int)uVar2 < 0) {
      return;
    }
    if (DAT_005e1784 <= (int)uVar2) {
      return;
    }
    local_168 = local_160;
    local_15c = local_164;
    FUN_0059d53d((double)(local_14c + (float)_DAT_005c5a88));
    iVar6 = __ftol();
    local_160 = (iVar6 + param_1) % (DAT_006c5494 * 0x20);
    if ((int)local_160 < 0) {
      local_160 = local_160 + DAT_006c5494 * 0x20;
    }
    FUN_0059d53d((double)(local_158 + (float)_DAT_005c5a88));
    iVar11 = __ftol();
    iVar6 = DAT_006c5498 * 0x20;
    local_164 = (iVar11 + param_2) % iVar6;
    uVar9 = uVar7;
    if ((int)local_164 < 0) {
      local_164 = local_164 + iVar6;
    }
  }
  iVar6 = DAT_006c5494 * 0x20;
  uVar2 = (int)(local_160 & 0xfffffffc) % iVar6;
  if ((int)uVar2 < 0) {
    uVar2 = uVar2 + iVar6;
  }
  iVar11 = DAT_006c5498 * 0x20;
  uVar5 = (int)(local_164 & 0xfffffffc) % iVar11;
  if ((int)uVar5 < 0) {
    uVar5 = uVar5 + iVar11;
  }
  local_168 = (int)(local_168 & 0xfffffffc) % iVar6;
  if ((int)local_168 < 0) {
    local_168 = local_168 + iVar6;
  }
  local_15c = (int)(local_15c & 0xfffffffc) % iVar11;
  if ((int)local_15c < 0) {
    local_15c = local_15c + iVar11;
  }
  uVar10 = uVar9 & 0xfffffffc;
  iVar6 = 0;
  local_158 = 0.0;
  uVar12 = DAT_006e1fa4;
  if ((int)DAT_006e1fa4 < (int)(uVar10 - 4)) {
    uVar12 = uVar10 - 4;
  }
  do {
    iVar11 = DAT_005e1784 + -4;
    if ((int)(uVar10 + 4) < DAT_005e1784 + -4) {
      iVar11 = uVar10 + 4;
    }
    if (iVar11 < (int)uVar12) {
      if (iVar6 == 0) {
        *param_5 = -(param_4 * *param_5);
        *param_6 = -(param_4 * *param_6);
        *param_7 = -(param_4 * *param_7);
        return;
      }
      if (iVar6 == 1) {
        fVar8 = ((param_4 + (float)_DAT_005c5390) *
                ((float)local_d0[0] * *param_6 +
                (float)local_68[0] * *param_5 + (float)local_138[0] * *param_7)) /
                (float)(local_138[0] * local_138[0] + local_d0[0] * local_d0[0] +
                       local_68[0] * local_68[0]);
        *param_5 = fVar8 * (float)local_68[0] - *param_5;
        *param_6 = fVar8 * (float)local_d0[0] - *param_6;
        *param_7 = fVar8 * (float)local_138[0] - *param_7;
        return;
      }
      local_154 = 0;
      local_168 = 0;
      local_15c = 0;
      if (0 < iVar6) {
        local_144 = 1.4013e-45;
        local_148 = 0.0;
        do {
          if ((int)local_144 < iVar6) {
            fVar8 = local_144;
            do {
              iVar11 = *(int *)((int)local_d0 + (int)local_148) * local_138[(int)fVar8] -
                       *(int *)((int)local_138 + (int)local_148) * local_d0[(int)fVar8];
              iVar3 = *(int *)((int)local_138 + (int)local_148) * local_68[(int)fVar8] -
                      *(int *)((int)local_68 + (int)local_148) * local_138[(int)fVar8];
              iVar13 = *(int *)((int)local_68 + (int)local_148) * local_d0[(int)fVar8] -
                       *(int *)((int)local_d0 + (int)local_148) * local_68[(int)fVar8];
              if ((float)_DAT_005c5380 <
                  (float)iVar3 * *param_6 + (float)iVar11 * *param_5 + (float)iVar13 * *param_7) {
                iVar11 = -iVar11;
                iVar3 = -iVar3;
                iVar13 = -iVar13;
              }
              local_15c = local_15c + iVar11;
              local_168 = local_168 + iVar3;
              local_154 = local_154 + iVar13;
              fVar8 = (float)((int)fVar8 + 1);
            } while ((int)fVar8 < iVar6);
          }
          local_148 = (float)((int)local_148 + 4);
          bVar1 = (int)local_144 < iVar6;
          local_144 = (float)((int)local_144 + 1);
        } while (bVar1);
      }
      fVar8 = -(((param_4 + (float)_DAT_005c5390) *
                ((float)(int)local_168 * *param_6 +
                (float)(int)local_15c * *param_5 + (float)local_154 * *param_7)) /
               (float)(int)(local_154 * local_154 + local_168 * local_168 + local_15c * local_15c));
      *param_5 = (float)(int)local_15c * fVar8 + *param_5;
      *param_6 = (float)(int)local_168 * fVar8 + *param_6;
      *param_7 = (float)local_154 * fVar8 + *param_7;
      return;
    }
    FUN_004e3c00(&local_15c);
    iVar3 = FUN_004e3c50();
    iVar11 = DAT_006c5498 << 5;
    local_164 = iVar3 % iVar11;
    while( true ) {
      if ((int)local_164 < 0) {
        local_164 = local_164 + iVar11;
      }
      iVar11 = FUN_0040fb00();
      if (4 < iVar11) break;
      FUN_004e3b90(&local_168);
      iVar3 = FUN_004e3be0();
      iVar11 = DAT_006c5494 << 5;
      local_160 = iVar3 % iVar11;
      while( true ) {
        if ((int)local_160 < 0) {
          local_160 = local_160 + iVar11;
        }
        iVar11 = FUN_0040f7a0();
        if (4 < iVar11) break;
        if ((((local_160 != local_168) || (local_164 != local_15c)) || (uVar12 != uVar10)) &&
           (((local_160 != uVar2 || (local_164 != uVar5)) || (uVar12 != (uVar7 & 0xfffffffc))))) {
          uVar14 = CONCAT44(uVar12,uVar2);
          FUN_004e3c00(&local_164);
          uVar4 = extraout_ECX_02;
          FUN_004e3b90(&local_160);
          iVar11 = FUN_004f6940(uVar4,uVar14);
          if (iVar11 != 0) {
            uVar15 = CONCAT44(uVar9,extraout_ECX_03) & 0xfffffffcffffffff;
            FUN_004e3c00(&local_15c);
            uVar4 = extraout_ECX_04;
            FUN_004e3b90(&local_160);
            iVar11 = FUN_004f6940(uVar4,uVar15);
            uVar4 = extraout_ECX_05;
            if (iVar11 == 0) {
LAB_004f70b4:
              uVar15 = CONCAT44(uVar9,uVar4) & 0xfffffffcffffffff;
              FUN_004e3c00(&local_164);
              uVar4 = extraout_ECX_10;
              FUN_004e3b90(&local_168);
              iVar11 = FUN_004f6940(uVar4,uVar15);
              uVar4 = extraout_ECX_11;
              if (iVar11 == 0) {
LAB_004f713a:
                uVar14 = CONCAT44(uVar12,uVar4);
                FUN_004e3c00(&local_15c);
                uVar4 = extraout_ECX_16;
                FUN_004e3b90(&local_168);
                iVar11 = FUN_004f6940(uVar4,uVar14);
                if (iVar11 == 0) goto LAB_004f7204;
                uVar14 = CONCAT44(uVar12,extraout_ECX_17);
                FUN_004e3c00(&local_15c);
                uVar4 = extraout_ECX_18;
                FUN_004e3b90(&local_160);
                iVar11 = FUN_004f6940(uVar4,uVar14);
                if (iVar11 == 0) {
                  uVar14 = CONCAT44(uVar12,extraout_ECX_19);
                  FUN_004e3c00(&local_164);
                  uVar4 = extraout_ECX_20;
                  FUN_004e3b90(&local_168);
                  iVar11 = FUN_004f6940(uVar4,uVar14);
                  if (iVar11 == 0) goto LAB_004f7204;
                }
              }
              else {
                uVar15 = CONCAT44(uVar9,extraout_ECX_11) & 0xfffffffcffffffff;
                FUN_004e3c00(&local_164);
                uVar4 = extraout_ECX_12;
                FUN_004e3b90(&local_160);
                iVar11 = FUN_004f6940(uVar4,uVar15);
                if (iVar11 == 0) {
                  uVar14 = CONCAT44(uVar12,extraout_ECX_13);
                  FUN_004e3c00(&local_164);
                  uVar4 = extraout_ECX_14;
                  FUN_004e3b90(&local_168);
                  iVar11 = FUN_004f6940(uVar4,uVar14);
                  uVar4 = extraout_ECX_15;
                  if (iVar11 == 0) goto LAB_004f713a;
                }
              }
            }
            else {
              uVar15 = CONCAT44(uVar9,extraout_ECX_05) & 0xfffffffcffffffff;
              FUN_004e3c00(&local_164);
              uVar4 = extraout_ECX_06;
              FUN_004e3b90(&local_160);
              iVar11 = FUN_004f6940(uVar4,uVar15);
              if (iVar11 == 0) {
                uVar14 = CONCAT44(uVar12,extraout_ECX_07);
                FUN_004e3c00(&local_15c);
                uVar4 = extraout_ECX_08;
                FUN_004e3b90(&local_160);
                iVar11 = FUN_004f6940(uVar4,uVar14);
                uVar4 = extraout_ECX_09;
                if (iVar11 == 0) goto LAB_004f70b4;
              }
            }
            iVar11 = FUN_0040f7a0();
            local_68[iVar6] = iVar11;
            iVar11 = FUN_0040fb00();
            local_d0[iVar6] = iVar11;
            local_138[iVar6] = uVar12 - (uVar7 & 0xfffffffc);
            iVar6 = iVar6 + 1;
          }
        }
LAB_004f7204:
        iVar11 = DAT_006c5494 << 5;
        local_160 = (int)(local_160 + 4) % iVar11;
      }
      iVar11 = DAT_006c5498 << 5;
      local_164 = (int)(local_164 + 4) % iVar11;
    }
    uVar12 = uVar12 + 4;
  } while( true );
}

