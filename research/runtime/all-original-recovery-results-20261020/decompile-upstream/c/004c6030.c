
void __fastcall FUN_004c6030(int param_1)

{
  byte bVar1;
  int *piVar2;
  int iVar3;
  int iVar4;
  int aiStack_20 [3];
  int iStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  int iStack_8;
  int iStack_4;
  
  (**(code **)(*(int *)(param_1 + 0xa4c) + 0x10))();
  FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(*(int *)(param_1 + 0x22) + 0x20),
               *(undefined4 *)(*(int *)(param_1 + 0x22) + 0x24));
  iVar4 = 0;
  if (-1 < *(int *)(param_1 + 0x42)) {
    piVar2 = (int *)(param_1 + 0x260);
    do {
      (**(code **)(*piVar2 + 0x10))();
      iVar4 = iVar4 + 1;
      piVar2 = (int *)((int)piVar2 + 0x7a);
    } while (iVar4 <= *(int *)(param_1 + 0x42));
    if (7 < iVar4) goto LAB_004c60a7;
  }
  iVar3 = 8 - iVar4;
  piVar2 = (int *)(param_1 + 0x630 + iVar4 * 0x7a);
  do {
    (**(code **)(*piVar2 + 0x10))();
    piVar2 = (int *)((int)piVar2 + 0x7a);
    iVar3 = iVar3 + -1;
  } while (iVar3 != 0);
LAB_004c60a7:
  if (*(int *)(param_1 + 0x3a) == 0) {
    if (*(int *)(param_1 + 0x3e) == 0) {
      aiStack_20[2] = 0;
      uStack_10 = 0;
      bVar1 = -(DAT_006de6d5 != 0) & 0xa0;
      if (bVar1 == 0) {
        aiStack_20[2] = 0x140;
      }
      else if (bVar1 == 0xa0) {
        aiStack_20[2] = 400;
      }
      iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      uStack_c = 0;
      aiStack_20[0] = 0;
      aiStack_20[1] = 0;
      iStack_8 = aiStack_20[2];
      iStack_4 = iStack_14;
      FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
      iStack_8 = 0;
      uStack_10 = 0;
      aiStack_20[0] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
      if (aiStack_20[0] == 0x280) {
        iStack_8 = 0x140;
      }
      else if (aiStack_20[0] == 800) {
        iStack_8 = 400;
      }
      iVar4 = 0;
      aiStack_20[0] = aiStack_20[0] - iStack_8;
      iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      aiStack_20[2] = iStack_8 + aiStack_20[0];
      uStack_c = 0;
      aiStack_20[1] = 0;
      iStack_4 = iStack_14;
      FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
      (**(code **)(*(int *)(param_1 + 0x76) + 0x10))();
      (**(code **)(*(int *)(param_1 + 299) + 0x10))();
      if (0 < *(int *)(param_1 + 0x1a2a)) {
        piVar2 = (int *)(param_1 + 0xaea);
        do {
          (**(code **)(*piVar2 + 0x10))();
          iVar4 = iVar4 + 1;
          piVar2 = (int *)((int)piVar2 + 0x7a);
        } while (iVar4 < *(int *)(param_1 + 0x1a2a));
      }
    }
    else {
      aiStack_20[2] = 0;
      uStack_10 = 0;
      bVar1 = -(DAT_006de6d5 != 0) & 0xa0;
      if (bVar1 == 0) {
        aiStack_20[2] = 0x140;
      }
      else if (bVar1 == 0xa0) {
        aiStack_20[2] = 400;
      }
      iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      uStack_c = 0;
      aiStack_20[0] = 0;
      aiStack_20[1] = 0;
      iStack_8 = aiStack_20[2];
      iStack_4 = iStack_14;
      FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
      if (*(int *)(param_1 + 0x3e) < 0x32) {
        aiStack_20[2] = 0;
        uStack_10 = 0;
        bVar1 = -(DAT_006de6d5 != 0) & 0xa0;
        if (bVar1 == 0) {
          aiStack_20[2] = 0x140;
        }
        else if (bVar1 == 0xa0) {
          aiStack_20[2] = 400;
        }
        uStack_c = 0;
        aiStack_20[1] = 0;
        iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
        aiStack_20[0] = aiStack_20[2] - ((0x32 - *(int *)(param_1 + 0x3e)) * aiStack_20[2]) / 0x32;
        iStack_8 = aiStack_20[2];
        iStack_4 = iStack_14;
        FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
      }
      if (*(int *)(param_1 + 0x3e) < 100) {
        iStack_8 = 0;
        uStack_10 = 0;
        aiStack_20[0] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
        if (aiStack_20[0] == 0x280) {
          iStack_8 = 0x140;
        }
        else if (aiStack_20[0] == 800) {
          iStack_8 = 400;
        }
        aiStack_20[0] = aiStack_20[0] - iStack_8;
        iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
        aiStack_20[2] = aiStack_20[0] + iStack_8;
        uStack_c = 0;
        aiStack_20[1] = 0;
        iStack_4 = iStack_14;
        FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
      }
      if (0x32 < *(int *)(param_1 + 0x3e)) {
        iStack_8 = 0;
        uStack_10 = 0;
        aiStack_20[0] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
        if (aiStack_20[0] == 0x280) {
          iStack_8 = 0x140;
        }
        else if (aiStack_20[0] == 800) {
          iStack_8 = 400;
        }
        aiStack_20[0] = aiStack_20[0] - iStack_8;
        iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
        uStack_c = 0;
        aiStack_20[1] = 0;
        aiStack_20[2] = (iStack_8 * (*(int *)(param_1 + 0x3e) + -0x32)) / 0x32 + aiStack_20[0];
        iStack_4 = iStack_14;
        FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
        FUN_004db7e0();
        return;
      }
    }
  }
  else {
    if (*(int *)(param_1 + 0x3a) < 100) {
      aiStack_20[2] = 0;
      uStack_10 = 0;
      bVar1 = -(DAT_006de6d5 != 0) & 0xa0;
      if (bVar1 == 0) {
        aiStack_20[2] = 0x140;
      }
      else if (bVar1 == 0xa0) {
        aiStack_20[2] = 400;
      }
      iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      uStack_c = 0;
      aiStack_20[0] = 0;
      aiStack_20[1] = 0;
      iStack_8 = aiStack_20[2];
      iStack_4 = iStack_14;
      FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
    }
    if (0x32 < *(int *)(param_1 + 0x3a)) {
      aiStack_20[2] = 0;
      uStack_10 = 0;
      bVar1 = -(DAT_006de6d5 != 0) & 0xa0;
      if (bVar1 == 0) {
        aiStack_20[2] = 0x140;
      }
      else if (bVar1 == 0xa0) {
        aiStack_20[2] = 400;
      }
      uStack_c = 0;
      aiStack_20[1] = 0;
      iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      aiStack_20[0] = aiStack_20[2] - ((*(int *)(param_1 + 0x3a) + -0x32) * aiStack_20[2]) / 0x32;
      iStack_8 = aiStack_20[2];
      iStack_4 = iStack_14;
      FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
    }
    iStack_8 = 0;
    uStack_10 = 0;
    aiStack_20[0] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
    if (aiStack_20[0] == 0x280) {
      iStack_8 = 0x140;
    }
    else if (aiStack_20[0] == 800) {
      iStack_8 = 400;
    }
    iVar4 = 0;
    aiStack_20[0] = aiStack_20[0] - iStack_8;
    iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
    aiStack_20[2] = aiStack_20[0] + iStack_8;
    uStack_c = 0;
    aiStack_20[1] = 0;
    iStack_4 = iStack_14;
    FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
    if (*(int *)(param_1 + 0x3a) < 0x32) {
      uStack_10 = 0;
      aiStack_20[0] = (-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x280;
      if (aiStack_20[0] == 0x280) {
        iVar4 = 0x140;
      }
      else if (aiStack_20[0] == 800) {
        iVar4 = 400;
      }
      aiStack_20[0] = aiStack_20[0] - iVar4;
      iStack_14 = (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0;
      aiStack_20[2] = (iVar4 * (0x32 - *(int *)(param_1 + 0x3a))) / 0x32 + aiStack_20[0];
      uStack_c = 0;
      aiStack_20[1] = 0;
      iStack_8 = iVar4;
      iStack_4 = iStack_14;
      FUN_0058cbc0(&DAT_006a49e0,&uStack_10,aiStack_20);
      FUN_004db7e0();
      return;
    }
  }
  FUN_004db7e0();
  return;
}

