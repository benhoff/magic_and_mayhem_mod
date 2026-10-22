
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

longlong __fastcall
FUN_00596490(undefined4 param_1,uint param_2,int param_3,int param_4,int param_5)

{
  int iVar1;
  byte *pbVar2;
  byte *pbVar3;
  uint uVar4;
  uint uVar5;
  undefined2 *puVar6;
  
  DAT_005f1e78 = param_3;
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
    puVar6 = (undefined2 *)((param_4 + _DAT_005f1e68) * 2 + DAT_00658174);
    DAT_005f1e6c = (int *)(param_3 + 0x28 + DAT_005f1e50 * 8);
    DAT_005f1e58 = (DAT_006a2dc8 - DAT_005f1e64) * 2;
    if (DAT_005f1e54 - DAT_005f1e50 != 0 && DAT_005f1e50 <= DAT_005f1e54) {
      pbVar2 = (byte *)(*DAT_005f1e6c + param_3);
      uVar4 = DAT_006c4830 + param_5;
      DAT_005f1e70 = DAT_005f1e54 - DAT_005f1e50;
      do {
        DAT_005f1e74 = DAT_005f1e64;
        uVar4 = uVar4 + 1 & 0xf;
        pbVar3 = pbVar2;
        do {
          pbVar2 = pbVar3 + 1;
          puVar6 = puVar6 + *pbVar3;
          DAT_005f1e74 = DAT_005f1e74 - (uint)*pbVar3;
          if (DAT_005f1e74 == 0) break;
          pbVar2 = pbVar3 + 2;
          uVar5 = (uint)pbVar3[1];
          iVar1 = (&DAT_00656640)[uVar4];
          DAT_005f1e74 = DAT_005f1e74 - uVar5;
          do {
            *puVar6 = (short)*(undefined4 *)(puVar6 + iVar1);
            puVar6 = puVar6 + 1;
            uVar5 = uVar5 - 1;
          } while (uVar5 != 0);
          pbVar3 = pbVar2;
        } while (DAT_005f1e74 != 0);
        puVar6 = (undefined2 *)((int)puVar6 + DAT_005f1e58);
        DAT_005f1e70 = DAT_005f1e70 + -1;
      } while (DAT_005f1e70 != 0);
    }
  }
  return (ulonglong)param_2 << 0x20;
}

