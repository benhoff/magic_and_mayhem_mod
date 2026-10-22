
void __fastcall FUN_005571a0(undefined4 *param_1)

{
  (**(code **)(*DAT_006f34e0 + 4))();
  DAT_006e1ef8 = DAT_006e1efc;
  DAT_006f34e0 = param_1;
  *(undefined4 **)(&DAT_006f34a0 + DAT_006f349c * 4) = param_1;
  (**(code **)*param_1)();
  return;
}

