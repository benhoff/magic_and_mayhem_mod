
void __fastcall FUN_0046ec30(int param_1)

{
  HPEN h;
  HGDIOBJ h_00;
  int iVar1;
  HDC local_8;
  int local_4;
  
  local_8 = (HDC)0x0;
  local_4 = param_1;
  if (DAT_006903e8 == 3) {
    h = CreatePen(0,1,200);
    h_00 = GetStockObject(5);
    if ((h != (HPEN)0x0) && (h_00 != (HGDIOBJ)0x0)) {
      iVar1 = (**(code **)(*DAT_006a49e4 + 0x44))(DAT_006a49e4,&local_8);
      if (iVar1 == 0) {
        SelectObject(local_8,h);
        SelectObject(local_8,h_00);
        Rectangle(local_8,DAT_00690438,DAT_0069043c,DAT_00690440,DAT_00690444);
        (**(code **)(*DAT_006a49e4 + 0x68))(DAT_006a49e4,local_8);
      }
      DeleteObject(h);
      DeleteObject(h_00);
    }
  }
  if (DAT_00644520 != -1) {
    if (DAT_006a2dd4 != 0) {
      FUN_004d69e0();
    }
    iVar1 = FUN_00463cb0();
    if (*(char *)(iVar1 + 0x1c9) != '\0') {
      FUN_00463cb0();
      FUN_00463d70();
    }
    if ((DAT_006a46e0 == 0) && (DAT_006c4634 != 0)) {
      FUN_004dca40();
    }
    if (DAT_006c4e65 != 0) {
      FUN_004d3680();
    }
    if (DAT_006de8a4 != 0) {
      FUN_004d9770();
    }
    if (DAT_006903f4 != 0) {
      FUN_004de860();
    }
    if (DAT_006905ac != 0) {
      FUN_004de1c0();
    }
    if (*(int **)(local_4 + 0x100b3) != (int *)0x0) {
      (**(code **)(**(int **)(local_4 + 0x100b3) + 0x10))();
    }
  }
  if (DAT_006ddf7c != 0) {
    FUN_004d84b0();
  }
  FUN_004da460();
  if (DAT_00689ba5 != 0) {
    FUN_00555990();
  }
  if (DAT_006c528d != 0) {
    FUN_004dac90();
  }
  FUN_004db7e0();
  if (DAT_006c485b != '\0') {
    FUN_005746b0();
  }
  DAT_006e1ee4 = 0;
  FUN_0058d660();
  return;
}

