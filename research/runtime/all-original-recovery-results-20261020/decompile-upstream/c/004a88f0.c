
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004a88f0(void)

{
  int iVar1;
  int cy;
  UINT uFlags;
  tagRECT tStack_110;
  undefined1 auStack_100 [256];
  
  FUN_004a37d0();
  FUN_004db6d0();
  FUN_004d83a0();
  FUN_0058acf0();
  if (DAT_006de6d5 == 0) {
    DAT_006c482c = &DAT_005e1700 + DAT_006de6d9 * 0x10;
  }
  else {
    DAT_006c482c = &DAT_005e1720 + DAT_006de6d9 * 0x10;
  }
  FUN_004f7970(DAT_006c482c);
  if (DAT_006a2920 == 0) {
    uFlags = 2;
    cy = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    iVar1 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
  }
  else {
    tStack_110.right = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    tStack_110.left = 0;
    tStack_110.top = 0;
    tStack_110.bottom = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    AdjustWindowRect(&tStack_110,0x12ca0000,0);
    iVar1 = tStack_110.right - tStack_110.left;
    uFlags = 0;
    cy = tStack_110.bottom - tStack_110.top;
    tStack_110.right = iVar1;
    tStack_110.bottom = cy;
  }
  SetWindowPos(DAT_00656614,(HWND)0x0,0,0,iVar1,cy,uFlags);
  FUN_004e40d0();
  FUN_004d5e50(0);
  FUN_004d9360(0);
  FUN_004db710();
  FUN_004d83b0();
  FUN_004a30b0();
  FUN_0059c1fd(auStack_100,s_sprites_heading_text__i_sft_005e0690,
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280);
  iVar1 = FUN_004a5ae0(auStack_100,2);
  if (iVar1 == 0) {
    FUN_0049e4e0();
  }
  _DAT_006b0630 = 2;
  FUN_0059c1fd(auStack_100,s_sprites_body_text__i_sft_005e0674,
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280);
  iVar1 = FUN_004a5ae0(auStack_100,2);
  if (iVar1 == 0) {
    FUN_0049e4e0();
  }
  _DAT_006568b0 = 2;
  return;
}

