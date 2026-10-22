
int FUN_00484930(void)

{
  undefined8 uVar1;
  undefined8 uVar2;
  undefined4 uVar3;
  undefined4 uVar4;
  uint uVar5;
  int iVar6;
  uint uVar7;
  
  uVar7 = 0;
  FUN_0057dc20((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0 + DAT_006e1f74 * 2);
  FUN_0057dc60(*(int *)(DAT_006c482c + 8) + DAT_006e1f70 * 2,
               *(int *)(DAT_006c482c + 0xc) + DAT_006e1f74 * 2);
  uVar3 = FUN_00597880(0xfa000);
  FUN_005898e0(0,0);
  uVar4 = FUN_0058b660();
  (**(code **)(*DAT_006a49e4 + 0x80))(DAT_006a49e4,uVar4);
  iVar6 = 0x14;
LAB_00484a0a:
  do {
    uVar1 = rdtsc();
    FUN_005899e0(uVar3,200,200);
    uVar2 = rdtsc();
    uVar7 = uVar7 + 1;
    if ((uint)uVar2 < (uint)uVar1) {
      if (uVar7 < 3) goto LAB_00484a0a;
    }
    uVar5 = (uint)uVar2 - (uint)uVar1;
    if ((uVar7 != 3) && (uVar5 < DAT_005da920)) {
      DAT_005da920 = uVar5;
    }
    uVar7 = 0;
    do {
      uVar1 = rdtsc();
      (*DAT_006903dc)(uVar3,400,200);
      uVar2 = rdtsc();
      uVar7 = uVar7 + 1;
      if ((uint)uVar1 <= (uint)uVar2) break;
    } while (uVar7 < 3);
    uVar5 = (uint)uVar2 - (uint)uVar1;
    if ((uVar7 != 3) && (uVar5 < DAT_005da924)) {
      DAT_005da924 = uVar5;
    }
    iVar6 = iVar6 + -1;
    if (iVar6 == 0) {
      iVar6 = DAT_005da920 - DAT_005da924;
      FUN_004d8850(iVar6,(iVar6 * 100) / (int)DAT_005da924);
      FUN_005979b0(uVar3);
      return iVar6;
    }
  } while( true );
}

