
undefined4 __fastcall FUN_0051a600(uint *param_1)

{
  uint uVar1;
  undefined4 uVar2;
  int iVar3;
  int iVar4;
  uint uVar5;
  uint uVar6;
  uint extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 uVar7;
  
  if ((DAT_006df184 != 1) || (DAT_006df1a0 != 0)) {
    if (param_1[0x17b] == 3) {
      if (param_1[0x193] < DAT_006def5c) {
        iVar3 = param_1[0x193] * 0xe4b + DAT_006def58;
      }
      else {
        iVar3 = 0;
      }
      iVar3 = FUN_0050bd60(iVar3,param_1[2],param_1[3],param_1[4]);
      if (iVar3 != 0) {
        return 0;
      }
    }
    else if (((param_1[0x17b] == 0xb) &&
             ((uVar1 = param_1[0x1a1], uVar1 == 1 || ((2 < (int)uVar1 && ((int)uVar1 < 6)))))) &&
            (param_1[0x193] != 0xffffffff)) {
      uVar2 = FUN_004970b0(param_1[0x193]);
      iVar3 = FUN_0050bd40(uVar2);
      if (iVar3 != 0) {
        return 0;
      }
    }
    iVar3 = FUN_0051ae60();
    if (iVar3 != 0) {
      uVar1 = iVar3 - 1;
      iVar4 = FUN_0051b710(uVar1);
      if (iVar4 != 0) {
        if (iVar3 - 1U < DAT_006def5c) {
          iVar4 = (iVar3 - 1U) * 0xe4b + DAT_006def58;
        }
        else {
          iVar4 = 0;
        }
        uVar6 = DAT_006def5c;
        if (((((DAT_006df184 == 0) && (*(int *)(iVar4 + 0x5ec) == 0xd)) &&
             (uVar6 = *param_1, *(uint *)(iVar4 + 0x6ec) == uVar6)) &&
            ((uVar6 = *(uint *)(iVar4 + 8), uVar6 == *(uint *)(iVar4 + 0x6f8) &&
             (uVar6 = *(uint *)(iVar4 + 0x6fc), *(uint *)(iVar4 + 0xc) == uVar6)))) &&
           (uVar6 = *(uint *)(iVar4 + 0x10), uVar6 == *(uint *)(iVar4 + 0x700))) {
          uVar5 = FUN_0054e200();
          uVar6 = 100;
          if (DAT_006e1f30 != 0) {
            FUN_00485b10(s_c__Projects_Chaos_Mapelem_cpp_005e9c10,0x333b);
            uVar6 = extraout_ECX;
          }
          if (9 < (uVar5 & 0x7fffffff) % 100) {
            FUN_0050db60(0xf);
            param_1[0x19b] = 6;
            *(undefined4 *)((int)param_1 + 0xb8b) = 1;
            param_1[0x1a2] = 0;
            return 1;
          }
        }
        uVar2 = *(undefined4 *)((int)param_1 + 0x973);
        FUN_0040e8a0(*(undefined4 *)((int)param_1 + 0x96f));
        uVar7 = extraout_ECX_00;
        FUN_0040e290(*(undefined4 *)((int)param_1 + 0x96b));
        iVar4 = FUN_0051b130(uVar7,uVar6,uVar2);
        if (iVar4 == iVar3) {
          iVar3 = 0;
          DAT_006df184 = 0;
        }
        else {
          iVar3 = FUN_0051b300();
          if (DAT_006df184 != 0) {
            DAT_006df1a0 = 1;
            return 1;
          }
        }
        DAT_006df1a0 = 0;
        if (iVar3 == 0) {
          if ((param_1[0x17b] == 0xd) && (param_1[0x1bb] == uVar1)) {
            param_1[0x1a2] = 0;
            FUN_0050ab30();
            *(uint *)((int)param_1 + 0x96b) = param_1[2];
            *(uint *)((int)param_1 + 0x96f) = param_1[3];
            *(uint *)((int)param_1 + 0x973) = param_1[4];
            return 1;
          }
          iVar3 = FUN_0051b7f0(uVar1);
          if (iVar3 != 0) {
            param_1[0x1a2] = 0;
            return 1;
          }
        }
      }
    }
  }
  return 0;
}

