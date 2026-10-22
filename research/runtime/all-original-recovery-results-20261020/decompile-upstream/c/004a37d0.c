
void FUN_004a37d0(void)

{
  if (DAT_00642c58 != (HANDLE)0x0) {
    CloseHandle(DAT_00642c58);
    DAT_00642c58 = (HANDLE)0x0;
  }
  FUN_0058b3e0();
  FUN_0058b3e0();
  return;
}

