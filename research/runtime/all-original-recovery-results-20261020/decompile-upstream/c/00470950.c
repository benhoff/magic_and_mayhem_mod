
void __fastcall FUN_00470950(int param_1)

{
  undefined4 uVar1;
  char cVar2;
  int iVar3;
  
  FUN_004dc4a0(((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x2a) / 2,
               (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1bc,0x30);
  DAT_006dbe9d = DAT_0068991c != 0 || DAT_006de6f1 != 0;
  FUN_0054e170(DAT_00656624);
  FUN_0046ea90(&DAT_006ddd58,DAT_00656620);
  iVar3 = FUN_0046e5e0();
  if (iVar3 == 0) {
    FUN_0049e680();
    FUN_00557130();
    return;
  }
  FUN_00464b90(&DAT_006a28f8);
  iVar3 = FUN_0057d310(s_sprites_lordking_spr_005da1ac,0x10);
  if ((iVar3 == 0) || (iVar3 = FUN_004644d0(s_sprites_lordking_ani_005da170,1), iVar3 == 0)) {
    FUN_0049e680();
    FUN_00557130();
  }
  DAT_006e980c = 0;
  DAT_006e9810 = 0;
  DAT_006e9814 = 0;
  DAT_006e9818 = 0;
  FUN_00473890();
  FUN_004a0e30();
  DAT_006c4830 = 0;
  FUN_00473100();
  FUN_00416b80();
  FUN_00416b80();
  FUN_0057b640();
  FUN_004dc5e0();
  if (DAT_006e1f0c == 0) {
    if (DAT_0068991c == 0) {
      if (DAT_00689920 == 1) {
        cVar2 = FUN_004665b0();
      }
      else {
        cVar2 = FUN_00465c60();
      }
    }
    else {
      cVar2 = FUN_00466420(DAT_00656630,DAT_00656634);
    }
  }
  else {
    cVar2 = FUN_004661c0();
  }
  if (cVar2 == '\0') {
    FUN_0049e680();
    FUN_00557130();
    return;
  }
  *(undefined4 *)(param_1 + 0x10317) = 0;
  if (((DAT_00689920 == 2) || (DAT_00689920 == 1)) && (0 < DAT_006c3778)) {
    *(undefined4 *)(param_1 + 0x10317) = 1;
  }
  FUN_0049cc80();
  FUN_0053aa30();
  if (DAT_006a49c0 != 0) {
    FUN_00597870(DAT_006a49c0);
    DAT_006a49c0 = 0;
  }
  FUN_004dc5e0();
  iVar3 = FUN_004eda90(&DAT_006ddd58,DAT_00656620,DAT_00656624);
  if (iVar3 == 0) {
    FUN_0049e680();
    FUN_00557130();
    return;
  }
  DAT_005fed78 = 0;
  FUN_004f3230(DAT_006e2020);
  FUN_004f7930(&DAT_006c5490);
  FUN_00534310();
  FUN_004dc5e0();
  if (DAT_006e1f3c != 0) {
    if (*(int *)((DAT_006eaf0c < 1) - 1 & DAT_006eaf08) == 0) {
      FUN_0053ab20(1);
      FUN_0053d710();
      FUN_0053bb40(1,0);
    }
    else {
      FUN_0053f710();
    }
  }
  FUN_00544e00();
  FUN_00448e60();
  FUN_00470500();
  FUN_0055a5b0();
  FUN_0055a650(&DAT_006dde58,DAT_00656620);
  FUN_005606f0();
  uVar1 = DAT_005e1784;
  if (DAT_006e266c != 0) {
    FUN_00597870(DAT_006e266c);
  }
  DAT_006e266c = FUN_00597850(uVar1);
  *(int *)(param_1 + 0xfdea) = DAT_00657cdc + DAT_006c4830;
  cVar2 = FUN_00479d40();
  if ((cVar2 != '\0') || (iVar3 = FUN_00479a10(PTR_s_AI_BRAIN_DAT_005c5d48), iVar3 != 0)) {
    FUN_00477380(0);
    iVar3 = FUN_00477d30(0);
    if (iVar3 == 0) {
      FUN_00479700();
    }
    else {
      FUN_004796f0();
      FUN_00479d30();
    }
  }
  FUN_00472970();
  DAT_006903e8 = 0;
  FUN_004a5850(0x10);
  *(undefined1 *)(param_1 + 0x100a0) = 0;
  *(undefined1 *)(param_1 + 0x100a1) = 0;
  *(undefined1 *)(param_1 + 0x100a3) = 0;
  *(undefined1 *)(param_1 + 0x100a5) = 0;
  *(undefined1 *)(param_1 + 0x100a7) = 0;
  *(undefined1 *)(param_1 + 0x100ad) = 0;
  *(undefined1 *)(param_1 + 0x100ae) = 0;
  *(undefined1 *)(param_1 + 0x100af) = 0;
  *(undefined4 *)(param_1 + 0x100a8) = 0;
  FUN_004da780();
  return;
}

