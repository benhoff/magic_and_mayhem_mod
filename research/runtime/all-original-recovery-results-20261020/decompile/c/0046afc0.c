
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_0046afc0(int param_1)

{
  char *pcVar1;
  char cVar2;
  undefined4 uVar3;
  int iVar4;
  int iVar5;
  undefined4 *puVar6;
  undefined4 extraout_ECX;
  undefined4 extraout_ECX_00;
  undefined4 extraout_ECX_01;
  uint uVar7;
  undefined4 uVar8;
  undefined4 uVar9;
  undefined4 local_74;
  undefined4 local_70;
  undefined4 local_6c;
  uint local_68;
  undefined4 uStack_64;
  undefined4 uStack_60;
  undefined4 local_54;
  int local_50;
  undefined4 local_48;
  undefined4 local_44;
  undefined4 local_40;
  undefined4 local_3c;
  undefined4 local_38;
  undefined4 local_34;
  undefined4 local_30;
  undefined1 local_2c;
  undefined4 local_2b;
  undefined1 local_27;
  undefined4 local_26;
  undefined4 local_22;
  undefined4 local_1e;
  undefined4 local_1a;
  undefined4 local_a;
  undefined4 local_6;
  
  if (*(int *)(param_1 + 0xfd62) == 0) {
    local_74 = 0;
    FUN_00473850();
    FUN_00537bd0(&local_74);
    if (DAT_006e1f61 == '\0') {
      if (DAT_006e1f60 != '\0') {
        iVar4 = FUN_0059cae2(s_ai_brain_dat_005d9fac,&DAT_005d96e0);
        if (iVar4 != 0) {
          FUN_0059c4c3(iVar4);
          FUN_00469ab0();
        }
        uVar3 = FUN_00473330();
        *(undefined4 *)(param_1 + 0x100df) = uVar3;
        FUN_0049e6c0(s_save_event_log_005e14d8,DAT_005e14d4);
        iVar4 = FUN_004ea5a0();
        if (iVar4 == 0) {
          FUN_0049e680();
        }
      }
    }
    else {
      uVar3 = FUN_00473330();
      *(undefined4 *)(param_1 + 0x100df) = uVar3;
      cVar2 = FUN_0049e760(s_save_event_log_005e14d8,DAT_005e14d4);
      if (cVar2 == '\0') {
        FUN_0049e860();
        FUN_0049e680();
        *(undefined1 *)(param_1 + 0x100a2) = 1;
      }
      else {
        iVar4 = FUN_004ea280();
        if (iVar4 == 0) {
          FUN_0049e680();
          *(undefined1 *)(param_1 + 0x100a2) = 1;
        }
      }
    }
  }
  if (*(char *)(param_1 + 0x100a2) != '\0') {
    if (*(int *)(param_1 + 0x10317) != 0) {
      FUN_00534b60();
      FUN_004de230();
    }
    FUN_00557130();
    return;
  }
  if ((((DAT_00689920 == 2) || (DAT_00689920 == 1)) || (DAT_006e1f0c != 0)) ||
     (*(char *)(param_1 + 0x100a6) == '\0')) {
LAB_0046b183:
    if (*(char *)(param_1 + 0x100a7) != '\0') goto LAB_0046b18b;
  }
  else {
    if (*(char *)(param_1 + 0x100a7) == '\0') {
      if (((DAT_00644520 == -1) || (*(char *)(param_1 + 0x100ad) != '\0')) ||
         ((*(char *)(param_1 + 0x100ae) != '\0' || (*(char *)(param_1 + 0x100af) != '\0')))) {
        *(undefined1 *)(param_1 + 0x100a7) = 1;
        *(undefined4 *)(param_1 + 0x100a8) = 0x78;
        if (*(int *)(param_1 + 0x10317) != 0) {
          FUN_00534b60();
          FUN_004de230();
        }
      }
      goto LAB_0046b183;
    }
LAB_0046b18b:
    iVar4 = *(int *)(param_1 + 0x100a8);
    *(int *)(param_1 + 0x100a8) = iVar4 + -1;
    if (iVar4 == 0) {
      if (DAT_00689920 == 5) {
        *(undefined1 *)(param_1 + 0x100a0) = 1;
      }
      else {
        *(undefined1 *)(param_1 + 0x100a1) = 1;
      }
      *(undefined1 *)(param_1 + 0x100a7) = 0;
    }
  }
  if (*(char *)(param_1 + 0x100a1) != '\0') {
    if (*(int *)(param_1 + 0x10317) != 0) {
      FUN_00534b60();
      FUN_004de230();
    }
    if (*(char *)(param_1 + 0x100a3) == '\0') {
      FUN_0046a150();
    }
    *(undefined1 *)(param_1 + 0x100a1) = 0;
    if (((DAT_00689920 == 2) || (DAT_00689920 == 1)) && (DAT_006e1f0c == 0)) {
      DAT_006def4c = 1;
      FUN_004a39a0();
      *(undefined1 *)(param_1 + 0x100b2) = 1;
      FUN_00557040();
      return;
    }
    if ((*(char *)(param_1 + 0x100a4) == '\0') && (DAT_00656620 == 1)) {
      iVar4 = FUN_00473900();
      FUN_00481860(*(undefined4 *)(iVar4 + 8));
      FUN_004819b0(0);
    }
    FUN_00557130();
    return;
  }
  if (*(char *)(param_1 + 0x100a0) != '\0') {
    if (*(int *)(param_1 + 0x10317) != 0) {
      FUN_00534b60();
      FUN_004de230();
    }
    *(undefined1 *)(param_1 + 0x100a1) = 1;
    *(undefined1 *)(param_1 + 0x100a0) = 0;
    cVar2 = FUN_0046a150();
    *(char *)(param_1 + 0x100a4) = cVar2;
    if (cVar2 == '\0') {
      if (((DAT_00644520 != -1) && (*(char *)(param_1 + 0x100ae) == '\0')) &&
         (*(char *)(param_1 + 0x100af) == '\0')) {
        return;
      }
    }
    else {
      _DAT_00659e2f = 1;
      FUN_00593ff0(PTR_s_Cornelius_005c5c54);
      FUN_00593a90();
      FUN_00473890();
      iVar4 = FUN_004a0e80();
      iVar5 = FUN_00593ff0(PTR_s_Cornelius_005c5c54);
      *(int *)(iVar5 + 0x58) = *(int *)(iVar5 + 0x58) + iVar4;
    }
    FUN_00482010(1);
    FUN_00481890();
    if (*(char *)(param_1 + 0x100ac) == '\0') {
      *(undefined1 *)(param_1 + 0x100ac) = 1;
      FUN_004711d0(1);
      FUN_004ffad0();
      FUN_0052baf0();
      uVar7 = 0;
      iVar4 = DAT_00689498;
      if (DAT_0068949c != 0) {
        do {
          if (*(int *)(iVar4 + 4) != 0) {
            FUN_004943f0();
          }
          uVar7 = uVar7 + 1;
          iVar4 = iVar4 + 0x22e;
        } while (uVar7 < DAT_0068949c);
      }
    }
    *(uint *)(param_1 + 0xfe42) = (uint)*(byte *)(param_1 + 0x100a4);
    if (((*(char *)(param_1 + 0x100a4) != '\0') && (DAT_006e2030 != '\0')) && (DAT_00656620 == 2)) {
      DAT_00659e50 = 0;
      _DAT_00659e3f = 1;
      _DAT_00657d3b = 1;
    }
    FUN_004a44b0();
    FUN_00557040();
    return;
  }
  if (((DAT_006e007c != 0) && (DAT_00690618 != '\0')) &&
     ((DAT_006906c6 == 0 && ((DAT_00644568 != '\0' && (DAT_00644616 == 0)))))) {
    FUN_00557130();
    return;
  }
  pcVar1 = *(char **)(param_1 + 0x100df);
  if (((pcVar1 != (char *)0x0) && (*pcVar1 != '\0')) && (pcVar1[1] == '\0')) {
    FUN_004d86e0(s_>>_PlayBack_Mode_>>_005d9f7c,0,1);
  }
  iVar4 = FUN_005383c0(*(undefined4 *)(param_1 + 0xfd62),1);
  if (iVar4 == 0) {
    return;
  }
  iVar4 = *(int *)(param_1 + 0xfd62) + 1;
  *(int *)(param_1 + 0xfd62) = iVar4;
  FUN_00487b10(iVar4);
  FUN_00485b00(*(undefined4 *)(param_1 + 0xfd62));
  if (*(int *)(param_1 + 0xfd5a) == 0) {
    DAT_006c4830 = DAT_006c4830 + 1;
    if (DAT_006c4830 == DAT_006e1f14) {
      DestroyWindow(DAT_00656614);
    }
    if (DAT_005e1410 != 0) {
      if (*(char *)(param_1 + 0x100b0) == '\0') {
        FUN_00415c40(DAT_005e15f4 / 2);
        FUN_00415c40(DAT_005e15f4 / 2);
      }
      else {
        DAT_006def6c = 0;
        DAT_006def64 = DAT_006def58;
        local_48 = 0;
        local_44 = 0;
        local_40 = 0;
        local_3c = 0;
        local_38 = 0;
        local_34 = 0;
        local_30 = 0;
        local_2c = 0;
        local_2b = 0;
        local_27 = 0;
        local_26 = 0;
        local_22 = 0;
        local_1e = 0;
        local_1a = 0;
        local_a = 0;
        local_6 = 0;
        local_54 = 0;
        DAT_006def60 = 0;
        DAT_006def68 = FUN_00412aa0;
        iVar4 = FUN_0052a710();
        while (iVar4 != 0) {
          local_50 = iVar4;
          FUN_00441560(&local_54);
          uVar9 = extraout_ECX;
          FUN_0040eeb0(*(undefined4 *)(iVar4 + 0x10));
          uVar8 = extraout_ECX_00;
          FUN_0040e8a0(*(undefined4 *)(iVar4 + 0xc));
          uVar3 = extraout_ECX_01;
          FUN_0040e290(*(undefined4 *)(iVar4 + 8));
          uVar3 = FUN_0053d7d0(uVar3,uVar8,uVar9);
          *(undefined4 *)(iVar4 + 0xde7) = uVar3;
          iVar4 = FUN_0052a710();
        }
        FUN_00415c40(0x7fffffff);
        FUN_00415c40(0x7fffffff);
        *(undefined1 *)(param_1 + 0x100b0) = 0;
      }
      if ((DAT_006c4830 & 0x1ff) == 0) {
        FUN_00477060();
      }
    }
  }
  if ((DAT_006deed0 == 0) && (DAT_006f2aa8 == 0)) {
    FUN_004a5070();
  }
  if (((DAT_005fc86c != '\0') && ((DAT_00689920 == 2 || (DAT_00689920 == 1)))) &&
     (DAT_006de6ac != 0)) {
    local_68 = DAT_005fc868;
    local_74 = 0;
    local_70 = 0x19;
    local_6c = 0x10;
    FUN_00537bd0(&local_74);
    DAT_005fc86c = '\0';
  }
  if (DAT_0068991c != 0) {
    if (((DAT_006de6ac != 0) && (*(int *)(param_1 + 0x10317) != 0)) &&
       (*(int *)(param_1 + 0xfd62) == 1)) {
      *(undefined1 *)(param_1 + 0x1031c) = 1;
    }
    if (*(char *)(param_1 + 0x1031c) != '\0') {
      FUN_00534aa0(1000,5);
      FUN_00534ad0(&LAB_00469a60);
      DAT_005fc86c = '\0';
    }
    if (DAT_005eab18 != '\0') {
      local_68 = DAT_006e1acc ^ DAT_006e1ac8;
      local_74 = 0;
      local_70 = 3;
      local_6c = 0x10;
      FUN_00537c70(&local_74,*(undefined4 *)(param_1 + 0xfd62));
    }
  }
  FUN_00537fe0(*(undefined4 *)(param_1 + 0xfd62));
  if (*(int *)(param_1 + 0xfd5a) == 0) {
    uVar7 = 0;
    iVar4 = DAT_006def58;
    if (DAT_006df180 != 0) {
      do {
        if (*(int *)(iVar4 + 4) != 0) {
          FUN_00509e50();
        }
        uVar7 = uVar7 + 1;
        iVar4 = iVar4 + 0xe4b;
      } while (uVar7 < DAT_006df180);
    }
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_00464ec0();
    FUN_0052a780();
    FUN_0052a9f0();
    if (DAT_006e1f3c != 0) {
      FUN_005404a0();
    }
  }
  else if (DAT_006c485b != '\0') {
    iVar4 = 10;
    do {
      FUN_005404a0();
      iVar4 = iVar4 + -1;
    } while (iVar4 != 0);
  }
  FUN_004724c0();
  if (*(int *)(param_1 + 0xfd5a) == 0) {
    uVar7 = 0;
    iVar4 = DAT_00689498;
    if (DAT_006898dc != 0) {
      do {
        if (*(int *)(iVar4 + 4) != 0) {
          FUN_004970e0();
        }
        uVar7 = uVar7 + 1;
        iVar4 = iVar4 + 0x22e;
      } while (uVar7 < DAT_006898dc);
    }
  }
  else {
    iVar5 = 0x7d;
    iVar4 = DAT_00689498;
    do {
      if (*(int *)(iVar4 + 4) != 0) {
        FUN_004970e0();
      }
      iVar4 = iVar4 + 0x22e;
      iVar5 = iVar5 + -1;
    } while (iVar5 != 0);
  }
  if (*(int *)(param_1 + 0xfd5a) == 0) {
    FUN_00534380();
    FUN_004f2e20();
    FUN_004f3900();
    FUN_00544d10();
    FUN_00545910();
    FUN_004e0f00();
    FUN_0046f5d0();
    if ((DAT_006deed0 == 0) && (DAT_006f2aa8 == 0)) {
      FUN_00571300();
    }
    iVar4 = FUN_00536f00(*(undefined4 *)(param_1 + 0xfd62),1);
    if (iVar4 == 0) {
      return;
    }
    if ((*(uint *)(param_1 + 0xfdea) < DAT_006c4830) && (iVar4 = FUN_004f7540(), iVar4 != 0)) {
      *(uint *)(param_1 + 0xfdea) = DAT_00657cdc + DAT_006c4830;
    }
    if (DAT_005e1404 != 0) {
      FUN_004f27a0();
    }
    FUN_00564e90(1,0,0xffffffff,0,0xffffffff,0xffffffff);
    FUN_00564b40();
  }
  if ((((DAT_006e1f00 == 0) && (DAT_006deed0 == 0)) && (DAT_006f2aa8 == 0)) &&
     ((DAT_0068991c == 0 || (DAT_006ead94 < *(int *)(param_1 + 0xfd62))))) {
    DAT_00642c60 = 0;
    FUN_004a45b0();
    if ((DAT_006de6d9 == 0) ||
       ((DAT_005e16fc != 0 || (*(int *)(DAT_006c482c + 0xc) <= DAT_00641ab4)))) {
      FUN_0058bf40(&DAT_006a4fe0,0,0);
      if (0 < DAT_005e16fc) {
        DAT_005e16fc = DAT_005e16fc + -1;
      }
    }
    else {
      FUN_0058c4a0(&DAT_006a4fe0,DAT_006c482c,0,0);
      iVar4 = FUN_004d7e10(&local_74,&uStack_64);
      if (iVar4 != 0) {
        FUN_0058c4a0(&DAT_006a4fe0,&local_74,local_74,local_70);
        FUN_0058c4a0(&DAT_006a4fe0,&uStack_64,uStack_64,uStack_60);
      }
    }
    DAT_00642c60 = 1;
  }
  FUN_00574410();
  if ((DAT_005fc864 != 0) || (*(int *)(param_1 + 0x10317) == 0)) goto LAB_0046bb7c;
  *(undefined1 *)(param_1 + 0x1031b) = 0;
  FUN_004da690(0x22);
  *(undefined1 *)(param_1 + 0x100a7) = 1;
  *(undefined4 *)(param_1 + 0x100a8) = 0x78;
  if (*(int *)(param_1 + 0x10317) != 0) {
    FUN_00534b60();
    FUN_004de230();
  }
  DAT_006905ac = 0;
  iVar4 = FUN_00466870(DAT_00644520);
  if (iVar4 != 0) {
    FUN_00509bd0(0xffffffff);
  }
  FUN_004d6c60();
  FUN_004d39b0();
  DAT_006903f4 = 0;
  if (*(int *)(param_1 + 0x100b3) != 0) {
    FUN_004cb9b0();
  }
  if (DAT_006de6d9 != 0) {
    DAT_005d9e38 = DAT_006de6d9;
    if (DAT_006de6d5 == 0) {
      if (((0 < DAT_006de6d9) && (DAT_006903e8 != 3)) && (DAT_006c4859 == '\0')) {
        DAT_006c482c = &DAT_005e1700 + (DAT_006de6d9 + -1) * 0x10;
        goto LAB_0046bb2e;
      }
    }
    else if (((0 < DAT_006de6d9) && (DAT_006903e8 != 3)) && (DAT_006c4859 == '\0')) {
      DAT_006c482c = &DAT_005e1720 + (DAT_006de6d9 + -1) * 0x10;
LAB_0046bb2e:
      DAT_006de6d9 = DAT_006de6d9 + -1;
    }
    FUN_004f7970(DAT_006c482c);
    FUN_004d7a90();
    DAT_005e16fc = 1;
  }
  DAT_006e9858 = 1;
  puVar6 = &DAT_00658f3e;
  do {
    *puVar6 = 0;
    puVar6 = (undefined4 *)((int)puVar6 + 0x133);
  } while ((int)puVar6 < 0x65940a);
  DAT_005fc864 = -1;
LAB_0046bb7c:
  if (*(int *)(param_1 + 0xfd5a) == 0) {
    FUN_0052b540();
  }
  if (DAT_006cbb70 != 0) {
    FUN_0046fea0();
  }
  if ((DAT_006e1f00 == 0) && (*(char *)(param_1 + 0x100b2) == '\0')) {
    FUN_004d9ba0();
    if (DAT_006c4859 == '\0') {
      FUN_00469b30();
    }
    FUN_0058d660();
    FUN_004fd280();
    FUN_0046ec30();
  }
  if ((DAT_005e04c8 == 0x16) && (*(char *)(param_1 + 0x100a2) == '\0')) {
    FUN_004a5860();
    FUN_004a39a0();
  }
  if (*(char *)(param_1 + 0x10326) != '\0') {
    *(undefined1 *)(param_1 + 0x10326) = 0;
    FUN_00557040();
    return;
  }
  if (*(char *)(param_1 + 0x100b1) != '\0') {
    *(undefined1 *)(param_1 + 0x100b1) = 0;
    FUN_005755d0();
    iVar4 = 0;
    do {
      if (iVar4 == -1) {
        puVar6 = (undefined4 *)0x0;
      }
      else {
        puVar6 = &DAT_00643a80 + iVar4 * 0x55;
      }
      if ((puVar6[1] != 0) && (puVar6[2] != 0)) goto LAB_0046bc68;
      iVar4 = iVar4 + 1;
    } while (iVar4 < 8);
    iVar4 = -1;
LAB_0046bc68:
    iVar4 = FUN_00594260(iVar4);
    if (iVar4 != 0) {
      DAT_006f5284 = DAT_006c3768;
      _DAT_006f2cf0 = 2;
      _DAT_006f2b30 = iVar4;
      *(undefined1 *)(param_1 + 0x100b2) = 1;
      FUN_00557040();
    }
  }
  return;
}

