
int __thiscall FUN_00501e00(int param_1,int param_2)

{
  int iVar1;
  undefined4 *puVar2;
  undefined4 uVar3;
  int iVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined4 *puVar9;
  int *piVar10;
  undefined2 *puVar11;
  int local_58;
  undefined4 *local_54;
  int local_50;
  int local_4c;
  undefined1 local_48 [4];
  undefined1 local_44 [4];
  int local_40;
  void *local_c;
  undefined1 *puStack_8;
  undefined4 local_4;
  
  uVar3 = param_2;
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005c2378;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  iVar4 = FUN_0059c359(param_1,0x4c,1,param_2);
  if (iVar4 == 1) {
    iVar4 = 0;
    if (0 < *(int *)(param_1 + 0xc)) {
      piVar10 = (int *)(param_1 + 0x6432);
      do {
        *piVar10 = *(int *)(param_1 + 0x10) * iVar4;
        iVar4 = iVar4 + 1;
        piVar10 = piVar10 + 1;
      } while (iVar4 < *(int *)(param_1 + 0xc));
    }
    iVar4 = 0;
    if (0 < *(int *)(param_1 + 8)) {
      piVar10 = (int *)(param_1 + 0x64b2);
      do {
        *piVar10 = *(int *)(param_1 + 4) * iVar4;
        iVar4 = iVar4 + 1;
        piVar10 = piVar10 + 1;
      } while (iVar4 < *(int *)(param_1 + 8));
    }
    if (*(int *)(param_1 + 0x4c) != 0) {
      FUN_00597870(*(int *)(param_1 + 0x4c));
    }
    iVar4 = *(int *)(param_1 + 0x14);
    uVar5 = FUN_00597850(iVar4 * 0xc);
    if (uVar5 == 0) {
      uVar5 = 0;
    }
    else if (-1 < iVar4 + -1) {
      puVar11 = (undefined2 *)(uVar5 + 4);
      do {
        puVar11[-2] = 0xffff;
        puVar11[-1] = 0xffff;
        *puVar11 = 0xffff;
        puVar11[1] = 0xffff;
        puVar11[2] = 0;
        puVar11[3] = 0;
        puVar11 = puVar11 + 6;
        iVar4 = iVar4 + -1;
      } while (iVar4 != 0);
    }
    *(uint *)(param_1 + 0x4c) = uVar5;
    if (uVar5 == 0) {
      param_2 = -1;
    }
    else {
      iVar4 = 0x4c;
      if (uVar5 < uVar5 + *(int *)(param_1 + 0x14) * 0xc) {
        do {
          iVar6 = FUN_0059c359(uVar5,0xc,1,param_2);
          if (iVar6 != 1) goto LAB_005023a1;
          uVar5 = uVar5 + 0xc;
          iVar4 = iVar4 + 0xc;
        } while (uVar5 < (uint)(*(int *)(param_1 + 0x4c) + *(int *)(param_1 + 0x14) * 0xc));
      }
      iVar6 = FUN_0059c359(param_1 + 0x50,4,1,param_2);
      if (iVar6 == 1) {
        iVar6 = FUN_00533550(param_2);
        if (iVar6 == -1) {
          param_2 = -1;
        }
        else {
          iVar7 = FUN_00533550(param_2);
          if (iVar7 == -1) {
            param_2 = -1;
          }
          else {
            iVar8 = FUN_0059c359(param_1 + 0x84,4,0x4b,param_2);
            if (iVar8 == 0x4b) {
              iVar8 = FUN_0059c359(param_1 + 0x1b0,4,0x78,param_2);
              if (iVar8 == 0x78) {
                iVar8 = FUN_0059c359(param_1 + 0x390,4,0x78,param_2);
                if (iVar8 == 0x78) {
                  iVar8 = FUN_0059c359(param_1 + 0x570,4,0x1c,param_2);
                  if (iVar8 == 0x1c) {
                    iVar8 = FUN_0059c359(param_1 + 0x5e0,4,0x7b,param_2);
                    if (iVar8 == 0x7b) {
                      *(int *)(param_1 + 0x7e0) =
                           ((*(int *)(param_1 + 0xc) + 1) / 2) * *(int *)(param_1 + 0x10);
                      iVar8 = FUN_0059c359(&local_50,4,1,param_2);
                      if (iVar8 == 1) {
                        if (*(int *)(param_1 + 0x7e0) == local_50) {
                          if (*(int *)(param_1 + 0x7cc) != 0) {
                            FUN_00597870(*(int *)(param_1 + 0x7cc));
                          }
                          iVar8 = FUN_00597850(*(undefined4 *)(param_1 + 0x7e0));
                          *(int *)(param_1 + 0x7cc) = iVar8;
                          if (iVar8 == 0) {
                            param_2 = -1;
                          }
                          else {
                            if (*(int *)(param_1 + 2000) != 0) {
                              FUN_00597870(*(int *)(param_1 + 2000));
                            }
                            iVar8 = FUN_00597850(*(undefined4 *)(param_1 + 0x7e0));
                            *(int *)(param_1 + 2000) = iVar8;
                            if (iVar8 == 0) {
                              param_2 = -1;
                            }
                            else {
                              if (*(int *)(param_1 + 0x7d4) != 0) {
                                FUN_00597870(*(int *)(param_1 + 0x7d4));
                              }
                              iVar8 = FUN_00597850(*(undefined4 *)(param_1 + 0x7e0));
                              *(int *)(param_1 + 0x7d4) = iVar8;
                              if (iVar8 == 0) {
                                param_2 = -1;
                              }
                              else {
                                if (*(int *)(param_1 + 0x7d8) != 0) {
                                  FUN_00597870(*(int *)(param_1 + 0x7d8));
                                }
                                iVar8 = FUN_00597850(*(undefined4 *)(param_1 + 0x7e0));
                                *(int *)(param_1 + 0x7d8) = iVar8;
                                if (iVar8 == 0) {
                                  param_2 = -1;
                                }
                                else {
                                  if (*(int *)(param_1 + 0x7dc) != 0) {
                                    FUN_00597870(*(int *)(param_1 + 0x7dc));
                                  }
                                  iVar8 = FUN_00597850(*(undefined4 *)(param_1 + 0x7e0));
                                  *(int *)(param_1 + 0x7dc) = iVar8;
                                  if (iVar8 == 0) {
                                    param_2 = -1;
                                  }
                                  else {
                                    iVar1 = *(int *)(param_1 + 0x7e0);
                                    iVar8 = FUN_0059c359(iVar8,1,iVar1,param_2);
                                    if (iVar8 == iVar1) {
                                      iVar6 = iVar4 + iVar6 + 0x3a18 + iVar7 +
                                              *(int *)(param_1 + 0x7e0);
                                      iVar4 = FUN_0059c359(&local_58,4,1,param_2);
                                      if (iVar4 == 1) {
                                        puVar2 = *(undefined4 **)(param_1 + 0x7e8);
                                        local_54 = (undefined4 *)*puVar2;
                                        while (local_54 != puVar2) {
                                          puVar9 = (undefined4 *)FUN_00502420(local_48,0);
                                          FUN_004bfeb0(local_44,*puVar9);
                                        }
                                        param_2 = iVar6;
                                        if (0 < local_58) {
                                          FUN_00464c50();
                                          local_40 = 0;
                                          local_4 = 0;
                                          FUN_00464ca0(&DAT_006c4518);
                                          local_54 = (undefined4 *)0x0;
                                          if (0 < local_58) {
                                            do {
                                              iVar4 = FUN_0059c359(&local_4c,4,1,uVar3);
                                              if (iVar4 != 1) goto LAB_005023a1;
                                              if (local_4c == -1) {
                                                local_40 = 0;
                                              }
                                              else {
                                                local_40 = DAT_006c54dc + local_4c * 0xc;
                                              }
                                              iVar4 = FUN_004652a0(uVar3);
                                              if ((iVar4 == -1) || (iVar4 + 4 == -1))
                                              goto LAB_005023a1;
                                              iVar6 = *(int *)(param_1 + 0x7e8);
                                              param_2 = param_2 + iVar4 + 4;
                                              iVar4 = FUN_00502490(iVar6,*(undefined4 *)(iVar6 + 4))
                                              ;
                                              *(int *)(iVar6 + 4) = iVar4;
                                              **(int **)(iVar4 + 4) = iVar4;
                                              FUN_005027e0();
                                              local_54 = (undefined4 *)((int)local_54 + 1);
                                              *(int *)(param_1 + 0x7ec) =
                                                   *(int *)(param_1 + 0x7ec) + 1;
                                            } while ((int)local_54 < local_58);
                                          }
                                          local_4 = 0xffffffff;
                                          local_40 = 0;
                                        }
                                        iVar4 = FUN_0059c359(param_1 + 0x7f0,4,1,uVar3);
                                        if (iVar4 == 1) {
                                          iVar4 = FUN_0059c359(param_1 + 0x7f4,4,1,uVar3);
                                          if (iVar4 == 1) {
                                            iVar4 = FUN_0059c359(param_1 + 0x7f8,4,1,uVar3);
                                            if (iVar4 == 1) {
                                              iVar4 = FUN_0059c359(param_1 + 0x7fc,4,1,uVar3);
                                              if (iVar4 == 1) {
                                                iVar4 = FUN_0059c359(param_1 + 0x800,4,1,uVar3);
                                                if (iVar4 == 1) {
                                                  iVar4 = FUN_0049f490(uVar3);
                                                  if (iVar4 == -1) {
LAB_005023a1:
                                                    param_2 = -1;
                                                  }
                                                  else {
                                                    DAT_005e41a8 = 1;
                                                    FUN_004f1890();
                                                    DAT_005e1780 = DAT_006c549c;
                                                    DAT_005e1784 = DAT_006c549c << 4;
                                                    DAT_006e1fa0 = 0;
                                                    DAT_006e1fa4 = 0;
                                                    FUN_00545570(DAT_005e1784);
                                                    param_2 = param_2 + 0x14;
                                                  }
                                                }
                                                else {
                                                  param_2 = -1;
                                                }
                                              }
                                              else {
                                                param_2 = -1;
                                              }
                                            }
                                            else {
                                              param_2 = -1;
                                            }
                                          }
                                          else {
                                            param_2 = -1;
                                          }
                                        }
                                        else {
                                          param_2 = -1;
                                        }
                                      }
                                      else {
                                        param_2 = -1;
                                      }
                                    }
                                    else {
                                      param_2 = -1;
                                    }
                                  }
                                }
                              }
                            }
                          }
                        }
                        else {
                          param_2 = -1;
                        }
                      }
                      else {
                        param_2 = -1;
                      }
                    }
                    else {
                      param_2 = -1;
                    }
                  }
                  else {
                    param_2 = -1;
                  }
                }
                else {
                  param_2 = -1;
                }
              }
              else {
                param_2 = -1;
              }
            }
            else {
              param_2 = -1;
            }
          }
        }
      }
      else {
        param_2 = -1;
      }
    }
  }
  else {
    param_2 = -1;
  }
  ExceptionList = local_c;
  return param_2;
}

