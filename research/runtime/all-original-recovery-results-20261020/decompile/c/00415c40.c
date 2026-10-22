
void __thiscall FUN_00415c40(int *param_1,int param_2)

{
  undefined8 uVar1;
  char cVar2;
  int *piVar3;
  int iVar4;
  undefined4 *puVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  bool bVar9;
  int local_20 [2];
  int local_18;
  undefined1 local_14 [4];
  undefined4 local_10;
  int local_c;
  int *local_8;
  
  param_1[5] = param_1[5] + 1;
  FUN_004181d0(1);
  if (*param_1 == 1) {
    if (DAT_006e1f4c == 0) {
      FUN_0041a060();
    }
    if (DAT_006e1f50 == 0) {
      FUN_0041d270();
    }
  }
  iVar7 = param_2;
  if (((DAT_006e1f54 == 0) || (*param_1 != 0)) && ((DAT_006e1f58 == 0 || (*param_1 != 1)))) {
    iVar8 = param_2 / 2;
    local_c = iVar8;
    FUN_004075b0(iVar8);
    DAT_005d854c = 2;
    if (*(int *)((int)param_1 + 0x11dab) == 0) {
      FUN_00428b40();
      piVar3 = *(int **)((int)param_1 + 0x1e);
      if (piVar3 != *(int **)((int)param_1 + 0x22)) {
        do {
          iVar4 = *piVar3;
          if (*(int *)(iVar4 + 0x35) != 0) {
            if (*(int *)(iVar4 + 0x89) == 1) {
              *(undefined4 *)(iVar4 + 0x89) = 3;
            }
            else if (*(int *)(iVar4 + 0x89) == 2) {
              *(undefined4 *)(iVar4 + 0x89) = 0;
            }
          }
          piVar3 = piVar3 + 1;
        } while (piVar3 != *(int **)((int)param_1 + 0x22));
      }
      FUN_00417580();
      FUN_00428e40();
      FUN_004181d0(2);
      *(undefined1 *)((int)param_1 + 0x19) = 1;
    }
    else {
      FUN_00428f80();
      *(undefined1 *)((int)param_1 + 0x19) = 0;
    }
    if ((char)param_1[6] == '\0') {
      if ((((*(char *)((int)param_1 + 0x11daa) != '\0') || (*(int *)((int)param_1 + 0x11db7) == 0))
          || ((*(int *)((int)param_1 + 0x11dbb) - *(int *)((int)param_1 + 0x11db7) & 0xfffffffcU) ==
              0)) && ((int)param_1 + 0x11db3 != (int)param_1 + 0x1a)) {
        if (*(int *)((int)param_1 + 0x1e) == 0) {
          local_8 = (int *)0x0;
        }
        else {
          local_8 = (int *)(*(int *)((int)param_1 + 0x22) - *(int *)((int)param_1 + 0x1e) >> 2);
        }
        piVar3 = (int *)FUN_00429790();
        if (piVar3 < local_8) {
          local_8 = (int *)FUN_00429790();
          piVar3 = (int *)FUN_0042a180();
          if (piVar3 < local_8) {
            FUN_0042a590(*(undefined4 *)((int)param_1 + 0x11db7),
                         *(undefined4 *)((int)param_1 + 0x11dbb));
            FUN_00597870(*(undefined4 *)((int)param_1 + 0x11db7));
            iVar7 = FUN_00429790();
            if (iVar7 < 0) {
              iVar7 = 0;
            }
            uVar6 = FUN_00597850(iVar7 << 2);
            *(undefined4 *)((int)param_1 + 0x11db7) = uVar6;
            uVar6 = FUN_0042a5a0(*(undefined4 *)((int)param_1 + 0x1e),
                                 *(undefined4 *)((int)param_1 + 0x22),uVar6);
            *(undefined4 *)((int)param_1 + 0x11dbb) = uVar6;
            *(undefined4 *)((int)param_1 + 0x11dbf) = uVar6;
          }
          else {
            local_8 = *(int **)((int)param_1 + 0x1e);
            iVar7 = FUN_00429790();
            local_8 = (int *)((int)local_8 + iVar7 * 4);
            FUN_0042fac0(*(undefined4 *)((int)param_1 + 0x11db7));
            FUN_0042a5a0(local_8,*(undefined4 *)((int)param_1 + 0x22),
                         *(undefined4 *)((int)param_1 + 0x11dbb));
            iVar7 = FUN_00429790();
            *(int *)((int)param_1 + 0x11dbb) = *(int *)((int)param_1 + 0x11db7) + iVar7 * 4;
          }
        }
        else {
          uVar6 = FUN_0042fac0(*(undefined4 *)((int)param_1 + 0x11db7));
          FUN_0042a590(uVar6,*(undefined4 *)((int)param_1 + 0x11dbb));
          iVar7 = FUN_00429790();
          *(int *)((int)param_1 + 0x11dbb) = *(int *)((int)param_1 + 0x11db7) + iVar7 * 4;
        }
      }
LAB_00415e95:
      iVar4 = *(int *)((int)param_1 + 0x56);
      if (((iVar4 == 0) && (*(int *)((int)param_1 + 0x86) == 0)) &&
         ((*(int *)((int)param_1 + 0xb6) == 0 &&
          ((iVar7 = param_2, iVar8 = local_c, *(int *)((int)param_1 + 0x11db7) == 0 ||
           ((*(int *)((int)param_1 + 0x11dbb) - *(int *)((int)param_1 + 0x11db7) & 0xfffffffcU) == 0
           )))))) goto LAB_00416384;
      iVar7 = *(int *)((int)param_1 + 0x11dab);
      if ((iVar7 != 2) && ((iVar7 != 3 && (iVar7 != 4)))) {
        while (iVar4 != 0) {
          *(undefined4 *)((int)param_1 + 0x11dab) = 1;
          piVar3 = *(int **)((int)param_1 + 0x46);
          FUN_0042d830(0xffffffff);
          iVar7 = *piVar3;
          *(int *)((int)param_1 + 0x11daf) = iVar7;
          if (*(int *)(iVar7 + 5) == 0) {
            FUN_00421f70(iVar7,local_20);
            iVar4 = FUN_00407600();
            iVar7 = param_2;
            iVar8 = local_c;
            if (iVar4 < 1) goto LAB_00416384;
          }
          if (*(int *)((int)param_1 + 0x46) == *(int *)((int)param_1 + 0x3e)) {
            FUN_0042b170();
          }
          if (*(int *)((int)param_1 + 0x56) != 0) {
            *(int *)((int)param_1 + 0x46) = *(int *)((int)param_1 + 0x46) + -4;
          }
          iVar7 = *(int *)((int)param_1 + 0x56) + -1;
          *(int *)((int)param_1 + 0x56) = iVar7;
          if (iVar7 == 0) {
            FUN_0042b170();
          }
          *(undefined4 *)((int)param_1 + 0x11dab) = 0;
          *(undefined4 *)((int)param_1 + 0x11daf) = 0;
          if (*(char *)((int)param_1 + 0x19) == '\0') {
            FUN_00428f80();
            FUN_00428b40();
            piVar3 = *(int **)((int)param_1 + 0x1e);
            if (piVar3 != *(int **)((int)param_1 + 0x22)) {
              do {
                iVar7 = *piVar3;
                if (*(int *)(iVar7 + 0x35) != 0) {
                  if (*(int *)(iVar7 + 0x89) == 1) {
                    *(undefined4 *)(iVar7 + 0x89) = 3;
                  }
                  else if (*(int *)(iVar7 + 0x89) == 2) {
                    *(undefined4 *)(iVar7 + 0x89) = 0;
                  }
                }
                piVar3 = piVar3 + 1;
              } while (piVar3 != *(int **)((int)param_1 + 0x22));
            }
            FUN_00417580();
            FUN_00428e40();
            FUN_004181d0(2);
            *(undefined1 *)((int)param_1 + 0x19) = 1;
          }
          iVar4 = *(int *)((int)param_1 + 0x56);
        }
      }
      iVar7 = *(int *)((int)param_1 + 0x11dab);
      if (iVar7 != 3) {
        if (iVar7 == 4) goto LAB_00416268;
        if (*(int *)((int)param_1 + 0x86) == 0) goto LAB_00416144;
        *(undefined4 *)((int)param_1 + 0x11dab) = 2;
        puVar5 = *(undefined4 **)((int)param_1 + 0x76);
        FUN_0042d830(0xffffffff);
        uVar6 = *puVar5;
        *(undefined4 *)((int)param_1 + 0x11daf) = uVar6;
        FUN_00423590(uVar6,local_20);
        iVar4 = FUN_00407600();
        iVar7 = param_2;
        iVar8 = local_c;
        if (0 < iVar4) goto code_r0x004160aa;
        goto LAB_00416384;
      }
LAB_00416144:
      if ((iVar7 != 4) && (*(int *)((int)param_1 + 0xb6) != 0)) {
        *(undefined4 *)((int)param_1 + 0x11dab) = 3;
        puVar5 = *(undefined4 **)((int)param_1 + 0xa6);
        FUN_0042d830(0xffffffff);
        uVar6 = *puVar5;
        *(undefined4 *)((int)param_1 + 0x11daf) = uVar6;
        FUN_00423590(uVar6,local_20);
        iVar4 = FUN_00407600();
        iVar7 = param_2;
        iVar8 = local_c;
        if (iVar4 < 1) goto LAB_00416384;
        if (*(int *)((int)param_1 + 0xa6) == *(int *)((int)param_1 + 0x9e)) {
          FUN_0042b170();
        }
        if (*(int *)((int)param_1 + 0xb6) != 0) {
          *(int *)((int)param_1 + 0xa6) = *(int *)((int)param_1 + 0xa6) + -4;
        }
        iVar7 = *(int *)((int)param_1 + 0xb6) + -1;
        *(int *)((int)param_1 + 0xb6) = iVar7;
        if (iVar7 == 0) {
          FUN_0042b170();
        }
        *(undefined4 *)((int)param_1 + 0x11dab) = 0;
        *(undefined4 *)((int)param_1 + 0x11daf) = 0;
        if (*(char *)((int)param_1 + 0x19) == '\0') {
          FUN_00428f80();
          FUN_00428b40();
          piVar3 = *(int **)((int)param_1 + 0x1e);
          if (piVar3 != *(int **)((int)param_1 + 0x22)) {
            do {
              iVar7 = *piVar3;
              if (*(int *)(iVar7 + 0x35) != 0) {
                if (*(int *)(iVar7 + 0x89) == 1) {
                  *(undefined4 *)(iVar7 + 0x89) = 3;
                }
                else if (*(int *)(iVar7 + 0x89) == 2) {
                  *(undefined4 *)(iVar7 + 0x89) = 0;
                }
              }
              piVar3 = piVar3 + 1;
            } while (piVar3 != *(int **)((int)param_1 + 0x22));
          }
          goto LAB_0041635e;
        }
        goto LAB_00415e95;
      }
LAB_00416268:
      if ((*(int *)((int)param_1 + 0x11db7) == 0) ||
         ((*(int *)((int)param_1 + 0x11dbb) - *(int *)((int)param_1 + 0x11db7) & 0xfffffffcU) == 0))
      goto LAB_00415e95;
      *(undefined4 *)((int)param_1 + 0x11dab) = 4;
      uVar6 = *(undefined4 *)(*(int *)((int)param_1 + 0x11dbb) + -4);
      *(undefined4 *)((int)param_1 + 0x11daf) = uVar6;
      FUN_00423590(uVar6,local_20);
      iVar4 = FUN_00407600();
      iVar7 = param_2;
      iVar8 = local_c;
      if (iVar4 < 1) goto LAB_00416384;
      FUN_0042f220(*(int *)((int)param_1 + 0x11dbb) + -4);
      FUN_0042a590(*(int *)((int)param_1 + 0x11dbb) + -4,*(int *)((int)param_1 + 0x11dbb));
      *(int *)((int)param_1 + 0x11dbb) = *(int *)((int)param_1 + 0x11dbb) + -4;
      *(undefined4 *)((int)param_1 + 0x11dab) = 0;
      *(undefined4 *)((int)param_1 + 0x11daf) = 0;
      if (*(char *)((int)param_1 + 0x19) != '\0') goto LAB_00415e95;
      FUN_00428f80();
      FUN_00428b40();
      piVar3 = *(int **)((int)param_1 + 0x1e);
      if (piVar3 != *(int **)((int)param_1 + 0x22)) {
        do {
          iVar7 = *piVar3;
          if (*(int *)(iVar7 + 0x35) != 0) {
            if (*(int *)(iVar7 + 0x89) == 1) {
              *(undefined4 *)(iVar7 + 0x89) = 3;
            }
            else if (*(int *)(iVar7 + 0x89) == 2) {
              *(undefined4 *)(iVar7 + 0x89) = 0;
            }
          }
          piVar3 = piVar3 + 1;
        } while (piVar3 != *(int **)((int)param_1 + 0x22));
      }
      goto LAB_0041635e;
    }
    FUN_00428b40();
LAB_00416384:
    FUN_004075b0(iVar7 - iVar8);
    if (*(char *)((int)param_1 + 0x19) == '\0') {
      FUN_00428f80();
    }
    DAT_005d854c = 3;
    if ((*(char *)((int)param_1 + 0x11daa) != '\0') || (*(int *)((int)param_1 + 0x11dcf) == 0)) {
      if ((int)param_1 + 0x11dc3 != (int)param_1 + 0xba) {
        uVar6 = *(undefined4 *)((int)param_1 + 0x11dc7);
        puVar5 = (undefined4 *)FUN_0042b200(&param_2);
        FUN_0042b6f0(&local_10,*puVar5,uVar6);
        *(undefined1 *)(param_1 + 0x4771) = *(undefined1 *)((int)param_1 + 0xbb);
        FUN_0042d8f0((int)param_1 + 0xba);
      }
      iVar7 = *(int *)((int)param_1 + 0x11dcf);
      goto joined_r0x004163fe;
    }
    do {
      iVar7 = local_18;
      if (DAT_006e1f64 != 0) {
        if (DAT_006e1f64 != 1) break;
        if (DAT_006a4910 != 0) {
          uVar1 = rdtsc();
          local_c = (int)uVar1;
          iVar7 = local_20[0] - (uint)(local_c - (int)((ulonglong)uVar1 >> 0x20)) / DAT_006a4914;
          if (iVar7 < 1) break;
        }
      }
      if (iVar7 < 1) break;
      local_8 = *(int **)(**(int **)((int)param_1 + 0x11dc7) + 0x10);
      if ((param_1[5] < local_8[4]) && ((char)local_8[5] == '\0')) {
        local_10 = DAT_005d854c;
        DAT_005d854c = 3;
        uVar6 = FUN_00407920();
        iVar7 = FUN_004079f0(uVar6);
        param_2 = CONCAT13(iVar7 != 0,(undefined3)param_2);
        iVar7 = FUN_004079f0(uVar6);
        piVar3 = local_8;
        while (local_8 = piVar3, iVar7 != 0) {
          iVar7 = FUN_004079f0(uVar6);
          piVar3 = local_8;
        }
        param_2._3_1_ = (char)((uint)param_2 >> 0x18);
        bVar9 = param_2._3_1_ != '\0';
        DAT_005d854c = local_10;
        if (bVar9) {
          cVar2 = (**(code **)(*piVar3 + 0x14))((int)param_1 + 0x11c02,param_1[5],local_20);
          *(bool *)(piVar3 + 5) = cVar2 == '\0';
        }
      }
      FUN_0042b320(local_14,**(undefined4 **)((int)param_1 + 0x11dc7));
      iVar7 = *(int *)((int)param_1 + 0x11dcf);
joined_r0x004163fe:
    } while (iVar7 != 0);
    FUN_00417580();
    *(undefined1 *)((int)param_1 + 0x11daa) = 0;
  }
  return;
code_r0x004160aa:
  if (*(int *)((int)param_1 + 0x76) == *(int *)((int)param_1 + 0x6e)) {
    FUN_0042b170();
  }
  if (*(int *)((int)param_1 + 0x86) != 0) {
    *(int *)((int)param_1 + 0x76) = *(int *)((int)param_1 + 0x76) + -4;
  }
  iVar7 = *(int *)((int)param_1 + 0x86) + -1;
  *(int *)((int)param_1 + 0x86) = iVar7;
  if (iVar7 == 0) {
    FUN_0042b170();
  }
  *(undefined4 *)((int)param_1 + 0x11dab) = 0;
  *(undefined4 *)((int)param_1 + 0x11daf) = 0;
  if (*(char *)((int)param_1 + 0x19) != '\0') goto LAB_00415e95;
  FUN_00428f80();
  FUN_00428b40();
  piVar3 = *(int **)((int)param_1 + 0x1e);
  if (piVar3 != *(int **)((int)param_1 + 0x22)) {
    do {
      iVar7 = *piVar3;
      if (*(int *)(iVar7 + 0x35) != 0) {
        if (*(int *)(iVar7 + 0x89) == 1) {
          *(undefined4 *)(iVar7 + 0x89) = 3;
        }
        else if (*(int *)(iVar7 + 0x89) == 2) {
          *(undefined4 *)(iVar7 + 0x89) = 0;
        }
      }
      piVar3 = piVar3 + 1;
    } while (piVar3 != *(int **)((int)param_1 + 0x22));
  }
LAB_0041635e:
  FUN_00417580();
  FUN_00428e40();
  FUN_004181d0(2);
  *(undefined1 *)((int)param_1 + 0x19) = 1;
  goto LAB_00415e95;
}

