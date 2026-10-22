
/* WARNING: Removing unreachable block (ram,0x0048c1b2) */
/* WARNING: Removing unreachable block (ram,0x0048c1c8) */
/* WARNING: Type propagation algorithm not settling */

void __fastcall FUN_0048b0f0(undefined4 *param_1)

{
  ushort uVar1;
  int iVar2;
  char cVar3;
  uint uVar4;
  int iVar5;
  undefined4 *puVar6;
  int iVar7;
  ushort *puVar8;
  int *piVar9;
  uint uVar10;
  undefined4 uVar11;
  int iVar12;
  int iVar13;
  int iVar14;
  uint uVar15;
  char *extraout_ECX;
  char *extraout_ECX_00;
  int extraout_ECX_01;
  undefined4 extraout_ECX_02;
  int *piVar16;
  uint uVar17;
  bool bVar18;
  undefined4 uVar19;
  int aiStack_33c [55];
  undefined4 uStack_260;
  undefined4 uStack_25c;
  undefined4 uStack_258;
  int iStack_254;
  int iStack_250;
  char *pcStack_24c;
  char *pcStack_248;
  int *piStack_244;
  undefined4 *local_230;
  int local_22c;
  int local_228;
  int local_218 [59];
  undefined4 local_12c;
  undefined4 local_128;
  char local_11c [8];
  char local_114 [8];
  char local_10c [256];
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c0278;
  local_c = ExceptionList;
  piVar9 = local_218;
  for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
    *piVar9 = 0;
    piVar9 = piVar9 + 1;
  }
  pcStack_24c = (char *)param_1[0x13];
  pcStack_248 = local_10c;
  piStack_244 = (int *)0xa;
  local_218[5] = 0xffffffff;
  local_218[4] = 0xffffffff;
  local_218[3] = 0xffffffff;
  local_218[2] = 0xffffffff;
  local_218[1] = 0xffffffff;
  local_218[0] = -1;
  local_218[9] = 0xffffffff;
  local_218[8] = 0xffffffff;
  local_218[7] = 0xffffffff;
  local_218[6] = 0xffffffff;
  local_218[0xc] = 7;
  local_218[10] = 1;
  local_218[0xb] = 0xffffffff;
  iStack_250 = 0x48b173;
  ExceptionList = &local_c;
  FUN_0059de2f();
  iVar12 = param_1[0x13];
  switch(iVar12) {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
  case 8:
  case 9:
  case 10:
  case 0xb:
  case 0xc:
  case 0xd:
  case 0xe:
  case 0xf:
  case 0x10:
  case 0x11:
  case 0x12:
  case 0x13:
  case 0x14:
  case 0x15:
  case 0x16:
  case 0x17:
  case 0x18:
  case 0x19:
  case 0x1a:
    if ((param_1[10] != 7) && (param_1[10] != 6)) goto LAB_0048cf4a;
    pcStack_248 = (char *)0x48b507;
    piStack_244 = (int *)iVar12;
    iVar12 = FUN_0048f490();
    piStack_244 = (int *)0x0;
    if (iVar12 != 0) {
      pcStack_248 = (char *)param_1[0x13];
      pcStack_24c = (char *)0x48b51c;
      iVar12 = FUN_0048f690();
      if (iVar12 != 0) {
        piStack_244 = (int *)0x8;
        pcStack_248 = (char *)0x48b52d;
        FUN_00494ab0();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048cd49;
  case 0x21:
    iVar12 = param_1[10];
    if (((iVar12 != 7) && (iVar12 != 6)) && (iVar12 != 0xb)) {
      *(undefined4 *)((int)param_1 + 0x1d6) = DAT_006b70da;
      if (*(ushort *)(param_1[0x65] + 4) != 0xffff) {
        piStack_244 = (int *)(uint)*(ushort *)(param_1[0x65] + 4);
        pcStack_248 = (char *)0x48d96c;
        uVar11 = FUN_004970b0();
        pcStack_248 = *(char **)((int)param_1 + 0x212);
        piStack_244 = (int *)((int)param_1 + 0x216);
        param_1[0x66] = uVar11;
        pcStack_24c = *(char **)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
        iStack_250 = 0x48d99b;
        FUN_00514860();
      }
      piStack_244 = (int *)0x48d9a2;
      FUN_0048aa50();
      ExceptionList = local_c;
      return;
    }
    piStack_244 = (int *)0x48d9ae;
    iVar12 = FUN_00489f90();
    bVar18 = iVar12 == 0;
    pcStack_248 = (char *)param_1[3];
    pcStack_24c = (char *)param_1[2];
    piStack_244 = (int *)(param_1[4] + -1);
    iStack_250 = 0x48d9cb;
    puVar8 = (ushort *)FUN_00490ea0();
    if (*(short *)param_1[0x65] == 0) {
      if (*puVar8 != 0) {
        piStack_244 = (int *)(uint)*puVar8;
        pcStack_248 = (char *)0x48d9f8;
        iVar12 = FUN_00491250();
        if (*(int *)(iVar12 + 0x94) == 0x10) goto LAB_0048db0b;
      }
      piStack_244 = (int *)param_1[0x65];
      pcStack_248 = (char *)0x48da12;
      FUN_00488330();
      piStack_244 = (int *)(param_1[4] + -1);
      param_1[4] = piStack_244;
      pcStack_24c = (char *)param_1[2];
      pcStack_248 = (char *)param_1[3];
      iStack_250 = 0x48da2e;
      iVar12 = FUN_0048eba0();
      while (iVar12 != 0) {
        piStack_244 = (int *)param_1[4];
        pcStack_248 = (char *)param_1[3];
        pcStack_24c = (char *)param_1[2];
        iStack_250 = 0x48da4c;
        uVar11 = FUN_00490ea0();
        param_1[0x65] = uVar11;
        pcStack_248 = (char *)param_1[3];
        pcStack_24c = (char *)param_1[2];
        piStack_244 = (int *)(param_1[4] + -1);
        iStack_250 = 0x48da69;
        puVar8 = (ushort *)FUN_00490ea0();
        piStack_244 = (int *)(uint)*(ushort *)param_1[0x65];
        pcStack_248 = (char *)0x48da81;
        uVar11 = FUN_00491250();
        param_1[100] = uVar11;
        if ((*(short *)param_1[0x65] == 0) && (*puVar8 != 0)) {
          piStack_244 = (int *)(uint)*puVar8;
          pcStack_248 = (char *)0x48daab;
          iVar12 = FUN_00491250();
          if (*(int *)(iVar12 + 0x94) == 0x10) goto LAB_0048dafd;
        }
        if ((*(short *)param_1[0x65] != 0) && (*(int *)(param_1[100] + 0x94) != 0x10))
        goto LAB_0048dafd;
        piStack_244 = (int *)(param_1[4] + -1);
        param_1[4] = piStack_244;
        pcStack_248 = (char *)param_1[3];
        pcStack_24c = (char *)param_1[2];
        iStack_250 = 0x48dae9;
        iVar12 = FUN_0048eba0();
      }
      bVar18 = true;
      param_1[4] = param_1[4] + 1;
LAB_0048dafd:
      piStack_244 = (int *)param_1[0x65];
      pcStack_248 = (char *)0x48db0b;
      FUN_004882b0();
    }
LAB_0048db0b:
    if (!bVar18) {
      piStack_244 = (int *)0x20;
      pcStack_248 = (char *)0x48db20;
      iVar12 = FUN_004906e0();
      if (iVar12 == 0) {
        piStack_244 = (int *)0x19;
        param_1[0x14] = *(undefined4 *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
        pcStack_248 = (char *)0x48db47;
        FUN_00494ab0();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048e462;
  case 0x22:
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x64;
    pcStack_24c = (char *)0x48e022;
    uVar4 = FUN_0048eb40();
    if (DAT_006e1f30 != 0) {
      piStack_244 = (int *)0xe16;
      pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
      pcStack_24c = (char *)0x48e041;
      FUN_00485b10();
    }
    if (uVar4 < *(uint *)(&DAT_006b1274 + param_1[0x13] * 0x2d1)) {
      piStack_244 = (int *)0x48e064;
      iVar12 = FUN_0049cdb0();
      if (iVar12 != 0) {
        piStack_244 = (int *)0x1;
        if (DAT_006db79a == 0) {
          pcStack_248 = (char *)(*(int *)(&DAT_005dda68 +
                                         *(int *)(&DAT_006b13e1 + param_1[0x13] * 0x2d1) * 0xaf0) <<
                                1);
          pcStack_24c = (char *)0x48e137;
          iVar12 = FUN_0048eb40();
          if (DAT_006e1f30 != 0) {
            piStack_244 = (int *)0xe24;
            pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
            pcStack_24c = (char *)0x48e156;
            FUN_00485b10();
          }
          iVar12 = (iVar12 + *(int *)(&DAT_006b13e1 + param_1[0x13] * 0x2d1) * 700) * 4;
          uVar4 = *(int *)(&DAT_005dda68 + iVar12) + param_1[2];
          uVar17 = *(int *)(&DAT_005dda6c + iVar12) + param_1[3];
        }
        else {
          pcStack_248 = (char *)0x48e09c;
          pcStack_248 = (char *)FUN_0049e250();
          pcStack_24c = (char *)0x48e0a7;
          uVar11 = FUN_0048eb40();
          if (DAT_006e1f30 != 0) {
            piStack_244 = (int *)0xe1d;
            pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
            pcStack_24c = (char *)0x48e0c6;
            FUN_00485b10();
          }
          pcStack_248 = local_11c;
          pcStack_24c = (char *)0x48e0f2;
          piStack_244 = (int *)uVar11;
          piVar9 = (int *)FUN_004840c0();
          uVar4 = param_1[2] + *piVar9;
          uVar17 = param_1[3] + piVar9[1];
        }
        if ((int)uVar4 < 0) {
          piStack_244 = (int *)0x48e1a2;
          iVar12 = FUN_0048eb80();
          uVar4 = uVar4 + iVar12;
        }
        piStack_244 = (int *)0x48e1ae;
        uVar10 = FUN_0048eb80();
        if (uVar10 <= uVar4) {
          piStack_244 = (int *)0x48e1bc;
          iVar12 = FUN_0048eb80();
          uVar4 = uVar4 - iVar12;
        }
        if ((int)uVar17 < 0) {
          piStack_244 = (int *)0x48e1cc;
          iVar12 = FUN_0048eb90();
          uVar17 = uVar17 + iVar12;
        }
        piStack_244 = (int *)0x48e1d8;
        uVar10 = FUN_0048eb90();
        if (uVar10 <= uVar17) {
          piStack_244 = (int *)0x48e1e6;
          iVar12 = FUN_0048eb90();
          uVar17 = uVar17 - iVar12;
        }
        local_218[0] = uVar4 * 0x20 + 0x10;
        local_218[1] = uVar17 * 0x20 + 0x10;
        piStack_244 = (int *)0x48e209;
        iVar12 = FUN_0048eb70();
        local_218[6] = param_1[0x11];
        local_218[2] = iVar12 * 0x10 + -0x10;
        local_218[4] = local_218[1];
        piStack_244 = local_218;
        local_218[3] = local_218[0];
        pcStack_248 = (char *)0x23;
        local_218[5] = 5;
        local_218[0xc] = 1;
        local_218[8] = 0x22;
        local_218[9] = DAT_006b737f;
        pcStack_24c = (char *)0x48e25a;
        FUN_00493f20();
      }
    }
    break;
  case 0x23:
    iVar12 = param_1[10];
    if (((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x48e576;
      iVar12 = FUN_00489f90();
      if (iVar12 == 0) {
        piStack_244 = (int *)0x48e581;
        FUN_0049a4a0();
        piStack_244 = (int *)0x48e588;
        FUN_004943f0();
      }
      piStack_244 = (int *)0x38;
      param_1[0x14] = DAT_006b767c;
      param_1[0x16] = 0xffffffff;
      pcStack_248 = (char *)0x48e59d;
      FUN_00494ab0();
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x2c0;
      uStack_260 = 0x48e5be;
      FUN_00571310();
    }
    else if ((param_1[0x17] == 4) && (*(ushort *)(param_1[0x65] + 2) != 0xffff)) {
      piStack_244 = (int *)(uint)*(ushort *)(param_1[0x65] + 2);
      pcStack_248 = (char *)0x48e4e7;
      FUN_0049e420();
      piStack_244 = (int *)0x48e4f0;
      iVar12 = FUN_0049e400();
      if (iVar12 == 0x28) {
        piStack_244 = (int *)0x48e501;
        iVar12 = FUN_0049e410();
        if (*(int *)(iVar12 + 0x20) == 0x23) {
          piStack_244 = (int *)0x48e50d;
          FUN_004943f0();
        }
      }
      piStack_244 = (int *)0x48e514;
      uVar4 = FUN_0049e3f0();
      piStack_244 = (int *)(uVar4 & 0xffff);
      if (piStack_244 != (int *)0xffff) {
        do {
          pcStack_248 = (char *)0x48e52f;
          FUN_0049e420();
          piStack_244 = (int *)0x48e538;
          iVar12 = FUN_0049e400();
          if (iVar12 == 0x28) {
            piStack_244 = (int *)0x48e544;
            iVar12 = FUN_0049e410();
            if (*(int *)(iVar12 + 0x20) == 0x23) {
              piStack_244 = (int *)0x48e550;
              FUN_004943f0();
            }
          }
          piStack_244 = (int *)0x48e557;
          uVar4 = FUN_0049e3f0();
          piStack_244 = (int *)(uVar4 & 0xffff);
        } while (piStack_244 != (int *)0xffff);
        piStack_244 = (int *)0x48e56a;
        FUN_00492450();
        ExceptionList = local_c;
        return;
      }
    }
    piStack_244 = (int *)0x48e5c5;
    FUN_00492450();
    break;
  case 0x26:
    iVar12 = param_1[10];
    if (((iVar12 != 7) && (iVar12 != 6)) && (iVar12 != 0xb)) {
      local_228 = param_1[2];
      piVar9 = &DAT_005e1784;
      local_230 = &DAT_005e178c;
      local_22c = DAT_006c54dc +
                  ((&DAT_006cb8c2)[param_1[4]] + (&DAT_006cb942)[param_1[3]] + local_228) * 0xc;
      uVar4 = DAT_006c5498;
      uVar17 = DAT_006c5494;
      do {
        if (local_230 == &DAT_005e178c) {
LAB_0048d277:
          uVar1 = *(ushort *)(local_22c + 4);
          if (uVar1 != 0xffff) {
            if (uVar1 < DAT_006def5c) {
              iVar12 = (uint)uVar1 * 0xe4b + (int)DAT_006def58;
            }
            else {
              iVar12 = 0;
            }
            param_1[0x66] = iVar12;
            local_228 = param_1[2];
            uVar4 = DAT_006c5498;
            uVar17 = DAT_006c5494;
            if (((*(int *)(iVar12 + 8) == local_228) && (*(int *)(iVar12 + 0xc) == param_1[3])) &&
               ((*(uint *)(*(int *)(iVar12 + 0xac) + 0xc) >> 1) + *(int *)(iVar12 + 0x10) ==
                param_1[4])) {
              if ((((*(int *)(iVar12 + 0xe4) == 0) ||
                   (iVar5 = **(int **)(iVar12 + 0xac), iVar5 == 0)) ||
                  ((iVar5 == 0x18 || ((iVar5 == 0x19 || (iVar5 == 0x1a)))))) ||
                 ((*(int *)(iVar12 + 0xf8) == 0 && (*(int *)(iVar12 + 0x98) == 0))))
              goto LAB_0048ce8d;
              iVar12 = *(int *)(iVar12 + 0x84);
              iVar5 = param_1[0x18];
              iVar14 = param_1[0x13] * 0x2d1;
              if (((*(int *)((int)&DAT_006b13d5 + iVar14) != 0) ||
                  (*(int *)((int)&DAT_006b13d9 + iVar14) != 0)) ||
                 (*(int *)(&DAT_006b13dd + iVar14) != 0)) {
                piStack_244 = (int *)0x48d39b;
                uVar4 = FUN_0054e200();
                if (DAT_006e1f30 != 0) {
                  piStack_244 = (int *)0xf2a;
                  pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
                  pcStack_24c = (char *)0x48d3c7;
                  FUN_00485b10();
                }
                if ((uint)(iVar5 * 100) / (uint)(iVar5 + iVar12) <= (uVar4 & 0x7fffffff) % 100)
                goto LAB_0048ce8d;
              }
              piStack_244 = (int *)0x48d3ea;
              FUN_00515f30();
              iStack_250 = param_1[4];
              iStack_254 = param_1[3];
              uStack_258 = param_1[2];
              piStack_244 = (int *)0x0;
              pcStack_248 = (char *)0x0;
              pcStack_24c = (char *)0x0;
              uStack_25c = 0x2db;
              uStack_260 = 0x48d40b;
              FUN_00571310();
              piStack_244 = (int *)0x48d412;
              FUN_004943f0();
              ExceptionList = local_c;
              return;
            }
          }
        }
        else {
          uVar10 = piVar9[-1] + local_228;
          if ((int)uVar10 < 0) {
            do {
              uVar10 = uVar10 + uVar17;
            } while ((int)uVar10 < 0);
          }
          else {
            for (; uVar17 <= uVar10; uVar10 = uVar10 - uVar17) {
            }
          }
          uVar15 = param_1[3] + *piVar9;
          if ((int)uVar15 < 0) {
            do {
              uVar15 = uVar15 + uVar4;
            } while ((int)uVar15 < 0);
          }
          else {
            for (; uVar4 <= uVar15; uVar15 = uVar15 - uVar4) {
            }
          }
          iVar12 = piVar9[1] + param_1[4];
          if ((-1 < iVar12) && (iVar12 < (int)DAT_006c549c)) {
            local_22c = DAT_006c54dc +
                        ((&DAT_006cb8c2)[iVar12] + (&DAT_006cb942)[uVar15] + uVar10) * 0xc;
            goto LAB_0048d277;
          }
        }
        piVar9 = piVar9 + 3;
        local_230 = local_230 + 3;
        if (0x5e18c7 < (int)piVar9) {
          piStack_244 = (int *)0x48d306;
          FUN_004943f0();
          ExceptionList = local_c;
          return;
        }
      } while( true );
    }
    piVar9 = param_1 + 0xb;
    piVar16 = local_218;
    for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
      *piVar16 = *piVar9;
      piVar9 = piVar9 + 1;
      piVar16 = piVar16 + 1;
    }
    piStack_244 = (int *)0x48d42f;
    iVar12 = FUN_004a07e0();
    if (iVar12 != 0) {
      iVar12 = param_1[0x13];
      piVar9 = local_218;
      piVar16 = aiStack_33c;
      for (iVar5 = 0x3f; iVar5 != 0; iVar5 = iVar5 + -1) {
        *piVar16 = *piVar9;
        piVar9 = piVar9 + 1;
        piVar16 = piVar16 + 1;
      }
      FUN_0049f5c0(*(undefined4 *)((int)param_1 + 0x1f6),*(undefined4 *)((int)param_1 + 0x1fa),
                   *(undefined4 *)((int)param_1 + 0x1fe),
                   *(undefined4 *)(&DAT_006b13c5 + iVar12 * 0x2d1),
                   *(undefined4 *)(&DAT_006b13c9 + iVar12 * 0x2d1),1,2);
      piStack_244 = (int *)0x48d492;
      FUN_0049f650();
      piStack_244 = (int *)0x48d499;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048ce8d;
  case 0x29:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) goto LAB_0048cbb0;
    iVar12 = param_1[10];
    if (((iVar12 == 7) || (iVar12 == 6)) || ((iVar12 == 0xb || (iVar12 == 0x1e)))) {
      piStack_244 = (int *)0x4d;
      pcStack_248 = (char *)0x48b692;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if ((iVar12 != 0) && (uVar4 = *(uint *)(iVar12 + 0xe4), uVar4 != 0)) {
      if (*(int *)(iVar12 + 0x7c) != 0) {
        ExceptionList = local_c;
        return;
      }
      pcStack_24c = *(char **)((int)param_1 + 0x212);
      pcStack_248 = (char *)((int)param_1 + 0x216);
      piStack_244 = (int *)0x1;
      iVar5 = *(int *)(iVar12 + 0x114);
      iVar12 = *(int *)(iVar12 + 0x8d5);
      iStack_250 = *(int *)(&DAT_006b1274 + param_1[0x13] * 0x2d1);
      iStack_254 = 0x48b5d8;
      FUN_005076f0();
      piStack_244 = (int *)0x48b5e3;
      FUN_00516150();
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      *(undefined4 *)(param_1[0x66] + 0x8d5) = 0;
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      uStack_25c = 0x2dc;
      uStack_260 = 0x48b614;
      FUN_00571310();
      if (DAT_00644520 < 0) {
        ExceptionList = local_c;
        return;
      }
      if (7 < DAT_00644520) {
        ExceptionList = local_c;
        return;
      }
      piStack_244 = (int *)param_1[0x66];
      if (*(int *)((int)piStack_244 + 0x24 + DAT_00644520 * 4) == 0) {
        ExceptionList = local_c;
        return;
      }
      if (((*(uint *)((int)piStack_244 + 0xe4) <= uVar4) && (iVar5 == 0)) && (iVar12 == 0)) {
        ExceptionList = local_c;
        return;
      }
      pcStack_248 = (char *)0x48b658;
      FUN_0049a350();
      ExceptionList = local_c;
      return;
    }
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x3f3;
    uStack_260 = 0x48b67e;
    FUN_00571310();
    uVar4 = param_1[0x11];
LAB_0048bd03:
    if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
LAB_0048cfea:
        iVar12 = 0;
      }
LAB_0048cfec:
      param_1[0x66] = iVar12;
      piStack_244 = (int *)0x48cffd;
      iVar12 = FUN_00411e50();
      if (iVar12 != 0) {
        piStack_244 = (int *)(*(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1) << 8);
        pcStack_248 = (char *)0x48d027;
        FUN_00507f00();
        piStack_244 = (int *)0x48d02e;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048ce8d;
  case 0x2a:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) goto LAB_0048cbb0;
    iVar12 = param_1[10];
    if ((((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x4c;
      pcStack_248 = (char *)0x48c495;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
      piStack_244 = *(int **)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
      pcStack_248 = (char *)0x48c466;
      FUN_00518100();
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x2c1;
      uStack_260 = 0x48c487;
      FUN_00571310();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048cd49;
  case 0x2b:
    iVar12 = param_1[10];
    if (((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) {
      piVar9 = param_1 + 0xb;
      piVar16 = local_218;
      for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
        *piVar16 = *piVar9;
        piVar9 = piVar9 + 1;
        piVar16 = piVar16 + 1;
      }
      piStack_244 = (int *)0x48bca1;
      iVar12 = FUN_004a07e0();
      if (iVar12 != 0) {
        uVar11 = 2;
        piVar9 = local_218;
        piVar16 = aiStack_33c;
        for (iVar5 = 0x3f; iVar12 = DAT_006b8ce0, uVar19 = DAT_006b8ce4, iVar5 != 0;
            iVar5 = iVar5 + -1) {
          *piVar16 = *piVar9;
          piVar9 = piVar9 + 1;
          piVar16 = piVar16 + 1;
        }
        goto LAB_0048dc59;
      }
    }
    else {
      puVar6 = &DAT_005e178c;
      piVar9 = &DAT_005e1784;
      iVar12 = DAT_006c54dc +
               ((&DAT_006cb8c2)[param_1[4]] + (&DAT_006cb942)[param_1[3]] + param_1[2]) * 0xc;
      while (puVar6 != &DAT_005e178c) {
        piStack_244 = (int *)(piVar9[-1] + param_1[2]);
        pcStack_248 = (char *)0x48b93b;
        iVar5 = FUN_0040e650();
        piStack_244 = (int *)(param_1[3] + *piVar9);
        pcStack_248 = (char *)0x48b950;
        iVar14 = FUN_004132e0();
        iVar7 = piVar9[1] + param_1[4];
        if ((-1 < iVar7) && (iVar7 < (int)DAT_006c549c)) {
          iVar12 = DAT_006c54dc + ((&DAT_006cb942)[iVar14] + (&DAT_006cb8c2)[iVar7] + iVar5) * 0xc;
          break;
        }
LAB_0048b9e5:
        piVar9 = piVar9 + 3;
        puVar6 = puVar6 + 3;
        if (0x5e18c7 < (int)piVar9) {
          piStack_244 = (int *)0x48b9fe;
          FUN_004943f0();
          ExceptionList = local_c;
          return;
        }
      }
      uVar1 = *(ushort *)(iVar12 + 4);
      if (uVar1 == 0xffff) goto LAB_0048b9e5;
      if (uVar1 < DAT_006def5c) {
        iVar5 = (uint)uVar1 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar5 = 0;
      }
      param_1[0x66] = iVar5;
      if (((*(int *)(iVar5 + 8) != param_1[2]) || (*(int *)(iVar5 + 0xc) != param_1[3])) ||
         ((*(uint *)(*(int *)(iVar5 + 0xac) + 0xc) >> 1) + *(int *)(iVar5 + 0x10) != param_1[4]))
      goto LAB_0048b9e5;
      if (*(int *)(iVar5 + 0x7c) == 0) {
        iVar12 = *(int *)(iVar5 + 0x84);
        iVar5 = param_1[0x18];
        iVar14 = param_1[0x13] * 0x2d1;
        if (((*(int *)((int)&DAT_006b13d5 + iVar14) != 0) ||
            (*(int *)((int)&DAT_006b13d9 + iVar14) != 0)) || (*(int *)(&DAT_006b13dd + iVar14) != 0)
           ) {
          piStack_244 = (int *)0x48ba9c;
          uVar4 = FUN_0054e200();
          if (DAT_006e1f30 != 0) {
            piStack_244 = (int *)0xf2a;
            pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
            pcStack_24c = (char *)0x48bac8;
            FUN_00485b10();
          }
          if ((uint)(iVar5 * 100) / (uint)(iVar5 + iVar12) <= (uVar4 & 0x7fffffff) % 100)
          goto LAB_0048ce8d;
        }
        if (*(int *)(param_1[0x66] + 0x90) != 0) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x48baf7;
          FUN_005180c0();
          piStack_244 = (int *)0x5;
          pcStack_248 = (char *)0x48bb0a;
          FUN_0054dce0();
        }
        if (*(int *)(param_1[0x66] + 0x8c) != 0) {
          if (*(int *)(param_1[0x66] + 0x8c) < 0x14) {
            piStack_244 = (int *)0x6;
          }
          else {
            piStack_244 = (int *)0x3;
          }
          pcStack_248 = (char *)0x48bb30;
          FUN_0054dba0();
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x48bb3d;
          FUN_00518080();
        }
        if (*(int *)(param_1[0x66] + 0x94) != 0) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x48bb54;
          FUN_00518100();
          piStack_244 = (int *)0x6;
          pcStack_248 = (char *)0x48bb67;
          FUN_0054dce0();
        }
        if (*(int *)(param_1[0x66] + 0x778) != 0) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x48bb7e;
          FUN_0051fb10();
        }
        piStack_244 = (int *)0x48bb89;
        iVar12 = FUN_0051fef0();
        if (iVar12 != 0) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x0;
          pcStack_24c = (char *)0x48bb9c;
          FUN_0051ff00();
        }
        if (*(int *)(param_1[0x66] + 0x77c) != 0) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x48bbb3;
          FUN_0051fb30();
        }
        if (*(int *)(param_1[0x66] + 0x110) != 0) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x48bbca;
          FUN_00520fe0();
        }
        iVar12 = param_1[0x66];
        if (*(int *)(iVar12 + 0x780) != 0) {
          *(undefined4 *)(iVar12 + 0x780) = 0;
          *(undefined4 *)(iVar12 + 0x788) = 0xffffffff;
          uVar4 = 0;
          iVar12 = *(int *)param_1[0x66];
          piVar9 = DAT_006def58;
          if (DAT_006df180 != 0) {
            do {
              if ((piVar9[1] != 0) && (iVar12 != *piVar9)) {
                pcStack_248 = (char *)0x48bc1a;
                piStack_244 = (int *)iVar12;
                FUN_00508b00();
              }
              uVar4 = uVar4 + 1;
              piVar9 = (int *)((int)piVar9 + 0xe4b);
            } while (uVar4 < DAT_006df180);
          }
        }
        iVar12 = param_1[0x66];
        if ((*(int *)(iVar12 + 0x9c) != 0) && (*(int *)(iVar12 + 0xa0) == 1)) {
          *(undefined4 *)(iVar12 + 0x9c) = 1;
        }
        if (*(int *)(param_1[0x66] + 0x798) != 0) {
          piStack_244 = (int *)0x48bc64;
          FUN_00520e70();
        }
        if (*(int *)(param_1[0x66] + 0x78c) != 0) {
          piStack_244 = (int *)0x48bc7d;
          FUN_005206e0();
          piStack_244 = (int *)0x48bc84;
          FUN_004943f0();
          ExceptionList = local_c;
          return;
        }
      }
      else if (*(int *)(iVar5 + 0xe4) != 0) {
        pcStack_248 = (char *)((int)param_1 + 0x216);
        piStack_244 = (int *)0x1;
        pcStack_24c = *(char **)((int)param_1 + 0x212);
        iStack_250 = -*(int *)(iVar5 + 0xe4);
        iStack_254 = 0x48ba30;
        FUN_005076f0();
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x2dd;
        uStack_260 = 0x48ba51;
        FUN_00571310();
        piStack_244 = (int *)0x48ba58;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048ce8d;
  case 0x2c:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) goto LAB_0048cbb0;
    iVar12 = param_1[10];
    if ((((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x4a;
      *(undefined4 *)((int)param_1 + 0x1da) = 0;
      pcStack_248 = (char *)0x48b8c1;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
      if (0 < DAT_005e1490) {
        piStack_244 = (int *)0x54;
        pcStack_248 = (char *)0x48b79c;
        iVar12 = FUN_00597850();
        local_4 = 0;
        if (iVar12 == 0) {
          puVar6 = (undefined4 *)0x0;
        }
        else {
          piStack_244 = (int *)DAT_005e148c;
          pcStack_248 = (char *)DAT_005e1488;
          pcStack_24c = DAT_005e1484;
          iStack_254 = param_1[7];
          iStack_250 = DAT_005e1480;
          uStack_258 = param_1[6];
          uStack_25c = param_1[5];
          uStack_260 = 0x48b7e1;
          puVar6 = (undefined4 *)FUN_005472c0();
        }
        local_4 = 0xffffffff;
        if ((puVar6 != (undefined4 *)0x0) && (param_1[0x12] != -1)) {
          uVar4 = *(uint *)(*(int *)(param_1[0x66] + 0xac) + 8);
          puVar6[0x11] = param_1[0x66];
          puVar6[0x12] = 1;
          puVar6[0x13] = 1;
          puVar6[0x14] = 1;
          puVar6[4] = (int)(((uVar4 & 0x7ffffff) * 0x10 + -4) * DAT_005e1490) / 200;
        }
        pcStack_248 = (char *)0x48b843;
        piStack_244 = puVar6;
        iVar12 = FUN_005457e0();
        if ((iVar12 == 0) && (puVar6 != (undefined4 *)0x0)) {
          piStack_244 = (int *)0x1;
          pcStack_248 = (char *)0x48b852;
          (**(code **)*puVar6)();
        }
      }
      if (*(int *)(param_1[0x66] + 0x110) != 0) {
        piStack_244 = (int *)0x48b869;
        FUN_004943f0();
      }
      piStack_244 = (int *)(DAT_006b8fb5 / *(uint *)(param_1[0x66] + 0x84));
      pcStack_248 = (char *)0x48b882;
      FUN_00520fe0();
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x2de;
      uStack_260 = 0x48b8a3;
      FUN_00571310();
      *(undefined4 *)((int)param_1 + 0x1da) = 1;
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048cd49;
  case 0x2e:
    if (param_1[0x12] != -1) {
      piStack_244 = (int *)0x43;
      pcStack_248 = (char *)0x48c4ac;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048cbb0;
  case 0x32:
    uVar4 = param_1[0x12];
    if (uVar4 != 0xffffffff) {
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar12 = 0;
      }
      param_1[0x66] = iVar12;
      uVar11 = DAT_006ba09b;
      if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
        *(undefined4 *)(iVar12 + 0x788) = 0xffffffff;
        *(undefined4 *)(iVar12 + 0x780) = uVar11;
        uVar4 = param_1[0x11];
        if ((uVar4 != 0xffffffff) &&
           ((uVar4 < DAT_006def5c && (iVar12 = uVar4 * 0xe4b + (int)DAT_006def58, iVar12 != 0)))) {
          piStack_244 = *(int **)(iVar12 + 0x174);
          pcStack_248 = (char *)0x64;
          pcStack_24c = (char *)0x48d122;
          FUN_004656c0();
        }
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x2da;
        uStack_260 = 0x48d143;
        FUN_00571310();
        piStack_244 = (int *)0x48d14a;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
    }
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x3f3;
    uStack_260 = 0x48d05b;
    FUN_00571310();
    uVar4 = param_1[0x11];
    if ((uVar4 == 0xffffffff) || (param_1[0x17] == 4)) goto LAB_0048ce8d;
    if (DAT_006def5c <= uVar4) goto LAB_0048d14f;
    iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    goto LAB_0048d151;
  case 0x34:
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar12 = param_1[0x11] * 0xe4b + (int)DAT_006def58;
      if ((iVar12 != 0) && (*(int *)(iVar12 + 0x8f1) <= *(int *)(iVar12 + 0x8f5))) {
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x3f3;
        uStack_260 = 0x48c6dc;
        FUN_00571310();
        uVar4 = param_1[0x11];
        if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
          if (uVar4 < DAT_006def5c) {
            iVar5 = uVar4 * 0xe4b + (int)DAT_006def58;
          }
          else {
            iVar5 = 0;
          }
          param_1[0x66] = iVar5;
          piStack_244 = (int *)0x48c715;
          iVar5 = FUN_00411e50();
          if (iVar5 != 0) {
            piStack_244 = (int *)(*(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1) << 8);
            pcStack_248 = (char *)0x48c73b;
            FUN_00507f00();
          }
        }
        piStack_244 = (int *)0x48c742;
        FUN_004943f0();
      }
    }
    else {
      iVar12 = 0;
    }
    if ((*(byte *)(param_1[100] + 0xb0) & 4) != 0) {
      *(undefined4 *)((int)param_1 + 0x202) = param_1[2];
      *(undefined4 *)((int)param_1 + 0x206) = param_1[3];
      *(undefined4 *)((int)param_1 + 0x20a) = param_1[4];
      piStack_244 = (int *)0x48c779;
      uVar4 = FUN_0054e200();
      uVar4 = (uVar4 & 0x7fffffff) % 3;
      if (DAT_006e1f30 != 0) {
        piStack_244 = (int *)0xbfd;
        pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
        pcStack_24c = (char *)0x48c7a5;
        FUN_00485b10();
      }
      if (uVar4 == 0) {
        piStack_244 = (int *)0xb;
        pcStack_248 = "j";
        FUN_0048f490();
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0xb;
        pcStack_24c = (char *)0x48c8f0;
        pcStack_248 = (char *)FUN_0048f690();
        if (pcStack_248 == (char *)0x0) {
          iStack_254 = param_1[3];
          uStack_258 = param_1[2];
          iStack_250 = param_1[4];
          uStack_25c = 0x3f3;
          uStack_260 = 0x48c916;
          pcStack_24c = pcStack_248;
          piStack_244 = (int *)pcStack_248;
          FUN_00571310();
          uVar4 = param_1[0x11];
          if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
            if (uVar4 < DAT_006def5c) {
              iVar5 = uVar4 * 0xe4b + (int)DAT_006def58;
            }
            else {
              iVar5 = 0;
            }
            param_1[0x66] = iVar5;
            piStack_244 = (int *)0x48c94c;
            iVar5 = FUN_00411e50();
joined_r0x0048c94e:
            if (iVar5 != 0) {
              iVar5 = *(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1);
LAB_0048c967:
              piStack_244 = (int *)(iVar5 << 8);
              pcStack_248 = (char *)0x48c972;
              FUN_00507f00();
            }
          }
LAB_0048c972:
          piStack_244 = (int *)0x48c979;
          FUN_004943f0();
        }
      }
      else if (uVar4 == 1) {
        piStack_244 = (int *)0xe;
        pcStack_248 = "j";
        FUN_0048f490();
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0xe;
        pcStack_24c = (char *)0x48c86e;
        pcStack_248 = (char *)FUN_0048f690();
        if (pcStack_248 == (char *)0x0) {
          iStack_250 = param_1[4];
          iStack_254 = param_1[3];
          uStack_258 = param_1[2];
          uStack_25c = 0x3f3;
          uStack_260 = 0x48c894;
          pcStack_24c = pcStack_248;
          piStack_244 = (int *)pcStack_248;
          FUN_00571310();
          uVar4 = param_1[0x11];
          if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
            if (uVar4 < DAT_006def5c) {
              iVar5 = uVar4 * 0xe4b + (int)DAT_006def58;
            }
            else {
              iVar5 = 0;
            }
            param_1[0x66] = iVar5;
            piStack_244 = (int *)0x48c8d2;
            iVar5 = FUN_00411e50();
            goto joined_r0x0048c94e;
          }
          goto LAB_0048c972;
        }
      }
      else if (uVar4 == 2) {
        piStack_244 = (int *)0xd;
        pcStack_248 = "j";
        FUN_0048f490();
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0xd;
        pcStack_24c = (char *)0x48c7d2;
        pcStack_248 = (char *)FUN_0048f690();
        if (pcStack_248 == (char *)0x0) {
          iStack_250 = param_1[4];
          uStack_258 = param_1[2];
          iStack_254 = param_1[3];
          uStack_25c = 0x3f3;
          uStack_260 = 0x48c7f8;
          pcStack_24c = pcStack_248;
          piStack_244 = (int *)pcStack_248;
          FUN_00571310();
          uVar4 = param_1[0x11];
          if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
            if (uVar4 < DAT_006def5c) {
              iVar5 = uVar4 * 0xe4b + (int)DAT_006def58;
            }
            else {
              iVar5 = 0;
            }
            param_1[0x66] = iVar5;
            piStack_244 = (int *)0x48c836;
            iVar5 = FUN_00411e50();
            if (iVar5 != 0) {
              iVar5 = *(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1);
              goto LAB_0048c967;
            }
          }
          goto LAB_0048c972;
        }
      }
      if (iVar12 != 0) {
        piStack_244 = *(int **)(iVar12 + 0x174);
        pcStack_248 = (char *)0x48c9a8;
        iVar12 = FUN_00518140();
        if (iVar12 != 0) {
          piStack_244 = (int *)0x18;
          pcStack_248 = (char *)0x48c9b5;
          FUN_00494ab0();
          ExceptionList = local_c;
          return;
        }
      }
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x3f3;
      uStack_260 = 0x48c9db;
      FUN_00571310();
      uVar4 = param_1[0x11];
      if ((uVar4 == 0xffffffff) || (param_1[0x17] == 4)) goto LAB_0048cf4a;
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar12 = 0;
      }
      param_1[0x66] = iVar12;
      piStack_244 = (int *)0x48ca19;
      iVar12 = FUN_00411e50();
joined_r0x0048cf26:
      if (iVar12 != 0) {
        piStack_244 = (int *)(*(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1) << 8);
        pcStack_248 = (char *)0x48cf4a;
        FUN_00507f00();
      }
LAB_0048cf4a:
      piStack_244 = (int *)0x48cf51;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) {
LAB_0048ca2d:
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
    }
    else {
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar12 = 0;
      }
      param_1[0x66] = iVar12;
      if ((iVar12 == 0) || (*(int *)(iVar12 + 0xe4) != 0)) goto LAB_0048ca2d;
      if ((DAT_006def5c <= (uint)param_1[0x11]) ||
         (iVar12 = param_1[0x11] * 0xe4b + (int)DAT_006def58, iVar12 == 0)) {
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x3f3;
        uStack_260 = 0x48cb78;
        FUN_00571310();
        uVar4 = param_1[0x11];
        if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
          if (DAT_006def5c <= uVar4) goto LAB_0048d14f;
          iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
          goto LAB_0048d151;
        }
        goto LAB_0048ce8d;
      }
      if (*(int *)(iVar12 + 0x8f1) <= *(int *)(iVar12 + 0x8f5)) {
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x3f3;
        uStack_260 = 0x48cae3;
        FUN_00571310();
        uVar4 = param_1[0x11];
        if ((uVar4 == 0xffffffff) || (param_1[0x17] == 4)) goto LAB_0048ce8d;
        if (DAT_006def5c <= uVar4) goto LAB_0048cfea;
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
        goto LAB_0048cfec;
      }
      piStack_244 = *(int **)(iVar12 + 0x174);
      pcStack_248 = (char *)0x48cb22;
      iVar12 = FUN_00518140();
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      uStack_258 = param_1[2];
      pcStack_24c = (char *)0x0;
      if (iVar12 != 0) {
        uStack_25c = 0x2e0;
        uStack_260 = 0x48cb4b;
        FUN_00571310();
        piStack_244 = (int *)0x48cb52;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
    }
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x3f3;
    uStack_260 = 0x48ca4e;
    FUN_00571310();
    uVar4 = param_1[0x11];
    if (uVar4 == 0xffffffff) goto LAB_0048ce8d;
    iVar12 = param_1[0x17];
    goto LAB_0048cbe0;
  case 0x36:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) goto LAB_0048cbb0;
    iVar12 = param_1[10];
    if ((((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x50;
      pcStack_248 = (char *)0x48c3ee;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
      bVar18 = true;
      iVar5 = *(int *)(*(int *)(iVar12 + 0xac) + 0x198);
      iVar14 = *(int *)(iVar12 + 0x18) + iVar5;
      iVar5 = *(int *)(iVar12 + 0x14) + iVar5;
      iVar12 = *(int *)(iVar12 + 0x1c);
      uVar4 = param_1[0x11];
      if (uVar4 != 0xffffffff) {
        if (uVar4 < DAT_006def5c) {
          iVar7 = uVar4 * 0xe4b + (int)DAT_006def58;
        }
        else {
          iVar7 = 0;
        }
        piStack_244 = *(int **)(iVar7 + 0x174);
        pcStack_248 = (char *)0x48bf47;
        iVar7 = FUN_00466870();
        if ((iVar7 != 0) && (*(int *)(iVar7 + 0x8f1) <= *(int *)(iVar7 + 0x8f5))) {
          bVar18 = false;
        }
      }
      iVar7 = *(int *)(param_1[0x66] + 0x84);
      iVar2 = param_1[0x13] * 0x2d1;
      iVar13 = *(int *)(&DAT_006b1274 + iVar2);
      if (((*(int *)((int)&DAT_006b13d5 + iVar2) != 0) ||
          (*(int *)((int)&DAT_006b13d9 + iVar2) != 0)) || (*(int *)(&DAT_006b13dd + iVar2) != 0)) {
        piStack_244 = (int *)0x48bfab;
        uVar4 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          piStack_244 = (int *)0xf2a;
          pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
          pcStack_24c = (char *)0x48bfd7;
          FUN_00485b10();
        }
        if ((uint)(iVar13 * 100) / (uint)(iVar13 + iVar7) <= (uVar4 & 0x7fffffff) % 100)
        goto LAB_0048c21d;
      }
      iVar7 = **(int **)(param_1[0x66] + 0xac);
      if (((iVar7 != 0) && (iVar7 != 0x18)) && ((iVar7 != 0x19 && ((iVar7 != 0x1a && (bVar18)))))) {
        uVar4 = param_1[0x11];
        if ((uVar4 != 0xffffffff) &&
           ((uVar4 < DAT_006def5c && (iVar7 = uVar4 * 0xe4b + (int)DAT_006def58, iVar7 != 0)))) {
          piStack_244 = *(int **)(iVar7 + 0x174);
          pcStack_248 = (char *)0x64;
          pcStack_24c = (char *)0x48c083;
          FUN_004656c0();
        }
        piStack_244 = *(int **)param_1[0x66];
        pcStack_248 = (char *)0x48c096;
        FUN_0056c650();
        if ((uint)param_1[0x11] < DAT_006def5c) {
          iVar7 = param_1[0x11] * 0xe4b + (int)DAT_006def58;
        }
        else {
          iVar7 = 0;
        }
        piStack_244 = *(int **)(iVar7 + 0x174);
        pcStack_248 = (char *)0x48c0c5;
        FUN_0051ec80();
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x2e1;
        uStack_260 = 0x48c0e6;
        FUN_00571310();
        if (DAT_00644520 < 0) {
          ExceptionList = local_c;
          return;
        }
        if (7 < DAT_00644520) {
          ExceptionList = local_c;
          return;
        }
        if (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0) {
          ExceptionList = local_c;
          return;
        }
        iVar7 = *(int *)(param_1[0x66] + 0xac);
        iVar13 = *(int *)(iVar7 + 0xc);
        local_22c = ((iVar13 + 1) * *(int *)(iVar7 + 8) + 1) * 6;
        if (local_22c < 1) {
          ExceptionList = local_c;
          return;
        }
        do {
          piStack_244 = (int *)0x24;
          pcStack_248 = (char *)0x48c141;
          iVar7 = FUN_00597850();
          local_4 = 2;
          if (iVar7 == 0) {
            puVar6 = (undefined4 *)0x0;
          }
          else {
            piStack_244 = (int *)0x48c167;
            uVar4 = FUN_0054e200();
            piStack_244 = (int *)0x48c179;
            uVar17 = FUN_0054e200();
            piStack_244 = (int *)0x48c18b;
            uVar10 = FUN_0054e200();
            pcStack_248 = (char *)param_1[0x66];
            piStack_244 = (int *)0x19;
            pcStack_24c = (char *)((int)((longlong)(ulonglong)(uVar4 & 0x7fffffff) %
                                        (longlong)(iVar13 << 4)) + iVar12);
            iStack_250 = ((uVar17 & 0x1f) - 0x10) + iVar14;
            iStack_254 = ((uVar10 & 0x1f) - 0x10) + iVar5;
            uStack_258 = 0x48c1dd;
            puVar6 = (undefined4 *)FUN_00549a00();
          }
          local_4 = 0xffffffff;
          if (puVar6 != (undefined4 *)0x0) {
            pcStack_248 = (char *)0x48c1fd;
            piStack_244 = puVar6;
            iVar7 = FUN_005457e0();
            if (iVar7 == 0) {
              piStack_244 = (int *)0x1;
              pcStack_248 = (char *)0x48c209;
              (**(code **)*puVar6)();
            }
          }
          local_22c = local_22c + -1;
        } while (local_22c != 0);
        ExceptionList = local_c;
        return;
      }
LAB_0048c21d:
      iVar7 = param_1[0x66];
      iVar13 = **(int **)(iVar7 + 0xac);
      if ((((iVar13 != 0) && (iVar13 != 0x18)) && (iVar13 != 0x19)) && (iVar13 != 0x1a)) {
        uVar4 = *(uint *)(iVar7 + 0x84);
        if (uVar4 == (*(int *)(&DAT_006b13e9 + param_1[0x13] * 0x2d1) * uVar4) / 100) {
          iVar13 = 0;
        }
        else {
          iVar13 = uVar4 - (*(int *)(&DAT_006b13e9 + param_1[0x13] * 0x2d1) * uVar4) / 100;
        }
        *(int *)(iVar7 + 0x84) = iVar13;
      }
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x3f3;
      uStack_260 = 0x48c2a6;
      FUN_00571310();
      if (DAT_00644520 < 0) {
        ExceptionList = local_c;
        return;
      }
      if (7 < DAT_00644520) {
        ExceptionList = local_c;
        return;
      }
      if (*(int *)(param_1[0x66] + 0x24 + DAT_00644520 * 4) == 0) {
        ExceptionList = local_c;
        return;
      }
      local_22c = 0;
      iVar7 = *(int *)(*(int *)(param_1[0x66] + 0xac) + 0x19c);
      if (DAT_005e14c4 < 1) {
        ExceptionList = local_c;
        return;
      }
      do {
        piStack_244 = (int *)0x48;
        pcStack_248 = (char *)0x48c300;
        iVar13 = FUN_00597850();
        local_4 = 3;
        if (iVar13 == 0) {
          puVar6 = (undefined4 *)0x0;
        }
        else {
          piStack_244 = (int *)0x48c322;
          uVar4 = FUN_0054e200();
          piStack_244 = (int *)0x48c334;
          uVar17 = FUN_0054e200();
          piStack_244 = (int *)0x48c346;
          uVar10 = FUN_0054e200();
          piStack_244 = (int *)(((uVar4 & 0x7fffffff) % 5 - 2) + iVar12 + iVar7);
          pcStack_248 = (char *)(((uVar17 & 0x7fffffff) % 5 - 2) + iVar14);
          pcStack_24c = (char *)(((uVar10 & 0x7fffffff) % 5 - 2) + iVar5);
          iStack_250 = 0x48c382;
          puVar6 = (undefined4 *)FUN_00545e90();
        }
        local_4 = 0xffffffff;
        if (puVar6 != (undefined4 *)0x0) {
          piStack_244 = (int *)DAT_005e14d0;
          pcStack_248 = (char *)DAT_005e14cc;
          pcStack_24c = (char *)DAT_005e14c8;
          iStack_250 = 0x48c3b2;
          FUN_00546030();
          pcStack_248 = (char *)0x48c3bd;
          piStack_244 = puVar6;
          iVar13 = FUN_005457e0();
          if (iVar13 == 0) {
            piStack_244 = (int *)0x1;
            pcStack_248 = (char *)0x48c3c9;
            (**(code **)*puVar6)();
          }
        }
        local_22c = local_22c + 1;
      } while (local_22c < DAT_005e14c4);
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048cd49;
  case 0x3a:
    if ((param_1[10] != 7) && (param_1[10] != 6)) {
      pcStack_248 = (char *)param_1[3];
      piStack_244 = (int *)(param_1[4] + 1);
      *(undefined4 *)((int)param_1 + 0x1da) = DAT_006bb71f;
      pcStack_24c = (char *)param_1[2];
      iStack_250 = 0x48db83;
      iVar12 = FUN_0048eba0();
      if (iVar12 == 0) {
        ExceptionList = local_c;
        return;
      }
      pcStack_248 = (char *)param_1[3];
      pcStack_24c = (char *)param_1[2];
      piStack_244 = (int *)(param_1[4] + 1);
      iStack_250 = 0x48dba2;
      iVar12 = FUN_00490ea0();
      if (*(ushort *)(iVar12 + 4) == 0xffff) {
        ExceptionList = local_c;
        return;
      }
      piStack_244 = (int *)(uint)*(ushort *)(iVar12 + 4);
      pcStack_248 = (char *)0x48dbc0;
      uVar11 = FUN_004970b0();
      param_1[0x66] = uVar11;
      piStack_244 = (int *)0x48dbcd;
      iVar12 = FUN_004938c0();
      if (iVar12 != 0) {
        ExceptionList = local_c;
        return;
      }
      pcStack_248 = *(char **)((int)param_1 + 0x212);
      piStack_244 = (int *)((int)param_1 + 0x216);
      pcStack_24c = (char *)0x48dbee;
      iVar12 = FUN_00440030();
      pcStack_24c = *(char **)(&DAT_006a652d + iVar12 * 0x5c9);
      iStack_250 = 0x48dc0a;
      FUN_00514860();
      ExceptionList = local_c;
      return;
    }
    piVar9 = param_1 + 0xb;
    piVar16 = local_218;
    for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
      *piVar16 = *piVar9;
      piVar9 = piVar9 + 1;
      piVar16 = piVar16 + 1;
    }
    piStack_244 = (int *)0x48dc27;
    iVar12 = FUN_004a07e0();
    if (iVar12 != 0) {
      uVar11 = 1;
      piVar9 = local_218;
      piVar16 = aiStack_33c;
      for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
        *piVar16 = *piVar9;
        piVar9 = piVar9 + 1;
        piVar16 = piVar16 + 1;
      }
      iVar12 = (DAT_006bb73b * 5 + 0xf) * 4;
      uVar19 = 0x14;
LAB_0048dc59:
      FUN_0049f5c0(*(undefined4 *)((int)param_1 + 0x1f6),*(undefined4 *)((int)param_1 + 0x1fa),
                   *(undefined4 *)((int)param_1 + 0x1fe),iVar12,uVar19,1,uVar11);
      piStack_244 = (int *)0x48dc7d;
      FUN_0049f650();
      piStack_244 = (int *)0x48dc84;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048ce8d;
  case 0x3c:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) goto LAB_0048cbb0;
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if (((iVar12 == 0) || (*(int *)(iVar12 + 0xe4) == 0)) || (*(int *)(iVar12 + 0x8cd) == -1))
    goto LAB_0048cd49;
    if ((uint)param_1[0x11] < DAT_006def5c) {
      iVar5 = param_1[0x11] * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar5 = 0;
    }
    if (*(int *)(iVar12 + 0x174) != *(int *)(iVar5 + 0x174)) goto LAB_0048bcdd;
    iVar12 = *(int *)(iVar12 + 0x7c);
    piStack_244 = (int *)0x48c533;
    FUN_0051e820();
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x2d8;
    uStack_260 = 0x48c554;
    FUN_00571310();
    iVar5 = param_1[0x66];
    if (*(int *)(iVar5 + 0x174) == -1) {
      puVar6 = (undefined4 *)0x0;
    }
    else {
      puVar6 = &DAT_00643a80 + *(int *)(iVar5 + 0x174) * 0x55;
    }
    if (puVar6[3] != -1) {
      iVar14 = 0x56;
      if (iVar12 == 0) {
        iVar14 = *(int *)(iVar5 + 0xa8);
      }
      iVar12 = *(int *)(iVar5 + 0xe4);
      piStack_244 = (int *)0x48c5a3;
      uVar4 = FUN_005075f0();
      piStack_244 = (int *)((uint)(*(int *)((int)&DAT_006b1270 + iVar14 * 0x2d1) * iVar12 * 0x100) /
                           uVar4);
      pcStack_248 = *(char **)(param_1[0x66] + 0x174);
      pcStack_24c = (char *)0x48c5d7;
      FUN_00466870();
      pcStack_248 = (char *)0x48c5de;
      FUN_00507f00();
      piStack_244 = (int *)0x48c5e5;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048ce8d;
  case 0x3e:
  case 0x59:
    piVar9 = param_1 + 0xb;
    piVar16 = local_218;
    for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
      *piVar16 = *piVar9;
      piVar9 = piVar9 + 1;
      piVar16 = piVar16 + 1;
    }
    piStack_244 = (int *)0x48b31e;
    iVar12 = FUN_004a07e0();
    if (iVar12 != 0) {
      iVar12 = param_1[0x13];
      piVar9 = local_218;
      piVar16 = aiStack_33c;
      for (iVar5 = 0x3f; iVar5 != 0; iVar5 = iVar5 + -1) {
        *piVar16 = *piVar9;
        piVar9 = piVar9 + 1;
        piVar16 = piVar16 + 1;
      }
      FUN_0049f5c0(*(undefined4 *)((int)param_1 + 0x1f6),*(undefined4 *)((int)param_1 + 0x1fa),
                   *(undefined4 *)((int)param_1 + 0x1fe),
                   *(undefined4 *)(&DAT_006b13c5 + iVar12 * 0x2d1),
                   *(undefined4 *)(&DAT_006b13c9 + iVar12 * 0x2d1),1,0);
      piStack_244 = (int *)0x48b381;
      FUN_0049f650();
      piStack_244 = (int *)0x48b388;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048ce8d;
  case 0x41:
  case 0x45:
  case 0x46:
  case 0x5b:
  case 0x5c:
    iVar5 = param_1[10];
    if (((iVar5 == 7) || (iVar5 == 6)) || (iVar5 == 0xb)) {
      piStack_244 = (int *)0x48d8d0;
      iVar12 = FUN_00489f90();
      if (iVar12 != 0) {
        piStack_244 = (int *)0x48d8df;
        cVar3 = FUN_004915d0();
        if (cVar3 != '\0') {
          piStack_244 = (int *)0x48d8ee;
          iVar12 = FUN_004906f0();
          if (iVar12 != 0) {
            param_1[6] = param_1[3] * 0x20 + 0x10;
            piStack_244 = (int *)0x20;
            param_1[5] = param_1[2] * 0x20 + 0x10;
            param_1[7] = param_1[4] << 4;
            param_1[0x18] = 0;
            pcStack_248 = (char *)0x48d92a;
            FUN_00494ab0();
            ExceptionList = local_c;
            return;
          }
        }
      }
      goto LAB_0048e462;
    }
    if (iVar5 == 0x20) {
      piStack_244 = (int *)0x48d879;
      iVar12 = FUN_0048fdd0();
      if (iVar12 != 0) {
        ExceptionList = local_c;
        return;
      }
      piStack_244 = (int *)0x48d888;
      FUN_0049a4a0();
      piStack_244 = (int *)0x48d88f;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    if (iVar5 == 0x21) {
      if (iVar12 == 0x46) {
        piStack_244 = (int *)0x48d8a9;
        FUN_00490ed0();
        ExceptionList = local_c;
        return;
      }
      if (iVar12 == 0x45) {
        piStack_244 = (int *)0x48d8ba;
        FUN_004912d0();
        ExceptionList = local_c;
        return;
      }
      piStack_244 = (int *)0x48d8c4;
      FUN_004908a0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048e469;
  case 0x44:
    if (param_1[10] == 6) {
      local_218[0] = param_1[0xb];
      local_218[3] = param_1[0xe];
      local_218[4] = param_1[0xf];
      local_218[1] = param_1[0xc];
      local_218[6] = param_1[0x11];
      local_218[2] = param_1[0xd];
      local_218[10] = param_1[0x15];
      local_218[5] = param_1[0x10];
      local_218[0xc] = 2;
      local_218[8] = 0x44;
      piStack_244 = (int *)0x48e9de;
      iVar12 = FUN_0049cdb0();
      if (iVar12 != 0) {
        local_218[0xd] = *param_1;
        piStack_244 = local_218;
        pcStack_248 = (char *)0x54;
        local_12c = 1;
        pcStack_24c = (char *)0x48e9fe;
        FUN_00493f20();
      }
      piStack_244 = (int *)0x4b;
      pcStack_248 = (char *)0x48ea07;
      FUN_00494ab0();
    }
    else {
      piStack_244 = (int *)0x48ea10;
      FUN_004930c0();
    }
    break;
  case 0x47:
    piStack_244 = (int *)DAT_006bdbbc;
    pcStack_248 = (char *)0x48b19e;
    FUN_0048ecf0();
    if ((param_1[0x11] != -1) && (param_1[0x12] != -1)) {
      iVar12 = param_1[0x66];
      *(int *)(iVar12 + 0x80d) = param_1[0x11] + 1;
      *(undefined4 *)(iVar12 + 0x811) = 100;
    }
    piStack_244 = (int *)0xd;
    pcStack_248 = (char *)0x48b1ca;
    FUN_00494ab0();
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x3f5;
    uStack_260 = 0x48b1eb;
    FUN_00571310();
    break;
  case 0x48:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 == (int *)0xffffffff) {
      piStack_244 = (int *)DAT_006bde8d;
      pcStack_248 = (char *)0x48dd00;
      FUN_0048ecf0();
    }
    else {
      pcStack_248 = (char *)0x48dc9b;
      uVar11 = FUN_004970b0();
      param_1[0x66] = uVar11;
      piStack_244 = (int *)0x48dca8;
      iVar12 = FUN_004937b0();
      if (iVar12 == 0) {
        pcStack_248 = *(char **)((int)param_1 + 0x212);
        piStack_244 = (int *)((int)param_1 + 0x216);
        pcStack_24c = *(char **)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
        iStack_250 = 0x48dcd9;
        FUN_00514860();
      }
      piStack_244 = (int *)param_1[0x11];
      if ((piStack_244 != (int *)0xffffffff) && (param_1[0x12] != -1)) {
        pcStack_248 = (char *)0x48dcf1;
        FUN_00493790();
      }
    }
    piStack_244 = (int *)0x1;
    pcStack_248 = (char *)0x64;
    pcStack_24c = (char *)0x48dd0d;
    uVar4 = FUN_0048eb40();
    if (DAT_006e1f30 != 0) {
      piStack_244 = (int *)0xdc7;
      pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
      pcStack_24c = (char *)0x48dd2c;
      FUN_00485b10();
    }
    piStack_244 = (int *)0x10;
    pcStack_248 = (char *)0x48dd39;
    iVar12 = FUN_004906e0();
    if (iVar12 == 0) {
      bVar18 = uVar4 < *(uint *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
LAB_0048dd54:
      if (!bVar18) {
        piStack_244 = (int *)0xe;
        param_1[0x13] = 0x3a;
        pcStack_248 = (char *)0x48dd6a;
        FUN_00494ab0();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048e469;
  case 0x4a:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 == (int *)0xffffffff) {
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x3f3;
      uStack_260 = 0x48d78f;
      FUN_00571310();
      piStack_244 = (int *)param_1[0x11];
      if ((piStack_244 == (int *)0xffffffff) || (param_1[0x17] == 4)) goto LAB_0048cf4a;
      pcStack_248 = (char *)0x48d7af;
      uVar11 = FUN_004970b0();
      param_1[0x66] = uVar11;
      piStack_244 = (int *)0x48d7bc;
      iVar12 = FUN_00492860();
      goto joined_r0x0048cf26;
    }
    iVar12 = param_1[10];
    if ((((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x42;
      pcStack_248 = (char *)0x48d856;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    pcStack_248 = (char *)0x48d7eb;
    iVar12 = FUN_004970b0();
    param_1[0x66] = iVar12;
    if (iVar12 != 0) {
      piStack_244 = (int *)0x48d800;
      iVar12 = FUN_00412aa0();
      if (iVar12 != 0) {
        piStack_244 = *(int **)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
        pcStack_248 = (char *)0x48d827;
        FUN_005180c0();
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x2ca;
        uStack_260 = 0x48d848;
        FUN_00571310();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048e462;
  case 0x4b:
    if ((param_1[10] == 7) || (param_1[10] == 6)) {
      param_1[0x16] = 0;
      param_1[0x14] = 0;
      *(undefined4 *)((int)param_1 + 0x1da) = 0;
      *(undefined4 *)((int)param_1 + 0x1d6) = 0x24;
      piStack_244 = (int *)0x48e942;
      FUN_00489f90();
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x2;
      *(undefined4 *)((int)param_1 + 0x1ea) = 0;
      pcStack_24c = (char *)0x48e955;
      uVar11 = FUN_0048eb40();
      *(undefined4 *)((int)param_1 + 0x1ee) = uVar11;
      if (DAT_006e1f30 != 0) {
        piStack_244 = (int *)0xef6;
        pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
        pcStack_24c = (char *)0x48e978;
        FUN_00485b10();
      }
      piStack_244 = (int *)0x9;
      pcStack_248 = (char *)0x48e981;
      FUN_00494ab0();
    }
    else if ((int)param_1[0x16] < *(int *)((int)param_1 + 0x1d6)) {
      piStack_244 = (int *)0x48e914;
      FUN_00492b40();
    }
    else {
      piStack_244 = (int *)0x48e91e;
      FUN_00492de0();
    }
    break;
  case 0x4c:
    if ((param_1[10] == 7) || (param_1[10] == 6)) {
      piStack_244 = (int *)0x48e8d7;
      FUN_0048a0c0();
      piStack_244 = (int *)0x48e8de;
      FUN_00493530();
      piStack_244 = (int *)0x48e8e5;
      FUN_004937c0();
      piStack_244 = (int *)0x12;
      pcStack_248 = (char *)0x48e8ee;
      FUN_00494ab0();
    }
    else {
      piStack_244 = (int *)0x48e61e;
      FUN_004970d0();
      piStack_244 = (int *)0x48e628;
      iVar12 = FUN_0052a670();
      while (iVar12 != 0) {
        piStack_244 = (int *)0x48e639;
        iVar5 = FUN_00412aa0();
        if (iVar5 != 0) {
          piStack_244 = (int *)0x48e648;
          iVar5 = FUN_00493780();
          if (iVar5 == 0) {
            piStack_244 = (int *)0x48e657;
            iVar5 = FUN_00491270();
            if (iVar5 != 0) {
              piStack_244 = (int *)0x48e666;
              iVar5 = FUN_004938b0();
              if (iVar5 == 0) {
                piStack_244 = (int *)0x48e675;
                piStack_244 = (int *)FUN_0044d110();
                pcStack_248 = *(char **)((int)param_1 + 0x212);
                pcStack_24c = (char *)0x48e687;
                FUN_00466a80();
                pcStack_248 = (char *)0x48e68e;
                iVar5 = FUN_0049a590();
                if (iVar5 != 0) {
                  piStack_244 = (int *)0x48e69d;
                  iVar5 = FUN_00440030();
                  piStack_244 = *(int **)(&DAT_006a6545 + iVar5 * 0x5c9);
                  pcStack_248 = *(char **)(&DAT_006b1274 + param_1[0x13] * 0x2d1);
                  pcStack_24c = (char *)0x48e6c9;
                  iVar5 = FUN_0048ec30();
                  if (iVar5 != 0) {
                    pcStack_248 = *(char **)(iVar12 + 0x10);
                    piStack_244 = (int *)(param_1[0x13] * 0x2d0);
                    pcStack_24c = (char *)0x48e6f4;
                    FUN_0040eeb0();
                    pcStack_24c = *(char **)(iVar12 + 0xc);
                    iStack_250 = 0x48e704;
                    pcStack_248 = extraout_ECX;
                    FUN_0040e8a0();
                    iStack_250 = *(undefined4 *)(iVar12 + 8);
                    iStack_254 = 0x48e714;
                    pcStack_24c = extraout_ECX_00;
                    FUN_0040e290();
                    iStack_254 = param_1[4];
                    uStack_258 = 0x48e724;
                    iStack_250 = extraout_ECX_01;
                    FUN_0040eeb0();
                    uStack_258 = param_1[3];
                    uStack_25c = 0x48e734;
                    iStack_254 = extraout_ECX_02;
                    FUN_0040e8a0();
                    uStack_25c = param_1[2];
                    uStack_260 = 0x48e744;
                    FUN_0040e290();
                    uStack_25c = 0x48e74d;
                    iVar5 = FUN_004e1380();
                    if (iVar5 != 0) {
                      uVar17 = *(int *)(iVar12 + 8) - param_1[2];
                      uVar4 = *(int *)(iVar12 + 0xc) - param_1[3];
                      piStack_244 = (int *)0x48e76f;
                      iVar5 = FUN_0048eb80();
                      if (iVar5 / 2 < (int)uVar17) {
                        piStack_244 = (int *)0x48e782;
                        iVar5 = FUN_0048eb80();
                        uVar17 = uVar17 - iVar5;
                      }
                      piStack_244 = (int *)0x48e78e;
                      iVar5 = FUN_0048eb80();
                      if ((int)uVar17 < -(iVar5 / 2)) {
                        piStack_244 = (int *)0x48e7a3;
                        iVar5 = FUN_0048eb80();
                        uVar17 = uVar17 + iVar5;
                      }
                      piStack_244 = (int *)0x48e7af;
                      iVar5 = FUN_0048eb90();
                      if (iVar5 / 2 < (int)uVar4) {
                        piStack_244 = (int *)0x48e7c2;
                        iVar5 = FUN_0048eb90();
                        uVar4 = uVar4 - iVar5;
                      }
                      piStack_244 = (int *)0x48e7ce;
                      iVar5 = FUN_0048eb90();
                      if ((int)uVar4 < -(iVar5 / 2)) {
                        piStack_244 = (int *)0x48e7e3;
                        iVar5 = FUN_0048eb90();
                        uVar4 = uVar4 + iVar5;
                      }
                      piStack_244 = (int *)0x48e7ee;
                      FUN_0048ebe0();
                      piStack_244 = (int *)0x48e7f7;
                      FUN_0048ec20();
                      if ((uVar17 & 0x80000000) == 0) {
                        if (uVar17 == 0) {
                          piStack_244 = *(int **)(iVar12 + 8);
                        }
                        else {
                          piStack_244 = (int *)(*(int *)(&DAT_006b13e9 + param_1[0x13] * 0x2d1) +
                                               *(int *)(iVar12 + 8));
                        }
                      }
                      else {
                        piStack_244 = (int *)(*(int *)(iVar12 + 8) -
                                             *(int *)(&DAT_006b13e9 + param_1[0x13] * 0x2d1));
                      }
                      pcStack_248 = (char *)0x48e844;
                      FUN_0048ebf0();
                      if ((uVar4 & 0x80000000) == 0) {
                        if (uVar4 == 0) {
                          piStack_244 = *(int **)(iVar12 + 0xc);
                        }
                        else {
                          piStack_244 = (int *)(*(int *)(&DAT_006b13e9 + param_1[0x13] * 0x2d1) +
                                               *(int *)(iVar12 + 0xc));
                        }
                      }
                      else {
                        piStack_244 = (int *)(*(int *)(iVar12 + 0xc) -
                                             *(int *)(&DAT_006b13e9 + param_1[0x13] * 0x2d1));
                      }
                      pcStack_248 = (char *)0x48e891;
                      FUN_0048ecc0();
                      piStack_244 = *(int **)(iVar12 + 0x10);
                      pcStack_248 = (char *)0x48e89e;
                      pcStack_248 = (char *)FUN_00414870();
                      pcStack_24c = (char *)0x48e8a8;
                      pcStack_24c = (char *)FUN_00414860();
                      iStack_250 = DAT_006be9d5;
                      iStack_254 = 0x48e8b7;
                      FUN_00522970();
                    }
                  }
                }
              }
            }
          }
        }
        piStack_244 = (int *)0x48e8c1;
        iVar12 = FUN_0052a670();
      }
    }
    break;
  case 0x4d:
    if ((param_1[10] != 7) && (param_1[10] != 6)) {
      if ((*(byte *)(DAT_006c54dc + 0xb +
                    ((&DAT_006cb8c2)[(int)param_1[4]] + (&DAT_006cb942)[(int)param_1[3]] +
                    param_1[2]) * 0xc) & 0x40) == 0) {
        piStack_244 = (int *)DAT_006beca6;
        iStack_254 = 0x48d6d4;
        iStack_250 = param_1[2];
        pcStack_24c = (char *)param_1[3];
        pcStack_248 = (char *)param_1[4];
        FUN_005340b0();
        DAT_006a49cc = 1;
      }
      piStack_244 = (int *)0x48d6e1;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    piVar9 = param_1 + 0xb;
    piVar16 = local_218;
    for (iVar12 = 0x3f; iVar12 != 0; iVar12 = iVar12 + -1) {
      *piVar16 = *piVar9;
      piVar9 = piVar9 + 1;
      piVar16 = piVar16 + 1;
    }
    piStack_244 = (int *)0x48d6fe;
    iVar12 = FUN_004a07e0();
    if (iVar12 != 0) {
      iVar12 = param_1[0x13];
      piVar9 = local_218;
      piVar16 = aiStack_33c;
      for (iVar5 = 0x3f; iVar5 != 0; iVar5 = iVar5 + -1) {
        *piVar16 = *piVar9;
        piVar9 = piVar9 + 1;
        piVar16 = piVar16 + 1;
      }
      FUN_0049f5c0(*(undefined4 *)((int)param_1 + 0x1f6),*(undefined4 *)((int)param_1 + 0x1fa),
                   *(undefined4 *)((int)param_1 + 0x1fe),
                   *(undefined4 *)(&DAT_006b13c5 + iVar12 * 0x2d1),5,1,2);
      piStack_244 = (int *)0x48d75b;
      FUN_0049f650();
      piStack_244 = (int *)0x48d762;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048ce8d;
  case 0x4e:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 == (int *)0xffffffff) {
      piStack_244 = (int *)0x48e3f7;
      FUN_0049a4a0();
      piStack_244 = (int *)0x48e3fe;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    pcStack_248 = (char *)0x48e40e;
    iVar12 = FUN_004970b0();
    param_1[0x66] = iVar12;
    if (iVar12 != 0) {
      piStack_244 = (int *)0x48e41f;
      iVar12 = FUN_00412aa0();
      if (iVar12 != 0) {
        *(undefined4 *)((int)param_1 + 0x1da) = 0;
        param_1[0x11] = param_1[0x12];
        param_1[0x12] = 0xffffffff;
        piStack_244 = (int *)0x48e441;
        uVar11 = FUN_0044d110();
        piStack_244 = (int *)0x34;
        *(undefined4 *)((int)param_1 + 0x212) = uVar11;
        pcStack_248 = (char *)0x48e450;
        FUN_00494ab0();
        pcStack_248 = (char *)0x2;
        pcStack_24c = (char *)0x48e45e;
        piStack_244 = param_1;
        iVar12 = FUN_0051ff00();
        if (iVar12 != 0) {
          iStack_250 = param_1[4];
          iStack_254 = param_1[3];
          uStack_258 = param_1[2];
LAB_0048e487:
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x0;
          pcStack_24c = (char *)0x0;
          uStack_25c = 0x2ef;
          uStack_260 = 0x48e496;
          FUN_00571310();
          ExceptionList = local_c;
          return;
        }
      }
    }
    goto LAB_0048e462;
  case 0x4f:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 == (int *)0xffffffff) {
      piStack_244 = (int *)0x48e301;
      FUN_0049a4a0();
      piStack_244 = (int *)0x48e308;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    iVar12 = param_1[10];
    if ((((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x49;
      pcStack_248 = (char *)0x48e378;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    pcStack_248 = (char *)0x48e32f;
    iVar12 = FUN_004970b0();
    param_1[0x66] = iVar12;
    if (iVar12 != 0) {
      piStack_244 = (int *)0x48e344;
      iVar12 = FUN_00412aa0();
      if (iVar12 != 0) {
        piStack_244 = (int *)0x48e357;
        iVar12 = FUN_00492860();
        if (iVar12 != 0) {
          ExceptionList = local_c;
          return;
        }
        piStack_244 = (int *)0x48e36a;
        FUN_00520690();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048e462;
  case 0x50:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) {
LAB_0048bcdd:
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x3f3;
      uStack_260 = 0x48bcfe;
      FUN_00571310();
      uVar4 = param_1[0x11];
      goto LAB_0048bd03;
    }
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if (((iVar12 != 0) && (*(int *)(iVar12 + 0x110) == 0)) &&
       ((*(int *)(iVar12 + 0xa0) == 0 && (*(int *)(iVar12 + 0xe4) != 0)))) {
      piStack_244 = (int *)0x48bd85;
      FUN_0050ab30();
      piStack_244 = (int *)0x48bd90;
      FUN_00522880();
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x2cc;
      uStack_260 = 0x48bdb1;
      FUN_00571310();
      if ((0 < DAT_005e14ac) && (*(int *)(param_1[0x66] + 0x798) == 0)) {
        piStack_244 = (int *)0x54;
        pcStack_248 = (char *)0x48bdd9;
        iVar12 = FUN_00597850();
        local_4 = 1;
        if (iVar12 == 0) {
          puVar6 = (undefined4 *)0x0;
        }
        else {
          piStack_244 = (int *)param_1[7];
          pcStack_248 = (char *)param_1[6];
          pcStack_24c = (char *)param_1[5];
          iStack_250 = 0x48bdfe;
          puVar6 = (undefined4 *)FUN_00547210();
        }
        local_4 = 0xffffffff;
        if ((puVar6 != (undefined4 *)0x0) && (param_1[0x12] != -1)) {
          uVar4 = *(uint *)(*(int *)(param_1[0x66] + 0xac) + 8);
          puVar6[0x11] = param_1[0x66];
          puVar6[0x12] = 1;
          puVar6[0x13] = 1;
          puVar6[0x14] = 1;
          puVar6[4] = (int)(((uVar4 & 0x7ffffff) * 0x10 + -4) * DAT_005e14ac) / 200;
        }
        pcStack_248 = (char *)0x48be60;
        piStack_244 = puVar6;
        iVar12 = FUN_005457e0();
        if (iVar12 == 0) {
          if (puVar6 != (undefined4 *)0x0) {
            piStack_244 = (int *)0x1;
            pcStack_248 = (char *)0x48be77;
            (**(code **)*puVar6)();
            piStack_244 = (int *)0x48be7e;
            FUN_004943f0();
            ExceptionList = local_c;
            return;
          }
          goto LAB_0048ce8d;
        }
      }
      goto LAB_0048e469;
    }
    goto LAB_0048cbb0;
  case 0x51:
    if ((param_1[10] == 7) || (param_1[10] == 6)) {
      piStack_244 = (int *)0x48d5e5;
      FUN_0048a0c0();
      piStack_244 = (int *)0x48d5ec;
      FUN_00493530();
      if (param_1[4] + 1 < DAT_006c549c) {
        piStack_244 = (int *)param_1[0x65];
        pcStack_248 = (char *)0x48d607;
        FUN_00488330();
        iVar12 = param_1[4] + 1;
        param_1[4] = iVar12;
        param_1[7] = iVar12 * 0x10;
        puVar8 = (ushort *)
                 (DAT_006c54dc +
                 ((&DAT_006cb942)[param_1[3]] + (&DAT_006cb8c2)[iVar12] + param_1[2]) * 0xc);
        param_1[0x65] = puVar8;
        piStack_244 = (int *)param_1[0x65];
        param_1[100] = DAT_0065660c + (uint)*puVar8 * 0x164;
        pcStack_248 = (char *)0x48d66b;
        FUN_004882b0();
      }
      piStack_244 = (int *)0x45;
      pcStack_248 = (char *)0x48d674;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    if ((uint)param_1[0x12] < DAT_006def5c) {
      iVar12 = param_1[0x12] * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
      iVar12 = *(int *)(iVar12 + 0x84);
      iVar14 = param_1[0x13] * 0x2d1;
      iVar5 = *(int *)(&DAT_006b13c5 + iVar14);
      if ((*(int *)((int)&DAT_006b13d5 + iVar14) != 0) ||
         ((*(int *)((int)&DAT_006b13d9 + iVar14) != 0 || (*(int *)(&DAT_006b13dd + iVar14) != 0))))
      {
        piStack_244 = (int *)0x48d52e;
        uVar4 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          piStack_244 = (int *)0xf2a;
          pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
          pcStack_24c = (char *)0x48d55a;
          FUN_00485b10();
        }
        if ((uint)(iVar5 * 100) / (uint)(iVar5 + iVar12) <= (uVar4 & 0x7fffffff) % 100) {
          ExceptionList = local_c;
          return;
        }
      }
      iVar12 = **(int **)(param_1[0x66] + 0xac);
      if (iVar12 == 0) {
        ExceptionList = local_c;
        return;
      }
      if (iVar12 == 0x18) {
        ExceptionList = local_c;
        return;
      }
      if (iVar12 == 0x19) {
        ExceptionList = local_c;
        return;
      }
      if (iVar12 == 0x1a) {
        ExceptionList = local_c;
        return;
      }
      piStack_244 = (int *)0x14;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x48d5ae;
      FUN_00520100();
      ExceptionList = local_c;
      return;
    }
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x3f3;
    uStack_260 = 0x48d5d4;
    FUN_00571310();
    uVar4 = param_1[0x11];
LAB_0048cf8c:
    if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar12 = 0;
      }
      param_1[0x66] = iVar12;
      iVar12 = **(int **)(iVar12 + 0xac);
joined_r0x0048ce5e:
      if ((((iVar12 == 0) || (iVar12 == 0x18)) || (iVar12 == 0x19)) || (iVar12 == 0x1a)) {
        piStack_244 = (int *)(*(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1) << 8);
        pcStack_248 = (char *)0x48ce8d;
        FUN_00507f00();
      }
    }
    goto LAB_0048ce8d;
  case 0x52:
    uVar4 = param_1[0x12];
    if (uVar4 == 0xffffffff) goto LAB_0048cbb0;
    iVar12 = param_1[10];
    if ((((iVar12 == 7) || (iVar12 == 6)) || (iVar12 == 0xb)) || (iVar12 == 0x1e)) {
      piStack_244 = (int *)0x48;
      pcStack_248 = (char *)0x48c67a;
      FUN_00494ab0();
      ExceptionList = local_c;
      return;
    }
    if (uVar4 < DAT_006def5c) {
      iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
    }
    else {
      iVar12 = 0;
    }
    param_1[0x66] = iVar12;
    if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
      piStack_244 = (int *)DAT_006bfabb;
      pcStack_248 = (char *)0x48c64b;
      FUN_0051fb10();
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      piStack_244 = (int *)0x0;
      pcStack_248 = (char *)0x0;
      pcStack_24c = (char *)0x0;
      uStack_25c = 0x2ce;
      uStack_260 = 0x48c66c;
      FUN_00571310();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048cd49;
  case 0x53:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 == (int *)0xffffffff) {
      piStack_244 = (int *)0x48e26d;
      FUN_0049a4a0();
      piStack_244 = (int *)0x48e274;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    pcStack_248 = (char *)0x48e284;
    iVar12 = FUN_004970b0();
    param_1[0x66] = iVar12;
    if (iVar12 != 0) {
      piStack_244 = (int *)0x48e299;
      iVar12 = FUN_00412aa0();
      if (iVar12 != 0) {
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x1;
        pcStack_24c = (char *)0x48e2af;
        iVar12 = FUN_0051ff00();
        if (iVar12 == 0) {
          piStack_244 = (int *)0x48e2ba;
          FUN_0049a4a0();
          piStack_244 = (int *)0x48e2c1;
          FUN_004943f0();
          ExceptionList = local_c;
          return;
        }
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x2ec;
        uStack_260 = 0x48e2e7;
        FUN_00571310();
        piStack_244 = (int *)0x48e2ee;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
    }
    goto LAB_0048e462;
  case 0x54:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 == (int *)0xffffffff) {
      piStack_244 = (int *)0x48e38b;
      FUN_0049a4a0();
      piStack_244 = (int *)0x48e392;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    pcStack_248 = (char *)0x48e3a2;
    iVar12 = FUN_004970b0();
    param_1[0x66] = iVar12;
    if (iVar12 != 0) {
      piStack_244 = (int *)0x48e3b7;
      iVar12 = FUN_00412aa0();
      if (iVar12 != 0) {
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x3;
        pcStack_24c = (char *)0x48e3ce;
        iVar12 = FUN_0051ff00();
        if (iVar12 == 0) {
          piStack_244 = (int *)0x48e3dd;
          FUN_0049a4a0();
          piStack_244 = (int *)0x48e3e4;
          FUN_004943f0();
          ExceptionList = local_c;
          return;
        }
        goto LAB_0048ce8d;
      }
    }
LAB_0048e462:
    piStack_244 = (int *)0x48e469;
    FUN_0049a4a0();
LAB_0048e469:
    piStack_244 = (int *)0x48e470;
    FUN_004943f0();
    break;
  case 0x55:
    uVar4 = param_1[0x12];
    if (uVar4 != 0xffffffff) {
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar12 = 0;
      }
      param_1[0x66] = iVar12;
      if ((iVar12 != 0) && (*(int *)(iVar12 + 0xe4) != 0)) {
        piStack_244 = *(int **)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
        pcStack_248 = (char *)0x48b6ef;
        FUN_00518080();
        iVar12 = param_1[0x66];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        iStack_250 = *(undefined4 *)(iVar12 + 0x10);
        iStack_254 = *(undefined4 *)(iVar12 + 0xc);
        uStack_258 = *(undefined4 *)(iVar12 + 8);
        uStack_25c = 0x2c8;
        uStack_260 = 0x48b716;
        FUN_00571310();
        piStack_244 = (int *)0x48b71d;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
    }
LAB_0048cd49:
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
LAB_0048cbc2:
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    uStack_25c = 0x3f3;
    uStack_260 = 0x48cbd1;
    FUN_00571310();
    uVar4 = param_1[0x11];
    if (uVar4 != 0xffffffff) {
      iVar12 = param_1[0x17];
LAB_0048cbe0:
      if (iVar12 != 4) {
        if (uVar4 < DAT_006def5c) {
          iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
        }
        else {
LAB_0048d14f:
          iVar12 = 0;
        }
LAB_0048d151:
        param_1[0x66] = iVar12;
        piStack_244 = (int *)0x48d162;
        iVar12 = FUN_00411e50();
        if (iVar12 != 0) {
          piStack_244 = (int *)(*(int *)((int)&DAT_006b1270 + param_1[0x13] * 0x2d1) << 8);
          pcStack_248 = (char *)0x48d18c;
          FUN_00507f00();
          piStack_244 = (int *)0x48d193;
          FUN_004943f0();
          ExceptionList = local_c;
          return;
        }
      }
    }
    goto LAB_0048ce8d;
  case 0x56:
    uVar4 = param_1[0x12];
    if (uVar4 != 0xffffffff) {
      if (uVar4 < DAT_006def5c) {
        iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
      }
      else {
        iVar12 = 0;
      }
      param_1[0x66] = iVar12;
      if ((iVar12 == 0) || (*(int *)(iVar12 + 0xe4) == 0)) goto LAB_0048bcdd;
      piStack_244 = (int *)param_1[0x65];
      pcStack_248 = (char *)0x48cc47;
      FUN_00488330();
      iVar5 = *(int *)(param_1[0x66] + 0x10);
      param_1[4] = iVar5;
      iVar5 = DAT_006c54dc +
              ((&DAT_006cb8c2)[iVar5] + (&DAT_006cb942)[param_1[3]] + param_1[2]) * 0xc;
      param_1[0x65] = iVar5;
      if ((*(byte *)(iVar5 + 0xb) & 0x40) == 0) {
        uVar1 = *(ushort *)(iVar5 + 2);
        if (uVar1 == 0xffff) {
          *(undefined2 *)(iVar5 + 2) = *(undefined2 *)param_1;
        }
        else {
          do {
            if ((uint)uVar1 < (uint)((int *)param_1[9])[1]) {
              iVar14 = *(int *)param_1[9] + (uint)uVar1 * 0x22e;
            }
            else {
              iVar14 = 0;
            }
            uVar1 = *(ushort *)(iVar14 + 0x1a0);
          } while (uVar1 != 0xffff);
          *(undefined2 *)(iVar14 + 0x1a0) = *(undefined2 *)param_1;
          *(undefined2 *)(param_1 + 0x68) = 0xffff;
        }
        *(ushort *)(iVar5 + 8) = *(ushort *)(iVar5 + 8) & 0xff7f;
      }
      param_1[100] = DAT_0065660c + (uint)*(ushort *)param_1[0x65] * 0x164;
      piStack_244 = *(int **)(param_1[0x66] + 0xa8);
      pcStack_248 = (char *)0x48cd26;
      iVar5 = FUN_0048f490();
      if (iVar5 != 0) {
        piStack_244 = (int *)0x1;
        pcStack_248 = *(char **)(param_1[0x66] + 0xa8);
        pcStack_24c = (char *)0x48cd43;
        iVar5 = FUN_0048f690();
        if (iVar5 == 0) goto LAB_0048cd49;
        iStack_250 = param_1[4];
        iStack_254 = param_1[3];
        uStack_258 = param_1[2];
        piStack_244 = (int *)0x0;
        pcStack_248 = (char *)0x0;
        pcStack_24c = (char *)0x0;
        uStack_25c = 0x2d6;
        uStack_260 = 0x48cd9b;
        FUN_00571310();
        piStack_244 = *(int **)(iVar12 + 0x94);
        pcStack_248 = (char *)0x48cda9;
        FUN_00518100();
        piStack_244 = *(int **)(iVar12 + 0x90);
        pcStack_248 = (char *)0x48cdb7;
        FUN_005180c0();
        piStack_244 = *(int **)(iVar12 + 0x778);
        pcStack_248 = (char *)0x48cdc5;
        FUN_0051fb10();
        piStack_244 = *(int **)(iVar12 + 0x8c);
        pcStack_248 = (char *)0x48cdd3;
        FUN_00518080();
        piStack_244 = (int *)0x48cdda;
        iVar12 = FUN_0051fef0();
        if (iVar12 == 1) {
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x1;
          pcStack_24c = (char *)0x48cf60;
          piStack_244 = (int *)FUN_0051ff00();
          if (piStack_244 == (int *)0x0) {
            iStack_254 = param_1[3];
            uStack_258 = param_1[2];
            iStack_250 = param_1[4];
            uStack_25c = 0x3f3;
            uStack_260 = 0x48cf86;
            pcStack_24c = (char *)piStack_244;
            pcStack_248 = (char *)piStack_244;
            FUN_00571310();
            uVar4 = param_1[0x11];
            goto LAB_0048cf8c;
          }
        }
        else {
          if (iVar12 == 2) {
            param_1[0x13] = 0x4e;
            param_1[0x11] = param_1[0x12];
            piStack_244 = (int *)0x34;
            param_1[0x12] = 0xffffffff;
            *(undefined4 *)((int)param_1 + 0x1da) = 0xffffffff;
            pcStack_248 = (char *)0x48cebb;
            FUN_00494ab0();
            pcStack_248 = (char *)0x2;
            pcStack_24c = (char *)0x48cec5;
            piStack_244 = param_1;
            iVar12 = FUN_0051ff00();
            iStack_254 = param_1[3];
            uStack_258 = param_1[2];
            iStack_250 = param_1[4];
            piStack_244 = (int *)0x0;
            pcStack_248 = (char *)0x0;
            pcStack_24c = (char *)0x0;
            if (iVar12 != 0) goto LAB_0048e487;
            uStack_25c = 0x3f3;
            uStack_260 = 0x48ceee;
            FUN_00571310();
            uVar4 = param_1[0x11];
            if ((uVar4 == 0xffffffff) || (param_1[0x17] == 4)) goto LAB_0048cf4a;
            if (uVar4 < DAT_006def5c) {
              iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
            }
            else {
              iVar12 = 0;
            }
            param_1[0x66] = iVar12;
            piStack_244 = (int *)0x48cf24;
            iVar12 = FUN_00411e50();
            goto joined_r0x0048cf26;
          }
          if (iVar12 != 3) {
            piStack_244 = (int *)0x48cdf2;
            FUN_004943f0();
            ExceptionList = local_c;
            return;
          }
          piStack_244 = (int *)0x0;
          pcStack_248 = (char *)0x3;
          pcStack_24c = (char *)0x48ce02;
          piStack_244 = (int *)FUN_0051ff00();
          if (piStack_244 == (int *)0x0) {
            iStack_250 = param_1[4];
            iStack_254 = param_1[3];
            uStack_258 = param_1[2];
            uStack_25c = 0x3f3;
            uStack_260 = 0x48ce28;
            pcStack_24c = (char *)piStack_244;
            pcStack_248 = (char *)piStack_244;
            FUN_00571310();
            uVar4 = param_1[0x11];
            if ((uVar4 != 0xffffffff) && (param_1[0x17] != 4)) {
              if (uVar4 < DAT_006def5c) {
                iVar12 = uVar4 * 0xe4b + (int)DAT_006def58;
              }
              else {
                iVar12 = 0;
              }
              param_1[0x66] = iVar12;
              iVar12 = **(int **)(iVar12 + 0xac);
              goto joined_r0x0048ce5e;
            }
          }
        }
        goto LAB_0048ce8d;
      }
    }
LAB_0048cbb0:
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    goto LAB_0048cbc2;
  case 0x57:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 != (int *)0xffffffff) {
      pcStack_248 = (char *)0x48dd85;
      uVar11 = FUN_004970b0();
      param_1[0x66] = uVar11;
      piStack_244 = (int *)0x48dd92;
      iVar12 = FUN_004937b0();
      if (iVar12 == 0) {
        pcStack_248 = *(char **)((int)param_1 + 0x212);
        pcStack_24c = (char *)param_1[0x18];
        piStack_244 = (int *)((int)param_1 + 0x216);
        iStack_250 = 0x48ddb3;
        FUN_00514860();
      }
      if (param_1[0x12] != -1) {
        piStack_244 = (int *)param_1[0x19];
        pcStack_248 = (char *)0x48ddc7;
        FUN_00493790();
      }
      piStack_244 = (int *)param_1[0x66];
      pcStack_248 = (char *)0x48ddd5;
      FUN_0048f120();
      piStack_244 = (int *)0x48dddc;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
LAB_0048ce8d:
    piStack_244 = (int *)0x48ce94;
    FUN_004943f0();
    break;
  case 0x58:
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x64;
    pcStack_24c = (char *)0x48ddf0;
    uVar4 = FUN_0048eb40();
    if (DAT_006e1f30 != 0) {
      piStack_244 = (int *)0xde6;
      pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
      pcStack_24c = (char *)0x48de0f;
      FUN_00485b10();
    }
    if (uVar4 < *(uint *)(&DAT_006b1274 + param_1[0x13] * 0x2d1)) {
      piStack_244 = (int *)0x48de32;
      iVar12 = FUN_0049cdb0();
      if (iVar12 != 0) {
        local_218[2] = 0;
        local_218[1] = 0;
        local_218[0] = 0;
        piStack_244 = (int *)0x1;
        if (DAT_006db79a == 0) {
          pcStack_248 = (char *)(*(int *)(&DAT_005dda68 +
                                         *(int *)(&DAT_006b13e1 + param_1[0x13] * 0x2d1) * 0xaf0) <<
                                1);
          pcStack_24c = (char *)0x48df11;
          iVar12 = FUN_0048eb40();
          if (DAT_006e1f30 != 0) {
            piStack_244 = (int *)0xdf7;
            pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
            pcStack_24c = (char *)0x48df30;
            FUN_00485b10();
          }
          iVar12 = (iVar12 + *(int *)(&DAT_006b13e1 + param_1[0x13] * 0x2d1) * 700) * 4;
          uVar4 = *(int *)(&DAT_005dda68 + iVar12) + param_1[2];
          uVar17 = *(int *)(&DAT_005dda6c + iVar12) + param_1[3];
        }
        else {
          pcStack_248 = (char *)0x48de76;
          pcStack_248 = (char *)FUN_0049e250();
          pcStack_24c = (char *)0x48de81;
          uVar11 = FUN_0048eb40();
          if (DAT_006e1f30 != 0) {
            piStack_244 = (int *)0xdf0;
            pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
            pcStack_24c = (char *)0x48dea0;
            FUN_00485b10();
          }
          pcStack_248 = local_114;
          pcStack_24c = (char *)0x48decc;
          piStack_244 = (int *)uVar11;
          piVar9 = (int *)FUN_004840c0();
          uVar4 = param_1[2] + *piVar9;
          uVar17 = param_1[3] + piVar9[1];
        }
        if ((int)uVar4 < 0) {
          piStack_244 = (int *)0x48df7c;
          iVar12 = FUN_0048eb80();
          uVar4 = uVar4 + iVar12;
        }
        piStack_244 = (int *)0x48df88;
        uVar10 = FUN_0048eb80();
        if (uVar10 <= uVar4) {
          piStack_244 = (int *)0x48df96;
          iVar12 = FUN_0048eb80();
          uVar4 = uVar4 - iVar12;
        }
        if ((int)uVar17 < 0) {
          piStack_244 = (int *)0x48dfa6;
          iVar12 = FUN_0048eb90();
          uVar17 = uVar17 + iVar12;
        }
        piStack_244 = (int *)0x48dfb2;
        uVar10 = FUN_0048eb90();
        if (uVar10 <= uVar17) {
          piStack_244 = (int *)0x48dfc0;
          iVar12 = FUN_0048eb90();
          uVar17 = uVar17 - iVar12;
        }
        local_218[6] = param_1[0x11];
        piStack_244 = local_218;
        local_218[3] = uVar4 * 0x20 + 0x10;
        local_218[4] = uVar17 * 0x20 + 0x10;
        pcStack_248 = (char *)0x15;
        local_218[5] = 0x10;
        local_218[0xc] = 2;
        local_218[8] = 0x48;
        local_128 = 0;
        pcStack_24c = (char *)0x48e00f;
        FUN_00493f20();
      }
    }
    break;
  case 0x5d:
  case 0x5e:
  case 0x5f:
  case 0x60:
    piStack_244 = (int *)param_1[0x12];
    if (piStack_244 != (int *)0xffffffff) {
      if ((uint)param_1[0x11] < DAT_006def5c) {
        pcStack_248 = (char *)0x48b3cc;
        FUN_005159c0();
        piStack_244 = (int *)0x48b3d3;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
      pcStack_248 = (char *)0x48b3e2;
      FUN_005159c0();
      piStack_244 = (int *)0x48b3e9;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    piStack_244 = *(int **)(&DAT_006b13c5 + iVar12 * 0x2d1);
    goto LAB_0048b457;
  case 0x61:
  case 0x65:
    if ((param_1[0x12] != -1) &&
       (piStack_244 = (int *)param_1[0x11], piStack_244 != (int *)0xffffffff)) {
      pcStack_248 = (char *)0x48e5e9;
      uVar11 = FUN_004970b0();
      param_1[0x66] = uVar11;
      piStack_244 = (int *)param_1[0x12];
      pcStack_248 = (char *)0x48e5fa;
      FUN_005159c0();
    }
    break;
  case 0x62:
    uVar4 = param_1[0x12];
    piStack_244 = (int *)DAT_006c27c7;
    if (uVar4 != 0xffffffff) {
      iVar12 = param_1[0x11];
      if (iVar12 != -1) {
        if (uVar4 < DAT_006def5c) {
          iVar5 = uVar4 * 0xe4b + (int)DAT_006def58;
        }
        else {
          iVar5 = 0;
        }
        *(undefined4 *)(iVar5 + 0x811) = 100;
        *(int *)(iVar5 + 0x80d) = iVar12 + 1;
      }
      pcStack_248 = *(char **)((int)param_1 + 0x212);
      piStack_244 = (int *)((int)param_1 + 0x216);
      pcStack_24c = *(char **)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
      iStack_250 = 0x48b4e2;
      FUN_00514860();
      piStack_244 = (int *)0x48b4e9;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    goto LAB_0048b457;
  case 99:
    piStack_244 = (int *)0x0;
    pcStack_248 = (char *)0x0;
    pcStack_24c = (char *)0x0;
    if (param_1[0x12] != -1) {
      iStack_250 = param_1[4];
      iStack_254 = param_1[3];
      uStack_258 = param_1[2];
      uStack_25c = 0x3f5;
      uStack_260 = 0x48b2bb;
      FUN_00571310();
      if ((uint)param_1[0x11] < DAT_006def5c) {
        piStack_244 = (int *)param_1[0x12];
        pcStack_248 = (char *)0x48b2e1;
        FUN_005159c0();
        piStack_244 = (int *)0x48b2e8;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
      piStack_244 = (int *)param_1[0x12];
      pcStack_248 = (char *)0x48b2fa;
      FUN_005159c0();
      piStack_244 = (int *)0x48b301;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    iStack_250 = param_1[4];
    iStack_254 = param_1[3];
    uStack_258 = param_1[2];
    uStack_25c = 0x3f5;
    uStack_260 = 0x48b21c;
    FUN_00571310();
    piStack_244 = *(int **)(&DAT_006b13c5 + param_1[0x13] * 0x2d1);
    pcStack_248 = (char *)0x48b237;
    FUN_0048ecf0();
    piStack_244 = (int *)0x48b241;
    uVar4 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      piStack_244 = (int *)0x9a5;
      pcStack_248 = s_c__Projects_Chaos_effects_cpp_005e0138;
      pcStack_24c = (char *)0x48b26e;
      FUN_00485b10();
    }
    if ((*(short *)param_1[0x65] != 0) && ((*(byte *)((short *)param_1[0x65] + 5) & 0x10) == 0)) {
      bVar18 = (uVar4 & 0x7fffffff) % 99 + 1 < *(uint *)(&DAT_006b13c9 + param_1[0x13] * 0x2d1);
      goto LAB_0048dd54;
    }
    goto LAB_0048e469;
  case 0x66:
    if (param_1[0x12] != 0xffffffff) {
      if ((uint)param_1[0x12] < DAT_006def5c) {
        piStack_244 = *(int **)((int)param_1 + 0x212);
        pcStack_248 = (char *)0x48b421;
        FUN_00515f70();
        piStack_244 = (int *)0x48b428;
        FUN_004943f0();
        ExceptionList = local_c;
        return;
      }
      piStack_244 = *(int **)((int)param_1 + 0x212);
      pcStack_248 = (char *)0x48b43d;
      FUN_00515f70();
      piStack_244 = (int *)0x48b444;
      FUN_004943f0();
      ExceptionList = local_c;
      return;
    }
    piStack_244 = (int *)DAT_006c330b;
LAB_0048b457:
    pcStack_248 = (char *)0x48b45e;
    FUN_0048ecf0();
    piStack_244 = (int *)0xc;
    pcStack_248 = (char *)0x48b467;
    FUN_00494ab0();
  }
  ExceptionList = local_c;
  return;
}

