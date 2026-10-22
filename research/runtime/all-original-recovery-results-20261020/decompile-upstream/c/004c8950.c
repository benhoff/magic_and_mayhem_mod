
int __thiscall FUN_004c8950(int param_1,int *param_2)

{
  uint *puVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  char *pcVar5;
  int iVar6;
  byte *pbVar7;
  int iVar8;
  int iVar9;
  undefined4 uVar10;
  uint uVar11;
  char *pcVar12;
  uint uVar13;
  byte *pbVar14;
  int iVar15;
  bool bVar16;
  int local_124;
  int local_120;
  byte local_10c [256];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c1abe;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar4 = FUN_00597850(0x38);
  local_4 = 0;
  iVar15 = 0;
  if (iVar4 != 0) {
    FUN_004dd970();
    *(undefined4 *)(iVar4 + 0x2c) = 0;
    *(undefined4 *)(iVar4 + 0x30) = 0;
    iVar15 = iVar4;
  }
  *(undefined4 *)(iVar15 + 4) = *(undefined4 *)(param_1 + 0x850);
  iVar4 = *param_2;
  local_4 = 0xffffffff;
  *param_2 = iVar4 + 1;
  for (iVar4 = FUN_0059c73c((int)*(char *)(iVar4 + 1)); iVar4 != 0;
      iVar4 = FUN_0059c73c((int)*(char *)(iVar4 + 1))) {
    iVar4 = *param_2;
    *param_2 = iVar4 + 1;
  }
  iVar4 = *param_2;
  pcVar12 = (char *)(iVar4 + 1);
  *param_2 = (int)pcVar12;
  cVar2 = *pcVar12;
  *param_2 = iVar4 + 2;
  for (uVar13 = 0; (cVar2 != '\"' && ((int)uVar13 < 0x100)); uVar13 = uVar13 + 1) {
    cVar2 = *(char *)*param_2;
    *param_2 = (int)((char *)*param_2 + 1);
  }
  if (*(int *)(iVar15 + 0x2c) != 0) {
    FUN_00597870(*(int *)(iVar15 + 0x2c));
  }
  pcVar5 = (char *)FUN_00597850(uVar13 + 1);
  *(char **)(iVar15 + 0x2c) = pcVar5;
  for (uVar11 = uVar13 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
    *(undefined4 *)pcVar5 = *(undefined4 *)pcVar12;
    pcVar12 = pcVar12 + 4;
    pcVar5 = pcVar5 + 4;
  }
  for (uVar11 = uVar13 & 3; uVar11 != 0; uVar11 = uVar11 - 1) {
    *pcVar5 = *pcVar12;
    pcVar12 = pcVar12 + 1;
    pcVar5 = pcVar5 + 1;
  }
  *(undefined1 *)(*(int *)(iVar15 + 0x2c) + uVar13) = 0;
  for (iVar4 = FUN_0059c73c((int)*(char *)*param_2); iVar4 != 0;
      iVar4 = FUN_0059c73c((int)*(char *)(iVar4 + 1))) {
    iVar4 = *param_2;
    *param_2 = iVar4 + 1;
  }
  iVar6 = FUN_0059c6e7((int)*(char *)*param_2);
  iVar4 = 0;
  while (iVar6 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar4] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar6 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar4 = iVar4 + 1;
  }
  local_10c[iVar4] = 0;
  local_124 = FUN_0059c5a4(local_10c);
  local_120 = 0;
  if (*(char *)*param_2 == 'x') {
    pcVar12 = (char *)*param_2 + 1;
    *param_2 = (int)pcVar12;
    iVar6 = FUN_0059c6e7((int)*pcVar12);
    iVar4 = 0;
    while (iVar6 != 0) {
      pbVar7 = (byte *)*param_2 + 1;
      local_10c[iVar4] = *(byte *)*param_2;
      *param_2 = (int)pbVar7;
      iVar6 = FUN_0059c6e7((int)(char)*pbVar7);
      iVar4 = iVar4 + 1;
    }
    local_10c[iVar4] = 0;
    local_120 = FUN_0059c5a4(local_10c);
  }
  for (iVar4 = FUN_0059c73c((int)*(char *)*param_2); iVar4 != 0;
      iVar4 = FUN_0059c73c((int)*(char *)(iVar4 + 1))) {
    iVar4 = *param_2;
    *param_2 = iVar4 + 1;
  }
  iVar6 = FUN_0059c6e7((int)*(char *)*param_2);
  iVar4 = 0;
  while (iVar6 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar4] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar6 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar4 = iVar4 + 1;
  }
  local_10c[iVar4] = 0;
  iVar6 = FUN_0059c5a4(local_10c);
  iVar4 = *param_2;
  *param_2 = iVar4 + 1;
  iVar8 = FUN_0059c6e7((int)*(char *)(iVar4 + 1));
  iVar4 = 0;
  while (iVar8 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar4] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar8 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar4 = iVar4 + 1;
  }
  local_10c[iVar4] = 0;
  iVar4 = FUN_0059c5a4(local_10c);
  for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
      iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
    iVar8 = *param_2;
    *param_2 = iVar8 + 1;
  }
  iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
  iVar8 = 0;
  while (iVar9 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  FUN_0059c5a4(local_10c);
  iVar8 = *param_2;
  *param_2 = iVar8 + 1;
  iVar9 = FUN_0059c6e7((int)*(char *)(iVar8 + 1));
  iVar8 = 0;
  while (iVar9 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  FUN_0059c5a4(local_10c);
  iVar8 = *param_2;
  *param_2 = iVar8 + 1;
  iVar9 = FUN_0059c6e7((int)*(char *)(iVar8 + 1));
  iVar8 = 0;
  while (iVar9 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  FUN_0059c5a4(local_10c);
  for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
      iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
    iVar8 = *param_2;
    *param_2 = iVar8 + 1;
  }
  iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
  iVar8 = 0;
  while (iVar9 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  FUN_0059c5a4(local_10c);
  iVar8 = *param_2;
  *param_2 = iVar8 + 1;
  iVar9 = FUN_0059c6e7((int)*(char *)(iVar8 + 1));
  iVar8 = 0;
  while (iVar9 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  FUN_0059c5a4(local_10c);
  iVar8 = *param_2;
  *param_2 = iVar8 + 1;
  iVar9 = FUN_0059c6e7((int)*(char *)(iVar8 + 1));
  iVar8 = 0;
  while (iVar9 != 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  FUN_0059c5a4(local_10c);
  for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
      iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
    iVar8 = *param_2;
    *param_2 = iVar8 + 1;
  }
  iVar9 = FUN_0059c73c((int)*(char *)*param_2);
  iVar8 = 0;
  while (iVar9 == 0) {
    pbVar7 = (byte *)*param_2 + 1;
    local_10c[iVar8] = *(byte *)*param_2;
    *param_2 = (int)pbVar7;
    iVar9 = FUN_0059c73c((int)(char)*pbVar7);
    iVar8 = iVar8 + 1;
  }
  local_10c[iVar8] = 0;
  pbVar14 = &DAT_005e2070;
  pbVar7 = local_10c;
  do {
    bVar3 = *pbVar7;
    bVar16 = bVar3 < *pbVar14;
    if (bVar3 != *pbVar14) {
LAB_004c8e67:
      iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
      goto LAB_004c8e6c;
    }
    if (bVar3 == 0) break;
    bVar3 = pbVar7[1];
    bVar16 = bVar3 < pbVar14[1];
    if (bVar3 != pbVar14[1]) goto LAB_004c8e67;
    pbVar7 = pbVar7 + 2;
    pbVar14 = pbVar14 + 2;
  } while (bVar3 != 0);
  iVar8 = 0;
LAB_004c8e6c:
  if (iVar8 == 0) {
    for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
        iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
      iVar8 = *param_2;
      *param_2 = iVar8 + 1;
    }
    iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
    iVar8 = 0;
    while (iVar9 != 0) {
      pbVar7 = (byte *)*param_2 + 1;
      local_10c[iVar8] = *(byte *)*param_2;
      *param_2 = (int)pbVar7;
      iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
      iVar8 = iVar8 + 1;
    }
    local_10c[iVar8] = 0;
    FUN_0059c5a4(local_10c);
    iVar8 = *param_2;
    *param_2 = iVar8 + 1;
    iVar9 = FUN_0059c6e7((int)*(char *)(iVar8 + 1));
    iVar8 = 0;
    while (iVar9 != 0) {
      pbVar7 = (byte *)*param_2 + 1;
      local_10c[iVar8] = *(byte *)*param_2;
      *param_2 = (int)pbVar7;
      iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
      iVar8 = iVar8 + 1;
    }
    local_10c[iVar8] = 0;
    uVar10 = FUN_0059c5a4(local_10c);
    iVar8 = DAT_006e981c;
    if (DAT_006e981c <= DAT_006e9820) {
      iVar8 = DAT_006e9820;
    }
    *(int *)(iVar15 + 0x20) = iVar8;
    *(undefined4 *)(iVar15 + 0x1c) = 0;
    uVar10 = FUN_00505c50(uVar10);
    *(undefined4 *)(iVar15 + 0x24) = uVar10;
    *(undefined4 *)(iVar15 + 0x28) = uVar10;
  }
  else {
    pbVar14 = &DAT_005e206c;
    pbVar7 = local_10c;
    do {
      bVar3 = *pbVar7;
      bVar16 = bVar3 < *pbVar14;
      if (bVar3 != *pbVar14) {
LAB_004c8f99:
        iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
        goto LAB_004c8f9e;
      }
      if (bVar3 == 0) break;
      bVar3 = pbVar7[1];
      bVar16 = bVar3 < pbVar14[1];
      if (bVar3 != pbVar14[1]) goto LAB_004c8f99;
      pbVar7 = pbVar7 + 2;
      pbVar14 = pbVar14 + 2;
    } while (bVar3 != 0);
    iVar8 = 0;
LAB_004c8f9e:
    if (iVar8 == 0) {
      for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
          iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
        iVar8 = *param_2;
        *param_2 = iVar8 + 1;
      }
      iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
      iVar8 = 0;
      while (iVar9 != 0) {
        pbVar7 = (byte *)*param_2 + 1;
        local_10c[iVar8] = *(byte *)*param_2;
        *param_2 = (int)pbVar7;
        iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
        iVar8 = iVar8 + 1;
      }
      local_10c[iVar8] = 0;
      FUN_0059c5a4(local_10c);
      iVar8 = *param_2;
      *param_2 = iVar8 + 1;
      iVar9 = FUN_0059c6e7((int)*(char *)(iVar8 + 1));
      iVar8 = 0;
      while (iVar9 != 0) {
        pbVar7 = (byte *)*param_2 + 1;
        local_10c[iVar8] = *(byte *)*param_2;
        *param_2 = (int)pbVar7;
        iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
        iVar8 = iVar8 + 1;
      }
      local_10c[iVar8] = 0;
      uVar10 = FUN_0059c5a4(local_10c);
      iVar8 = FUN_00505bf0(uVar10);
      if (iVar8 == 0) {
        if (iVar15 == 0) {
          ExceptionList = local_c;
          return 0;
        }
        iVar4 = *(int *)(iVar15 + 0x2c);
joined_r0x004c97d4:
        if (iVar4 != 0) {
          FUN_00597870(iVar4);
        }
        FUN_004dd9a0();
        FUN_00597870(iVar15);
        ExceptionList = local_c;
        return 0;
      }
      iVar8 = DAT_006e981c;
      if (DAT_006e981c <= DAT_006e9820) {
        iVar8 = DAT_006e9820;
      }
      *(undefined4 *)(iVar15 + 0x1c) = 0;
      *(int *)(iVar15 + 0x20) = iVar8;
      uVar10 = FUN_00505bf0(uVar10);
      *(undefined4 *)(iVar15 + 0x24) = uVar10;
      *(undefined4 *)(iVar15 + 0x28) = uVar10;
    }
    else {
      pcVar12 = s_HEALTH_005e2064;
      pbVar7 = local_10c;
      do {
        bVar3 = *pbVar7;
        bVar16 = bVar3 < (byte)*pcVar12;
        if (bVar3 != *pcVar12) {
LAB_004c90fa:
          iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
          goto LAB_004c90ff;
        }
        if (bVar3 == 0) break;
        bVar3 = pbVar7[1];
        bVar16 = bVar3 < (byte)pcVar12[1];
        if (bVar3 != pcVar12[1]) goto LAB_004c90fa;
        pbVar7 = pbVar7 + 2;
        pcVar12 = pcVar12 + 2;
      } while (bVar3 != 0);
      iVar8 = 0;
LAB_004c90ff:
      if (iVar8 == 0) {
        for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
            iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
          iVar8 = *param_2;
          *param_2 = iVar8 + 1;
        }
        iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
        iVar8 = 0;
        while (iVar9 != 0) {
          pbVar7 = (byte *)*param_2 + 1;
          local_10c[iVar8] = *(byte *)*param_2;
          *param_2 = (int)pbVar7;
          iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
          iVar8 = iVar8 + 1;
        }
        local_10c[iVar8] = 0;
        iVar8 = FUN_0059c5a4(local_10c);
        *(undefined4 *)(iVar15 + 0x20) = DAT_006e9824;
        *(undefined4 *)(iVar15 + 0x1c) = 0;
        uVar10 = *(undefined4 *)((int)&DAT_006a5f98 + iVar8 * 0x5c9);
        *(undefined4 *)(iVar15 + 0x24) = uVar10;
        *(undefined4 *)(iVar15 + 0x28) = uVar10;
      }
      else {
        pcVar12 = s_MAGIC_RESIST_005e2054;
        pbVar7 = local_10c;
        do {
          bVar3 = *pbVar7;
          bVar16 = bVar3 < (byte)*pcVar12;
          if (bVar3 != *pcVar12) {
LAB_004c91d0:
            iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
            goto LAB_004c91d5;
          }
          if (bVar3 == 0) break;
          bVar3 = pbVar7[1];
          bVar16 = bVar3 < (byte)pcVar12[1];
          if (bVar3 != pcVar12[1]) goto LAB_004c91d0;
          pbVar7 = pbVar7 + 2;
          pcVar12 = pcVar12 + 2;
        } while (bVar3 != 0);
        iVar8 = 0;
LAB_004c91d5:
        if (iVar8 == 0) {
          for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
              iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
            iVar8 = *param_2;
            *param_2 = iVar8 + 1;
          }
          iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
          iVar8 = 0;
          while (iVar9 != 0) {
            pbVar7 = (byte *)*param_2 + 1;
            local_10c[iVar8] = *(byte *)*param_2;
            *param_2 = (int)pbVar7;
            iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
            iVar8 = iVar8 + 1;
          }
          local_10c[iVar8] = 0;
          iVar8 = FUN_0059c5a4(local_10c);
          *(undefined4 *)(iVar15 + 0x20) = DAT_006e9828;
          *(undefined4 *)(iVar15 + 0x1c) = 0;
          uVar10 = *(undefined4 *)(&DAT_006a5fa8 + iVar8 * 0x5c9);
          *(undefined4 *)(iVar15 + 0x24) = uVar10;
          *(undefined4 *)(iVar15 + 0x28) = uVar10;
        }
        else {
          pcVar12 = s_INTELLIGENCE_005e2044;
          pbVar7 = local_10c;
          do {
            bVar3 = *pbVar7;
            bVar16 = bVar3 < (byte)*pcVar12;
            if (bVar3 != *pcVar12) {
LAB_004c92a6:
              iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
              goto LAB_004c92ab;
            }
            if (bVar3 == 0) break;
            bVar3 = pbVar7[1];
            bVar16 = bVar3 < (byte)pcVar12[1];
            if (bVar3 != pcVar12[1]) goto LAB_004c92a6;
            pbVar7 = pbVar7 + 2;
            pcVar12 = pcVar12 + 2;
          } while (bVar3 != 0);
          iVar8 = 0;
LAB_004c92ab:
          if (iVar8 == 0) {
            for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
              iVar8 = *param_2;
              *param_2 = iVar8 + 1;
            }
            iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
            iVar8 = 0;
            while (iVar9 != 0) {
              pbVar7 = (byte *)*param_2 + 1;
              local_10c[iVar8] = *(byte *)*param_2;
              *param_2 = (int)pbVar7;
              iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
              iVar8 = iVar8 + 1;
            }
            local_10c[iVar8] = 0;
            iVar8 = FUN_0059c5a4(local_10c);
            *(undefined4 *)(iVar15 + 0x20) = DAT_006e982c;
            *(undefined4 *)(iVar15 + 0x1c) = 0;
            uVar10 = *(undefined4 *)(&DAT_006a5f94 + iVar8 * 0x5c9);
            *(undefined4 *)(iVar15 + 0x24) = uVar10;
            *(undefined4 *)(iVar15 + 0x28) = uVar10;
          }
          else {
            pcVar12 = s_WEIGHT_005e203c;
            pbVar7 = local_10c;
            do {
              bVar3 = *pbVar7;
              bVar16 = bVar3 < (byte)*pcVar12;
              if (bVar3 != *pcVar12) {
LAB_004c937c:
                iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                goto LAB_004c9381;
              }
              if (bVar3 == 0) break;
              bVar3 = pbVar7[1];
              bVar16 = bVar3 < (byte)pcVar12[1];
              if (bVar3 != pcVar12[1]) goto LAB_004c937c;
              pbVar7 = pbVar7 + 2;
              pcVar12 = pcVar12 + 2;
            } while (bVar3 != 0);
            iVar8 = 0;
LAB_004c9381:
            if (iVar8 == 0) {
              for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                  iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                iVar8 = *param_2;
                *param_2 = iVar8 + 1;
              }
              iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
              iVar8 = 0;
              while (iVar9 != 0) {
                pbVar7 = (byte *)*param_2 + 1;
                local_10c[iVar8] = *(byte *)*param_2;
                *param_2 = (int)pbVar7;
                iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                iVar8 = iVar8 + 1;
              }
              local_10c[iVar8] = 0;
              iVar8 = FUN_0059c5a4(local_10c);
              *(undefined4 *)(iVar15 + 0x20) = DAT_006e9830;
              *(undefined4 *)(iVar15 + 0x1c) = 0;
              uVar10 = *(undefined4 *)((int)&DAT_006a6531 + iVar8 * 0x5c9);
              *(undefined4 *)(iVar15 + 0x24) = uVar10;
              *(undefined4 *)(iVar15 + 0x28) = uVar10;
            }
            else {
              pcVar12 = s_VIEW_RANGE_005e2030;
              pbVar7 = local_10c;
              do {
                bVar3 = *pbVar7;
                bVar16 = bVar3 < (byte)*pcVar12;
                if (bVar3 != *pcVar12) {
LAB_004c9452:
                  iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                  goto LAB_004c9457;
                }
                if (bVar3 == 0) break;
                bVar3 = pbVar7[1];
                bVar16 = bVar3 < (byte)pcVar12[1];
                if (bVar3 != pcVar12[1]) goto LAB_004c9452;
                pbVar7 = pbVar7 + 2;
                pcVar12 = pcVar12 + 2;
              } while (bVar3 != 0);
              iVar8 = 0;
LAB_004c9457:
              if (iVar8 == 0) {
                for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                    iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                  iVar8 = *param_2;
                  *param_2 = iVar8 + 1;
                }
                iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                iVar8 = 0;
                while (iVar9 != 0) {
                  pbVar7 = (byte *)*param_2 + 1;
                  local_10c[iVar8] = *(byte *)*param_2;
                  *param_2 = (int)pbVar7;
                  iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                  iVar8 = iVar8 + 1;
                }
                local_10c[iVar8] = 0;
                iVar8 = FUN_0059c5a4(local_10c);
                uVar13 = DAT_006e9838;
                if ((int)DAT_006e9838 <= (int)DAT_006e9834) {
                  uVar13 = DAT_006e9834;
                }
                *(uint *)(iVar15 + 0x20) = uVar13;
                *(undefined4 *)(iVar15 + 0x1c) = 0;
                uVar10 = *(undefined4 *)(&DAT_006a5fb8 + iVar8 * 0x5c9);
                *(undefined4 *)(iVar15 + 0x24) = uVar10;
                *(undefined4 *)(iVar15 + 0x28) = uVar10;
              }
              else {
                pcVar12 = s_FIRE_RANGE_005e2024;
                pbVar7 = local_10c;
                do {
                  bVar3 = *pbVar7;
                  bVar16 = bVar3 < (byte)*pcVar12;
                  if (bVar3 != *pcVar12) {
LAB_004c9534:
                    iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                    goto LAB_004c9539;
                  }
                  if (bVar3 == 0) break;
                  bVar3 = pbVar7[1];
                  bVar16 = bVar3 < (byte)pcVar12[1];
                  if (bVar3 != pcVar12[1]) goto LAB_004c9534;
                  pbVar7 = pbVar7 + 2;
                  pcVar12 = pcVar12 + 2;
                } while (bVar3 != 0);
                iVar8 = 0;
LAB_004c9539:
                if (iVar8 == 0) {
                  for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                      iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                    iVar8 = *param_2;
                    *param_2 = iVar8 + 1;
                  }
                  iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                  iVar8 = 0;
                  while (iVar9 != 0) {
                    pbVar7 = (byte *)*param_2 + 1;
                    local_10c[iVar8] = *(byte *)*param_2;
                    *param_2 = (int)pbVar7;
                    iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                    iVar8 = iVar8 + 1;
                  }
                  local_10c[iVar8] = 0;
                  iVar8 = FUN_0059c5a4(local_10c);
                  uVar13 = DAT_006e9838;
                  if ((int)DAT_006e9838 <= (int)DAT_006e9834) {
                    uVar13 = DAT_006e9834;
                  }
                  uVar11 = DAT_006f56a8;
                  if (((int)DAT_006f56a8 < (int)uVar13) &&
                     (uVar11 = DAT_006e9834, (int)DAT_006e9834 < (int)DAT_006e9838)) {
                    uVar11 = DAT_006e9838;
                  }
                  *(uint *)(iVar15 + 0x20) = uVar11;
                  *(undefined4 *)(iVar15 + 0x1c) = 0;
                  uVar10 = *(undefined4 *)(&DAT_006a5fb4 + iVar8 * 0x5c9);
                  *(undefined4 *)(iVar15 + 0x24) = uVar10;
                  *(undefined4 *)(iVar15 + 0x28) = uVar10;
                }
                else {
                  pcVar12 = s_GROUND_SPEED_005e2014;
                  pbVar7 = local_10c;
                  do {
                    bVar3 = *pbVar7;
                    bVar16 = bVar3 < (byte)*pcVar12;
                    if (bVar3 != *pcVar12) {
LAB_004c962c:
                      iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                      goto LAB_004c9631;
                    }
                    if (bVar3 == 0) break;
                    bVar3 = pbVar7[1];
                    bVar16 = bVar3 < (byte)pcVar12[1];
                    if (bVar3 != pcVar12[1]) goto LAB_004c962c;
                    pbVar7 = pbVar7 + 2;
                    pcVar12 = pcVar12 + 2;
                  } while (bVar3 != 0);
                  iVar8 = 0;
LAB_004c9631:
                  if (iVar8 == 0) {
                    for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                        iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                      iVar8 = *param_2;
                      *param_2 = iVar8 + 1;
                    }
                    iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                    iVar8 = 0;
                    while (iVar9 != 0) {
                      pbVar7 = (byte *)*param_2 + 1;
                      local_10c[iVar8] = *(byte *)*param_2;
                      *param_2 = (int)pbVar7;
                      iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                      iVar8 = iVar8 + 1;
                    }
                    local_10c[iVar8] = 0;
                    iVar9 = FUN_0059c5a4(local_10c);
                    iVar8 = DAT_006e983c;
                    if (DAT_006e983c <= DAT_006e9840) {
                      iVar8 = DAT_006e9840;
                    }
                    *(int *)(iVar15 + 0x20) = iVar8;
                    *(undefined4 *)(iVar15 + 0x1c) = 0;
                    uVar10 = *(undefined4 *)(&DAT_006a653d + iVar9 * 0x5c9);
                    *(undefined4 *)(iVar15 + 0x24) = uVar10;
                    *(undefined4 *)(iVar15 + 0x28) = uVar10;
                  }
                  else {
                    pcVar12 = s_FLYING_SPEED_005e2004;
                    pbVar7 = local_10c;
                    do {
                      bVar3 = *pbVar7;
                      bVar16 = bVar3 < (byte)*pcVar12;
                      if (bVar3 != *pcVar12) {
LAB_004c970e:
                        iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                        goto LAB_004c9713;
                      }
                      if (bVar3 == 0) break;
                      bVar3 = pbVar7[1];
                      bVar16 = bVar3 < (byte)pcVar12[1];
                      if (bVar3 != pcVar12[1]) goto LAB_004c970e;
                      pbVar7 = pbVar7 + 2;
                      pcVar12 = pcVar12 + 2;
                    } while (bVar3 != 0);
                    iVar8 = 0;
LAB_004c9713:
                    if (iVar8 == 0) {
                      for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                          iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                        iVar8 = *param_2;
                        *param_2 = iVar8 + 1;
                      }
                      iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                      iVar8 = 0;
                      while (iVar9 != 0) {
                        pbVar7 = (byte *)*param_2 + 1;
                        local_10c[iVar8] = *(byte *)*param_2;
                        *param_2 = (int)pbVar7;
                        iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                        iVar8 = iVar8 + 1;
                      }
                      local_10c[iVar8] = 0;
                      iVar8 = FUN_0059c5a4(local_10c);
                      iVar8 = *(int *)(&DAT_006a6541 + iVar8 * 0x5c9);
                      if (iVar8 == 0) {
                        if (iVar15 == 0) {
                          ExceptionList = local_c;
                          return 0;
                        }
                        iVar4 = *(int *)(iVar15 + 0x2c);
                        goto joined_r0x004c97d4;
                      }
                      iVar9 = DAT_006e983c;
                      if (DAT_006e983c <= DAT_006e9840) {
                        iVar9 = DAT_006e9840;
                      }
                      *(undefined4 *)(iVar15 + 0x1c) = 0;
                      *(int *)(iVar15 + 0x20) = iVar9;
                      *(int *)(iVar15 + 0x24) = iVar8;
                      *(int *)(iVar15 + 0x28) = iVar8;
                    }
                    else {
                      pcVar12 = s_BRAVERY_005e1ffc;
                      pbVar7 = local_10c;
                      do {
                        bVar3 = *pbVar7;
                        bVar16 = bVar3 < (byte)*pcVar12;
                        if (bVar3 != *pcVar12) {
LAB_004c9823:
                          iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                          goto LAB_004c9828;
                        }
                        if (bVar3 == 0) break;
                        bVar3 = pbVar7[1];
                        bVar16 = bVar3 < (byte)pcVar12[1];
                        if (bVar3 != pcVar12[1]) goto LAB_004c9823;
                        pbVar7 = pbVar7 + 2;
                        pcVar12 = pcVar12 + 2;
                      } while (bVar3 != 0);
                      iVar8 = 0;
LAB_004c9828:
                      if (iVar8 == 0) {
                        for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                            iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                          iVar8 = *param_2;
                          *param_2 = iVar8 + 1;
                        }
                        iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                        iVar8 = 0;
                        while (iVar9 != 0) {
                          pbVar7 = (byte *)*param_2 + 1;
                          local_10c[iVar8] = *(byte *)*param_2;
                          *param_2 = (int)pbVar7;
                          iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                          iVar8 = iVar8 + 1;
                        }
                        local_10c[iVar8] = 0;
                        iVar8 = FUN_0059c5a4(local_10c);
                        *(undefined4 *)(iVar15 + 0x20) = DAT_006e9844;
                        *(undefined4 *)(iVar15 + 0x1c) = 0;
                        uVar10 = *(undefined4 *)(&DAT_006a6545 + iVar8 * 0x5c9);
                        *(undefined4 *)(iVar15 + 0x24) = uVar10;
                        *(undefined4 *)(iVar15 + 0x28) = uVar10;
                      }
                      else {
                        pcVar12 = s_SPELL_RANGE_005e1ff0;
                        pbVar7 = local_10c;
                        do {
                          bVar3 = *pbVar7;
                          bVar16 = bVar3 < (byte)*pcVar12;
                          if (bVar3 != *pcVar12) {
LAB_004c98f9:
                            iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                            goto LAB_004c98fe;
                          }
                          if (bVar3 == 0) break;
                          bVar3 = pbVar7[1];
                          bVar16 = bVar3 < (byte)pcVar12[1];
                          if (bVar3 != pcVar12[1]) goto LAB_004c98f9;
                          pbVar7 = pbVar7 + 2;
                          pcVar12 = pcVar12 + 2;
                        } while (bVar3 != 0);
                        iVar8 = 0;
LAB_004c98fe:
                        if (iVar8 == 0) {
                          for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                              iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                            iVar8 = *param_2;
                            *param_2 = iVar8 + 1;
                          }
                          iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                          iVar8 = 0;
                          while (iVar9 != 0) {
                            pbVar7 = (byte *)*param_2 + 1;
                            local_10c[iVar8] = *(byte *)*param_2;
                            *param_2 = (int)pbVar7;
                            iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                            iVar8 = iVar8 + 1;
                          }
                          local_10c[iVar8] = 0;
                          iVar8 = FUN_0059c5a4(local_10c);
                          uVar13 = DAT_006e9838;
                          if ((int)DAT_006e9838 <= (int)DAT_006e9834) {
                            uVar13 = DAT_006e9834;
                          }
                          uVar11 = DAT_006f56a8;
                          if (((int)DAT_006f56a8 < (int)uVar13) &&
                             (uVar11 = DAT_006e9834, (int)DAT_006e9834 < (int)DAT_006e9838)) {
                            uVar11 = DAT_006e9838;
                          }
                          puVar1 = (uint *)((int)&DAT_006b1268 + iVar8 * 0x2d1);
                          uVar13 = *puVar1;
                          if (uVar13 <= uVar11) {
                            uVar11 = DAT_006e9838;
                            if ((int)DAT_006e9838 <= (int)DAT_006e9834) {
                              uVar11 = DAT_006e9834;
                            }
                            uVar13 = DAT_006f56a8;
                            if (((int)DAT_006f56a8 < (int)uVar11) &&
                               (uVar13 = DAT_006e9834, (int)DAT_006e9834 < (int)DAT_006e9838)) {
                              uVar13 = DAT_006e9838;
                            }
                          }
                          *(undefined4 *)(iVar15 + 0x1c) = 0;
                          *(uint *)(iVar15 + 0x20) = uVar13;
                          uVar13 = *puVar1;
                          *(uint *)(iVar15 + 0x24) = uVar13;
                          *(uint *)(iVar15 + 0x28) = uVar13;
                        }
                        else {
                          pcVar12 = s_MANA_COST_005e1fe4;
                          pbVar7 = local_10c;
                          do {
                            bVar3 = *pbVar7;
                            bVar16 = bVar3 < (byte)*pcVar12;
                            if (bVar3 != *pcVar12) {
LAB_004c9a1d:
                              iVar8 = (1 - (uint)bVar16) - (uint)(bVar16 != 0);
                              goto LAB_004c9a22;
                            }
                            if (bVar3 == 0) break;
                            bVar3 = pbVar7[1];
                            bVar16 = bVar3 < (byte)pcVar12[1];
                            if (bVar3 != pcVar12[1]) goto LAB_004c9a1d;
                            pbVar7 = pbVar7 + 2;
                            pcVar12 = pcVar12 + 2;
                          } while (bVar3 != 0);
                          iVar8 = 0;
LAB_004c9a22:
                          if (iVar8 == 0) {
                            for (iVar8 = FUN_0059c73c((int)*(char *)*param_2); iVar8 != 0;
                                iVar8 = FUN_0059c73c((int)*(char *)(iVar8 + 1))) {
                              iVar8 = *param_2;
                              *param_2 = iVar8 + 1;
                            }
                            iVar9 = FUN_0059c6e7((int)*(char *)*param_2);
                            iVar8 = 0;
                            while (iVar9 != 0) {
                              pbVar7 = (byte *)*param_2 + 1;
                              local_10c[iVar8] = *(byte *)*param_2;
                              *param_2 = (int)pbVar7;
                              iVar9 = FUN_0059c6e7((int)(char)*pbVar7);
                              iVar8 = iVar8 + 1;
                            }
                            local_10c[iVar8] = 0;
                            iVar8 = FUN_0059c5a4(local_10c);
                            *(undefined4 *)(iVar15 + 0x20) = DAT_006f56a4;
                            *(undefined4 *)(iVar15 + 0x1c) = 0;
                            uVar10 = *(undefined4 *)((int)&DAT_006b1270 + iVar8 * 0x2d1);
                            *(undefined4 *)(iVar15 + 0x24) = uVar10;
                            *(undefined4 *)(iVar15 + 0x28) = uVar10;
                          }
                        }
                      }
                    }
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  if (DAT_006de6d5 == 0) {
    iVar6 = (iVar6 * 8) / 10;
    iVar4 = (iVar4 * 8) / 10;
    local_124 = (local_124 << 3) / 10;
    local_120 = (local_120 << 3) / 10;
  }
  *(int *)(iVar15 + 0x30) = DAT_006e2058 + local_124;
  *(int *)(iVar15 + 0x34) = DAT_006e205c + local_120;
  *(undefined4 *)(iVar15 + 8) = 0;
  *(undefined4 *)(iVar15 + 0xc) = 0;
  *(int *)(iVar15 + 0x10) = iVar6;
  *(int *)(iVar15 + 0x14) = iVar4;
  for (iVar4 = FUN_0059c73c((int)*(char *)*param_2); iVar4 != 0;
      iVar4 = FUN_0059c73c((int)*(char *)(iVar4 + 1))) {
    iVar4 = *param_2;
    *param_2 = iVar4 + 1;
  }
  *param_2 = *param_2 + -1;
  ExceptionList = local_c;
  return iVar15;
}

