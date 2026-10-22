
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void FUN_004e3e40(void)

{
  bool bVar1;
  DWORD DVar2;
  uint uVar3;
  undefined4 unaff_EBX;
  undefined4 unaff_EDI;
  uint uVar4;
  undefined8 uVar5;
  undefined1 local_14 [20];
  
  if (DAT_006f60b0 != 0) {
    DAT_006f60b0 = 0;
  }
  if (DAT_006e1ee0 != 0) {
    if (DAT_006e80fc == 10) {
      DVar2 = GetTickCount();
      FUN_0059c1fd(local_14,s_fps___02_2f_005e331c,
                   (double)(_DAT_005c6fc8 / ((float)DVar2 - (float)(int)DAT_006e2984)));
      FUN_004d86e0(local_14,3,100);
      DAT_006e80fc = 1;
      DAT_006e2984 = GetTickCount();
    }
    else {
      DAT_006e80fc = DAT_006e80fc + 1;
    }
  }
  if (DAT_006e1eb8 != 0) {
    return;
  }
  uVar5 = CONCAT44(unaff_EBX,unaff_EDI);
  bVar1 = false;
  DAT_006e1f00 = 0;
  uVar4 = DAT_006dbe95;
  if ((DAT_006e1efc != 0) && (DAT_005e1418 != 0)) {
    if ((int)DAT_006e1ef8 < (int)DAT_005e1418) {
      if (DAT_006e1ef8 == 0) {
        DAT_006e1ef8 = DAT_006e1efc;
      }
      else {
        DAT_006e1ef8 = DAT_006e1ef8 - 1;
      }
      if (DAT_006e1f04 == 0) {
        bVar1 = true;
        DAT_006e1f00 = 1;
      }
    }
    else {
      DAT_006e1ef8 = DAT_006e1ef8 - 1;
    }
    if (DAT_005e1418 != 0) {
      uVar4 = (uint)(((longlong)(int)(DAT_006e1efc + 1) * (longlong)(int)DAT_006dbe95 & 0xffffffffU)
                    / (ulonglong)DAT_006e1efc);
    }
  }
  if (!bVar1) {
    DVar2 = GetTickCount();
    if (DVar2 - DAT_006e8100 < uVar4) {
      DVar2 = GetTickCount();
      DAT_006e80f8 = DAT_006e80f8 + ((DAT_006e8100 + uVar4) - DVar2);
      DVar2 = GetTickCount();
      uVar3 = DVar2 - DAT_006e8100;
      while (uVar3 < uVar4) {
        DVar2 = GetTickCount();
        uVar3 = DVar2 - DAT_006e8100;
      }
    }
    DAT_006e8100 = GetTickCount();
    DAT_006e80f4 = DAT_006e80f4 + 1;
  }
  if (DAT_006e1ebc != 0) {
    FUN_00484930(uVar5);
  }
  if (((DAT_006e1ee0 == 0) || (DAT_006e80f0 == 0)) ||
     (DVar2 = GetTickCount(), DVar2 - 1000 < DAT_006e80ec)) goto LAB_004e4069;
  DVar2 = GetTickCount();
  uVar3 = (DVar2 - DAT_006e80ec) / DAT_006e80f0;
  DAT_006e80ec = GetTickCount();
  if (DAT_005e1418 != 0) {
    if (((uint)((ulonglong)uVar4 * 0x10cccccccd >> 0x24) < uVar3) &&
       (((int)DAT_005e1418 < DAT_005e1414 || (DAT_005e1418 != DAT_006e1efc)))) {
      if (DAT_006e1efc == 0) {
        DAT_006e1efc = 3;
      }
      else if (((int)DAT_006e1efc < 2) || (DAT_005e1418 != 1)) {
        DAT_005e1418 = DAT_005e1418 + 1;
LAB_004e4051:
        DAT_006e1efc = DAT_006e1efc + 1;
      }
      else {
        DAT_006e1efc = DAT_006e1efc - 1;
      }
    }
    else if (((uVar4 / 0x14 < DAT_006e80f8) || (DAT_006dbe99 + -1 + DAT_005e1418 < DAT_006e80f0)) &&
            (DAT_006e1efc != 0)) {
      if ((int)DAT_005e1418 < 2) {
        if (DAT_006e1efc != 3) goto LAB_004e4051;
        DAT_006e1efc = 0;
      }
      else {
        DAT_006e1efc = DAT_006e1efc - 1;
        DAT_005e1418 = DAT_005e1418 - 1;
      }
    }
    DAT_006e80f8 = 0;
  }
  DAT_006e80f0 = 0;
  DAT_006e80f4 = 0;
LAB_004e4069:
  DAT_006e80f0 = DAT_006e80f0 + 1;
  DAT_006e1f04 = 0;
  return;
}

