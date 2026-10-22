
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_005575b0(int param_1)

{
  char cVar1;
  int iVar2;
  DWORD DVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  char *pcVar7;
  undefined4 uVar8;
  undefined1 auStack_100 [256];
  
  FUN_0059c1fd(auStack_100,s_sprites_heading_text__i_sft_005e0690,
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280);
  iVar2 = FUN_004a5ae0(auStack_100,2);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  _DAT_006b0630 = 2;
  FUN_0059c1fd(auStack_100,s_sprites_body_text__i_sft_005e0674,
               (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280);
  iVar2 = FUN_004a5ae0(auStack_100,2);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  _DAT_006568b0 = 2;
  iVar2 = FUN_004a5ae0(s_sprites_toolTip_text_sft_005ec0e4,2);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  _DAT_006df3f0 = 2;
  iVar2 = FUN_004a5ae0(s_sprites_yellowtext_sft_005ec0cc,2);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  _DAT_006a5368 = 2;
  iVar2 = FUN_0057d310(s_sprites_Buttons_spr_005ec0b8,2);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  FUN_0057d310(s_sprites_border_spr_005ec0a4,2);
  FUN_004dba20(&DAT_006df1c0);
  FUN_004db650();
  FUN_004dc0f0(0);
  FUN_004d8630(&DAT_006a5138);
  FUN_004d85f0();
  FUN_004d82f0();
  FUN_0049c280();
  if (DAT_00642c64 != 0) {
    FUN_004a30b0();
    FUN_00534aa0(0x14,5);
    FUN_00534ad0(&LAB_004eaaf0);
  }
  iVar2 = FUN_00502b20(&DAT_006a5f80);
  if (iVar2 == 0) {
    FUN_0049e4e0();
  }
  FUN_0054d070();
  if (DAT_006de6d5 == 0) {
    DAT_006c482c = &DAT_005e1700 + DAT_006de6d9 * 0x10;
  }
  else {
    DAT_006c482c = &DAT_005e1720 + DAT_006de6d9 * 0x10;
  }
  FUN_004f7860(&DAT_006c5490,&DAT_006a49e0,&DAT_006de558,DAT_006c482c,0);
  FUN_004db4f0(&DAT_00656680);
  FUN_004dad50();
  FUN_004d9360(1);
  FUN_00554830();
  FUN_004d5e50(1);
  FUN_004d3690();
  FUN_004da200();
  FUN_004dc830();
  DVar3 = timeGetTime();
  FUN_0054e170(DVar3);
  *(undefined4 *)(param_1 + 0x26) = 0;
  if (DAT_006de6c5 == '\0') {
    *(undefined **)(param_1 + 0x22) = &DAT_00657ce0;
    return;
  }
  FUN_004d28b0(&DAT_006b0400);
  FUN_004d2960(&DAT_006a49e0);
  if ((DAT_006de6c0 == 0) || ((*(byte *)(DAT_006de6c0 + 4) & 2) == 0)) {
    uVar8 = 0xf;
  }
  else {
    uVar8 = 0xe;
  }
  uVar8 = FUN_0058a680(uVar8);
  FUN_004d28c0(uVar8);
  FUN_004d2970(4);
  _DAT_006f348d = 200;
  _DAT_006f3491 = 200;
  _DAT_006f3495 = 200;
  FUN_004d2930(&stack0xfffffeec);
  FUN_004d2800();
  FUN_0058bf40(&DAT_006a4fe0,0,0);
  uVar4 = 0xffffffff;
  pcVar6 = s_realms_multiplayer_multiplayer_c_005ec060;
  do {
    pcVar7 = pcVar6;
    if (uVar4 == 0) break;
    uVar4 = uVar4 - 1;
    pcVar7 = pcVar6 + 1;
    cVar1 = *pcVar6;
    pcVar6 = pcVar7;
  } while (cVar1 != '\0');
  uVar4 = ~uVar4;
  pcVar6 = pcVar7 + -uVar4;
  pcVar7 = (char *)&DAT_006ddd58;
  for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
    *(undefined4 *)pcVar7 = *(undefined4 *)pcVar6;
    pcVar6 = pcVar6 + 4;
    pcVar7 = pcVar7 + 4;
  }
  for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
    *pcVar7 = *pcVar6;
    pcVar6 = pcVar6 + 1;
    pcVar7 = pcVar7 + 1;
  }
  FUN_004ca370();
  return;
}

