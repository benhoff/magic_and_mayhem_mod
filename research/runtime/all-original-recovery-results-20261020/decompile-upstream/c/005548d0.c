
/* WARNING: Removing unreachable block (ram,0x00554c61) */

void __fastcall FUN_005548d0(int param_1)

{
  undefined4 *puVar1;
  ushort uVar2;
  uint uVar3;
  undefined4 uVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 *puVar7;
  int iVar8;
  uint uVar9;
  int iVar10;
  uint uVar11;
  int unaff_ESI;
  ushort *puVar12;
  ushort *local_50;
  int local_4c;
  ushort local_36 [27];
  
  *(undefined4 *)(param_1 + 0x658e) = 1;
  *(int *)(param_1 + 0x6178) = DAT_006c5494;
  *(uint *)(param_1 + 0x617c) = DAT_006c5498;
  *(int *)(param_1 + 0xd6) = (*(int *)(param_1 + 0xca) - *(int *)(param_1 + 0xe2)) + -8;
  local_36[3] = 0x27;
  local_36[4] = 0x27;
  *(undefined4 *)(param_1 + 0xda) = 6;
  local_36[8] = 0xa6;
  local_36[9] = 0xa6;
  local_36[10] = 0xa6;
  *(undefined4 *)(param_1 + 0xa4) = 0;
  local_36[0xc] = 0xa1;
  local_36[0xd] = 0xa1;
  *(undefined4 *)(param_1 + 0x88) = 0;
  *(undefined4 *)(param_1 + 0x84) = 0;
  *(undefined4 *)(param_1 + 0xde) = 0xfffffffe;
  puVar7 = (undefined4 *)(param_1 + 0x12d8);
  for (iVar8 = 5000; iVar8 != 0; iVar8 = iVar8 + -1) {
    *puVar7 = 0xff00ff;
    puVar7 = puVar7 + 1;
  }
  *(undefined4 *)(param_1 + 0x60fc) = 0;
  *(undefined2 *)(param_1 + 0xbc) = 0;
  *(undefined4 *)(param_1 + 0x6120) = 0;
  *(undefined4 *)(param_1 + 0x6100) = 1;
  *(undefined4 *)(param_1 + 0x6104) = 1;
  *(undefined4 *)(param_1 + 0x6108) = 1;
  *(undefined4 *)(param_1 + 0x610c) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x6110) = 0xffffffff;
  *(undefined4 *)(param_1 + 0x6114) = 0xffffffff;
  local_50 = (ushort *)(param_1 + 0xaa);
  *(undefined4 *)(param_1 + 0x6118) = 0xffffffff;
  local_36[0] = 0xff;
  local_36[1] = 0;
  local_36[2] = 0xff;
  local_36[5] = 0x47;
  local_36[6] = 0xbd;
  local_36[7] = 0xfe;
  local_36[0xb] = 0xff;
  local_36[0xe] = 0xd6;
  local_36[0xf] = 0x92;
  local_36[0x10] = 0xff;
  local_36[0x11] = 0xf6;
  local_36[0x12] = 0xe5;
  local_36[0x13] = 0x28;
  local_36[0x14] = 0xf0;
  local_36[0x15] = 0x7c;
  local_36[0x16] = 0xbb;
  local_36[0x17] = 0xff;
  local_36[0x18] = 0xff;
  local_36[0x19] = 0xff;
  *(undefined4 *)(param_1 + 0x611c) = 1;
  puVar12 = local_36;
  local_4c = 9;
  do {
    uVar3 = (uint)puVar12[-1];
    uVar9 = (uint)puVar12[1];
    uVar11 = (uint)*puVar12;
    if (0xff < puVar12[-1]) {
      uVar3 = 0xff;
    }
    if (0xff < *puVar12) {
      uVar11 = 0xff;
    }
    if (0xff < puVar12[1]) {
      uVar9 = 0xff;
    }
    if (DAT_006e1f88 == 0) {
      uVar2 = (ushort)(((uVar3 & 0xfff8) << 5 | uVar11 & 0xfc) << 3);
    }
    else {
      uVar2 = (ushort)(((uVar3 & 0xf8) << 5 | uVar11 & 0xf8) << 2);
    }
    puVar12 = puVar12 + 3;
    *local_50 = uVar2 | (ushort)(uVar9 >> 3) & 0x1f;
    local_50 = local_50 + 1;
    local_4c = local_4c + -1;
  } while (local_4c != 0);
  uVar4 = FUN_0058b660();
  *(undefined4 *)(param_1 + 0xbe) = uVar4;
  *(uint *)(param_1 + 0xca) = DAT_006a49f8 >> 1;
  uVar3 = DAT_006c5498;
  iVar8 = DAT_006c5494;
  *(undefined4 *)(param_1 + 0x613c) = 6;
  iVar5 = *(int *)(param_1 + 0xca) + -8;
  *(int *)(param_1 + 0x6140) = iVar5;
  *(uint *)(param_1 + 0x6138) = (iVar5 - uVar3) - iVar8;
  *(uint *)(param_1 + 0x6144) = (uVar3 + iVar8 >> 1) + *(int *)(param_1 + 0x613c);
  if (*(int *)(param_1 + 0x6158) != 0) {
    FUN_004a4f70();
    *(undefined4 *)(param_1 + -0x1d) = 0xffffffff;
  }
  iVar8 = param_1 + -0x21;
  (**(code **)(*(int *)(param_1 + -0x21) + 8))();
  *(undefined4 *)(param_1 + 0x6158) = 1;
  (**(code **)(*DAT_006a49e4 + 0x80))();
  if (*(int *)(param_1 + 8) != 0) {
    FUN_00597870();
    *(undefined4 *)(param_1 + 8) = 0;
  }
  if (*(int *)(param_1 + 0x6178) * *(int *)(param_1 + 0x617c) != 0) {
    uVar4 = FUN_00597850();
    *(undefined4 *)(param_1 + 8) = uVar4;
  }
  FUN_00555020();
  FUN_005553f0();
  FUN_00554e00();
  if ((*(int *)(param_1 + 0x612c) != 0) &&
     (iVar10 = *(int *)(param_1 + 0x6130) - *(int *)(param_1 + 0x612c), iVar5 = iVar10 >> 0x1f,
     iVar10 / 0xc + iVar5 != iVar5)) {
    puVar7 = *(undefined4 **)(param_1 + 0x6130);
    puVar1 = *(undefined4 **)(param_1 + 0x612c);
    for (puVar6 = *(undefined4 **)(param_1 + 0x6130); puVar6 != puVar7; puVar6 = puVar6 + 3) {
      *puVar1 = *puVar6;
      puVar1[1] = puVar6[1];
      puVar1[2] = puVar6[2];
      iVar8 = unaff_ESI;
      puVar1 = puVar1 + 3;
    }
    *(undefined4 **)(param_1 + 0x6130) = puVar1;
  }
  iVar5 = *(int *)(param_1 + 0x6596);
  if ((iVar5 != 0) && ((*(int *)(param_1 + 0x659a) - iVar5 & 0xfffffff8U) != 0)) {
    *(int *)(param_1 + 0x659a) = iVar5;
  }
  iVar5 = 0;
  if (0 < DAT_006c5c84) {
    do {
      FUN_004f3840(iVar5,&stack0xffffffa4);
      FUN_00556ad0(*(undefined4 *)(param_1 + 0x6130));
      iVar5 = iVar5 + 1;
    } while (iVar5 < DAT_006c5c84);
  }
  if (*(int *)(param_1 + 0xd2) != 0) {
    FUN_005979b0();
  }
  uVar4 = FUN_00597880();
  uVar3 = 0;
  *(undefined4 *)(param_1 + 0xd2) = uVar4;
  *(undefined4 *)(param_1 + 0xbe) = 0;
  *(undefined4 *)(param_1 + 0xc2) = 0;
  *(undefined4 *)(param_1 + 0xca) = 0;
  *(undefined4 *)(param_1 + 0xce) = 0;
  if (*(int *)(param_1 + 0x65a2) == 0) {
    puVar7 = (undefined4 *)FUN_00597850();
    if (puVar7 == (undefined4 *)0x0) {
      puVar7 = (undefined4 *)0x0;
    }
    else {
      puVar7[1] = &LAB_00552530;
      puVar7[2] = 0;
      puVar7[3] = iVar8;
      *puVar7 = &PTR_LAB_005c75b0;
    }
    *(undefined4 **)(param_1 + 0x65a2) = puVar7;
    *(undefined4 **)(param_1 + 0x61b0) = puVar7;
  }
  if (0 < DAT_006898dc) {
    iVar8 = 0;
    do {
      if (uVar3 < DAT_0068949c) {
        iVar5 = iVar8 + DAT_00689498;
      }
      else {
        iVar5 = 0;
      }
      if ((*(int *)(iVar5 + 4) != 0) &&
         ((*(int *)(iVar5 + 0x28) == 0x3e || (*(int *)(iVar5 + 0x28) == 0x41)))) {
        uVar9 = *(uint *)(iVar5 + 0xc);
        if ((int)uVar9 < 0) {
          do {
            uVar9 = uVar9 + DAT_006c5498;
          } while ((int)uVar9 < 0);
        }
        else {
          for (; DAT_006c5498 <= uVar9; uVar9 = uVar9 - DAT_006c5498) {
          }
        }
        FUN_0040e290(*(undefined4 *)(iVar5 + 8));
        FUN_00556820();
      }
      uVar3 = uVar3 + 1;
      iVar8 = iVar8 + 0x22e;
    } while ((int)uVar3 < DAT_006898dc);
  }
  return;
}

