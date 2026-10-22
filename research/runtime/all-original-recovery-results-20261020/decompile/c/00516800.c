
undefined4 __fastcall FUN_00516800(int *param_1)

{
  bool bVar1;
  uint uVar2;
  int iVar3;
  int iVar4;
  undefined4 *puVar5;
  int *piVar6;
  int local_c;
  int *local_4;
  
  bVar1 = false;
  local_c = -1;
  if (((param_1[0x28] == 0) &&
      (((param_1[0x1e6] == 0 || (*(int *)(param_1[0x2b] + 0x3c) != 0)) ||
       (*(int *)((int)param_1 + 0x7cd) == 3)))) &&
     (((*(int *)((int)param_1 + 0xde3) == 0 && ((char)param_1[0x1c7] == '\0')) &&
      (*(char *)((int)param_1 + 0x723) == '\0')))) {
    uVar2 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x2ab3);
    }
    if ((uVar2 & 0x7fffffff) % 100 < 0xb) {
      switch(param_1[0x2a]) {
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
      case 0x10:
      case 0x12:
      case 0x13:
      case 0x14:
      case 0x15:
      case 0x17:
        DAT_006def60 = 0;
        DAT_006def64 = DAT_006def58;
        while (piVar6 = DAT_006def64, DAT_006def60 < DAT_006df180) {
          while (piVar6[1] == 0) {
            DAT_006def60 = DAT_006def60 + 1;
            piVar6 = (int *)((int)piVar6 + 0xe4b);
            DAT_006def64 = piVar6;
            if (DAT_006df180 <= DAT_006def60) goto LAB_00516a88;
          }
          DAT_006def64 = (int *)((int)piVar6 + 0xe4b);
          DAT_006def60 = DAT_006def60 + 1;
          if (piVar6 == (int *)0x0) break;
          if ((piVar6[1] != 0) && (*param_1 != *piVar6)) {
            iVar3 = param_1[0x5d];
            if (iVar3 == -1) {
              puVar5 = (undefined4 *)0x0;
            }
            else {
              puVar5 = &DAT_00643a80 + iVar3 * 0x55;
            }
            if (((((*(char *)((int)puVar5 + piVar6[0x5d] + 0x18) == '\0') && (piVar6[0x39] != 0)) &&
                 (-1 < iVar3)) && ((iVar3 < 8 && (piVar6[iVar3 + 9] != 0)))) &&
               (iVar3 = *(int *)(piVar6[0x2b] + 0x198),
               iVar3 = FUN_005151b0(piVar6[5] + iVar3,piVar6[6] + iVar3,
                                    piVar6[7] + *(int *)(piVar6[0x2b] + 0x19c)), iVar3 != 0)) {
              iVar4 = FUN_004f4b30(param_1[4],piVar6[2],piVar6[3],piVar6[4]);
              iVar3 = *(int *)(param_1[0x2b] + 0x2f0 + piVar6[0x2a] * 4);
              if (iVar4 <= iVar3 * 2) {
                uVar2 = FUN_0054e200();
                if (DAT_006e1f30 != 0) {
                  FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x2af4);
                }
                iVar3 = iVar3 * 4 + (uVar2 & 0x7fffffff) % 0x32;
                if (local_c < iVar3) {
                  bVar1 = true;
                  local_c = iVar3;
                  local_4 = piVar6;
                }
              }
            }
          }
        }
LAB_00516a88:
        if (bVar1) {
          FUN_005084f0(local_4[5] + *(int *)(local_4[0x2b] + 0x198),
                       local_4[6] + *(int *)(local_4[0x2b] + 0x198));
          local_4[0x1a0] = *param_1 + 1;
          return 1;
        }
      }
    }
  }
  return 0;
}

