
void __fastcall FUN_0052a780(int *param_1)

{
  int iVar1;
  bool bVar2;
  int iVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  undefined4 *puVar6;
  undefined4 uVar7;
  undefined4 uVar8;
  undefined4 local_54;
  int local_50;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined1 local_2c;
  undefined4 local_2b;
  undefined1 local_27;
  undefined4 local_26;
  undefined4 local_22;
  undefined4 local_1e;
  undefined4 local_1a;
  undefined4 local_a;
  undefined4 local_6;
  
  iVar3 = 0;
  bVar2 = false;
  param_1[0x8c] = 0x35;
  if (DAT_006e1f44 != 0) {
    param_1[0x8c] = 0;
  }
  do {
    if ((bVar2) || (param_1[0x8c] < 1)) break;
    if ((uint)param_1[0x96] < (uint)param_1[1]) {
      iVar5 = param_1[0x96] * 0xe4b + *param_1;
    }
    else {
      iVar5 = 0;
    }
    if ((*(int *)(iVar5 + 4) != 0) && (*(int *)(iVar5 + 0xe4) != 0)) {
      iVar3 = FUN_00519ae0();
      if (iVar3 == 0) {
        if (param_1[0x8b] == 0) goto LAB_0052a7fa;
        param_1[0x8b] = 0;
      }
      if (param_1[0x8b] != 0) break;
    }
LAB_0052a7fa:
    iVar5 = param_1[0x96];
    param_1[0x96] = iVar5 + 1;
    if (param_1[0x8a] <= iVar5 + 1) {
      param_1[0x96] = 0;
      bVar2 = true;
    }
  } while (iVar3 == 0);
  iVar3 = *param_1;
  iVar5 = 0;
  if (0 < param_1[0x8a]) {
    do {
      if ((*(int *)(iVar3 + 4) != 0) && (*(int *)(iVar3 + 0xe4) != 0)) {
        if (param_1[0x97] == iVar5 % 0x14) {
          FUN_00522ad0();
          FUN_00518420();
          FUN_005244f0();
          if ((DAT_005e1410 != 0) && (*(int *)(iVar3 + 0xa8) != 0x16)) {
            local_48 = 0;
            local_44 = 0;
            local_40 = 0;
            local_3c = 0;
            local_38 = 0;
            local_34 = 0;
            local_30 = 0;
            local_2c = 0;
            local_2b = 0;
            local_27 = 0;
            local_26 = 0;
            local_22 = 0;
            local_1e = 0;
            local_1a = 0;
            local_a = 0;
            local_6 = 0;
            local_54 = 0;
            local_50 = iVar3;
            FUN_00441560(&local_54);
            uVar8 = extraout_ECX;
            FUN_0040eeb0(*(undefined4 *)(iVar3 + 0x10));
            uVar7 = extraout_ECX_00;
            FUN_0040e8a0(*(undefined4 *)(iVar3 + 0xc));
            uVar4 = extraout_ECX_01;
            FUN_0040e290(*(undefined4 *)(iVar3 + 8));
            uVar4 = FUN_0053d7d0(uVar4,uVar7,uVar8);
            *(undefined4 *)(iVar3 + 0xde7) = uVar4;
          }
        }
        if ((DAT_0068991c == 0) && (param_1[0x99] == iVar5 % 0x5a)) {
          iVar1 = *(int *)(iVar3 + 0x174);
          if (iVar1 == -1) {
            puVar6 = (undefined4 *)0x0;
          }
          else {
            puVar6 = &DAT_00643a80 + iVar1 * 0x55;
          }
          if (((puVar6[3] == -1) || (*(int *)(iVar3 + 0x704) != 0)) && (iVar1 != DAT_00644520)) {
            FUN_00523470();
          }
        }
      }
      iVar5 = iVar5 + 1;
      iVar3 = iVar3 + 0xe4b;
    } while (iVar5 < param_1[0x8a]);
  }
  iVar3 = param_1[0x97];
  param_1[0x97] = iVar3 + 1;
  if (0x13 < iVar3 + 1) {
    param_1[0x97] = 0;
  }
  iVar3 = param_1[0x99];
  param_1[0x99] = iVar3 + 1;
  if (0x59 < iVar3 + 1) {
    param_1[0x99] = 0;
  }
  return;
}

