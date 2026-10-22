
undefined4 __thiscall FUN_00536f00(int param_1,int param_2,char param_3)

{
  bool bVar1;
  DWORD DVar2;
  uint uVar3;
  LPCSTR lpText;
  LPCSTR lpCaption;
  int iVar4;
  int iVar5;
  undefined4 uVar6;
  code *pcVar7;
  uint uVar8;
  int *piVar9;
  UINT uType;
  int local_118;
  int local_114;
  undefined4 local_110;
  undefined4 local_10c;
  undefined4 local_108;
  int local_104;
  undefined1 auStack_100 [256];
  
  pcVar7 = timeGetTime_exref;
  if ((DAT_0068991c == 0) || (*(int *)(*(int *)(param_1 + 8) + 300) == 0)) {
    return 1;
  }
  DVar2 = timeGetTime();
  *(DWORD *)(param_1 + 4) = DVar2;
  local_118 = DAT_005eab14;
  DVar2 = timeGetTime();
  uVar8 = DVar2 + 15000;
  if (((DAT_006f34e0 == 0) || (*(int *)(DAT_006f34e0 + 4) != 2)) ||
     (bVar1 = true, DAT_006c4830 <= DAT_006ead94 + 1U)) {
    bVar1 = false;
  }
  do {
    FUN_00537100();
    if (param_2 < *(int *)(param_1 + 0xac)) {
      return 1;
    }
    if ((bVar1) && (uVar3 = (*pcVar7)(), uVar8 < uVar3)) {
      ShowCursor(1);
      lpText = (LPCSTR)FUN_0058a680(0x17);
      uType = 0x42034;
      lpCaption = (LPCSTR)FUN_0058a680(0x16);
      iVar4 = MessageBoxA((HWND)0x0,lpText,lpCaption,uType);
      ShowCursor(0);
      iVar5 = (*pcVar7)();
      uVar8 = iVar5 + 15000;
      if (iVar4 == 6) {
        DAT_006dbc19 = 1;
        return 0;
      }
    }
    iVar4 = (*pcVar7)();
    if (local_118 <= iVar4 - *(int *)(param_1 + 4)) {
      piVar9 = (int *)(param_1 + 0x9c);
      local_114 = 4;
      do {
        if ((piVar9[-0x23] != 0) && (*piVar9 <= param_2)) {
          local_110 = 0;
          local_10c = 0;
          local_108 = 0x10;
          local_104 = *piVar9;
          iVar4 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 8) + 0x68))
                            ((int *)**(undefined4 **)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),
                             piVar9[-0x23],0,&local_110,0x10);
          pcVar7 = timeGetTime_exref;
          if (iVar4 != 0) {
            uVar6 = FUN_00534d10();
            FUN_0059c1fd(auStack_100,s__s_directplay_error_code____s_005eaba0,
                         s_host_send__resend_message__reque_005eae38,uVar6);
            pcVar7 = timeGetTime_exref;
          }
        }
        piVar9 = piVar9 + 1;
        local_114 = local_114 + -1;
      } while (local_114 != 0);
      if (param_3 == '\0') {
        return 1;
      }
      FUN_005383c0(param_2,0);
      uVar6 = (*pcVar7)();
      *(undefined4 *)(param_1 + 4) = uVar6;
      local_118 = local_118 * 2;
      if (2999 < local_118) {
        local_118 = 3000;
      }
    }
    iVar4 = FUN_00557460();
    if (iVar4 == 0) {
      return 0;
    }
  } while( true );
}

