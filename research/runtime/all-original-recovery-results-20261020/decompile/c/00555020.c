
void __fastcall FUN_00555020(int param_1)

{
  short *psVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  undefined4 uVar5;
  undefined4 uVar6;
  int iVar7;
  int iVar8;
  short sVar9;
  int iVar10;
  undefined4 *puVar11;
  int local_30;
  undefined4 *local_2c;
  int *local_1c;
  uint local_18;
  undefined4 local_4;
  
  iVar10 = 0;
  iVar8 = *(int *)(param_1 + 0x29);
  if (0 < *(int *)(param_1 + 0x619d)) {
    do {
      iVar4 = 0;
      if (0 < *(int *)(param_1 + 0x6199)) {
        do {
          *(undefined2 *)(iVar8 + 0xc) = 0;
          iVar8 = iVar8 + 0x10;
          iVar4 = iVar4 + 1;
        } while (iVar4 < *(int *)(param_1 + 0x6199));
      }
      iVar10 = iVar10 + 1;
    } while (iVar10 < *(int *)(param_1 + 0x619d));
  }
  uVar3 = DAT_006c5c80;
  local_2c = *(undefined4 **)(param_1 + 0x29);
  local_30 = 0;
  if (0 < *(int *)(param_1 + 0x619d)) {
    local_1c = &DAT_006cb942;
    do {
      iVar8 = 0;
      if (0 < *(int *)(param_1 + 0x6199)) {
        do {
          iVar10 = *local_1c;
          psVar1 = (short *)(DAT_006c54dc + ((&DAT_006cb8c2)[uVar3] + iVar10 + iVar8) * 0xc);
          uVar5 = __ftol();
          uVar6 = __ftol();
          iVar10 = (&DAT_006cb8c2)[uVar3 >> 1] + iVar10 + DAT_006c5c5c + iVar8;
          if ((*psVar1 == 0) && ((*(byte *)((int)psVar1 + 0xb) & 0x40) == 0)) {
            local_4 = CONCAT22(local_4._2_2_,(-(ushort)(DAT_006e1f88 != 0) & 0xf760) + 0x1154);
          }
          else {
            local_4 = (uint)local_4._2_2_ << 0x10;
          }
          puVar11 = local_2c + 4;
          iVar8 = iVar8 + 1;
          *local_2c = uVar5;
          local_2c[1] = uVar6;
          local_2c[2] = iVar10;
          local_2c[3] = local_4;
          local_2c = puVar11;
        } while (iVar8 < *(int *)(param_1 + 0x6199));
      }
      local_30 = local_30 + 1;
      local_1c = local_1c + 1;
    } while (local_30 < *(int *)(param_1 + 0x619d));
  }
  local_18 = 0;
  iVar8 = DAT_006c54dc;
  if (DAT_006c549c != 0) {
    do {
      local_2c = *(undefined4 **)(param_1 + 0x29);
      local_30 = 0;
      if (0 < *(int *)(param_1 + 0x619d)) {
        local_1c = &DAT_006cb942;
        do {
          iVar10 = 0;
          if (0 < *(int *)(param_1 + 0x6199)) {
            do {
              iVar4 = *local_1c;
              iVar2 = *(int *)(DAT_0065660c + 0xcc +
                              (uint)*(ushort *)
                                     (iVar8 + (iVar4 + iVar10 + (&DAT_006cb8c2)[local_18]) * 0xc) *
                              0x164);
              if (local_18 == 0) {
                iVar7 = 0;
              }
              else {
                iVar7 = (&DAT_006cb8be)[local_18] + iVar4 + iVar10;
                if ((*(byte *)(iVar8 + 10 + iVar7 * 0xc) & 1) == 0) {
                  iVar7 = *(int *)(DAT_0065660c + 0xcc +
                                  (uint)*(ushort *)(iVar8 + iVar7 * 0xc) * 0x164);
                }
                else {
                  iVar7 = 1;
                }
              }
              if (((((iVar2 == 8) || (iVar2 == 4)) || (iVar2 == 6)) || (iVar2 == 3)) ||
                 ((iVar2 == 2 && (((iVar7 == 1 || (iVar7 == 4)) || (iVar7 == 5)))))) {
                uVar5 = __ftol();
                uVar6 = __ftol();
                iVar8 = (&DAT_006cb8c2)[local_18 >> 1] + iVar4 + DAT_006c5c5c;
                if (iVar2 == 8) {
                  sVar9 = (-(ushort)(DAT_006e1f88 != 0) & 0xe280) + 0x3b07;
LAB_00555366:
                  local_4 = CONCAT22(local_4._2_2_,sVar9);
                }
                else {
                  if (iVar2 == 4) {
                    sVar9 = (-(ushort)(DAT_006e1f88 != 0) & 0xd6e0) + 0x5224;
                    goto LAB_00555366;
                  }
                  if ((iVar2 == 2) || (iVar2 == 6)) {
                    local_4 = CONCAT22(local_4._2_2_,
                                       (-(ushort)(DAT_006e1f88 != 0) & 0xb6c0) + 0x9282);
                  }
                  if (iVar2 == 3) {
                    sVar9 = (-(ushort)(DAT_006e1f88 != 0) & 0xdf00) + 0x4208;
                    goto LAB_00555366;
                  }
                }
                *local_2c = uVar5;
                local_2c[1] = uVar6;
                local_2c[2] = iVar8 + iVar10;
                local_2c[3] = local_4;
                iVar8 = DAT_006c54dc;
              }
              local_2c = local_2c + 4;
              iVar10 = iVar10 + 1;
            } while (iVar10 < *(int *)(param_1 + 0x6199));
          }
          local_30 = local_30 + 1;
          local_1c = local_1c + 1;
        } while (local_30 < *(int *)(param_1 + 0x619d));
      }
      local_18 = local_18 + 1;
    } while (local_18 < DAT_006c549c);
  }
  return;
}

