
void FUN_004a4910(void)

{
  DWORD DVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  uint *puVar6;
  tagPOINT local_58;
  tagPOINT local_50;
  tagPOINT local_48;
  int local_40;
  int local_3c;
  int local_38;
  int local_34;
  uint local_30;
  uint local_2c;
  int local_28;
  int local_24;
  uint local_20;
  uint local_1c;
  int local_14;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  int local_4;
  
  if (DAT_005e04c8 != 0x16) {
    local_48.x = 0;
    local_48.y = 0;
    if (((-1 < DAT_005e04c8) && (DAT_005e04c8 < 0x16)) &&
       (DVar1 = WaitForSingleObject(DAT_006e1ec0,10), DVar1 == 0)) {
      if (DAT_006c5038 != 0) {
        if (0 < DAT_00641fc0) {
          DAT_00641fc0 = DAT_00641fc0 + -1;
        }
        if (DAT_00642c60 != 0) {
          ClientToScreen(DAT_00656614,&local_48);
          local_20 = local_48.x;
          iVar4 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280 + local_48.x;
          local_1c = local_48.y;
          local_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0 + local_48.y;
          GetCursorPos(&local_50);
          local_58.x = local_50.x;
          local_58.y = local_50.y;
          ScreenToClient(DAT_00656614,&local_58);
          iVar2 = DAT_005e04c8 * 0x44;
          local_58.y = local_58.y - *(int *)(iVar2 + 0x64267c);
          local_58.x = local_58.x - *(int *)(iVar2 + 0x642678);
          local_50.x = local_50.x - *(int *)(iVar2 + 0x642678);
          local_50.x = (local_50.x < 1) - 1 & local_50.x;
          local_50.y = local_50.y - *(int *)(iVar2 + 0x64267c);
          local_50.y = (local_50.y < 1) - 1 & local_50.y;
          if (((DAT_00641aa0 != local_58.x) || (DAT_00641aa4 != local_58.y)) ||
             ((*(int *)(iVar2 + 0x642680) != 1 && (DAT_00641fc0 < 1)))) {
            WaitForSingleObject(DAT_00642c58,0xffffffff);
            if (DAT_00642c54 != 0) {
              local_8 = DAT_00641a80 - DAT_00641a78;
              local_4 = DAT_00641a84 - DAT_00641a7c;
              local_10 = 0;
              local_c = 0;
              if ((local_8 != 0) && (local_4 != 0)) {
                FUN_0058c360(&DAT_006a4fe0,&local_10,((int)DAT_00641aa0 < 0) - 1 & DAT_00641aa0,
                             ((int)DAT_00641aa4 < 0) - 1 & DAT_00641aa4);
              }
            }
            DAT_00641a78 = (local_58.x < 0) - 1 & local_58.x;
            DAT_00641a80 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
            if ((int)DAT_00641a80 <= (int)DAT_00641a78) {
              DAT_00641a78 = DAT_00641a80;
            }
            DAT_00641a7c = (local_58.y < 0) - 1 & local_58.y;
            DAT_00641a84 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
            if ((int)DAT_00641a84 <= (int)DAT_00641a7c) {
              DAT_00641a7c = DAT_00641a84;
            }
            uVar5 = local_58.x + 0x28;
            uVar3 = uVar5;
            if ((int)DAT_00641a80 <= (int)uVar5) {
              uVar3 = DAT_00641a80;
            }
            if ((int)uVar3 < 0) {
              DAT_00641a80 = 0;
            }
            else if ((int)uVar5 < (int)DAT_00641a80) {
              DAT_00641a80 = uVar5;
            }
            uVar3 = local_58.y + 0x28;
            uVar5 = uVar3;
            if ((int)DAT_00641a84 <= (int)uVar3) {
              uVar5 = DAT_00641a84;
            }
            if ((int)uVar5 < 0) {
              DAT_00641a84 = 0;
            }
            else if ((int)uVar3 < (int)DAT_00641a84) {
              DAT_00641a84 = uVar3;
            }
            if ((DAT_00641a78 != DAT_00641a80) && (DAT_00641a7c != DAT_00641a84)) {
              if (DAT_006f68d8 == 0) {
                local_30 = local_20;
                if ((int)local_20 <= local_50.x) {
                  local_30 = local_50.x;
                }
                local_2c = local_1c;
                if ((int)local_1c <= local_50.y) {
                  local_2c = local_50.y;
                }
                local_28 = local_50.x + 0x28U;
                if (iVar4 <= (int)(local_50.x + 0x28U)) {
                  local_28 = iVar4;
                }
                local_24 = local_50.y + 0x28;
                if (local_14 <= local_24) {
                  local_24 = local_14;
                }
                puVar6 = &local_30;
              }
              else {
                puVar6 = &DAT_00641a78;
              }
              FUN_0058c360(&DAT_00641fc8,puVar6,0,0);
              DAT_00642c54 = 1;
              DAT_00641aa0 = local_58.x;
              DAT_00641aa4 = local_58.y;
            }
            ReleaseMutex(DAT_00642c58);
            iVar2 = (local_58.x - ((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280)) + 0x28;
            iVar4 = (local_58.y - ((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0)) + 0x28;
            local_38 = DAT_00641a98;
            if ((0 < iVar2) && (local_38 = DAT_00641a98 - iVar2, local_38 < DAT_00641a90)) {
              local_38 = DAT_00641a90;
            }
            local_34 = DAT_00641a9c;
            if ((0 < iVar4) && (local_34 = DAT_00641a9c - iVar4, local_34 < DAT_00641a94)) {
              local_34 = DAT_00641a94;
            }
            local_40 = DAT_00641a90;
            if ((local_58.x < 0) &&
               (local_40 = DAT_00641a90 - local_58.x, local_38 < DAT_00641a90 - local_58.x)) {
              local_40 = local_38;
            }
            local_3c = DAT_00641a94;
            if ((local_58.y < 0) &&
               (local_3c = DAT_00641a94 - local_58.y, local_34 < DAT_00641a94 - local_58.y)) {
              local_3c = local_34;
            }
            if ((((local_40 != local_38) && (local_3c != local_34)) &&
                (FUN_0058ca90(&DAT_006a4fe0,&local_40,(local_58.x < 0) - 1 & local_58.x,
                              (local_58.y < 0) - 1 & local_58.y), DAT_00641fc0 < 1)) &&
               ((iVar2 = *(int *)(DAT_005e04c8 * 0x44 + 0x642680), iVar2 != 1 &&
                (DAT_005e04c8 != 0x16)))) {
              DAT_00642c5c = DAT_00642c5c + 1;
              if (iVar2 <= DAT_00642c5c) {
                DAT_00642c5c = 0;
              }
              DAT_00641fc0 = *(int *)(&DAT_006426b8 + DAT_005e04c8 * 0x44);
              DAT_00641a90 = (*(int *)(&DAT_00642684 + (DAT_00642c5c + DAT_005e04c8 * 0x11) * 4) %
                             DAT_00641a8c) * 0x28;
              DAT_00641a98 = DAT_00641a90 + 0x28;
              DAT_00641a94 = (*(int *)(&DAT_00642684 + (DAT_00642c5c + DAT_005e04c8 * 0x11) * 4) /
                             DAT_00641a8c) * 0x28;
              DAT_00641a9c = DAT_00641a94 + 0x28;
            }
          }
        }
      }
      ReleaseMutex(DAT_006e1ec0);
    }
  }
  return;
}

