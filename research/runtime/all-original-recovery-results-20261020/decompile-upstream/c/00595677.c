
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_00595677(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  byte *pbVar2;
  undefined2 *puVar3;
  
  if ((*(int *)(param_3 + 8) != 0) && (*(int *)(param_3 + 4) != 0)) {
    param_4 = param_4 - *(int *)(param_3 + 0xc);
    if ((param_4 < DAT_006e0008) ||
       (DAT_005f1e64 = *(int *)(param_3 + 4), DAT_006a49b8 <= param_4 + DAT_005f1e64)) {
      return CONCAT44(param_2,1);
    }
    DAT_005f1e54 = *(int *)(param_3 + 8);
    DAT_005f1e50 = 0;
    param_5 = param_5 - *(int *)(param_3 + 0x10);
    _DAT_005f1e68 = param_5;
    if (param_5 < DAT_006cbb6c) {
      DAT_005f1e50 = -(param_5 - DAT_006cbb6c);
      _DAT_005f1e68 = param_5 - (param_5 - DAT_006cbb6c);
    }
    if (DAT_00656618 <= param_5 + DAT_005f1e54) {
      DAT_005f1e54 = DAT_00656618 - param_5;
    }
    _DAT_005f1e68 = (int)ROUND((float10)_DAT_005f1e68 * (float10)DAT_006a2dc8);
    puVar3 = (undefined2 *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
    DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
    DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
    iVar1 = DAT_005f1e54 - DAT_005f1e50;
    if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
      do {
        DAT_005f1e70 = iVar1;
        DAT_005f1e74 = DAT_005f1e64;
        iVar1 = DAT_005f1e64;
        pbVar2 = (byte *)(*DAT_005f1e6c + param_3);
        do {
          DAT_005f1e74 = DAT_005f1e74 - (uint)*pbVar2;
          puVar3 = puVar3 + *pbVar2;
          if (DAT_005f1e74 == 0) break;
          iVar1 = CONCAT31((int3)((uint)iVar1 >> 8),pbVar2[1]);
          DAT_005f1e74 = DAT_005f1e74 - iVar1;
          do {
            *puVar3 = 0;
            puVar3 = puVar3 + 1;
            iVar1 = iVar1 + -1;
          } while (iVar1 != 0);
          iVar1 = 0;
          pbVar2 = pbVar2 + 2;
        } while (DAT_005f1e74 != 0);
        puVar3 = (undefined2 *)((int)puVar3 + DAT_005f1e58);
        DAT_005f1e6c = DAT_005f1e6c + 2;
        DAT_005f1e70 = DAT_005f1e70 + -1;
        iVar1 = DAT_005f1e70;
      } while (DAT_005f1e70 != 0);
    }
  }
  return (ulonglong)param_2 << 0x20;
}

