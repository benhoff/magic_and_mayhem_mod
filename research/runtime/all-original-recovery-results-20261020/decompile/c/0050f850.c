
void __fastcall FUN_0050f850(int param_1)

{
  ushort uVar1;
  int iVar2;
  bool bVar3;
  int *piVar4;
  uint uVar5;
  uint uVar6;
  uint uVar7;
  int local_c;
  int *local_8;
  int local_4;
  
  local_c = -1;
  uVar7 = 0;
  bVar3 = false;
  local_8 = &DAT_005e5330;
  if (*(int *)(*(int *)(param_1 + 0xac) + 8) != 2) {
    local_8 = &DAT_005e5310;
  }
  local_4 = 4;
  do {
    uVar5 = *local_8 + *(int *)(param_1 + 8);
    uVar6 = local_8[1] + *(int *)(param_1 + 0xc);
    if ((int)uVar5 < 0) {
      uVar5 = uVar5 + DAT_006c5494;
    }
    if (DAT_006c5494 <= uVar5) {
      uVar5 = uVar5 - DAT_006c5494;
    }
    if ((int)uVar6 < 0) {
      uVar6 = uVar6 + DAT_006c5498;
    }
    if (DAT_006c5498 <= uVar6) {
      uVar6 = uVar6 - DAT_006c5498;
    }
    uVar1 = *(ushort *)
             (DAT_006c54dc +
              ((&DAT_006cb942)[uVar6] + (&DAT_006cb8c2)[*(int *)(param_1 + 0x10)] + uVar5) * 0xc +
             10);
    if (((uVar1 & 0x80) != 0) || ((uVar1 & 0x100) != 0)) {
      if (((uVar1 & 0x100) == 0) ||
         ((((iVar2 = **(int **)(param_1 + 0xac), iVar2 != 0 && (iVar2 != 0x18)) && (iVar2 != 0x19))
          && (iVar2 != 0x1a)))) {
        if (!bVar3) goto LAB_0050fa51;
      }
      else {
        bVar3 = false;
        piVar4 = (int *)(param_1 + 0x17c + uVar7 * 0x18);
        do {
          if (((*piVar4 == 0x33) || (*piVar4 == 0x10)) && (piVar4[5] != 0)) {
            bVar3 = true;
            iVar2 = param_1 + uVar7 * 0x18;
            if (((*(int *)(param_1 + 0x18c + uVar7 * 0x18) != 0) &&
                (*(uint *)(iVar2 + 0x17c) < 0x55)) && (*(int *)(iVar2 + 400) != 0)) {
              FUN_00468b10();
              if (*(int *)(param_1 + 0x174) == DAT_00644520) {
                FUN_004d7180(uVar7);
              }
              if ((*(int *)(iVar2 + 0x18c) == 0) && (*(uint *)(iVar2 + 0x17c) < 0x54)) {
                *(undefined4 *)(iVar2 + 400) = 0;
                *(int *)(param_1 + 0x178) = *(int *)(param_1 + 0x178) + -1;
              }
            }
            goto LAB_0050f9e5;
          }
          uVar7 = uVar7 + 1;
          piVar4 = piVar4 + 6;
        } while (uVar7 < *(uint *)(param_1 + 0x178));
LAB_0050fa51:
        if (((uVar1 & 0x80) == 0) && (DAT_006e1f0c == 0)) {
          if (*(int *)(param_1 + 0x174) == DAT_00644520) {
            FUN_004da690(1);
          }
          goto LAB_0050fa21;
        }
      }
LAB_0050f9e5:
      FUN_004f35d0(uVar5,uVar6,*(undefined4 *)(param_1 + 0x10),*(undefined4 *)(param_1 + 0x174),
                   &local_c);
      if (local_c != -1) {
        FUN_00571310(local_c,uVar5,uVar6,*(undefined4 *)(param_1 + 0x10),0,0,0);
      }
    }
LAB_0050fa21:
    local_8 = local_8 + 2;
    local_4 = local_4 + -1;
    if (local_4 == 0) {
      return;
    }
  } while( true );
}

