
void FUN_004a45b0(void)

{
  DWORD DVar1;
  int iVar2;
  uint uVar3;
  int iVar4;
  uint uVar5;
  tagPOINT local_38;
  int local_30;
  int local_2c;
  int local_28;
  int local_24;
  RECT local_20;
  RECT local_10;
  
  if (((DAT_00642c64 != 0) && (DAT_006c5038 != 0)) && (DAT_005e04c8 != 0x16)) {
    local_20.left = 0;
    local_20.top = 0;
    local_20.right = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    local_20.bottom = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    GetCursorPos(&local_38);
    ScreenToClient(DAT_00656614,&local_38);
    local_38.x = local_38.x - *(int *)(DAT_005e04c8 * 0x44 + 0x642678);
    local_38.y = local_38.y - *(int *)(DAT_005e04c8 * 0x44 + 0x64267c);
    local_10.bottom = local_38.y + 0x28;
    local_10.right = local_38.x + 0x28;
    local_10.left = local_38.x;
    local_10.top = local_38.y;
    DVar1 = WaitForSingleObject(DAT_00642c58,10);
    if (DVar1 == 0) {
      IntersectRect((LPRECT)&DAT_00641a78,&local_10,&local_20);
      if ((DAT_00641a78 != DAT_00641a80) && (DAT_00641a7c != DAT_00641a84)) {
        if (DAT_006e1ee4 == 0) {
          FUN_0058c4a0(&DAT_00641fc8,&DAT_00641a78,0,0);
        }
        else {
          FUN_0058d5b0(2,&DAT_00641fc8,&DAT_00641a78,0,0,1);
        }
        DAT_00641aa0 = local_38.x;
        DAT_00641aa4 = local_38.y;
        DAT_00642c54 = 1;
      }
      if (((DAT_00641fc0 < 1) && (iVar2 = *(int *)(DAT_005e04c8 * 0x44 + 0x642680), iVar2 != 1)) &&
         (DAT_005e04c8 != 0x16)) {
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
      ReleaseMutex(DAT_00642c58);
      iVar2 = (local_38.x - local_20.right) + 0x28;
      iVar4 = (local_38.y - local_20.bottom) + 0x28;
      local_28 = DAT_00641a98;
      if ((0 < iVar2) && (local_28 = DAT_00641a98 - iVar2, local_28 < DAT_00641a90)) {
        local_28 = DAT_00641a90;
      }
      local_24 = DAT_00641a9c;
      if ((0 < iVar4) &&
         (iVar4 = DAT_00641a9c - iVar4, local_24 = DAT_00641a94, DAT_00641a94 <= iVar4)) {
        local_24 = iVar4;
      }
      local_30 = DAT_00641a90;
      if ((local_38.x < 0) &&
         (local_30 = DAT_00641a90 - local_38.x, local_28 < DAT_00641a90 - local_38.x)) {
        local_30 = local_28;
      }
      local_2c = DAT_00641a94;
      if ((local_38.y < 0) && (local_2c = local_24, DAT_00641a94 - local_38.y <= local_24)) {
        local_2c = DAT_00641a94 - local_38.y;
      }
      if ((local_30 != local_28) && (local_2c != local_24)) {
        uVar3 = (local_38.y < 0) - 1 & local_38.y;
        uVar5 = (local_38.x < 0) - 1 & local_38.x;
        if (DAT_006e1ee4 == 0) {
          FUN_0058c8a0(&DAT_006a49e0,&local_30,uVar5,uVar3);
        }
        else {
          FUN_0058d5b0(0,&DAT_006a49e0,&local_30,uVar5,uVar3,1);
        }
        DAT_00641aa8 = local_38.x & (local_38.x < 0) - 1;
        DAT_00641aac = local_38.y & (local_38.y < 0) - 1;
        DAT_00641ab0 = (DAT_00641aa8 - local_30) + local_28;
        DAT_00641ab4 = (DAT_00641aac - local_2c) + local_24;
      }
    }
  }
  return;
}

