
void __fastcall FUN_00555990(int param_1)

{
  int *piVar1;
  short sVar2;
  undefined4 uVar3;
  uint uVar4;
  int *piVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  undefined4 *puVar9;
  
  uVar3 = DAT_00689961;
  *(undefined4 *)(param_1 + 0xa4) = DAT_00689961;
  switch(uVar3) {
  case 0:
    *(int *)(param_1 + 0x6180) = DAT_006e202c / 2 + DAT_00689969;
    break;
  case 1:
    *(int *)(param_1 + 0x6180) = DAT_006e202c / 2 + DAT_00689969;
    goto LAB_00555a2f;
  case 2:
    *(int *)(param_1 + 0x6180) = DAT_00689969 - DAT_006e202c / 2;
LAB_00555a2f:
    *(int *)(param_1 + 0x6184) = DAT_0068996d - DAT_006e202c / 2;
    goto switchD_005559aa_default;
  case 3:
    *(int *)(param_1 + 0x6180) = DAT_00689969 - DAT_006e202c / 2;
    break;
  default:
    goto switchD_005559aa_default;
  }
  *(int *)(param_1 + 0x6184) = DAT_006e202c / 2 + DAT_0068996d;
switchD_005559aa_default:
  iVar8 = *(int *)(param_1 + 0x6178);
  if (iVar8 + -1 < *(int *)(param_1 + 0x6180)) {
    *(int *)(param_1 + 0x6180) = *(int *)(param_1 + 0x6180) % iVar8;
  }
  iVar7 = *(int *)(param_1 + 0x6180);
  if (iVar7 < 0) {
    iVar6 = (iVar8 + -1) - iVar7;
    *(int *)(param_1 + 0x6180) = (iVar7 - iVar6 % iVar8) + iVar6;
  }
  iVar8 = *(int *)(param_1 + 0x617c);
  if (iVar8 + -1 < *(int *)(param_1 + 0x6184)) {
    *(int *)(param_1 + 0x6184) = *(int *)(param_1 + 0x6184) % iVar8;
  }
  iVar7 = *(int *)(param_1 + 0x6184);
  if (iVar7 < 0) {
    iVar6 = (iVar8 + -1) - iVar7;
    *(int *)(param_1 + 0x6184) = (iVar7 - iVar6 % iVar8) + iVar6;
  }
  *(undefined4 *)(param_1 + 0xa4) = DAT_00689961;
  if (*(int *)(param_1 + 4) != 0) {
    if (DAT_006e1f88 == 0) {
      *(undefined2 *)(param_1 + 0xa8) = 0x7bef;
    }
    else {
      *(undefined2 *)(param_1 + 0xa8) = 0x7def;
    }
    FUN_00552550();
    *(undefined4 *)(param_1 + 0xe2) = DAT_006c5494;
    *(undefined4 *)(param_1 + 0xe6) = DAT_006c5498;
    if (*(int *)(param_1 + 0x60fc) == 5) {
      *(short *)(param_1 + 0xbc) = *(short *)(param_1 + 0xbc) + 1;
      *(uint *)(param_1 + 0x6120) = *(uint *)(param_1 + 0x6120) ^ 1;
      *(undefined4 *)(param_1 + 0x60fc) = 0;
      iVar8 = *(int *)(param_1 + 0x6124) + 1;
      *(int *)(param_1 + 0x6124) = iVar8;
      if (2 < iVar8) {
        *(undefined4 *)(param_1 + 0x6124) = 0;
      }
      if (*(short *)(param_1 + 0xbc) == 5) {
        *(undefined2 *)(param_1 + 0xbc) = 0;
      }
    }
    *(int *)(param_1 + 0x60fc) = *(int *)(param_1 + 0x60fc) + 1;
    uVar3 = FUN_0058b660();
    *(undefined4 *)(param_1 + 0xbe) = uVar3;
    uVar4 = DAT_006a49f8 >> 1;
    *(uint *)(param_1 + 0xca) = uVar4;
    *(uint *)(param_1 + 0xd6) = (uVar4 - *(int *)(param_1 + 0xe2)) + -8;
    *(undefined4 *)(param_1 + 0xda) = 6;
    iVar8 = *(int *)(param_1 + 0xbe) + (*(int *)(param_1 + 0xd6) + *(int *)(param_1 + 0xca) * 6) * 2
    ;
    *(int *)(param_1 + 0xc6) = iVar8;
    if ((*(int *)(param_1 + 0xca) != *(int *)(param_1 + 0xce)) ||
       (*(int *)(param_1 + 0xbe) != *(int *)(param_1 + 0xc2))) {
      iVar7 = 0;
      if (0 < *(int *)(param_1 + 0x617c) + *(int *)(param_1 + 0x6178)) {
        do {
          iVar7 = iVar7 + 1;
          *(int *)(*(int *)(param_1 + 0xd2) + -4 + iVar7 * 4) = iVar8;
          iVar8 = iVar8 + *(int *)(param_1 + 0xca) * 2;
        } while (iVar7 < *(int *)(param_1 + 0x617c) + *(int *)(param_1 + 0x6178));
      }
      *(undefined4 *)(param_1 + 0xce) = *(undefined4 *)(param_1 + 0xca);
      *(undefined4 *)(param_1 + 0xc2) = *(undefined4 *)(param_1 + 0xbe);
    }
    switch(*(undefined4 *)(param_1 + 0xa4)) {
    case 0:
    case 2:
      FUN_005527a0();
      break;
    case 1:
      FUN_00552f20();
      break;
    case 3:
      FUN_00552b50();
    }
    FUN_00553850();
    iVar8 = 0;
    if (0 < *(int *)(param_1 + 0x60f8)) {
      piVar5 = (int *)(param_1 + 0xf0);
      do {
        piVar1 = piVar5 + -1;
        iVar7 = *piVar5;
        iVar8 = iVar8 + 1;
        piVar5 = (int *)((int)piVar5 + 0x12);
        *(undefined2 *)(param_1 + 0x12d8 + (iVar7 + *piVar1 * 100) * 2) = 0xff;
      } while (iVar8 < *(int *)(param_1 + 0x60f8));
    }
    *(undefined4 *)(param_1 + 0x60f8) = 0;
    puVar9 = *(undefined4 **)(param_1 + 0x6596);
    if (puVar9 != *(undefined4 **)(param_1 + 0x659a)) {
      do {
        FUN_00556380(*puVar9,puVar9[1],*(undefined4 *)(param_1 + 0x658e),
                     (-(uint)(DAT_006e1f88 != 0) & 0xfffffc00) + 0x7ff);
        puVar9 = puVar9 + 2;
      } while (puVar9 != *(undefined4 **)(param_1 + 0x659a));
    }
    iVar8 = *(int *)(param_1 + 0x658e) + 1;
    *(int *)(param_1 + 0x658e) = iVar8;
    if (0xf < iVar8) {
      *(undefined4 *)(param_1 + 0x658e) = 1;
    }
  }
  if (*(int *)(param_1 + 0x61e6) != 0) {
    FUN_0057dc20(400);
    FUN_0057dc60((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280,300);
    uVar3 = *(undefined4 *)(param_1 + 0xd6);
    iVar8 = *(int *)(param_1 + 0xe6) / 2 + -1 + *(int *)(param_1 + 0xda);
    iVar7 = **(int **)(param_1 + 0x61c6) + *(int *)(param_1 + 0x61ca);
    if (*(int *)(iVar7 + 0x1c) == -1) {
      (*DAT_006903dc)(iVar7,uVar3,iVar8);
    }
    else {
      sVar2 = (*DAT_006c4e3c)(iVar7,uVar3,iVar8,0,0);
      if (sVar2 == 1) {
        FUN_0057de00(iVar7,uVar3,iVar8,0,0);
      }
    }
  }
  (**(code **)(*DAT_006a49e4 + 0x80))(DAT_006a49e4,*(undefined4 *)(param_1 + 0xbe));
  return;
}

