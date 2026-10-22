
void __fastcall FUN_004c16e0(undefined4 *param_1)

{
  int *piVar1;
  int local_14;
  void *pvStack_c;
  undefined1 *puStack_8;
  int local_4;
  
  puStack_8 = &LAB_005c1890;
  pvStack_c = ExceptionList;
  ExceptionList = &pvStack_c;
  *param_1 = &PTR_LAB_005c6a8c;
  *(undefined ***)((int)param_1 + 0x21) = &PTR_FUN_005c6a7c;
  local_4 = 0x11;
  if (*(int *)((int)param_1 + 0xf3) != 0) {
    FUN_005979b0(*(int *)((int)param_1 + 0xf3));
  }
  if (*(int *)((int)param_1 + 0x29) != 0) {
    FUN_00597870(*(int *)((int)param_1 + 0x29));
  }
  FUN_004a4f70();
  param_1[1] = 0xffffffff;
  piVar1 = (int *)((int)param_1 + 0x2d);
  local_14 = 2;
  do {
    if ((int *)*piVar1 != (int *)0x0) {
      (**(code **)(*(int *)*piVar1 + 4))(1);
      *piVar1 = 0;
    }
    piVar1 = piVar1 + 1;
    local_14 = local_14 + -1;
  } while (local_14 != 0);
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x65b7));
  *(undefined4 *)((int)param_1 + 0x65b7) = 0;
  *(undefined4 *)((int)param_1 + 0x65bb) = 0;
  *(undefined4 *)((int)param_1 + 0x65bf) = 0;
  local_4._0_1_ = 0xf;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x65a3),*(undefined4 *)((int)param_1 + 0x65a7));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x65a3));
  *(undefined4 *)((int)param_1 + 0x65a3) = 0;
  *(undefined4 *)((int)param_1 + 0x65a7) = 0;
  *(undefined4 *)((int)param_1 + 0x65ab) = 0;
  local_4._0_1_ = 0xe;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6593),*(undefined4 *)((int)param_1 + 0x6597));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6593));
  *(undefined4 *)((int)param_1 + 0x6593) = 0;
  *(undefined4 *)((int)param_1 + 0x6597) = 0;
  *(undefined4 *)((int)param_1 + 0x659b) = 0;
  local_4._0_1_ = 0xd;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6583),*(undefined4 *)((int)param_1 + 0x6587));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6583));
  *(undefined4 *)((int)param_1 + 0x6583) = 0;
  *(undefined4 *)((int)param_1 + 0x6587) = 0;
  *(undefined4 *)((int)param_1 + 0x658b) = 0;
  local_4._0_1_ = 0xc;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6573),*(undefined4 *)((int)param_1 + 0x6577));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6573));
  *(undefined4 *)((int)param_1 + 0x6573) = 0;
  *(undefined4 *)((int)param_1 + 0x6577) = 0;
  *(undefined4 *)((int)param_1 + 0x657b) = 0;
  local_4._0_1_ = 0xb;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6563),*(undefined4 *)((int)param_1 + 0x6567));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6563));
  *(undefined4 *)((int)param_1 + 0x6563) = 0;
  *(undefined4 *)((int)param_1 + 0x6567) = 0;
  *(undefined4 *)((int)param_1 + 0x656b) = 0;
  local_4._0_1_ = 10;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6553),*(undefined4 *)((int)param_1 + 0x6557));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6553));
  *(undefined4 *)((int)param_1 + 0x6553) = 0;
  *(undefined4 *)((int)param_1 + 0x6557) = 0;
  *(undefined4 *)((int)param_1 + 0x655b) = 0;
  local_4._0_1_ = 9;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6543),*(undefined4 *)((int)param_1 + 0x6547));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6543));
  *(undefined4 *)((int)param_1 + 0x6543) = 0;
  *(undefined4 *)((int)param_1 + 0x6547) = 0;
  *(undefined4 *)((int)param_1 + 0x654b) = 0;
  local_4._0_1_ = 8;
  FUN_0042a140(*(undefined4 *)((int)param_1 + 0x6533),*(undefined4 *)((int)param_1 + 0x6537));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x6533));
  *(undefined4 *)((int)param_1 + 0x6533) = 0;
  *(undefined4 *)((int)param_1 + 0x6537) = 0;
  *(undefined4 *)((int)param_1 + 0x653b) = 0;
  local_4._0_1_ = 7;
  FUN_004c1ab0();
  local_4._0_1_ = 6;
  FUN_0058b3e0();
  local_4._0_1_ = 5;
  FUN_004c1b10(*(undefined4 *)((int)param_1 + 0x618d),*(undefined4 *)((int)param_1 + 0x6191));
  FUN_00597870(*(undefined4 *)((int)param_1 + 0x618d));
  *(undefined4 *)((int)param_1 + 0x618d) = 0;
  *(undefined4 *)((int)param_1 + 0x6191) = 0;
  *(undefined4 *)((int)param_1 + 0x6195) = 0;
  local_4._0_1_ = 4;
  FUN_004c1a00();
  FUN_004c1660();
  FUN_004c1630();
  local_4._0_1_ = 1;
  FUN_004c1ab0();
  local_4 = (uint)local_4._1_3_ << 8;
  FUN_004c1ab0();
  local_4 = 0xffffffff;
  *param_1 = &PTR_LAB_005c5c20;
  FUN_004a4f70();
  param_1[1] = 0xffffffff;
  ExceptionList = pvStack_c;
  return;
}

