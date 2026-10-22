
undefined4 __fastcall FUN_005164f0(int *param_1)

{
  bool bVar1;
  bool bVar2;
  int *piVar3;
  int *piVar4;
  int iVar5;
  uint uVar6;
  int iVar7;
  undefined4 *puVar8;
  int local_10;
  int *local_4;
  
  bVar2 = false;
  local_10 = -1;
  bVar1 = false;
  if (((param_1[0x28] == 0) &&
      (((param_1[0x1e6] == 0 || (*(int *)(param_1[0x2b] + 0x3c) != 0)) ||
       (*(int *)((int)param_1 + 0x7cd) == 3)))) &&
     ((*(int *)((int)param_1 + 0xde3) == 0 &&
      (((param_1[0x2a] != 0x12 || (param_1[0x42] == 0)) && (*(int *)(param_1[0x2b] + 0x34) != 0)))))
     ) {
    if (((param_1[0x2a] == 0x12) && (param_1[0x42] != 0)) ||
       ((DAT_006c4830 < (uint)param_1[0x58] || ((param_1[0x59] != 0 || (param_1[0x186] != 0)))))) {
      bVar1 = true;
    }
    DAT_006def60 = 0;
    piVar4 = DAT_006def58;
    do {
      do {
        do {
          do {
            while( true ) {
              piVar3 = piVar4;
              DAT_006def64 = piVar3;
              if (DAT_006df180 <= DAT_006def60) goto LAB_005167b9;
              if (piVar3[1] != 0) break;
              DAT_006def60 = DAT_006def60 + 1;
              piVar4 = (int *)((int)piVar3 + 0xe4b);
            }
            DAT_006def64 = (int *)((int)piVar3 + 0xe4b);
            DAT_006def60 = DAT_006def60 + 1;
            if (piVar3 == (int *)0x0) goto LAB_005167b9;
            piVar4 = DAT_006def64;
          } while ((piVar3[1] == 0) || (piVar4 = DAT_006def64, *param_1 == *piVar3));
          iVar5 = param_1[0x5d];
          if (iVar5 == -1) {
            puVar8 = (undefined4 *)0x0;
          }
          else {
            puVar8 = &DAT_00643a80 + iVar5 * 0x55;
          }
          piVar4 = DAT_006def64;
        } while ((((*(char *)((int)puVar8 + piVar3[0x5d] + 0x18) != '\0') ||
                  (iVar7 = piVar3[0x39], piVar4 = DAT_006def64, iVar7 == 0)) ||
                 (piVar4 = DAT_006def64, iVar5 < 0)) ||
                ((piVar4 = DAT_006def64, 7 < iVar5 ||
                 (piVar4 = DAT_006def64, piVar3[iVar5 + 9] == 0))));
        iVar5 = FUN_004f4b30(param_1[4],piVar3[2],piVar3[3],piVar3[4]);
        iVar5 = (700 - iVar7) + iVar5 * -0x14;
        if (iVar5 < 0) {
          iVar5 = 0;
        }
        if ((param_1[0x2a] == 8) || (param_1[0x2a] == 0x14)) {
          iVar7 = 10;
        }
        else if ((piVar3[0x3e] == 0) || (param_1[0x3e] != 0)) {
          iVar7 = *(int *)(param_1[0x2b] + 0x280 + piVar3[0x2a] * 4);
        }
        else {
          iVar7 = *(int *)(param_1[0x2b] + 0x280 + piVar3[0x2a] * 4) / 3;
        }
        iVar5 = iVar7 * 5 + iVar5;
        iVar7 = *(int *)piVar3[0x2b];
        if ((((iVar7 == 0) || (iVar7 == 0x18)) || (iVar7 == 0x19)) || (iVar7 == 0x1a)) {
          iVar5 = iVar5 + 100;
        }
        uVar6 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x2a7a);
        }
        iVar5 = iVar5 + (uVar6 & 0x7fffffff) % 0x32;
        piVar4 = DAT_006def64;
      } while (iVar5 <= local_10);
      iVar7 = FUN_00515620(piVar3,*(int *)(param_1[0x2b] + 0x34) << 5);
      if (iVar7 != 0) {
        bVar2 = true;
        local_10 = iVar5;
        local_4 = piVar3;
      }
      piVar4 = DAT_006def64;
    } while (!bVar1);
LAB_005167b9:
    if (bVar2) {
      iVar5 = *local_4;
      param_1[0x18e] = *param_1;
      param_1[0x193] = iVar5;
      param_1[399] = iVar5;
      param_1[0x187] = param_1[0x15c];
      return 1;
    }
  }
  return 0;
}

