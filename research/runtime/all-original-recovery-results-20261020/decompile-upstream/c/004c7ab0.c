
int __fastcall FUN_004c7ab0(undefined1 *param_1)

{
  char *pcVar1;
  char cVar2;
  byte bVar3;
  int iVar4;
  int *piVar5;
  undefined4 uVar6;
  short sVar7;
  int iVar8;
  int iVar9;
  uint uVar10;
  undefined4 *puVar11;
  undefined4 *puVar12;
  int iVar13;
  int *piVar14;
  uint uVar15;
  int iVar16;
  int iVar17;
  char *pcVar18;
  uint uStack_1e0;
  uint uStack_1dc;
  int local_1d8;
  char *pcStack_1d4;
  int iStack_1d0;
  int iStack_1cc;
  int iStack_1c4;
  int iStack_1c0;
  int iStack_1bc;
  int iStack_1b8;
  undefined4 uStack_1b4;
  char *pcStack_1b0;
  undefined1 auStack_1ac [4];
  int local_1a8;
  undefined4 local_1a4;
  undefined4 local_1a0;
  int iStack_19c;
  int local_198;
  undefined4 local_194;
  undefined4 uStack_18c;
  undefined4 uStack_188;
  undefined4 uStack_184;
  undefined2 uStack_180;
  undefined4 auStack_178 [26];
  undefined4 uStack_110;
  undefined1 auStack_10c [256];
  void *pvStack_c;
  undefined1 *puStack_8;
  uint uStack_4;
  
  uStack_4 = 0xffffffff;
  puStack_8 = &LAB_005c1a9c;
  pvStack_c = ExceptionList;
  puVar12 = (undefined4 *)(param_1 + 0x804);
  local_1d8 = 0;
  if ((*(uint *)(param_1 + 0x804) < *(uint *)(param_1 + 0x800)) &&
     (ExceptionList = &pvStack_c, *(int *)(param_1 + 0x848) != 0)) {
    while( true ) {
      iVar8 = FUN_0058b660();
      if (iVar8 != 0) {
        FUN_0057dc20(*(undefined4 *)(*(int *)(param_1 + 0x84c) + 0x14));
        FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                     (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
        if (*(char *)*puVar12 != '\0') {
          FUN_004a6190((char *)*puVar12,param_1 + 0x838,*(undefined4 *)(param_1 + 0x80c),0,0,0,0x10,
                       puVar12);
        }
        (**(code **)(**(int **)(*(int *)(param_1 + 0x84c) + 4) + 0x80))
                  (*(int **)(*(int *)(param_1 + 0x84c) + 4),iVar8);
      }
      if (*(char *)*puVar12 == '\0') {
        *puVar12 = param_1;
        *(undefined1 **)(param_1 + 0x800) = param_1;
        *param_1 = 0;
        *(undefined4 *)(param_1 + 0x818) = *(undefined4 *)(param_1 + 0x81c);
        *(undefined4 *)(param_1 + 0x81c) = 0;
        iVar8 = DAT_0065689c;
        local_1d8 = 1;
        iVar9 = FUN_004a6970();
        *(int *)(param_1 + 0x83c) = iVar8 - iVar9;
        *(undefined4 *)(param_1 + 0x80c) = *(undefined4 *)(param_1 + 0x838);
      }
      if (((*(char *)*puVar12 != '\0') || (*(int *)(param_1 + 0x844) < *(int *)(param_1 + 0x83c)))
         || (uVar10 = FUN_004a6970(),
            (uint)(*(int *)(param_1 + 0x844) - *(int *)(param_1 + 0x83c)) <= uVar10)) {
        piVar14 = *(int **)(param_1 + 0x834);
        iVar8 = piVar14[2];
        iVar9 = piVar14[1];
        if (iVar8 < iVar9) {
          puVar11 = (undefined4 *)(iVar8 * 0x10 + *piVar14);
          *(undefined4 *)(param_1 + 0x838) = *puVar11;
          *(undefined4 *)(param_1 + 0x83c) = puVar11[1];
          *(undefined4 *)(param_1 + 0x840) = puVar11[2];
          *(undefined4 *)(param_1 + 0x844) = puVar11[3];
          piVar14[2] = piVar14[2] + 1;
        }
        uVar10 = (uint)(iVar8 < iVar9);
        *(uint *)(param_1 + 0x848) = uVar10;
        if (uVar10 != 0) {
          *(undefined4 *)(param_1 + 0x80c) = *(undefined4 *)(param_1 + 0x838);
        }
      }
      if (local_1d8 != 0) break;
      if (*(int *)(param_1 + 0x848) == 0) {
        ExceptionList = pvStack_c;
        return 0;
      }
    }
  }
  else {
    local_1d8 = 1;
    ExceptionList = &pvStack_c;
  }
  pcVar1 = param_1 + 0x854;
  if (param_1[0x854] != '\0') {
    uVar10 = 0xffffffff;
    pcVar18 = pcVar1;
    do {
      if (uVar10 == 0) break;
      uVar10 = uVar10 - 1;
      cVar2 = *pcVar18;
      pcVar18 = pcVar18 + 1;
    } while (cVar2 != '\0');
    if (4 < ~uVar10 - 1) {
      local_1a8 = 0;
      local_1a4 = 0;
      local_1a0 = 0;
      local_194 = 0;
      local_198 = 0;
      iStack_19c = 0;
      uStack_18c = 0;
      uStack_188 = 0;
      uStack_184 = 0;
      uStack_180 = 0;
      DAT_006f694c = 0;
      puVar12 = auStack_178;
      for (iVar8 = 0x1b; iVar8 != 0; iVar8 = iVar8 + -1) {
        *puVar12 = 0;
        puVar12 = puVar12 + 1;
      }
      uVar10 = 0xffffffff;
      pcVar18 = pcVar1;
      do {
        if (uVar10 == 0) break;
        uVar10 = uVar10 - 1;
        cVar2 = *pcVar18;
        pcVar18 = pcVar18 + 1;
      } while (cVar2 != '\0');
      uStack_4 = 0;
      auStack_178[0] = 0x6c;
      auStack_178[1] = 7;
      uStack_110 = 0x840;
      iVar8 = __strcmpi(param_1 + ~uVar10 + 0x850,&DAT_005e1fe0);
      if (iVar8 == 0) {
        FUN_00486950();
        uStack_4 = CONCAT31(uStack_4._1_3_,1);
        FUN_0059c1fd(auStack_10c,s__s_ix_i__s_005e1f8c,"Interface\\Grimoire\\",
                     (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                     (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,pcVar1);
        iVar8 = FUN_00486ae0(auStack_10c);
        if (iVar8 == 0) {
          FUN_0059c1fd(auStack_10c,s__sGENERIC__s_005e1fd0,"Interface\\Grimoire\\",pcVar1);
          iVar8 = FUN_00486ae0(auStack_10c);
          if (iVar8 != 0) goto LAB_004c7dd0;
        }
        else {
LAB_004c7dd0:
          auStack_178[3] = *(undefined4 *)(iStack_1d0 + 4);
          auStack_178[2] = *(undefined4 *)(iStack_1d0 + 8);
          FUN_0058ad90(auStack_178,s_c__Projects_Chaos_Grimoire_CPP_005e1efc,0x9c7);
          FUN_0058d240(&iStack_1d0);
          FUN_0058d0e0(0,0,0xff);
        }
        uStack_4 = uStack_4 & 0xffffff00;
        FUN_00486960();
      }
      else {
        uVar10 = 0xffffffff;
        pcVar18 = pcVar1;
        do {
          if (uVar10 == 0) break;
          uVar10 = uVar10 - 1;
          cVar2 = *pcVar18;
          pcVar18 = pcVar18 + 1;
        } while (cVar2 != '\0');
        iVar8 = __strcmpi(param_1 + ~uVar10 + 0x850,(char *)&PTR_BYTE_005e1fcc);
        if (iVar8 == 0) {
          iStack_1c4 = 0;
          iStack_1c0 = 0;
          uStack_1b4 = 0;
          iStack_1b8 = 0;
          iStack_1bc = 0;
          uStack_4 = CONCAT31(uStack_4._1_3_,2);
          FUN_0059c1fd(auStack_10c,s__s_ix_i__s_005e1f8c,"Interface\\Grimoire\\",
                       (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                       (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0,pcVar1);
          iVar8 = FUN_0054ad50(auStack_10c);
          if (iVar8 == 0) {
            FUN_0059c1fd(auStack_10c,s__sGENERIC__s_005e1fd0,"Interface\\Grimoire\\",pcVar1);
            iVar8 = FUN_0054ad50(auStack_10c);
            if (iVar8 != 0) goto LAB_004c7f00;
          }
          else {
LAB_004c7f00:
            auStack_178[2] = uStack_1b4;
            auStack_178[3] = iStack_1b8;
            FUN_0058ad90(auStack_178,s_c__Projects_Chaos_Grimoire_CPP_005e1efc,0x9d7);
            FUN_0054b2d0(auStack_1ac);
            FUN_0058d0e0(0,0,0xff);
          }
          uStack_4._0_1_ = 3;
          FUN_004c88c0();
          iStack_1bc = 0;
          iStack_1b8 = 0;
          uStack_1b4 = 0;
          uStack_4 = (uint)uStack_4._1_3_ << 8;
          if (iStack_1c4 != 0) {
            FUN_005979b0(iStack_1c4);
          }
        }
      }
      if (local_1a8 != 0) {
        puVar12 = (undefined4 *)(param_1 + 0x838);
        if (*(uint *)(param_1 + 0x840) < (uint)(iStack_19c + *(int *)(param_1 + 0x838))) {
LAB_004c7fc2:
          piVar14 = *(int **)(param_1 + 0x834);
          iVar8 = piVar14[2];
          iVar9 = piVar14[1];
          if (iVar8 < iVar9) {
            puVar11 = (undefined4 *)(iVar8 * 0x10 + *piVar14);
            *puVar12 = *puVar11;
            *(undefined4 *)(param_1 + 0x83c) = puVar11[1];
            *(undefined4 *)(param_1 + 0x840) = puVar11[2];
            *(undefined4 *)(param_1 + 0x844) = puVar11[3];
            piVar14[2] = piVar14[2] + 1;
          }
          uVar10 = (uint)(iVar8 < iVar9);
          *(uint *)(param_1 + 0x848) = uVar10;
          if (uVar10 != 0) {
            *(undefined4 *)(param_1 + 0x80c) = *puVar12;
          }
        }
        else if ((*(uint *)(param_1 + 0x844) < (uint)(local_198 + *(int *)(param_1 + 0x83c))) ||
                ((int)*(uint *)(param_1 + 0x844) < *(int *)(param_1 + 0x83c))) goto LAB_004c7fc2;
        if (*(int *)(param_1 + 0x848) != 0) {
          if (DAT_006e1ee4 == 0) {
            FUN_0058c140(*(undefined4 *)(param_1 + 0x84c),*puVar12,*(undefined4 *)(param_1 + 0x83c))
            ;
          }
          else {
            FUN_0058d5b0(1,*(undefined4 *)(param_1 + 0x84c),0,*puVar12,
                         *(undefined4 *)(param_1 + 0x83c),1);
          }
          *pcVar1 = '\0';
          *(int *)(param_1 + 0x83c) = *(int *)(param_1 + 0x83c) + local_198;
        }
      }
      uStack_4 = 0xffffffff;
      FUN_0058b3e0();
    }
  }
  if (local_1d8 == 0) {
    ExceptionList = pvStack_c;
    return 0;
  }
  if ((*(int *)(param_1 + 0x848) != 0) && (*(int *)(param_1 + 0x820) != 0)) {
    piVar14 = *(int **)(param_1 + 0x834);
    iVar8 = piVar14[2];
    iVar9 = piVar14[1];
    if (iVar8 < iVar9) {
      puVar12 = (undefined4 *)(iVar8 * 0x10 + *piVar14);
      *(undefined4 *)(param_1 + 0x838) = *puVar12;
      *(undefined4 *)(param_1 + 0x83c) = puVar12[1];
      *(undefined4 *)(param_1 + 0x840) = puVar12[2];
      *(undefined4 *)(param_1 + 0x844) = puVar12[3];
      piVar14[2] = piVar14[2] + 1;
    }
    uVar10 = (uint)(iVar8 < iVar9);
    *(uint *)(param_1 + 0x848) = uVar10;
    if (uVar10 != 0) {
      *(undefined4 *)(param_1 + 0x80c) = *(undefined4 *)(param_1 + 0x838);
    }
    *(undefined4 *)(param_1 + 0x820) = 0;
  }
  if (*(int *)(param_1 + 0x824) != 0) {
    *(undefined4 *)(param_1 + 0x824) = 0;
    *(undefined4 *)(param_1 + 0x848) = 0;
  }
  iVar8 = *(int *)(param_1 + 0x95c);
  if (iVar8 != 0) {
    iVar9 = *(int *)(iVar8 + 0x30) + *(int *)(param_1 + 0x838);
    iVar16 = *(int *)(iVar8 + 8) + iVar9;
    iStack_1bc = *(int *)(iVar8 + 0x10) + (iVar9 - DAT_006e2058);
    iVar9 = *(int *)(*(int *)(param_1 + 0x95c) + 0x34) + *(int *)(param_1 + 0x83c);
    iVar17 = *(int *)(iVar8 + 0xc) + iVar9;
    iVar9 = *(int *)(iVar8 + 0x14) + iVar9;
    iVar8 = iStack_1bc;
    if (((*(int *)(param_1 + 0x840) < iStack_1bc) || (*(int *)(param_1 + 0x844) < iVar9)) ||
       (*(int *)(param_1 + 0x844) < *(int *)(param_1 + 0x83c))) {
      piVar14 = *(int **)(param_1 + 0x834);
      iVar13 = piVar14[2];
      iVar4 = piVar14[1];
      if (iVar13 < iVar4) {
        puVar12 = (undefined4 *)(iVar13 * 0x10 + *piVar14);
        *(undefined4 *)(param_1 + 0x838) = *puVar12;
        *(undefined4 *)(param_1 + 0x83c) = puVar12[1];
        *(undefined4 *)(param_1 + 0x840) = puVar12[2];
        *(undefined4 *)(param_1 + 0x844) = puVar12[3];
        *(int *)(*(int *)(param_1 + 0x834) + 8) = *(int *)(*(int *)(param_1 + 0x834) + 8) + 1;
      }
      uVar10 = (uint)(iVar13 < iVar4);
      *(uint *)(param_1 + 0x848) = uVar10;
      if (uVar10 != 0) {
        iVar13 = *(int *)(param_1 + 0x95c);
        *(int *)(param_1 + 0x80c) = *(int *)(param_1 + 0x838);
        iVar8 = *(int *)(iVar13 + 0x30) + *(int *)(param_1 + 0x838);
        iVar16 = *(int *)(iVar13 + 8) + iVar8;
        iVar8 = *(int *)(iVar13 + 0x10) + (iVar8 - DAT_006e2058);
        iVar9 = *(int *)(param_1 + 0x83c) + *(int *)(*(int *)(param_1 + 0x95c) + 0x34);
        iVar17 = *(int *)(iVar13 + 0xc) + iVar9;
        iVar9 = *(int *)(iVar13 + 0x14) + iVar9;
      }
    }
    if (*(int *)(param_1 + 0x848) != 0) {
      iVar13 = *(int *)(param_1 + 0x95c);
      uStack_1dc = (*(int *)(iVar13 + 0x34) - iVar17) + iVar9;
      pcStack_1d4 = *(char **)(iVar13 + 0x2c);
      *(int *)(iVar13 + 8) = iVar16;
      *(int *)(iVar13 + 0xc) = iVar17;
      *(int *)(iVar13 + 0x10) = iVar8;
      *(int *)(iVar13 + 0x14) = iVar9;
      iVar8 = FUN_004a6970();
      if ((int)uStack_1dc < iVar8) {
        iVar9 = iVar9 + (iVar8 - uStack_1dc);
        uStack_1dc = iVar8;
      }
      if (((pcStack_1d4 != (char *)0x0) && (*pcStack_1d4 != '\0')) &&
         (iStack_1d0 = FUN_0058b660(), iStack_1d0 != 0)) {
        iStack_1c4 = *(int *)(param_1 + 0x838);
        iStack_1bc = iVar16 + -8;
        iStack_1c0 = iVar17 - *(int *)(*(int *)(param_1 + 0x95c) + 0x34);
        iStack_1b8 = iVar9;
        FUN_004a5a90(&uStack_1e0,2);
        iVar8 = iStack_1d0;
        FUN_0057dc20(*(undefined4 *)(*(int *)(param_1 + 0x84c) + 0x14));
        FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                     (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
        FUN_004a5ce0(pcStack_1d4,&iStack_1c4,0,0,0,uStack_1e0,0);
        (**(code **)(**(int **)(*(int *)(param_1 + 0x84c) + 4) + 0x80))
                  (*(int **)(*(int *)(param_1 + 0x84c) + 4),iVar8);
      }
      FUN_004dd9b0(*(undefined4 *)(param_1 + 0x84c));
      iVar8 = *(int *)(param_1 + 0x95c);
      if (iVar8 != 0) {
        if (*(int *)(iVar8 + 0x2c) != 0) {
          FUN_00597870(*(int *)(iVar8 + 0x2c));
        }
        FUN_004dd9a0();
        FUN_00597870(iVar8);
      }
      *(undefined4 *)(param_1 + 0x95c) = 0;
      *(uint *)(param_1 + 0x83c) = *(int *)(param_1 + 0x83c) + uStack_1dc + 4;
    }
  }
  if (*(int *)(param_1 + 0x828) == 0) {
    ExceptionList = pvStack_c;
    return local_1d8;
  }
  if ((*(int *)(param_1 + 0x954) != 0) &&
     (iVar8 = *(int *)(param_1 + 0x954) + *(int *)(param_1 + 0x82c) * 0x1fc, iVar8 != 0)) {
    iVar9 = *(int *)(param_1 + 0x830);
    iVar16 = *(int *)(iVar8 + 0xc + iVar9 * 4);
    if ((iVar16 == 0) || (iVar17 = iVar9, *(int *)(iVar16 + 4) != iVar9)) {
      iVar17 = 0;
      piVar14 = (int *)(iVar8 + 0xc);
      do {
        if ((*piVar14 != 0) && (*(int *)(*piVar14 + 4) == iVar9)) break;
        iVar17 = iVar17 + 1;
        piVar14 = piVar14 + 1;
      } while (iVar17 < 0x7c);
    }
    if ((iVar17 != 0x7c) && (*(int *)(iVar8 + 0xc + iVar17 * 4) != 0)) {
      if ((*(int *)(param_1 + 0x844) < *(int *)(param_1 + 0x83c)) ||
         (*(int *)(param_1 + 0x844) - *(int *)(param_1 + 0x83c) < 0x29)) {
        piVar14 = *(int **)(param_1 + 0x834);
        iVar8 = piVar14[2];
        iVar9 = piVar14[1];
        if (iVar8 < iVar9) {
          puVar12 = (undefined4 *)(iVar8 * 0x10 + *piVar14);
          *(undefined4 *)(param_1 + 0x838) = *puVar12;
          *(undefined4 *)(param_1 + 0x83c) = puVar12[1];
          *(undefined4 *)(param_1 + 0x840) = puVar12[2];
          *(undefined4 *)(param_1 + 0x844) = puVar12[3];
          piVar14[2] = piVar14[2] + 1;
        }
        uVar10 = (uint)(iVar8 < iVar9);
        *(uint *)(param_1 + 0x848) = uVar10;
        if (uVar10 != 0) {
          *(undefined4 *)(param_1 + 0x80c) = *(undefined4 *)(param_1 + 0x838);
        }
      }
      if (*(int *)(param_1 + 0x848) == 0) {
        ExceptionList = pvStack_c;
        return local_1d8;
      }
      iVar9 = *(int *)(param_1 + 0x82c);
      iVar16 = *(int *)(param_1 + 0x830);
      iStack_1d0 = *(int *)(*(int *)(param_1 + 0x84c) + 0x20);
      iStack_1cc = *(int *)(*(int *)(param_1 + 0x84c) + 0x24);
      iVar8 = *(int *)(param_1 + 0x954) + iVar9 * 0x1fc;
      iVar17 = *(int *)(iVar8 + 0xc + iVar16 * 4);
      if ((iVar17 == 0) || (iVar13 = iVar16, *(int *)(iVar17 + 4) != iVar16)) {
        iVar13 = 0;
        piVar14 = (int *)(iVar8 + 0xc);
        do {
          if ((*piVar14 != 0) && (*(int *)(*piVar14 + 4) == iVar16)) break;
          iVar13 = iVar13 + 1;
          piVar14 = piVar14 + 1;
        } while (iVar13 < 0x7c);
      }
      if (iVar13 == 0x7c) {
        puVar12 = (undefined4 *)0x0;
      }
      else {
        puVar12 = *(undefined4 **)(iVar8 + 0xc + iVar13 * 4);
      }
      pcStack_1b0 = (char *)*puVar12;
      iVar17 = *(int *)(iVar8 + 0xc + iVar16 * 4);
      if ((iVar17 == 0) || (iVar13 = iVar16, *(int *)(iVar17 + 4) != iVar16)) {
        iVar13 = 0;
        piVar14 = (int *)(iVar8 + 0xc);
        do {
          if ((*piVar14 != 0) && (*(int *)(*piVar14 + 4) == iVar16)) break;
          iVar13 = iVar13 + 1;
          piVar14 = piVar14 + 1;
        } while (iVar13 < 0x7c);
      }
      iVar17 = 0;
      if (iVar13 != 0x7c) {
        iVar17 = *(int *)(iVar8 + 0xc + iVar13 * 4);
      }
      uStack_1e0 = *(uint *)(iVar17 + 8);
      iVar8 = FUN_005908e0();
      if (*(int *)(&DAT_0065bb86 + iVar8 * 0x93a) == 0) {
        uStack_1dc = 0;
      }
      else {
        iVar8 = FUN_005908e0();
        uStack_1dc = (uint)*(byte *)(*(int *)(&DAT_0065bb86 + iVar8 * 0x93a) + iVar9 * 0x7c + iVar16
                                    );
      }
      if (DAT_006e1f20 == 1) {
        uStack_1dc = 4;
      }
      iVar9 = *(int *)(param_1 + 0x830);
      pcStack_1d4 = (char *)0x0;
      iVar8 = *(int *)(param_1 + 0x954) + *(int *)(param_1 + 0x82c) * 0x1fc;
      iVar16 = *(int *)(iVar8 + 0xc + iVar9 * 4);
      if ((iVar16 == 0) || (iVar17 = iVar9, *(int *)(iVar16 + 4) != iVar9)) {
        iVar17 = 0;
        piVar14 = (int *)(iVar8 + 0xc);
        do {
          if ((*piVar14 != 0) && (*(int *)(*piVar14 + 4) == iVar9)) break;
          iVar17 = iVar17 + 1;
          piVar14 = piVar14 + 1;
        } while (iVar17 < 0x7c);
      }
      if (iVar17 != 0x7c) {
        pcStack_1d4 = *(char **)(iVar8 + 0xc + iVar17 * 4);
      }
      piVar14 = (int *)((int)pcStack_1d4 + 0xc + uStack_1dc * 4);
      uVar10 = uStack_1dc;
      do {
        if (*piVar14 != 0) {
          piVar14 = (int *)(param_1 + 0x838);
          FUN_004c4f80(*piVar14 + iStack_1d0,*(int *)(param_1 + 0x83c) + iStack_1cc,
                       iVar9 + *(int *)(param_1 + 0x82c) * 0x7c,uStack_1e0,
                       *(undefined4 *)(param_1 + 0x84c));
          iStack_1d0 = FUN_0058b660();
          if (iStack_1d0 != 0) {
            FUN_0057dc20(*(undefined4 *)(*(int *)(param_1 + 0x84c) + 0x14));
            FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,
                         (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0);
            if ((uStack_1e0 == 0xffffffff) ||
               (piVar5 = *(int **)(param_1 + 0x958), piVar5 == (int *)0x0)) {
              FUN_004a5ce0(&DAT_005e1fc8,piVar14,0,0,0,0x10,0);
            }
            else {
              uVar10 = uStack_1e0;
              if (*(uint *)(*piVar5 + 0xc) < uStack_1e0) {
                uVar10 = 0;
              }
              iVar8 = *piVar14;
              uVar6 = *(undefined4 *)(param_1 + 0x83c);
              iVar9 = *(int *)(piVar5[2] + uVar10 * 4) + piVar5[3];
              if (*(int *)(iVar9 + 0x1c) == -1) {
                (*DAT_006903dc)(iVar9,iVar8,uVar6);
              }
              else {
                sVar7 = (*DAT_006c4e3c)(iVar9,iVar8,uVar6,0,0);
                if (sVar7 == 1) {
                  FUN_0057de00(iVar9,iVar8,uVar6,0,0);
                }
              }
            }
            pcVar1 = pcStack_1b0;
            *(int *)(param_1 + 0x80c) = *(int *)(param_1 + 0x80c) + 0x2a;
            if ((pcStack_1b0 != (char *)0x0) && (*pcStack_1b0 != '\0')) {
              iVar8 = FUN_004a6970();
              iVar9 = -1;
              *(int *)(param_1 + 0x83c) = *(int *)(param_1 + 0x83c) + (0x28 - iVar8);
              *(undefined4 *)(param_1 + 0x818) = 1;
              uVar10 = 0;
              pcVar18 = pcVar1;
              goto code_r0x004c87a8;
            }
            iVar8 = FUN_004a6970();
            *(int *)(param_1 + 0x83c) = *(int *)(param_1 + 0x83c) + (0x28 - iVar8);
            goto LAB_004c8837;
          }
          break;
        }
        uVar10 = uVar10 - 1;
        piVar14 = piVar14 + -1;
      } while (-1 < (int)uVar10);
    }
  }
LAB_004c884e:
  *(undefined4 *)(param_1 + 0x828) = 0;
  *(undefined4 *)(param_1 + 0x82c) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x830) = 0xffffffff;
  ExceptionList = pvStack_c;
  return local_1d8;
  while( true ) {
    iVar9 = iVar9 + -1;
    cVar2 = *pcVar18;
    pcVar18 = pcVar18 + 1;
    if (cVar2 == '\0') break;
code_r0x004c87a8:
    if (iVar9 == 0) break;
  }
  if (iVar9 != -2) {
    do {
      bVar3 = pcVar1[uVar10];
      pcStack_1b0 = (char *)CONCAT31(pcStack_1b0._1_3_,bVar3);
      if ((((0x1f < bVar3) && (bVar3 < 0x100)) || (bVar3 == 10)) || (bVar3 == 9)) {
        **(byte **)(param_1 + 0x800) = bVar3;
        iVar8 = *(int *)(param_1 + 0x800);
        *(undefined1 **)(param_1 + 0x800) = (undefined1 *)(iVar8 + 1);
        *(undefined1 *)(iVar8 + 1) = 0;
      }
      uVar15 = 0xffffffff;
      uVar10 = uVar10 + 1;
      pcVar18 = pcVar1;
      do {
        if (uVar15 == 0) break;
        uVar15 = uVar15 - 1;
        cVar2 = *pcVar18;
        pcVar18 = pcVar18 + 1;
      } while (cVar2 != '\0');
    } while (uVar10 < ~uVar15 - 1);
  }
  **(undefined1 **)(param_1 + 0x800) = 10;
  iVar8 = *(int *)(param_1 + 0x800);
  *(undefined1 **)(param_1 + 0x800) = (undefined1 *)(iVar8 + 1);
  *(undefined1 *)(iVar8 + 1) = 0;
LAB_004c8837:
  (**(code **)(**(int **)(*(int *)(param_1 + 0x84c) + 4) + 0x80))
            (*(int **)(*(int *)(param_1 + 0x84c) + 4),iStack_1d0);
  goto LAB_004c884e;
}

