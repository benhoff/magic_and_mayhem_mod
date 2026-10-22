
void __fastcall FUN_004633a0(int param_1)

{
  char cVar1;
  undefined4 *puVar2;
  int iVar3;
  uint uVar4;
  uint uVar5;
  char *pcVar6;
  char *pcVar7;
  char *pcVar8;
  undefined4 local_16c [26];
  undefined4 local_104;
  char local_100 [256];
  
  if (*(int *)(param_1 + 0x57) == 0) {
    puVar2 = local_16c;
    for (iVar3 = 0x1b; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    local_16c[0] = 0x6c;
    local_16c[1] = 7;
    local_104 = 0x40;
    local_16c[3] = 0x30;
    local_16c[2] = 0x30;
    FUN_0058ad90(local_16c,s_c__Projects_Chaos_alliedface_cpp_005d962c,0x79);
    FUN_0058bc10(0x1f);
    FUN_0058d0e0(0,0,0xff);
  }
  if (*(int *)(param_1 + 0x25) == 0) {
    puVar2 = local_16c;
    for (iVar3 = 0x1b; iVar3 != 0; iVar3 = iVar3 + -1) {
      *puVar2 = 0;
      puVar2 = puVar2 + 1;
    }
    local_16c[0] = 0x6c;
    local_16c[1] = 7;
    local_104 = 0x40;
    local_16c[3] = 0x60;
    local_16c[2] = 0x30;
    FUN_0058ad90(local_16c,s_c__Projects_Chaos_alliedface_cpp_005d962c,0x86);
    FUN_0058bc10(0x1f);
    FUN_0058d0e0(0,0,0xff);
  }
  if (*(int *)(param_1 + 0x93) == 0) {
    puVar2 = (undefined4 *)FUN_00597850(0x10);
    if (puVar2 == (undefined4 *)0x0) {
      puVar2 = (undefined4 *)0x0;
    }
    else {
      puVar2[1] = &LAB_00462f50;
      puVar2[2] = 0;
      puVar2[3] = param_1;
      *puVar2 = &PTR_LAB_005c5c38;
    }
    *(undefined4 **)(param_1 + 0x93) = puVar2;
    *(undefined4 **)(param_1 + 0x49) = puVar2;
    *(undefined4 *)(param_1 + 0x7b) = *(undefined4 *)(param_1 + 0x93);
  }
  if (*(int *)(param_1 + 0x89) != 0) {
    if ((*(int *)(param_1 + 0x25) != 0) && (*(int *)(param_1 + 0x57) != 0)) {
      uVar4 = 0xffffffff;
      pcVar6 = s_bitmaps__005d9600;
      do {
        pcVar8 = pcVar6;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar8 = pcVar6 + 1;
        cVar1 = *pcVar6;
        pcVar6 = pcVar8;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      pcVar6 = pcVar8 + -uVar4;
      pcVar8 = local_100;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pcVar6;
        pcVar6 = pcVar6 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar8 = *pcVar6;
        pcVar6 = pcVar6 + 1;
        pcVar8 = pcVar8 + 1;
      }
      uVar4 = 0xffffffff;
      pcVar6 = *(char **)(param_1 + 0x85);
      do {
        pcVar8 = pcVar6;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar8 = pcVar6 + 1;
        cVar1 = *pcVar6;
        pcVar6 = pcVar8;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      iVar3 = -1;
      pcVar6 = local_100;
      do {
        pcVar7 = pcVar6;
        if (iVar3 == 0) break;
        iVar3 = iVar3 + -1;
        pcVar7 = pcVar6 + 1;
        cVar1 = *pcVar6;
        pcVar6 = pcVar7;
      } while (cVar1 != '\0');
      pcVar6 = pcVar8 + -uVar4;
      pcVar8 = pcVar7 + -1;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pcVar6;
        pcVar6 = pcVar6 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar8 = *pcVar6;
        pcVar6 = pcVar6 + 1;
        pcVar8 = pcVar8 + 1;
      }
      uVar4 = 0xffffffff;
      pcVar6 = s_face_bmp_005d95f4;
      do {
        pcVar8 = pcVar6;
        if (uVar4 == 0) break;
        uVar4 = uVar4 - 1;
        pcVar8 = pcVar6 + 1;
        cVar1 = *pcVar6;
        pcVar6 = pcVar8;
      } while (cVar1 != '\0');
      uVar4 = ~uVar4;
      iVar3 = -1;
      pcVar6 = local_100;
      do {
        pcVar7 = pcVar6;
        if (iVar3 == 0) break;
        iVar3 = iVar3 + -1;
        pcVar7 = pcVar6 + 1;
        cVar1 = *pcVar6;
        pcVar6 = pcVar7;
      } while (cVar1 != '\0');
      pcVar6 = pcVar8 + -uVar4;
      pcVar8 = pcVar7 + -1;
      for (uVar5 = uVar4 >> 2; uVar5 != 0; uVar5 = uVar5 - 1) {
        *(undefined4 *)pcVar8 = *(undefined4 *)pcVar6;
        pcVar6 = pcVar6 + 4;
        pcVar8 = pcVar8 + 4;
      }
      for (uVar4 = uVar4 & 3; uVar4 != 0; uVar4 = uVar4 - 1) {
        *pcVar8 = *pcVar6;
        pcVar6 = pcVar6 + 1;
        pcVar8 = pcVar8 + 1;
      }
      FUN_0058bc10(0x1f);
      FUN_0058d1a0(local_100);
      FUN_004638a0();
    }
    FUN_004638a0();
  }
  return;
}

