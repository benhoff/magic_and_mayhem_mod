
void __fastcall FUN_0058aa90(HWND param_1)

{
  int iVar1;
  HDC hdc;
  
  iVar1 = DirectDrawCreate(DAT_006e1ecc,&DAT_006f98c4,0);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  iVar1 = (**(code **)(*DAT_006f98c4 + 0x50))(DAT_006f98c4,param_1,8);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  iVar1 = (**(code **)*DAT_006f98c4)(DAT_006f98c4,&DAT_005c7960,&DAT_006f98c8);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  iVar1 = (**(code **)*DAT_006f98c4)(DAT_006f98c4,&DAT_005c7970,&DAT_006f98cc);
  if (iVar1 != 0) {
    FUN_00484c00(iVar1);
  }
  if (DAT_006f68d8 == 0) {
    iVar1 = (**(code **)(*DAT_006f98c8 + 0x10))(DAT_006f98c8,0,&DAT_006f98d0,0);
    if (iVar1 != 0) {
      FUN_00484c00(iVar1);
    }
    iVar1 = (**(code **)(*DAT_006f98d0 + 0x20))(DAT_006f98d0,0,param_1);
    if (iVar1 != 0) {
      if (DAT_006f98c8 != (int *)0x0) {
        while (DAT_006f6950 != 0) {
          FUN_0058b3e0();
        }
        if (DAT_006f68d8 == 0) {
          if (DAT_006f98d0 != (int *)0x0) {
            (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
            DAT_006f98d0 = (int *)0x0;
            if (DAT_006f68d8 != 0) goto LAB_0058abaf;
          }
        }
        else {
LAB_0058abaf:
          (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
          DAT_006f68d8 = 0;
        }
        (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
        (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
        (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
        DAT_006f98c4 = (int *)0x0;
        DAT_006f98c8 = (int *)0x0;
        DAT_006f98cc = (int *)0x0;
        DAT_006f98c0 = 0;
      }
      DestroyWindow(DAT_00656614);
    }
  }
  DAT_006f68d8 = 0;
  hdc = GetDC(param_1);
  if (hdc != (HDC)0x0) goto LAB_0058acb6;
  if (DAT_006f98c8 != (int *)0x0) {
    while (DAT_006f6950 != 0) {
      FUN_0058b3e0();
    }
    if (DAT_006f68d8 == 0) {
      if (DAT_006f98d0 != (int *)0x0) {
        (**(code **)(*DAT_006f98d0 + 8))(DAT_006f98d0);
        DAT_006f98d0 = (int *)0x0;
        if (DAT_006f68d8 != 0) goto LAB_0058ac63;
      }
    }
    else {
LAB_0058ac63:
      (**(code **)(*DAT_006f98c8 + 0x4c))(DAT_006f98c8);
      DAT_006f68d8 = 0;
    }
    (**(code **)(*DAT_006f98c4 + 8))(DAT_006f98c4);
    (**(code **)(*DAT_006f98c8 + 8))(DAT_006f98c8);
    (**(code **)(*DAT_006f98cc + 8))(DAT_006f98cc);
    DAT_006f98c4 = (int *)0x0;
    DAT_006f98c8 = (int *)0x0;
    DAT_006f98cc = (int *)0x0;
    DAT_006f98c0 = 0;
  }
  DestroyWindow(DAT_00656614);
LAB_0058acb6:
  iVar1 = GetDeviceCaps(hdc,0xc);
  DAT_006f8174 = (uint)(iVar1 < 9);
  ReleaseDC(param_1,hdc);
  FUN_0059b7e2(FUN_0058acf0);
  return;
}

