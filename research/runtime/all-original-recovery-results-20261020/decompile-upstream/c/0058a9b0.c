
void __fastcall FUN_0058a9b0(undefined4 param_1,undefined4 param_2)

{
  int iVar1;
  int *piVar2;
  undefined *puVar3;
  
  iVar1 = DirectDrawCreate(DAT_006e1ecc,&DAT_006f98c4,0);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  iVar1 = (**(code **)(*DAT_006f98c4 + 0x50))(DAT_006f98c4,param_1,0x11);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  puVar3 = &DAT_005c7960;
  piVar2 = DAT_006f98c4;
  iVar1 = (**(code **)*DAT_006f98c4)(DAT_006f98c4,&DAT_005c7960,&DAT_006f98c8);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  iVar1 = (**(code **)*DAT_006f98c4)(DAT_006f98c4,&DAT_005c7970,&DAT_006f98cc);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  iVar1 = (**(code **)(*DAT_006f98c8 + 0x54))(DAT_006f98c8,param_2,piVar2,puVar3,0,0);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  DAT_006f68d8 = 1;
  DAT_006f8174 = (uint)(puVar3 < (undefined *)0x9);
  FUN_0059b7e2(FUN_0058acf0);
  return;
}

