
undefined4 __thiscall FUN_005383c0(int param_1,int param_2,char param_3)

{
  bool bVar1;
  bool bVar2;
  char *pcVar3;
  DWORD DVar4;
  undefined4 *puVar5;
  uint uVar6;
  int iVar7;
  undefined4 uVar8;
  LPCSTR lpText;
  LPCSTR lpCaption;
  uint uVar9;
  UINT uType;
  int local_118;
  undefined4 local_114;
  undefined4 local_110;
  undefined4 local_10c;
  int local_108;
  undefined4 local_104;
  undefined1 auStack_100 [256];
  
  *(int *)(param_1 + 0x38) = param_2;
  if (DAT_0068991c != 0) {
    local_118 = DAT_005eab14;
    DVar4 = timeGetTime();
    *(DWORD *)(param_1 + 4) = DVar4;
    bVar1 = true;
    DVar4 = timeGetTime();
    uVar9 = DVar4 + 15000;
    if (((DAT_006f34e0 == 0) || (*(int *)(DAT_006f34e0 + 4) != 2)) ||
       (bVar2 = true, DAT_006c4830 <= DAT_006ead94 + 1U)) {
      bVar2 = false;
    }
    do {
      FUN_00538780();
      if (((*(int *)(param_1 + 0x20) != 0) &&
          (puVar5 = (undefined4 *)(*(int *)(param_1 + 0x18) * 0x15 + *(int *)(param_1 + 0x10)),
          puVar5 != (undefined4 *)0x0)) &&
         ((*(char *)(puVar5 + 5) != '\0' && (puVar5[4] == *(int *)(param_1 + 0x38))))) {
        FUN_00539080(*puVar5,puVar5[3],0);
        *(int *)(param_1 + 0x3c) = *(int *)(param_1 + 0x3c) + 1;
        if (*(int *)(param_1 + 0x20) != 0) {
          uVar6 = *(int *)(param_1 + 0x18) + 1;
          *(uint *)(param_1 + 0x18) = -(uint)(uVar6 != *(uint *)(param_1 + 0x14)) & uVar6;
          *(int *)(param_1 + 0x20) = *(int *)(param_1 + 0x20) + -1;
        }
      }
      if (*(int *)(param_1 + 0x38) < *(int *)(param_1 + 0x3c)) {
        return 1;
      }
      if (bVar1) {
        bVar1 = false;
      }
      DVar4 = timeGetTime();
      if (local_118 <= (int)(DVar4 - *(int *)(param_1 + 4))) {
        local_108 = *(int *)(param_1 + 0x38);
        local_114 = 0;
        local_110 = 0;
        local_10c = 0x10;
        iVar7 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 8) + 0x68))
                          ((int *)**(undefined4 **)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),1,0,
                           &local_114,0x10);
        if (iVar7 != 0) {
          uVar8 = FUN_00534d10();
          FUN_0059c1fd(auStack_100,s__s_directplay_error_code____s_005eaba0,
                       s_client_send__resend_message__req_005eaf70,uVar8);
        }
        if (param_3 == '\0') {
          return 1;
        }
        FUN_00536f00(param_2,0);
        DVar4 = timeGetTime();
        *(DWORD *)(param_1 + 4) = DVar4;
        local_118 = local_118 * 2;
        if (2999 < local_118) {
          local_118 = 3000;
        }
      }
      if ((bVar2) && (DVar4 = timeGetTime(), uVar9 < DVar4)) {
        ShowCursor(1);
        lpText = (LPCSTR)FUN_0058a680(0x17);
        uType = 0x42034;
        lpCaption = (LPCSTR)FUN_0058a680(0x16);
        iVar7 = MessageBoxA((HWND)0x0,lpText,lpCaption,uType);
        ShowCursor(0);
        DVar4 = timeGetTime();
        uVar9 = DVar4 + 15000;
        if (iVar7 == 6) {
          DAT_006dbc19 = 1;
          return 0;
        }
      }
      iVar7 = FUN_00557460();
    } while (iVar7 != 0);
    return 0;
  }
  if (param_2 < *(int *)(param_1 + 0x3c)) {
    return 1;
  }
  puVar5 = *(undefined4 **)(param_1 + 0x10);
  pcVar3 = (char *)FUN_00473330();
  if ((*pcVar3 == '\0') || (pcVar3[1] != '\0')) {
    uVar8 = *puVar5;
  }
  else {
    local_114 = 0;
    local_110 = 1;
    local_10c = 0x14;
    local_104 = 0xffffffff;
    FUN_0049ecf0(&local_114,0x14);
    if (local_108 < param_2) {
      FUN_0049e680();
      DestroyWindow(DAT_00656614);
    }
    if (local_108 != param_2) goto LAB_00538488;
    FUN_0049eb40(*puVar5,0x400);
    uVar8 = *puVar5;
  }
  FUN_00539080(uVar8,0x400,0);
LAB_00538488:
  *(int *)(param_1 + 0x3c) = *(int *)(param_1 + 0x3c) + 1;
  return 1;
}

