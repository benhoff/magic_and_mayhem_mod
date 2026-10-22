
void __thiscall FUN_00572f30(int param_1,int param_2)

{
  char cVar1;
  bool bVar2;
  int iVar3;
  undefined4 *puVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  int iVar10;
  char *pcVar11;
  undefined4 *puVar12;
  uint uStack_208;
  undefined4 local_204;
  undefined4 local_200;
  int local_1fc;
  int local_1f8;
  undefined4 local_1f4;
  undefined4 local_1f0;
  undefined4 local_1ec;
  undefined4 local_1e8;
  undefined4 local_1e4;
  undefined4 local_1e0;
  undefined4 local_1dc;
  undefined4 local_1d8;
  int local_1d4;
  int local_1d0;
  int local_1cc;
  int local_1c8;
  int local_1c0;
  int local_1b8;
  undefined4 local_1b4;
  undefined4 local_1b0;
  undefined4 local_1ac;
  undefined4 local_1a8;
  undefined4 local_1a4;
  undefined4 local_19c;
  undefined4 local_198;
  undefined4 local_194;
  undefined2 local_190;
  undefined4 local_188;
  undefined4 local_184;
  undefined4 local_180;
  undefined4 local_17c;
  undefined4 local_178 [26];
  undefined4 local_110;
  undefined1 local_10c [256];
  void *pvStack_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  puStack_8 = &LAB_005c2c3b;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *(int *)(param_1 + 0x3e) = param_2;
  local_1b8 = 0;
  local_1b4 = 0;
  local_1b0 = 0;
  local_1a4 = 0;
  local_1a8 = 0;
  local_1ac = 0;
  local_19c = 0;
  local_198 = 0;
  local_194 = 0;
  local_190 = 0;
  DAT_006f694c = 0;
  iVar10 = *(int *)(param_2 + 0xc) + -1;
  local_4 = 0;
  bVar2 = false;
  local_188 = 0;
  local_184 = 0;
  local_180 = 800;
  local_17c = 600;
  if (DAT_006de6d9 != 0) {
    local_17c = 0x1cc;
  }
  local_1fc = param_1;
  local_1f8 = iVar10;
  FUN_004f80f0(*(undefined4 *)(param_2 + 0x31),*(undefined4 *)(param_2 + 0x35),
               *(undefined4 *)(param_2 + 0x39),&local_1cc);
  if (DAT_006de6d5 == 0) {
    if (((0x1f < local_1cc) && (local_1cc < 0x260)) && ((0x1f < local_1c8 && (local_1c8 < 0x134))))
    {
LAB_0057304b:
      *(int *)(param_2 + 0x28) = local_1cc;
      *(int *)(param_2 + 0x2c) = local_1c8;
      goto LAB_00573051;
    }
LAB_00573059:
    *(int *)(param_2 + 0x28) = (*(int *)(param_2 + 0x28) * 100) / 0x50;
    *(int *)(param_2 + 0x2c) = (*(int *)(param_2 + 0x2c) * 100) / 0x50;
  }
  else {
    if ((((0x1f < local_1cc) && (local_1cc < 0x300)) && (0x1f < local_1c8)) && (local_1c8 < 0x1ac))
    goto LAB_0057304b;
LAB_00573051:
    if (DAT_006de6d5 == 0) goto LAB_00573059;
  }
  if (*(char *)(param_2 + 0x30) == '\0') {
    uVar5 = 0xffffffff;
    pcVar11 = *(char **)(param_2 + 0x14);
    do {
      if (uVar5 == 0) break;
      uVar5 = uVar5 - 1;
      cVar1 = *pcVar11;
      pcVar11 = pcVar11 + 1;
    } while (cVar1 != '\0');
    uVar6 = 0xffffffff;
    pcVar11 = *(char **)(param_2 + 0x18);
    do {
      if (uVar6 == 0) break;
      uVar6 = uVar6 - 1;
      cVar1 = *pcVar11;
      pcVar11 = pcVar11 + 1;
    } while (cVar1 != '\0');
    iVar10 = FUN_005736e0(&local_188,(undefined4 *)(param_2 + 0x28),~uVar6 - 1,~uVar5 - 1);
    local_1f8 = iVar10;
    if (iVar10 != -1) goto LAB_005730fc;
    local_1f8 = 2;
    *(undefined4 *)(param_2 + 0x28) = 400;
    iVar10 = 2;
    *(undefined4 *)(param_2 + 0x2c) = 300;
    bVar2 = true;
  }
  else {
LAB_005730fc:
    if (iVar10 < 0) goto LAB_005736a4;
  }
  if (iVar10 < DAT_006f4788) {
    puVar4 = (undefined4 *)(iVar10 * 0x50 + DAT_006f4784);
    local_1f4 = puVar4[4];
    local_1f0 = puVar4[5];
    local_1ec = puVar4[6];
    local_1e8 = puVar4[7];
    local_1e4 = *puVar4;
    local_1e0 = puVar4[1];
    local_1dc = puVar4[2];
    local_1d8 = puVar4[3];
    local_204 = puVar4[8];
    local_200 = puVar4[9];
    if ((bVar2) || (*(char *)(param_2 + 0x30) != '\0')) {
      iVar10 = puVar4[10];
      local_1d0 = puVar4[0xb];
    }
    else {
      local_1c0 = puVar4[0xb];
      iVar10 = puVar4[10];
      if ((puVar4[0x10] != -1) && (puVar4[0x11] != -1)) {
        iVar10 = *(int *)(param_2 + 0x28) - puVar4[0x10];
        local_1c0 = local_1c0 + ((*(int *)(param_2 + 0x2c) - puVar4[0xb]) - puVar4[0x11]);
      }
      local_1d0 = local_1c0;
    }
    local_1d4 = iVar10;
    if (DAT_006de6d5 == 0) {
      local_1f4 = __ftol();
      local_1f0 = __ftol();
      local_1ec = __ftol();
      local_1e8 = __ftol();
      local_1e4 = __ftol();
      local_1e0 = __ftol();
      local_1dc = __ftol();
      local_1d8 = __ftol();
      local_204 = __ftol();
      local_200 = __ftol();
      iVar10 = __ftol();
      local_1d0 = __ftol();
      puVar4 = puVar4 + 0xc;
    }
    else {
      puVar4 = puVar4 + 0xe;
    }
    iVar3 = local_1fc;
    puVar12 = local_178;
    for (iVar7 = 0x1b; iVar7 != 0; iVar7 = iVar7 + -1) {
      *puVar12 = 0;
      puVar12 = puVar12 + 1;
    }
    local_178[0] = 0x6c;
    local_178[1] = 7;
    local_110 = 0x800;
    local_178[3] = *puVar4;
    local_178[2] = puVar4[1];
    iVar7 = local_1fc + 0xc;
    FUN_0058ad90(local_178,s_c__Projects_Chaos_speechctrl_cpp_005f09cc,0x149);
    iVar9 = local_1fc;
    if (*(int *)(local_1fc + 0x10) == 0) {
      FUN_0049e680();
      DestroyWindow(DAT_00656614);
    }
    FUN_0058bc10(0x1f);
    FUN_0058d0e0(0,0,0xff);
    *(int *)(iVar3 + 0x2c) = iVar10;
    *(int *)(iVar3 + 0x30) = local_1d0;
    iVar10 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    iVar8 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    FUN_0059c1fd(local_10c,s_Speechbox__ix_i_Speech_ix_i__i_j_005f098c,iVar8,iVar10,iVar8,iVar10,
                 local_1f8 + 1);
    iVar10 = FUN_0058d320(local_10c);
    if (iVar10 == 0) {
      FUN_0049e4e0();
      DestroyWindow(DAT_00656614);
    }
    if (*(int *)(param_2 + 0x1c) != 0) {
      puVar4 = local_178;
      for (iVar10 = 0x1b; iVar10 != 0; iVar10 = iVar10 + -1) {
        *puVar4 = 0;
        puVar4 = puVar4 + 1;
      }
      local_178[0] = 0x6c;
      local_178[1] = 7;
      local_110 = 0x800;
      if (DAT_006de6d5 == 0) {
        local_178[3] = 0x40;
        local_178[2] = 0x50;
      }
      else {
        local_178[3] = 0x50;
        local_178[2] = 100;
      }
      FUN_0058ad90(local_178,s_c__Projects_Chaos_speechctrl_cpp_005f09cc,0x167);
      if (local_1b8 == 0) {
        FUN_0049e680();
        DestroyWindow(DAT_00656614);
      }
      FUN_0058bc10(0x1f);
      FUN_0058d0e0(0,0,0xff);
      iVar10 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      iVar9 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
      FUN_0059c1fd(local_10c,s_portraits__ix_i__s__ix_i_JPG_005f096c,iVar9,iVar10,
                   *(undefined4 *)(param_2 + 0x1c),iVar9,iVar10);
      iVar10 = FUN_0058d320(local_10c);
      iVar9 = local_1fc;
      if (iVar10 == 0) {
        FUN_0049e4e0();
        DestroyWindow(DAT_00656614);
        iVar9 = local_1fc;
      }
    }
    FUN_0058db80(0,0,0xff,0x14);
    FUN_0058db80(0,0,0xff,0x14);
    if (*(int *)(param_2 + 0x1c) != 0) {
      if (DAT_006de6dd == 0) {
        FUN_0058bf40(iVar7,local_204,local_200);
      }
      else if (DAT_006e1ee4 == 0) {
        FUN_0058c140(iVar7,local_204,local_200);
      }
      else {
        FUN_0058d5b0(1,iVar7,0,local_204,local_200,1);
      }
    }
    iVar10 = FUN_0058b660();
    if (iVar10 != 0) {
      FUN_0057dc20(*(undefined4 *)(iVar9 + 0x20));
      uStack_208 = uStack_208 & 0xffffff00;
      FUN_004a5a90(&uStack_208,8);
      FUN_004a5a90(&uStack_208,4);
      if (*(int *)(param_2 + 0x14) != 0) {
        FUN_0057dc60(local_1dc,local_1d8);
        FUN_004a5ce0(*(undefined4 *)(param_2 + 0x14),&local_1e4,0,0,0,uStack_208,0);
      }
      FUN_004a5a90(&uStack_208,0x10);
      if (*(int *)(param_2 + 0x18) != 0) {
        FUN_0057dc60(local_1ec,local_1e8);
        FUN_004a5ce0(*(undefined4 *)(param_2 + 0x18),&local_1f4,0,0,0,uStack_208,0);
      }
      (**(code **)(**(int **)(iVar3 + 0x10) + 0x80))(*(int **)(iVar3 + 0x10),iVar10);
      if (*(int *)(iVar9 + 0x42) == 0) {
        puVar4 = (undefined4 *)FUN_00597850(0xc);
        if (puVar4 == (undefined4 *)0x0) {
          puVar4 = (undefined4 *)0x0;
        }
        else {
          puVar4[1] = &LAB_005723b0;
          puVar4[2] = iVar9;
          *puVar4 = &PTR_LAB_005c7718;
        }
        *(undefined4 **)(iVar9 + 0x42) = puVar4;
        *(undefined4 **)(iVar9 + 0x34) = puVar4;
      }
    }
  }
LAB_005736a4:
  local_4 = 0xffffffff;
  FUN_0058b3e0();
  ExceptionList = pvStack_c;
  return;
}

