
void FUN_004eab00(void)

{
  DWORD DVar1;
  int iVar2;
  
  DVar1 = GetPrivateProfileStringA
                    (s_HEADER_005da478,s_Language_005e419c,&DAT_005faf28,&DAT_006e2880,0x100,
                     s_cfg_common_dialog_text_cfg_005da65c);
  if (DVar1 != 0) {
    iVar2 = __strcmpi(&DAT_006e2880,s_English_005e4194);
    if (iVar2 == 0) {
      DAT_006e1ed4 = iVar2;
      return;
    }
    iVar2 = __strcmpi(&DAT_006e2880,s_French_005e418c);
    if (iVar2 == 0) {
      DAT_006e1ed4 = 1;
      return;
    }
    iVar2 = __strcmpi(&DAT_006e2880,s_Spanish_005e4184);
    if (iVar2 == 0) {
      DAT_006e1ed4 = 2;
      return;
    }
    iVar2 = __strcmpi(&DAT_006e2880,s_German_005e417c);
    if (iVar2 == 0) {
      DAT_006e1ed4 = 3;
      return;
    }
    iVar2 = __strcmpi(&DAT_006e2880,s_Italian_005e4174);
    if (iVar2 == 0) {
      DAT_006e1ed4 = 4;
      return;
    }
    iVar2 = __strcmpi(&DAT_006e2880,s_American_005e4168);
    if (iVar2 == 0) {
      DAT_006e1ed4 = 5;
    }
  }
  return;
}

