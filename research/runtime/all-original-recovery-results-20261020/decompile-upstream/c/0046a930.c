
void __fastcall FUN_0046a930(int param_1)

{
  char cVar1;
  int iVar2;
  int iVar3;
  undefined1 *puVar4;
  undefined4 local_150;
  undefined1 local_14c [16];
  undefined1 local_13c [16];
  undefined1 local_12c [276];
  undefined4 local_18;
  void *local_c;
  undefined1 *puStack_8;
  uint local_4;
  
  local_4 = 0xffffffff;
  puStack_8 = &LAB_005bf486;
  local_c = ExceptionList;
  ExceptionList = &local_c;
  FUN_00559820();
  local_4 = 0;
  FUN_004d27b0();
  local_4 = CONCAT31(local_4._1_3_,1);
  FUN_00477060();
  cVar1 = FUN_00479d40();
  if (cVar1 != '\0') {
    FUN_00559230(&DAT_005fc260);
    iVar2 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    iVar3 = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    FUN_0059c1fd(&DAT_005fc464,s_Interface_ExperienceProcessing___005d9edc,iVar3,iVar2,&DAT_005fc260
                 ,iVar3,iVar2);
    FUN_0058d320(&DAT_005fc464);
    FUN_00558580(6,1);
    FUN_004d28b0(local_150);
    FUN_004d2960(&DAT_006a49e0);
    FUN_004d28c0(local_12c);
    FUN_004d2970(local_18);
    if (DAT_006de6d5 == 0) {
      puVar4 = local_14c;
    }
    else {
      puVar4 = local_13c;
    }
    FUN_004d2930(puVar4);
    FUN_004d2800();
    FUN_004dc4a0(((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x2a) / 2,
                 (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1bc,8);
    FUN_00477380(param_1 + 0xfd66);
    iVar2 = FUN_00477d30(param_1 + 0xfd66);
    if (iVar2 == 0) {
      FUN_00479700();
    }
    else {
      FUN_004796f0();
      FUN_00479d30();
    }
    FUN_004dc6e0();
  }
  local_4 = local_4 & 0xffffff00;
  FUN_004d27d0();
  local_4 = 0xffffffff;
  FUN_00559880();
  ExceptionList = local_c;
  return;
}

