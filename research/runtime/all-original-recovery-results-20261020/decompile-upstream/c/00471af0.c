
int __thiscall FUN_00471af0(int param_1,undefined4 param_2)

{
  char cVar1;
  undefined4 *puVar2;
  int iVar3;
  int iVar4;
  int iVar5;
  uint uVar6;
  uint uVar7;
  int *piVar8;
  char *pcVar9;
  char *pcVar10;
  char *pcVar11;
  undefined4 uVar12;
  int local_134;
  int local_130;
  int local_12c;
  int local_128;
  undefined4 local_124;
  undefined4 local_120;
  undefined4 local_11c;
  undefined4 local_118;
  undefined4 local_114;
  undefined1 local_110 [16];
  char local_100 [256];
  
  local_12c = param_1;
  FUN_004dc4a0(((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x2a) / 2,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1bc,7);
  FUN_004dc5e0();
  if (DAT_006c54dc != 0) {
    FUN_00477060();
  }
  FUN_005458b0();
  FUN_00464b90(&DAT_006a28f8);
  iVar3 = FUN_0057d310(s_sprites_lordking_spr_005da1ac,0x10);
  if ((iVar3 == 0) || (iVar3 = FUN_004644d0(s_sprites_lordking_ani_005da170,1), iVar3 == 0)) {
    FUN_0049e680();
    FUN_00557130();
  }
  iVar3 = FUN_0059c359(param_1 + 0xfc5a,1,0x100,param_2);
  if (iVar3 != 0x100) {
    return -1;
  }
  iVar3 = FUN_0046e5e0();
  if (iVar3 == 0) {
    FUN_0049e680();
    FUN_00557130();
    return -1;
  }
  iVar3 = FUN_0059c359(&DAT_006e980c,4,4,param_2);
  uVar6 = -(uint)(iVar3 != 4) & 0xffffffef;
  local_134 = uVar6 + 0x10;
  if (local_134 == -1) {
    return -1;
  }
  uVar12 = param_2;
  FUN_00473890(param_2);
  local_134 = FUN_004a0f70(uVar12);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = uVar6 + local_134;
  iVar4 = FUN_0059c359(&DAT_006c4830,4,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  local_134 = FUN_00501e00(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = local_134 + iVar3;
  FUN_004f7930(&DAT_006c5490);
  local_134 = FUN_004ffcb0(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  DAT_005fed78 = DAT_00689961;
  FUN_00473270();
  if (*(int *)(param_1 + 0xfc52) != 0) {
    local_124 = *(undefined4 *)(param_1 + 0xfc3a);
    local_120 = *(undefined4 *)(param_1 + 0xfc3e);
    local_11c = *(undefined4 *)(param_1 + 0xfc42);
    local_118 = *(undefined4 *)(param_1 + 0xfc46);
    FUN_004736e0();
    FUN_004730c0();
    FUN_004733a0(local_110,*(undefined4 *)(param_1 + 0xfc2a),*(undefined4 *)(param_1 + 0xfc2e),
                 *(undefined4 *)(param_1 + 0xfc32),*(undefined4 *)(param_1 + 0xfc36),local_124,
                 local_120,local_11c,local_118);
  }
  iVar4 = FUN_0059c359(&local_134,4,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar3 = iVar3 + 0x118;
  iVar4 = param_1;
  if (0 < local_134) {
    local_128 = 0;
    do {
      iVar4 = FUN_0059c359(&local_114,4,1,param_2);
      if (iVar4 != 1) {
        return -1;
      }
      iVar3 = iVar3 + 4;
      local_130 = FUN_00501970(local_114);
      if (*(int *)(param_1 + 0xfc52) == 0) {
LAB_00471e2d:
        FUN_00472ee0();
        *(int *)(param_1 + 0xfc42) = *(int *)(param_1 + 0xfc42) + 4;
        FUN_00473320();
      }
      else {
        puVar2 = *(undefined4 **)(param_1 + 0xfc42);
        if (puVar2 == *(undefined4 **)(param_1 + 0xfc3e)) goto LAB_00471e2d;
        *(undefined4 **)(param_1 + 0xfc42) = puVar2 + 1;
        if (puVar2 + 1 == *(undefined4 **)(param_1 + 0xfc3e)) {
          FUN_00473320();
          FUN_00472ee0();
        }
        else if (puVar2 != (undefined4 *)0x0) {
          *puVar2 = local_130;
        }
      }
      local_128 = local_128 + 1;
      *(int *)(param_1 + 0xfc52) = *(int *)(param_1 + 0xfc52) + 1;
      iVar4 = local_12c;
    } while (local_128 < local_134);
  }
  *(undefined4 *)(iVar4 + 0xfc1e) = 0;
  local_134 = FUN_004e1050(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  local_134 = FUN_00466a30(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  local_134 = FUN_0052c160(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  iVar5 = FUN_0059c359(&DAT_006e1fa8,4,1,param_2);
  if (iVar5 != 1) {
    return -1;
  }
  iVar5 = FUN_0059c359(&DAT_006e1f94,4,1,param_2);
  if (iVar5 != 1) {
    return -1;
  }
  iVar5 = FUN_0059c359(&DAT_006e1f98,4,1,param_2);
  if (iVar5 != 1) {
    return -1;
  }
  local_134 = FUN_0049dcf0(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = local_134 + iVar3;
  local_134 = FUN_00544fb0(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  local_134 = FUN_0057b6c0();
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  iVar5 = FUN_0059c359(&DAT_006e1f3c,4,1,param_2);
  if (iVar5 != 1) {
    return -1;
  }
  iVar3 = iVar3 + 0x10;
  if (DAT_006e1f3c == 0) {
    FUN_0053aa30();
  }
  else {
    local_134 = FUN_00542f60(param_2);
    if (local_134 == -1) {
      return -1;
    }
    iVar3 = iVar3 + local_134;
  }
  local_134 = FUN_00534850(param_2);
  if (local_134 == -1) {
    return -1;
  }
  iVar3 = iVar3 + local_134;
  piVar8 = &DAT_006581a4;
  do {
    iVar5 = FUN_0059c359(piVar8,4,1,param_2);
    if (iVar5 == 1) {
      iVar5 = 4;
      if (*piVar8 != 0) {
        iVar5 = FUN_0059c359(piVar8 + 1,4,1,param_2);
        if (iVar5 == 1) {
          iVar5 = FUN_0059c359(piVar8 + 2,4,1,param_2);
          if (iVar5 == 1) {
            iVar5 = FUN_0059c359(piVar8 + 3,4,1,param_2);
            if (iVar5 == 1) {
              iVar5 = 0x10;
            }
            else {
              iVar5 = -1;
            }
          }
          else {
            iVar5 = -1;
          }
        }
        else {
          iVar5 = -1;
        }
      }
    }
    else {
      iVar5 = -1;
    }
    if (iVar5 == -1) {
      return -1;
    }
    piVar8 = piVar8 + 5;
    iVar3 = iVar3 + iVar5;
    local_134 = iVar5;
  } while ((int)piVar8 < 0x658974);
  local_134 = FUN_0056cd40(param_2);
  if (local_134 == -1) {
    return -1;
  }
  local_130 = iVar3 + local_134;
  if (DAT_006e266c != 0) {
    FUN_00597870(DAT_006e266c);
    DAT_006e266c = 0;
  }
  uVar12 = DAT_005e1784;
  if (DAT_006e266c != 0) {
    FUN_00597870(DAT_006e266c);
  }
  DAT_006e266c = FUN_00597850(uVar12);
  iVar4 = FUN_0059c359(iVar4 + 0xfdea,4,1,param_2);
  iVar3 = local_12c;
  if (iVar4 != 1) {
    return -1;
  }
  local_130 = local_130 + 4;
  uVar6 = 0xffffffff;
  pcVar9 = (char *)(local_12c + 0xfc5a);
  do {
    pcVar11 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar11 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar11;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  pcVar9 = pcVar11 + -uVar6;
  pcVar11 = local_100;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar11 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar11 = pcVar11 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar11 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar11 = pcVar11 + 1;
  }
  uVar6 = 0xffffffff;
  pcVar9 = &DAT_005da1f8;
  do {
    pcVar11 = pcVar9;
    if (uVar6 == 0) break;
    uVar6 = uVar6 - 1;
    pcVar11 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar11;
  } while (cVar1 != '\0');
  uVar6 = ~uVar6;
  iVar4 = -1;
  pcVar9 = local_100;
  do {
    pcVar10 = pcVar9;
    if (iVar4 == 0) break;
    iVar4 = iVar4 + -1;
    pcVar10 = pcVar9 + 1;
    cVar1 = *pcVar9;
    pcVar9 = pcVar10;
  } while (cVar1 != '\0');
  pcVar9 = pcVar11 + -uVar6;
  pcVar11 = pcVar10 + -1;
  for (uVar7 = uVar6 >> 2; uVar7 != 0; uVar7 = uVar7 - 1) {
    *(undefined4 *)pcVar11 = *(undefined4 *)pcVar9;
    pcVar9 = pcVar9 + 4;
    pcVar11 = pcVar11 + 4;
  }
  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
    *pcVar11 = *pcVar9;
    pcVar9 = pcVar9 + 1;
    pcVar11 = pcVar11 + 1;
  }
  FUN_00487570();
  FUN_00416b80();
  FUN_00416b80();
  *(undefined1 *)(iVar3 + 0x100b0) = 1;
  FUN_004f27a0();
  FUN_00479a10(PTR_s_AI_BRAIN_DAT_005c5d48);
  FUN_00470d40();
  if (*(int *)(iVar3 + 0x100b3) != 0) {
    iVar4 = -1;
    if (DAT_00689999 == 3) {
      iVar4 = 1;
    }
    iVar5 = DAT_00689995;
    if (DAT_00689999 < DAT_00689995) {
      iVar5 = DAT_00689999;
    }
    if (iVar5 < 1) {
      iVar5 = 1;
    }
    if (DAT_00689999 < iVar5) {
      iVar5 = DAT_00689999;
    }
    if (iVar5 < 1) {
      iVar5 = 1;
    }
    FUN_004cb7d0(iVar5 + iVar4,iVar5 + iVar4,0xffffffff);
  }
  FUN_004dc6e0();
  DAT_006903e8 = 0;
  FUN_004a5850(0x10);
  iVar4 = FUN_0059c359(iVar3 + 0x100a0,1,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar4 = FUN_0059c359(iVar3 + 0x100a1,1,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar4 = FUN_0059c359(iVar3 + 0x100a3,1,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar4 = FUN_0059c359(iVar3 + 0x100a5,1,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar4 = FUN_0059c359(iVar3 + 0x100a7,1,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar4 = FUN_0059c359(iVar3 + 0x100ad,1,1,param_2);
  if (iVar4 != 1) {
    return -1;
  }
  iVar4 = FUN_0059c359(iVar3 + 0x100ae,1,1,param_2);
  if (iVar4 == 1) {
    iVar4 = FUN_0059c359(iVar3 + 0x100af,1,1,param_2);
    if (iVar4 != 1) {
      return -1;
    }
    iVar3 = FUN_0059c359(iVar3 + 0x100a8,4,1,param_2);
    if (iVar3 != 1) {
      return -1;
    }
    iVar3 = FUN_0059c359(&DAT_006e202c,4,1,param_2);
    if (iVar3 != 1) {
      return -1;
    }
    uVar12 = param_2;
    FUN_00463cb0(param_2);
    local_134 = FUN_004641b0(uVar12);
    if (local_134 != -1) {
      iVar3 = local_130 + local_134;
      iVar4 = FUN_0059c359(&DAT_006a2890,4,1,param_2);
      if (iVar4 != 1) {
        return -1;
      }
      iVar4 = FUN_0059c359(&DAT_00657cdc,4,1,param_2);
      if (iVar4 != 1) {
        return -1;
      }
      FUN_004da780();
      return iVar3 + 0x18;
    }
    return -1;
  }
  return -1;
}

