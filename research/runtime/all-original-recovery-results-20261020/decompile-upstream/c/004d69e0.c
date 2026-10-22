
void __fastcall FUN_004d69e0(int param_1)

{
  int *piVar1;
  int iVar2;
  int iVar3;
  int *piVar4;
  uint uStack_4;
  
  if (*(int *)(param_1 + 0x1876) != 0) {
    if (*(char *)(param_1 + 0x1920) == '\0') {
      iVar3 = *(int *)(param_1 + 0x191c) + 1;
      *(int *)(param_1 + 0x191c) = iVar3;
      if (0xb < iVar3) {
        *(undefined4 *)(param_1 + 0x191c) = 0;
      }
      iVar3 = *(int *)(param_1 + 0x1908);
      if (iVar3 != -1) {
        iVar2 = *(int *)(param_1 + 0xfc + iVar3 * 0xc6);
        *(int *)(param_1 + iVar3 * 0xc6 + 0x165) = *(int *)(param_1 + 0x191c) + 0xde;
        (**(code **)(iVar2 + 0x10))();
      }
      iVar3 = *(int *)(param_1 + 0x190c);
      if (iVar3 != -1) {
        iVar2 = *(int *)(param_1 + 0x1596 + iVar3 * 0xc6);
        *(int *)(param_1 + iVar3 * 0xc6 + 0x15ff) = *(int *)(param_1 + 0x191c) + 0xde;
        (**(code **)(iVar2 + 0x10))();
      }
      uStack_4 = 0;
      if (*(int *)(param_1 + 0x183c) != 0) {
        piVar4 = (int *)(param_1 + 0x17ec);
        do {
          iVar3 = piVar4[1];
          piVar4[1] = iVar3 + -1;
          if (iVar3 + -1 < 1) {
            iVar3 = *(int *)(param_1 + 0x139 + *piVar4 * 0xc6);
            piVar1 = (int *)(param_1 + 0xfc + *piVar4 * 0xc6);
            *(undefined4 *)((int)piVar1 + 0x39) = 0;
            if (iVar3 != 2) {
              piVar1[2] = 1;
            }
            (**(code **)(*piVar1 + 0x10))();
            iVar3 = param_1 + 0xfc + *piVar4 * 0xc6;
            *(undefined4 *)(iVar3 + 0x3d) = 0;
            *(undefined4 *)(iVar3 + 8) = 1;
            FUN_004d53d0(*piVar4,*(undefined4 *)(param_1 + 0x1b1 + *piVar4 * 0xc6));
            iVar3 = *(int *)(param_1 + 0x183c) + -1;
            *(int *)(param_1 + 0x183c) = iVar3;
            FUN_0059cb60((int *)(param_1 + 0x17ec),param_1 + 0x17f4,iVar3 * 8);
          }
          uStack_4 = uStack_4 + 1;
          piVar4 = piVar4 + 2;
        } while (uStack_4 < *(uint *)(param_1 + 0x183c));
      }
    }
    if ((DAT_006de6dd == 0) || (DAT_006de6d9 != 0)) {
      FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x1892),*(undefined4 *)(param_1 + 0x1896)
                  );
    }
    else if (DAT_006e1ee4 == 0) {
      FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x1892),*(undefined4 *)(param_1 + 0x1896)
                  );
    }
    else {
      FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(param_1 + 0x1892),
                   *(undefined4 *)(param_1 + 0x1896),1);
    }
    if (*(int *)(param_1 + 0x1910) != 0) {
      if ((DAT_006de6dd != 0) && (DAT_006de6d9 == 0)) {
        if (DAT_006e1ee4 == 0) {
          FUN_0058c140(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x1860),
                       *(undefined4 *)(param_1 + 0x1864));
          return;
        }
        FUN_0058d5b0(1,&DAT_006a49e0,0,*(undefined4 *)(param_1 + 0x1860),
                     *(undefined4 *)(param_1 + 0x1864),1);
        return;
      }
      FUN_0058bf40(&DAT_006a49e0,*(undefined4 *)(param_1 + 0x1860),*(undefined4 *)(param_1 + 0x1864)
                  );
    }
  }
  return;
}

