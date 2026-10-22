
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_0056b1e0(undefined4 param_1,int param_2)

{
  byte bVar1;
  int iVar2;
  byte *pbVar3;
  int iVar4;
  int *piVar5;
  byte *pbVar6;
  undefined4 uVar7;
  uint uVar8;
  uint extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  int iVar9;
  byte *pbVar10;
  bool bVar11;
  undefined4 uVar12;
  undefined4 uVar13;
  int iVar14;
  int local_114;
  int local_110;
  int local_10c;
  int local_108;
  undefined4 local_104;
  undefined1 local_100 [256];
  
  iVar4 = *(int *)(param_2 + 0x3d4);
  local_10c = 0;
  local_110 = 0;
  local_114 = 0;
  local_108 = 0;
  local_104 = param_1;
  if (iVar4 == 1) {
    iVar4 = 0;
    if (0 < DAT_006f37fc) {
      pbVar6 = (byte *)(DAT_006f37f8 + 1);
      piVar5 = DAT_006f37f8;
      do {
        if (*piVar5 != 0) {
          pbVar10 = (byte *)(param_2 + 0x3d8);
          pbVar3 = pbVar6;
          do {
            bVar1 = *pbVar3;
            bVar11 = bVar1 < *pbVar10;
            if (bVar1 != *pbVar10) {
LAB_0056b300:
              iVar2 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
              goto LAB_0056b305;
            }
            if (bVar1 == 0) break;
            bVar1 = pbVar3[1];
            bVar11 = bVar1 < pbVar10[1];
            if (bVar1 != pbVar10[1]) goto LAB_0056b300;
            pbVar3 = pbVar3 + 2;
            pbVar10 = pbVar10 + 2;
          } while (bVar1 != 0);
          iVar2 = 0;
LAB_0056b305:
          if (iVar2 == 0) {
            if ((DAT_006f37fc <= iVar4) || (piVar5 == (int *)0x0)) break;
            uVar8 = piVar5[10];
            if (uVar8 == 0xffffffff) {
              if ((piVar5[9] != 4) && (piVar5[9] != 2)) break;
              local_114 = piVar5[0xb];
              local_110 = piVar5[0xc];
              local_10c = piVar5[0xd];
            }
            else {
              if (uVar8 < DAT_006def5c) {
                iVar4 = uVar8 * 0xe4b + DAT_006def58;
              }
              else {
                iVar4 = 0;
              }
              local_114 = *(int *)(iVar4 + 8);
              local_110 = *(int *)(iVar4 + 0xc);
              local_10c = *(int *)(*(int *)(iVar4 + 0xac) + 0xc) + *(int *)(iVar4 + 0x10);
            }
            local_108 = 1;
            break;
          }
        }
        iVar4 = iVar4 + 1;
        piVar5 = piVar5 + 0x10;
        pbVar6 = pbVar6 + 0x40;
      } while (iVar4 < DAT_006f37fc);
    }
  }
  else if (iVar4 == 2) {
    iVar4 = FUN_0056ba90(param_2 + 0x3f8,&local_114,&local_110,&local_10c);
    if (iVar4 != 0) {
      local_108 = 1;
    }
  }
  else if (iVar4 == 3) {
    iVar4 = *(int *)(param_2 + 0x418);
    _DAT_006b1258 = 0;
    _DAT_006b125c = DAT_006b1248;
    iVar2 = FUN_00544d60();
    while (iVar2 != 0) {
      if (iVar4 == *(int *)(iVar2 + 0x2a)) {
        local_114 = *(int *)(iVar2 + 8);
        local_108 = 1;
        local_110 = *(int *)(iVar2 + 0xc);
        local_10c = *(int *)(iVar2 + 0x10);
        break;
      }
      iVar2 = FUN_00544d60();
    }
  }
  if ((*(int *)(param_2 + 0x41c) != 0) && (local_108 != 0)) {
    FUN_004f7a00(local_114,local_110,local_10c);
  }
  iVar4 = *(int *)(param_2 + 0x3d0);
  if (iVar4 != -1) {
    iVar4 = iVar4 + -0x61;
  }
  uVar7 = *(undefined4 *)(param_2 + 0x2cc);
  iVar2 = param_2 + 0x2d0;
  FUN_00473890(uVar7,iVar2,iVar4);
  FUN_004a0d70(uVar7,iVar2,iVar4);
  iVar4 = FUN_00473890();
  iVar4 = *(int *)(iVar4 + 4);
  iVar2 = 0;
  if ((0 < iVar4) && (iVar9 = 0, 0 < iVar4)) {
    do {
      iVar14 = iVar9;
      FUN_00473890(iVar9);
      piVar5 = (int *)FUN_004a0e10(iVar14);
      iVar2 = iVar2 + *piVar5;
      iVar9 = iVar9 + 1;
    } while (iVar9 < iVar4);
  }
  pbVar10 = &DAT_005fc874;
  pbVar6 = (byte *)(param_2 + 0x2d0);
  do {
    bVar1 = *pbVar6;
    uVar8 = (uint)bVar1;
    bVar11 = bVar1 < *pbVar10;
    if (bVar1 != *pbVar10) {
LAB_0056b44a:
      iVar4 = (1 - (uint)bVar11) - (uint)(bVar11 != 0);
      goto LAB_0056b44f;
    }
    if (bVar1 == 0) break;
    bVar1 = pbVar6[1];
    uVar8 = (uint)bVar1;
    bVar11 = bVar1 < pbVar10[1];
    if (bVar1 != pbVar10[1]) goto LAB_0056b44a;
    pbVar6 = pbVar6 + 2;
    pbVar10 = pbVar10 + 2;
  } while (bVar1 != 0);
  iVar4 = 0;
LAB_0056b44f:
  if (iVar4 != 0) {
    iVar9 = 0;
    piVar5 = DAT_006f3a70;
    iVar4 = DAT_006f3a78;
    if (0 < DAT_006f3a78) {
      do {
        if ((*piVar5 != 0) && (piVar5[1] == 8)) {
          iVar9 = iVar9 + piVar5[0xb3];
        }
        iVar4 = iVar4 + -1;
        piVar5 = piVar5 + 0x11f;
      } while (iVar4 != 0);
    }
    uVar7 = FUN_0058a6b0(iVar2,iVar9);
    FUN_0059c1fd(local_100,s__s__d__s___d__d__005ed914,param_2 + 0x2d0,
                 *(undefined4 *)(param_2 + 0x2cc),uVar7);
    FUN_004da770(local_100);
    uVar8 = extraout_ECX;
  }
  if (local_108 != 0) {
    uVar7 = *(undefined4 *)(param_2 + 0x2cc);
    FUN_0040eeb0(local_10c);
    uVar13 = extraout_ECX_00;
    FUN_0040e8a0(local_110);
    uVar12 = extraout_ECX_01;
    FUN_0040e290(local_114);
    FUN_0049d720(uVar12,uVar13,uVar8,uVar7);
  }
  if ((*(int *)(param_2 + 0x2c) != -1) && (*(int *)(param_2 + 0x30) == 0)) {
    FUN_00571310(*(int *)(param_2 + 0x2c),local_114,local_110,local_10c,0,0,0);
  }
  FUN_00564d60(param_2);
  return;
}

