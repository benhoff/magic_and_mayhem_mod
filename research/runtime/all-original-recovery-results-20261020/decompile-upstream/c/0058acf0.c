
void FUN_0058acf0(void)

{
  if (DAT_006f98c8 == (int *)0x0) {
    return;
  }
  while (DAT_006f6950 != 0) {
    FUN_0058b3e0();
  }
  if (DAT_006f68d8 == 0) {
    if (DAT_006f98d0 == (int *)0x0) goto LAB_0058ad50;
    (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
    DAT_006f98d0 = (int *)0x0;
    if (DAT_006f68d8 == 0) goto LAB_0058ad50;
  }
  (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
  DAT_006f68d8 = 0;
LAB_0058ad50:
  (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
  (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
  (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
  DAT_006f98cc = (int *)0x0;
  DAT_006f98c8 = (int *)0x0;
  DAT_006f98c4 = (int *)0x0;
  DAT_006f98c0 = 0;
  return;
}

