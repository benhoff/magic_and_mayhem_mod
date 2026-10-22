
undefined4 __fastcall FUN_00516220(int *param_1)

{
  bool bVar1;
  int iVar2;
  uint uVar3;
  undefined4 *puVar4;
  int iVar5;
  int *piVar6;
  int local_10;
  int *local_4;
  
  bVar1 = false;
  local_10 = -1;
  if (((param_1[0x28] == 0) &&
      (((param_1[0x1e6] == 0 || (*(int *)(param_1[0x2b] + 0x3c) != 0)) ||
       (*(int *)((int)param_1 + 0x7cd) == 3)))) &&
     (((*(int *)((int)param_1 + 0xde3) == 0 && ((char)param_1[0x1c7] == '\0')) &&
      (*(char *)((int)param_1 + 0x723) == '\0')))) {
    DAT_006def60 = 0;
    DAT_006def64 = DAT_006def58;
    while (piVar6 = DAT_006def64, DAT_006def60 < DAT_006df180) {
      while (piVar6[1] == 0) {
        DAT_006def60 = DAT_006def60 + 1;
        piVar6 = (int *)((int)piVar6 + 0xe4b);
        DAT_006def64 = piVar6;
        if (DAT_006df180 <= DAT_006def60) goto LAB_005164bc;
      }
      DAT_006def64 = (int *)((int)piVar6 + 0xe4b);
      DAT_006def60 = DAT_006def60 + 1;
      if (piVar6 == (int *)0x0) break;
      if ((piVar6[1] != 0) && (*param_1 != *piVar6)) {
        iVar2 = param_1[0x5d];
        if (iVar2 == -1) {
          puVar4 = (undefined4 *)0x0;
        }
        else {
          puVar4 = &DAT_00643a80 + iVar2 * 0x55;
        }
        if ((((*(char *)((int)puVar4 + piVar6[0x5d] + 0x18) == '\0') && (piVar6[0x39] != 0)) &&
            ((((-1 < iVar2 && ((iVar2 < 8 && (piVar6[iVar2 + 9] != 0)))) ||
              ((param_1[0x23] != 0 && (piVar6[0x23] != 0)))) &&
             (iVar2 = FUN_0050bd60(piVar6,param_1[2],param_1[3],param_1[4]), iVar2 != 0)))) &&
           ((piVar6[0x186] == 0 ||
            (((piVar6[0x17b] != 3 &&
              ((piVar6[0x17b] != 0xb || ((piVar6[0x1a1] != 5 && (piVar6[0x1a1] != 1)))))) ||
             (piVar6[0x193] != *param_1)))))) {
          iVar2 = 500 - piVar6[0x39];
          if (iVar2 < 0) {
            iVar2 = 0;
          }
          if ((piVar6[0x3e] == 0) || (param_1[0x3e] != 0)) {
            iVar5 = *(int *)(param_1[0x2b] + 0x1a0 + piVar6[0x2a] * 4);
          }
          else {
            iVar5 = *(int *)(param_1[0x2b] + 0x1a0 + piVar6[0x2a] * 4) / 3;
          }
          iVar2 = iVar5 * 5 + iVar2;
          iVar5 = *(int *)piVar6[0x2b];
          if ((((iVar5 == 0) || (iVar5 == 0x18)) || (iVar5 == 0x19)) || (iVar5 == 0x1a)) {
            iVar2 = iVar2 + 100;
          }
          uVar3 = FUN_0054e200();
          if (DAT_006e1f30 != 0) {
            FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x2a30);
          }
          iVar2 = iVar2 + (uVar3 & 0x7fffffff) % 0x32;
          if (local_10 < iVar2) {
            bVar1 = true;
            local_10 = iVar2;
            local_4 = piVar6;
          }
        }
      }
    }
LAB_005164bc:
    if (bVar1) {
      param_1[0x193] = *local_4;
      return 1;
    }
  }
  return 0;
}

