
void __fastcall FUN_00514040(int *param_1)

{
  int iVar1;
  uint uVar2;
  int iVar3;
  int *piVar4;
  uint uVar5;
  
  iVar1 = DAT_006b9f46;
  if (param_1[0x39] != 0) {
    DAT_006def64 = DAT_006def58;
    DAT_006def60 = 0;
    uVar5 = DAT_006df180;
    while (piVar4 = DAT_006def64, DAT_006def60 < uVar5) {
      while (piVar4[1] == 0) {
        DAT_006def60 = DAT_006def60 + 1;
        piVar4 = (int *)((int)piVar4 + 0xe4b);
        if (uVar5 <= DAT_006def60) {
          DAT_006def64 = piVar4;
          return;
        }
      }
      DAT_006def64 = (int *)((int)piVar4 + 0xe4b);
      DAT_006def60 = DAT_006def60 + 1;
      if (piVar4 == (int *)0x0) {
        return;
      }
      if ((((piVar4 != param_1) && (piVar4[0x39] != 0)) &&
          (iVar3 = *(int *)piVar4[0x2b], iVar3 != 0)) &&
         ((((iVar3 != 0x18 && (iVar3 != 0x19)) &&
           ((iVar3 != 0x1a && ((piVar4[0x193] != *param_1 && (piVar4[0x1e1] == 0)))))) &&
          (*(int *)((int)piVar4 + 0xde3) == 0)))) {
        uVar2 = FUN_0054e200();
        if (DAT_006e1f30 != 0) {
          FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x26a1);
        }
        iVar3 = FUN_00515270(param_1);
        uVar5 = DAT_006df180;
        if (((iVar3 != 0) &&
            ((uVar2 & 0x7fffffff) % 100 < (uint)(iVar1 * 100) / (uint)(param_1[0x22] + iVar1))) &&
           (iVar3 = FUN_0050ad10(param_1), uVar5 = DAT_006df180, iVar3 != 0)) {
          piVar4[0x1e1] = 1;
          uVar5 = DAT_006df180;
        }
      }
    }
  }
  return;
}

