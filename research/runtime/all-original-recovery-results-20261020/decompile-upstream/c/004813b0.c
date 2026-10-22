
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

char * __fastcall FUN_004813b0(MCIERROR param_1)

{
  char cVar1;
  char *pcVar2;
  uint uVar3;
  uint uVar4;
  char *pcVar5;
  
  if ((int)param_1 < 0x124) {
    if (param_1 == 0x123) {
      pcVar2 = (char *)FUN_0058a6b0();
      uVar3 = 0xffffffff;
      do {
        pcVar5 = pcVar2;
        if (uVar3 == 0) break;
        uVar3 = uVar3 - 1;
        pcVar5 = pcVar2 + 1;
        cVar1 = *pcVar2;
        pcVar2 = pcVar5;
      } while (cVar1 != '\0');
      uVar3 = ~uVar3;
      pcVar2 = pcVar5 + -uVar3;
      pcVar5 = (char *)&DAT_005fd2a0;
      for (uVar4 = uVar3 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
        *(undefined4 *)pcVar5 = *(undefined4 *)pcVar2;
        pcVar2 = pcVar2 + 4;
        pcVar5 = pcVar5 + 4;
      }
      for (uVar3 = uVar3 & 3; uVar3 != 0; uVar3 = uVar3 - 1) {
        *pcVar5 = *pcVar2;
        pcVar2 = pcVar2 + 1;
        pcVar5 = pcVar5 + 1;
      }
      return (char *)&DAT_005fd2a0;
    }
    switch(param_1) {
    case 0:
      return s_False_error_condition__check_cod_005da678;
    case 0xfffffff8:
      return s_Error_trying_to_play_the_CD__005da778;
    case 0xfffffff9:
      return s_No_AUX_sound_device_found__005da69c;
    case 0xfffffffa:
      return s_Error_setting_the_CD_volume_leve_005da6b8;
    case 0xfffffffb:
      return s_Error_getting_the_CD_volume_leve_005da6dc;
    case 0xfffffffc:
      return s_Play_list_empty__005da700;
    case 0xfffffffd:
      return s_Play_list_has_finished__Play_aga_005da714;
    case 0xfffffffe:
      return s_Play_list_is_now_full__005da744;
    case 0xffffffff:
      return s_No_window_handle_supplied__005da75c;
    }
  }
  _DAT_005fd3a4 = mciGetErrorStringA(param_1,(LPSTR)&DAT_005fd2a0,0x100);
  pcVar2 = (char *)&DAT_005fd2a0;
  if (_DAT_005fd3a4 == 0) {
    pcVar2 = s_Undefined_Error___005da648;
  }
  return pcVar2;
}

