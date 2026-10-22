
void FUN_004a4380(void)

{
  uint uVar1;
  uint uVar2;
  undefined4 local_10;
  undefined4 local_c;
  int local_8;
  int local_4;
  
  if (((((DAT_00642c64 != 0) && (DAT_005e04c8 != 0x16)) && (DAT_006a49e4 != 0)) &&
      ((-1 < DAT_00641a78 && (-1 < DAT_00641a7c)))) && ((-1 < DAT_00641a80 && (-1 < DAT_00641a84))))
  {
    WaitForSingleObject(DAT_00642c58,0xffffffff);
    if (DAT_00642c54 != 0) {
      local_8 = DAT_00641a80 - DAT_00641a78;
      local_10 = 0;
      local_4 = DAT_00641a84 - DAT_00641a7c;
      local_c = 0;
      if ((local_8 != 0) && (local_4 != 0)) {
        uVar2 = ((int)DAT_00641aa4 < 0) - 1 & DAT_00641aa4;
        uVar1 = DAT_00641aa0 & ((int)DAT_00641aa0 < 0) - 1;
        if (DAT_006e1ee4 != 0) {
          FUN_0058d5b0(2,&DAT_006a49e0,&local_10,uVar1,uVar2,1);
          ReleaseMutex(DAT_00642c58);
          return;
        }
        FUN_0058c4a0(&DAT_006a49e0,&local_10,uVar1,uVar2);
      }
    }
    ReleaseMutex(DAT_00642c58);
  }
  return;
}

