
void __fastcall FUN_004c6910(int param_1)

{
  int iVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  int iVar5;
  int *piVar6;
  int iVar7;
  int iVar8;
  undefined4 *puVar9;
  undefined4 uVar10;
  
  iVar8 = 0;
  if ((*(char **)(param_1 + 0x52) == (char *)0x0) || (**(char **)(param_1 + 0x52) == '\0')) {
    iVar7 = *(int *)(param_1 + 0x46);
    *(undefined4 *)(param_1 + 0x52) = 0;
    iVar4 = *(int *)(param_1 + 0x16) + *(int *)(param_1 + 0x42) * 0x1fc;
    iVar2 = *(int *)(iVar4 + 0xc + iVar7 * 4);
    if ((iVar2 == 0) || (iVar1 = iVar7, *(int *)(iVar2 + 4) != iVar7)) {
      iVar1 = 0;
      piVar6 = (int *)(iVar4 + 0xc);
      do {
        if ((*piVar6 != 0) && (*(int *)(*piVar6 + 4) == iVar7)) break;
        iVar1 = iVar1 + 1;
        piVar6 = piVar6 + 1;
      } while (iVar1 < 0x7c);
    }
    if (iVar1 != 0x7c) {
      iVar8 = *(int *)(iVar4 + 0xc + iVar1 * 4);
    }
LAB_004c69ef:
    iVar8 = FUN_004c23b0(*(undefined4 *)(iVar8 + 4));
    if (iVar8 != 0) {
      iVar4 = *(int *)(iVar8 + 4);
      iVar7 = *(int *)(param_1 + 0x42);
      iVar2 = FUN_005908e0();
      if (*(int *)(&DAT_0065bb86 + iVar2 * 0x93a) == 0) {
        uVar3 = 0;
      }
      else {
        iVar2 = FUN_005908e0();
        uVar3 = (uint)*(byte *)(*(int *)(&DAT_0065bb86 + iVar2 * 0x93a) + iVar7 * 0x7c + iVar4);
      }
      if (DAT_006e1f20 == 1) {
        uVar3 = 4;
      }
      piVar6 = (int *)(iVar8 + 0xc + uVar3 * 4);
LAB_004c6a83:
      if (*piVar6 == 0) goto code_r0x004c6a88;
      if (iVar8 != 0) {
        *(undefined4 *)(param_1 + 0x4a) = 1;
        *(undefined4 *)(param_1 + 0x52) = 0;
        *(undefined4 *)(param_1 + 0x46) = *(undefined4 *)(iVar8 + 4);
        iVar8 = 0;
        if (0 < *(int *)(param_1 + 0x1a2a)) {
          puVar9 = (undefined4 *)(param_1 + 0xaee);
          do {
            FUN_004ca950();
            FUN_004a4f70();
            *puVar9 = 0xffffffff;
            iVar8 = iVar8 + 1;
            puVar9 = (undefined4 *)((int)puVar9 + 0x7a);
          } while (iVar8 < *(int *)(param_1 + 0x1a2a));
        }
        *(undefined4 *)(param_1 + 0x1a2a) = 0;
        FUN_004c5520(*(undefined4 *)(param_1 + 0x42),*(undefined4 *)(param_1 + 0x46),
                     *(undefined4 *)(param_1 + 0x4a),*(undefined4 *)(param_1 + 0x2e));
        FUN_004c5520(*(undefined4 *)(param_1 + 0x42),*(undefined4 *)(param_1 + 0x46),
                     *(int *)(param_1 + 0x4a) + 1,*(undefined4 *)(param_1 + 0x32));
        *(undefined4 *)(param_1 + 0x36) = 0;
        goto LAB_004c6ce5;
      }
    }
    iVar8 = *(int *)(param_1 + 0x42) + 1;
    if (iVar8 < 10) {
      iVar7 = iVar8 * 0x7c;
      iVar4 = iVar8 * 0x1fc;
      do {
        iVar2 = *(int *)(iVar4 + 0xc + *(int *)(param_1 + 0x16));
        if (iVar2 != 0) {
          iVar1 = *(int *)(iVar2 + 4);
          iVar5 = FUN_005908e0();
          if (*(int *)(&DAT_0065bb86 + iVar5 * 0x93a) == 0) {
            uVar3 = 0;
          }
          else {
            iVar5 = FUN_005908e0();
            uVar3 = (uint)*(byte *)(iVar7 + *(int *)(&DAT_0065bb86 + iVar5 * 0x93a) + iVar1);
          }
          if (DAT_006e1f20 == 1) {
            uVar3 = 4;
          }
          piVar6 = (int *)(iVar2 + 0xc + uVar3 * 4);
LAB_004c6bb7:
          if (*piVar6 == 0) goto code_r0x004c6bbc;
          if (iVar8 < 10) {
            *(int *)(param_1 + 0x42) = iVar8;
            *(undefined4 *)(param_1 + 0x4a) = 1;
            iVar4 = 0;
            *(undefined4 *)(param_1 + 0x46) =
                 *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x16) + 0xc + iVar8 * 0x1fc) + 4);
            if (0 < *(int *)(param_1 + 0x1a2a)) {
              puVar9 = (undefined4 *)(param_1 + 0xaee);
              do {
                FUN_004ca950();
                FUN_004a4f70();
                *puVar9 = 0xffffffff;
                iVar4 = iVar4 + 1;
                puVar9 = (undefined4 *)((int)puVar9 + 0x7a);
              } while (iVar4 < *(int *)(param_1 + 0x1a2a));
            }
            iVar4 = 0;
            *(undefined4 *)(param_1 + 0x1a2a) = 0;
            if (iVar8 < 8) {
              if (-1 < iVar8) {
                iVar4 = iVar8 + 1;
                iVar8 = iVar4;
                do {
                  FUN_004ca930();
                  FUN_004ca950();
                  iVar8 = iVar8 + -1;
                } while (iVar8 != 0);
                if (7 < iVar4) goto LAB_004c6caf;
              }
              iVar4 = 8 - iVar4;
              do {
                FUN_004ca950();
                FUN_004ca930();
                iVar4 = iVar4 + -1;
              } while (iVar4 != 0);
            }
LAB_004c6caf:
            FUN_004c5520(*(undefined4 *)(param_1 + 0x42),*(undefined4 *)(param_1 + 0x46),
                         *(undefined4 *)(param_1 + 0x4a),*(undefined4 *)(param_1 + 0x2e));
            FUN_004c5520(*(undefined4 *)(param_1 + 0x42),*(undefined4 *)(param_1 + 0x46),
                         *(int *)(param_1 + 0x4a) + 1,*(undefined4 *)(param_1 + 0x32));
            *(undefined4 *)(param_1 + 0x36) = 0;
            goto LAB_004c6ce5;
          }
          break;
        }
LAB_004c6bcc:
        iVar4 = iVar4 + 0x1fc;
        iVar8 = iVar8 + 1;
        iVar7 = iVar7 + 0x7c;
      } while (iVar4 < 0x13d8);
    }
  }
  else {
    iVar8 = 0;
    *(int *)(param_1 + 0x4a) = *(int *)(param_1 + 0x4a) + 2;
    if (0 < *(int *)(param_1 + 0x1a2a)) {
      puVar9 = (undefined4 *)(param_1 + 0xaee);
      do {
        FUN_004ca950();
        FUN_004a4f70();
        *puVar9 = 0xffffffff;
        iVar8 = iVar8 + 1;
        puVar9 = (undefined4 *)((int)puVar9 + 0x7a);
      } while (iVar8 < *(int *)(param_1 + 0x1a2a));
    }
    *(undefined4 *)(param_1 + 0x1a2a) = 0;
    FUN_004c5520(*(undefined4 *)(param_1 + 0x42),*(undefined4 *)(param_1 + 0x46),
                 *(undefined4 *)(param_1 + 0x4a),*(undefined4 *)(param_1 + 0x2e));
    FUN_004c5520(*(undefined4 *)(param_1 + 0x42),*(undefined4 *)(param_1 + 0x46),
                 *(int *)(param_1 + 0x4a) + 1,*(undefined4 *)(param_1 + 0x32));
    *(undefined4 *)(param_1 + 0x36) = 0;
LAB_004c6ce5:
    *(undefined4 *)(param_1 + 0x3a) = 1;
  }
  if (*(int *)(param_1 + 0x3a) != 0) {
    FUN_0056f000(0x328,0,0,0,0,0xffffffff,0xffffffff);
    FUN_004ca950();
    FUN_004ca950();
    if (*(int *)(param_1 + 0x52) == 0) {
      uVar10 = 0xffffffff;
    }
    else {
      uVar10 = 3;
    }
    FUN_004d4780(uVar10);
    if (*(int *)(param_1 + 0x4a) < 2) {
      uVar10 = 0xffffffff;
    }
    else {
      uVar10 = 1;
    }
    FUN_004d4780(uVar10);
    FUN_004ca9b0(*(undefined4 *)(param_1 + 0x2e));
    FUN_004ca9b0(*(undefined4 *)(param_1 + 0x32));
    if (((1 < *(int *)(param_1 + 0x4a)) || (*(int *)(param_1 + 0x42) != 0)) ||
       (*(int *)(param_1 + 0x46) != 0)) {
      FUN_004ca930();
      (**(code **)(*(int *)(param_1 + 0x76) + 0x10))();
    }
    iVar8 = FUN_004c6700();
    if (iVar8 != 0) {
      FUN_004ca930();
      (**(code **)(*(int *)(param_1 + 299) + 0x10))();
    }
  }
  return;
code_r0x004c6a88:
  uVar3 = uVar3 - 1;
  piVar6 = piVar6 + -1;
  if ((int)uVar3 < 0) goto LAB_004c69ef;
  goto LAB_004c6a83;
code_r0x004c6bbc:
  uVar3 = uVar3 - 1;
  piVar6 = piVar6 + -1;
  if ((int)uVar3 < 0) goto LAB_004c6bcc;
  goto LAB_004c6bb7;
}

