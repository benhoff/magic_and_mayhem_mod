
undefined4 FUN_004eda90(undefined4 param_1,int param_2)

{
  char cVar1;
  bool bVar2;
  undefined4 uVar3;
  undefined4 *puVar4;
  int iVar5;
  char *pcVar6;
  int *piVar7;
  int *piVar8;
  int extraout_ECX;
  int iVar9;
  uint uVar10;
  uint uVar11;
  int iVar12;
  int iVar13;
  int *piVar14;
  int *piVar15;
  int iVar16;
  undefined4 *puVar17;
  undefined4 *puVar18;
  int *piVar19;
  ushort *puVar20;
  undefined4 *puVar21;
  char *pcVar22;
  char *pcVar23;
  int *piStack00000010;
  uint uStack00000014;
  int in_stack_0000001c;
  undefined4 *puStack00000024;
  int in_stack_00000028;
  int iStack00000038;
  int iStack0000003c;
  int iStack00000040;
  int iStack00000044;
  undefined4 *puStack0000004c;
  int iStack00000050;
  int in_stack_00000054;
  undefined4 *puStack00000058;
  int *piStack0000005c;
  ushort *in_stack_00000094;
  undefined4 in_stack_0000009c;
  int in_stack_00000804;
  int in_stack_00000808;
  undefined4 in_stack_00000824;
  void *in_stack_000047e8;
  void *local_c;
  undefined1 *local_8;
  int local_4;
  
  local_4 = 0xffffffff;
  local_8 = &LAB_005c22bd;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_0059c000();
  iStack00000050 = 0;
  local_c = (void *)0x0;
  local_8 = (undefined1 *)0x0;
  puVar4 = &stack0x00000094;
  for (iVar9 = 0x19; iVar9 != 0; iVar9 = iVar9 + -1) {
    *puVar4 = 0;
    puVar4 = puVar4 + 1;
  }
  bVar2 = true;
  if ((DAT_006e1ee8 != 0) && (iStack00000050 = FUN_0059cae2(), iStack00000050 != 0)) {
    FUN_0059d50b();
    FUN_0059d50b();
  }
  iVar9 = FUN_0052d240();
  if (iVar9 == 0) {
    FUN_0049ee50();
    FUN_00532f00();
    if (local_c != (void *)0x0) {
      FUN_005979b0();
      local_c = (void *)0x0;
      local_8 = (undefined1 *)0x0;
    }
    FUN_004a1550();
    FUN_00532d70();
    uVar3 = 0;
  }
  else {
    FUN_004dc5e0();
    FUN_00532f00();
    FUN_0049ee50();
    *(undefined4 *)(extraout_ECX + 0x50) = in_stack_00000824;
    puVar4 = (undefined4 *)FUN_00597880();
    if (puVar4 == (undefined4 *)0x0) {
      FUN_0049ee50();
      FUN_00532f00();
      if (local_c != (void *)0x0) {
        FUN_005979b0();
        local_c = (void *)0x0;
        local_8 = (undefined1 *)0x0;
      }
      FUN_004a1550();
      FUN_00532d70();
      uVar3 = 0;
    }
    else {
      iVar9 = FUN_004ed8f0();
      if (iVar9 == 0) {
        FUN_0049ee50();
        FUN_00532f00();
        FUN_005979b0();
        if (local_c != (void *)0x0) {
          FUN_005979b0();
          local_c = (void *)0x0;
          local_8 = (undefined1 *)0x0;
        }
        FUN_004a1550();
        FUN_00532d70();
        uVar3 = 0;
      }
      else {
        if (iStack00000050 != 0) {
          FUN_0059d50b();
          FUN_0059d50b();
          FUN_0059d50b();
          FUN_0059d50b();
          FUN_0059d50b();
          FUN_0059d50b();
          FUN_0059d50b();
        }
        iVar9 = FUN_00532d80();
        FUN_004dc5e0();
        local_4 = 0;
        if (0 < in_stack_00000804) {
          puStack0000004c = &stack0x00000094;
          piStack0000005c = (int *)(iVar9 + 0xd);
          do {
            param_1 = 0;
            if (0 < in_stack_00000808) {
              puStack00000058 = puStack0000004c;
              piVar19 = piStack0000005c;
              do {
                iVar5 = FUN_0052e690();
                if (iVar5 == 0) {
                  FUN_0049ee50();
                  FUN_00532f00();
                  FUN_005979b0();
                  if (local_c != (void *)0x0) {
                    FUN_005979b0();
                    local_c = (void *)0x0;
                    local_8 = (undefined1 *)0x0;
                  }
                  FUN_004a1550();
                  FUN_00532d70();
                  ExceptionList = in_stack_000047e8;
                  return 0;
                }
                if (iStack00000050 != 0) {
                  FUN_0059d50b();
                  FUN_0059d50b();
                  FUN_0059d50b();
                  FUN_0059d50b();
                  FUN_0059d50b();
                  FUN_0059d50b();
                }
                if ((*piVar19 == 0) && (piVar19[1] == 0)) {
                  iVar5 = FUN_004eeaf0();
                  if (iVar5 == 0) {
                    FUN_0049ee50();
                    FUN_00532f00();
                    FUN_005979b0();
                    if (local_c != (void *)0x0) {
                      FUN_005979b0();
                      local_c = (void *)0x0;
                      local_8 = (undefined1 *)0x0;
                    }
                    FUN_004a1550();
                    FUN_00532d70();
                    ExceptionList = in_stack_000047e8;
                    return 0;
                  }
                  uVar10 = 0xffffffff;
                  pcVar6 = &stack0x000000f8;
                  do {
                    pcVar23 = pcVar6;
                    if (uVar10 == 0) break;
                    uVar10 = uVar10 - 1;
                    pcVar23 = pcVar6 + 1;
                    cVar1 = *pcVar6;
                    pcVar6 = pcVar23;
                  } while (cVar1 != '\0');
                  uVar10 = ~uVar10;
                  pcVar6 = pcVar23 + -uVar10;
                  pcVar23 = &stack0x000001f8;
                  for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                    *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                    pcVar6 = pcVar6 + 4;
                    pcVar23 = pcVar23 + 4;
                  }
                  for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                    *pcVar23 = *pcVar6;
                    pcVar6 = pcVar6 + 1;
                    pcVar23 = pcVar23 + 1;
                  }
                  pcVar6 = _strchr(&stack0x000001f8,0x2e);
                  if (pcVar6 != (char *)0x0) {
                    *pcVar6 = '\0';
                  }
                  uVar10 = 0xffffffff;
                  pcVar6 = &DAT_005e4638;
                  do {
                    pcVar23 = pcVar6;
                    if (uVar10 == 0) break;
                    uVar10 = uVar10 - 1;
                    pcVar23 = pcVar6 + 1;
                    cVar1 = *pcVar6;
                    pcVar6 = pcVar23;
                  } while (cVar1 != '\0');
                  uVar10 = ~uVar10;
                  iVar5 = -1;
                  pcVar6 = &stack0x000001f8;
                  do {
                    pcVar22 = pcVar6;
                    if (iVar5 == 0) break;
                    iVar5 = iVar5 + -1;
                    pcVar22 = pcVar6 + 1;
                    cVar1 = *pcVar6;
                    pcVar6 = pcVar22;
                  } while (cVar1 != '\0');
                  pcVar6 = pcVar23 + -uVar10;
                  pcVar23 = pcVar22 + -1;
                  for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                    *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                    pcVar6 = pcVar6 + 4;
                    pcVar23 = pcVar23 + 4;
                  }
                  for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                    *pcVar23 = *pcVar6;
                    pcVar6 = pcVar6 + 1;
                    pcVar23 = pcVar23 + 1;
                  }
                  FUN_00532f00();
                  iVar5 = FUN_00532f30();
                  if (iVar5 == 0) {
                    uVar10 = 0xffffffff;
                    pcVar6 = s_could_not_be_loaded_005e4620;
                    do {
                      pcVar23 = pcVar6;
                      if (uVar10 == 0) break;
                      uVar10 = uVar10 - 1;
                      pcVar23 = pcVar6 + 1;
                      cVar1 = *pcVar6;
                      pcVar6 = pcVar23;
                    } while (cVar1 != '\0');
                    uVar10 = ~uVar10;
                    iVar5 = -1;
                    pcVar6 = &stack0x000001f8;
                    do {
                      pcVar22 = pcVar6;
                      if (iVar5 == 0) break;
                      iVar5 = iVar5 + -1;
                      pcVar22 = pcVar6 + 1;
                      cVar1 = *pcVar6;
                      pcVar6 = pcVar22;
                    } while (cVar1 != '\0');
                    pcVar6 = pcVar23 + -uVar10;
                    pcVar23 = pcVar22 + -1;
                    for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                      *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                      pcVar6 = pcVar6 + 4;
                      pcVar23 = pcVar23 + 4;
                    }
                    for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                      *pcVar23 = *pcVar6;
                      pcVar6 = pcVar6 + 1;
                      pcVar23 = pcVar23 + 1;
                    }
                    MessageBoxA((HWND)0x0,&stack0x000001f8,s_MAP_LOAD_ERROR_005e4610,0x42030);
                    DestroyWindow(DAT_00656614);
                  }
                  uVar10 = 0xffffffff;
                  pcVar6 = &stack0x000000f8;
                  do {
                    pcVar23 = pcVar6;
                    if (uVar10 == 0) break;
                    uVar10 = uVar10 - 1;
                    pcVar23 = pcVar6 + 1;
                    cVar1 = *pcVar6;
                    pcVar6 = pcVar23;
                  } while (cVar1 != '\0');
                  uVar10 = ~uVar10;
                  pcVar6 = pcVar23 + -uVar10;
                  pcVar23 = &stack0x00000600;
                  for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                    *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                    pcVar6 = pcVar6 + 4;
                    pcVar23 = pcVar23 + 4;
                  }
                  for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                    *pcVar23 = *pcVar6;
                    pcVar6 = pcVar6 + 1;
                    pcVar23 = pcVar23 + 1;
                  }
                  pcVar6 = _strchr(&stack0x00000600,0x2e);
                  if (pcVar6 != (char *)0x0) {
                    *pcVar6 = '\0';
                  }
                  uVar10 = 0xffffffff;
                  pcVar6 = &DAT_005e4608;
                  do {
                    pcVar23 = pcVar6;
                    if (uVar10 == 0) break;
                    uVar10 = uVar10 - 1;
                    pcVar23 = pcVar6 + 1;
                    cVar1 = *pcVar6;
                    pcVar6 = pcVar23;
                  } while (cVar1 != '\0');
                  uVar10 = ~uVar10;
                  iVar5 = -1;
                  pcVar6 = &stack0x00000600;
                  do {
                    pcVar22 = pcVar6;
                    if (iVar5 == 0) break;
                    iVar5 = iVar5 + -1;
                    pcVar22 = pcVar6 + 1;
                    cVar1 = *pcVar6;
                    pcVar6 = pcVar22;
                  } while (cVar1 != '\0');
                  pcVar6 = pcVar23 + -uVar10;
                  pcVar23 = pcVar22 + -1;
                  for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                    *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                    pcVar6 = pcVar6 + 4;
                    pcVar23 = pcVar23 + 4;
                  }
                  for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                    *pcVar23 = *pcVar6;
                    pcVar6 = pcVar6 + 1;
                    pcVar23 = pcVar23 + 1;
                  }
                  FUN_0049ee50();
                  iVar5 = FUN_0049ee80();
                  if (iVar5 == 0) {
                    bVar2 = false;
                  }
                  iVar5 = *(int *)(param_2 + 0xc);
                  if (iStack00000050 != 0) {
                    FUN_0059d50b();
                  }
                  if ((*(int *)(param_2 + 0x18) == 1) && (*(int *)(param_2 + 0x1c) == 1)) {
                    FUN_004ef6b0();
                    if (iStack00000050 != 0) {
                      FUN_0059d50b();
                    }
                    *puStack00000058 = 1;
                  }
                  else {
                    iStack00000040 = 0;
                    if (0 < *(int *)(param_2 + 0x18)) {
                      do {
                        iStack0000003c = 0;
                        if (0 < *(int *)(param_2 + 0x1c)) {
                          do {
                            piVar15 = (int *)&stack0x00000094;
                            iStack00000038 = 0;
                            piStack00000010 = (int *)(iVar9 + 0xd);
                            do {
                              iStack00000044 = 0;
                              piVar14 = piStack00000010;
                              piVar8 = piVar15;
                              do {
                                piVar7 = piVar14;
                                if ((((piVar7[-3] == piVar19[-3]) && (*piVar7 == iStack00000040)) &&
                                    (piVar7[1] == iStack0000003c)) &&
                                   ((piVar7[-1] == piVar19[-1] && (*piVar8 == 0))))
                                goto LAB_004ee1f6;
                                iStack00000044 = iStack00000044 + 1;
                                piVar8 = piVar8 + 1;
                                piVar14 = (int *)((int)piVar7 + 0x25);
                              } while (iStack00000044 < in_stack_00000808);
                              piStack00000010 = (int *)((int)piStack00000010 + 0xb9);
                              iStack00000038 = iStack00000038 + 1;
                              piVar15 = piVar15 + 5;
                            } while (iStack00000038 < in_stack_00000804);
LAB_004ee1f6:
                            if ((iStack00000038 == in_stack_00000804) &&
                               (iStack00000044 == in_stack_00000808)) {
                              FUN_0049ee50();
                              FUN_00532f00();
                              FUN_005979b0();
                              if (local_c != (void *)0x0) {
                                FUN_005979b0();
                                local_c = (void *)0x0;
                                local_8 = (undefined1 *)0x0;
                              }
                              FUN_004a1550();
                              FUN_00532d70();
                              ExceptionList = in_stack_000047e8;
                              return 0;
                            }
                            iVar13 = *(int *)(extraout_ECX + 0x50);
                            piStack00000010 = (int *)0x0;
                            iVar12 = (piVar7[1] * *(int *)(param_2 + 4) + *piVar7) * iVar13;
                            puStack00000024 = puVar4;
                            if (0 < iVar5) {
                              do {
                                iVar16 = 0;
                                puVar17 = (undefined4 *)
                                          (in_stack_00000054 + iVar12 * 0xc +
                                          *(int *)(param_2 + 0x10) * (int)piStack00000010 * 0xc);
                                if (0 < iVar13) {
                                  do {
                                    puVar18 = puVar17;
                                    puVar21 = puStack00000024;
                                    for (uVar10 = (uint)(iVar13 * 0xc) >> 2; uVar10 != 0;
                                        uVar10 = uVar10 - 1) {
                                      *puVar21 = *puVar18;
                                      puVar18 = puVar18 + 1;
                                      puVar21 = puVar21 + 1;
                                    }
                                    for (iVar13 = 0; iVar13 != 0; iVar13 = iVar13 + -1) {
                                      *(undefined1 *)puVar21 = *(undefined1 *)puVar18;
                                      puVar18 = (undefined4 *)((int)puVar18 + 1);
                                      puVar21 = (undefined4 *)((int)puVar21 + 1);
                                    }
                                    iVar13 = *(int *)(extraout_ECX + 0x50);
                                    puStack00000024 = puStack00000024 + iVar13 * 3;
                                    iVar16 = iVar16 + 1;
                                    puVar17 = puVar17 + *(int *)(param_2 + 0x18) * iVar13 * 3;
                                  } while (iVar16 < iVar13);
                                }
                                piStack00000010 = (int *)((int)piStack00000010 + 1);
                              } while ((int)piStack00000010 < iVar5);
                            }
                            FUN_004ef6b0();
                            (&stack0x00000094)[iStack00000038 * 5 + iStack00000044] = (ushort *)0x1;
                            if (iStack00000050 != 0) {
                              FUN_0059d50b();
                            }
                            iStack0000003c = iStack0000003c + 1;
                          } while (iStack0000003c < *(int *)(param_2 + 0x1c));
                        }
                        iStack00000040 = iStack00000040 + 1;
                      } while (iStack00000040 < *(int *)(param_2 + 0x18));
                    }
                    if (iStack00000050 != 0) {
                      FUN_0059d50b();
                    }
                  }
                  FUN_004ef5e0();
                  FUN_004eefc0();
                  if (bVar2) {
                    FUN_004ef2c0();
                  }
                  if (DAT_006e1f3c != 0) {
                    uVar10 = 0xffffffff;
                    pcVar6 = &stack0x000000f8;
                    do {
                      pcVar23 = pcVar6;
                      if (uVar10 == 0) break;
                      uVar10 = uVar10 - 1;
                      pcVar23 = pcVar6 + 1;
                      cVar1 = *pcVar6;
                      pcVar6 = pcVar23;
                    } while (cVar1 != '\0');
                    uVar10 = ~uVar10;
                    pcVar6 = pcVar23 + -uVar10;
                    pcVar23 = &stack0x00000500;
                    for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                      *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                      pcVar6 = pcVar6 + 4;
                      pcVar23 = pcVar23 + 4;
                    }
                    for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                      *pcVar23 = *pcVar6;
                      pcVar6 = pcVar6 + 1;
                      pcVar23 = pcVar23 + 1;
                    }
                    pcVar6 = _strchr(&stack0x00000500,0x2e);
                    if (pcVar6 != (char *)0x0) {
                      *pcVar6 = '\0';
                    }
                    uVar10 = 0xffffffff;
                    pcVar6 = &DAT_005e45d0;
                    do {
                      pcVar23 = pcVar6;
                      if (uVar10 == 0) break;
                      uVar10 = uVar10 - 1;
                      pcVar23 = pcVar6 + 1;
                      cVar1 = *pcVar6;
                      pcVar6 = pcVar23;
                    } while (cVar1 != '\0');
                    uVar10 = ~uVar10;
                    iVar5 = -1;
                    pcVar6 = &stack0x00000500;
                    do {
                      pcVar22 = pcVar6;
                      if (iVar5 == 0) break;
                      iVar5 = iVar5 + -1;
                      pcVar22 = pcVar6 + 1;
                      cVar1 = *pcVar6;
                      pcVar6 = pcVar22;
                    } while (cVar1 != '\0');
                    pcVar6 = pcVar23 + -uVar10;
                    pcVar23 = pcVar22 + -1;
                    for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
                      *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
                      pcVar6 = pcVar6 + 4;
                      pcVar23 = pcVar23 + 4;
                    }
                    for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
                      *pcVar23 = *pcVar6;
                      pcVar6 = pcVar6 + 1;
                      pcVar23 = pcVar23 + 1;
                    }
                    iVar5 = *(int *)(param_2 + 0x18);
                    iVar13 = *(int *)(param_2 + 0x1c);
                    if ((1 < iVar5) || (1 < iVar13)) {
                      switch(piVar19[-1]) {
                      case 0:
                        in_stack_0000001c = local_4;
                        in_stack_00000028 = param_1;
                        break;
                      case 1:
                        in_stack_00000028 = param_1;
                        in_stack_0000001c = (local_4 - iVar13) + 1;
                        break;
                      case 2:
                        in_stack_0000001c = (local_4 - iVar5) + 1;
                        in_stack_00000028 = (param_1 - iVar13) + 1;
                        break;
                      case 3:
                        in_stack_0000001c = local_4;
                        in_stack_00000028 = (param_1 - iVar5) + 1;
                      }
                      if (in_stack_0000001c < 0) {
                        in_stack_0000001c = in_stack_0000001c + in_stack_00000804;
                      }
                      if (in_stack_00000028 < 0) {
                        in_stack_00000028 = in_stack_00000028 + in_stack_00000808;
                      }
                    }
                    FUN_0053eaa0();
                  }
                }
                param_1 = param_1 + 1;
                piVar19 = (int *)((int)piVar19 + 0x25);
                puStack00000058 = puStack00000058 + 1;
              } while (param_1 < in_stack_00000808);
            }
            local_4 = local_4 + 1;
            puStack0000004c = puStack0000004c + 5;
            piStack0000005c = (int *)((int)piStack0000005c + 0xb9);
          } while (local_4 < in_stack_00000804);
        }
        FUN_004dc5e0();
        uVar10 = 0xffffffff;
        pcVar6 = &stack0x000000f8;
        do {
          pcVar23 = pcVar6;
          if (uVar10 == 0) break;
          uVar10 = uVar10 - 1;
          pcVar23 = pcVar6 + 1;
          cVar1 = *pcVar6;
          pcVar6 = pcVar23;
        } while (cVar1 != '\0');
        uVar10 = ~uVar10;
        pcVar6 = pcVar23 + -uVar10;
        pcVar23 = &stack0x00000700;
        for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
          *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
          pcVar6 = pcVar6 + 4;
          pcVar23 = pcVar23 + 4;
        }
        for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
          *pcVar23 = *pcVar6;
          pcVar6 = pcVar6 + 1;
          pcVar23 = pcVar23 + 1;
        }
        pcVar6 = _strchr(&stack0x00000700,0);
        cVar1 = *pcVar6;
        while (cVar1 != '\\') {
          pcVar23 = pcVar6 + -1;
          pcVar6 = pcVar6 + -1;
          cVar1 = *pcVar23;
        }
        pcVar6[1] = '\0';
        uVar10 = 0xffffffff;
        pcVar6 = s_doors_cfg_005e45c4;
        do {
          pcVar23 = pcVar6;
          if (uVar10 == 0) break;
          uVar10 = uVar10 - 1;
          pcVar23 = pcVar6 + 1;
          cVar1 = *pcVar6;
          pcVar6 = pcVar23;
        } while (cVar1 != '\0');
        uVar10 = ~uVar10;
        iVar9 = -1;
        pcVar6 = &stack0x00000700;
        do {
          pcVar22 = pcVar6;
          if (iVar9 == 0) break;
          iVar9 = iVar9 + -1;
          pcVar22 = pcVar6 + 1;
          cVar1 = *pcVar6;
          pcVar6 = pcVar22;
        } while (cVar1 != '\0');
        pcVar6 = pcVar23 + -uVar10;
        pcVar23 = pcVar22 + -1;
        for (uVar11 = uVar10 >> 2; uVar11 != 0; uVar11 = uVar11 - 1) {
          *(undefined4 *)pcVar23 = *(undefined4 *)pcVar6;
          pcVar6 = pcVar6 + 4;
          pcVar23 = pcVar23 + 4;
        }
        for (uVar10 = uVar10 & 3; uVar10 != 0; uVar10 = uVar10 - 1) {
          *pcVar23 = *pcVar6;
          pcVar6 = pcVar6 + 1;
          pcVar23 = pcVar23 + 1;
        }
        FUN_00487570();
        if (iStack00000050 != 0) {
          FUN_0059d50b();
          FUN_0059c4c3();
        }
        puVar4 = *(undefined4 **)(extraout_ECX + 0x7e8);
        puVar20 = *(ushort **)(extraout_ECX + 0x4c);
        puVar17 = (undefined4 *)*puVar4;
        while (puVar17 != puVar4) {
          puVar17 = (undefined4 *)*puVar17;
          piVar19 = (int *)FUN_00502420();
          piVar19 = (int *)*piVar19;
          *(int *)piVar19[1] = *piVar19;
          *(int *)(*piVar19 + 4) = piVar19[1];
          FUN_00502800();
          FUN_00597870();
          *(int *)(extraout_ECX + 0x7ec) = *(int *)(extraout_ECX + 0x7ec) + -1;
        }
        uStack00000014 = 0;
        if (*(int *)(extraout_ECX + 0x14) != 0) {
          do {
            puVar20[1] = 0xffff;
            if ((*puVar20 == 0) || (*(int *)(DAT_0065660c + 0x94 + (uint)*puVar20 * 0x164) != 0x10))
            {
              puVar20[5] = puVar20[5] & 0xfffe;
            }
            else {
              *(byte *)(puVar20 + 5) = (byte)puVar20[5] | 1;
            }
            if ((puVar20[5] & 8) != 0) {
              FUN_00464c50();
              in_stack_00000094 = puVar20;
              FUN_00464ca0();
              FUN_00464cb0();
              piVar19 = (int *)**(undefined4 **)(extraout_ECX + 0x7e8);
              piVar15 = (int *)piVar19[1];
              piVar8 = (int *)FUN_00597850();
              piVar14 = piVar19;
              if (piVar19 == (int *)0x0) {
                piVar14 = piVar8;
              }
              *piVar8 = (int)piVar14;
              if (piVar15 == (int *)0x0) {
                piVar15 = piVar8;
              }
              piVar8[1] = (int)piVar15;
              piVar19[1] = (int)piVar8;
              *(int **)piVar8[1] = piVar8;
              FUN_005027e0();
              *(int *)(extraout_ECX + 0x7ec) = *(int *)(extraout_ECX + 0x7ec) + 1;
              FUN_004eb710();
              in_stack_00000094 = (ushort *)0x0;
            }
            puVar20[5] = puVar20[5] & 0xdfff;
            puVar20[2] = 0xffff;
            uStack00000014 = uStack00000014 + 1;
            puVar20 = puVar20 + 6;
          } while (uStack00000014 < *(uint *)(extraout_ECX + 0x14));
        }
        FUN_004dc5e0();
        FUN_004ecdd0();
        FUN_00545570();
        FUN_004ecf20();
        DAT_005e1780 = DAT_006c549c;
        DAT_006e1fa0 = 0;
        DAT_005e1784 = DAT_006c549c << 4;
        DAT_006e1fa4 = 0;
        FUN_004ecda0();
        if ((DAT_00689920 == 2) || (DAT_00689920 == 1)) {
          FUN_004eff00();
        }
        else {
          FUN_004efc80();
        }
        FUN_004f06b0();
        FUN_0049ee50();
        FUN_00532f00();
        FUN_005979b0();
        if (local_c != (void *)0x0) {
          FUN_005979b0();
          local_c = (void *)0x0;
          local_8 = (undefined1 *)0x0;
        }
        FUN_004a1550();
        FUN_00532d70();
        uVar3 = 1;
      }
    }
  }
  ExceptionList = in_stack_000047e8;
  return uVar3;
}

