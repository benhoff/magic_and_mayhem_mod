
/* WARNING: Removing unreachable block (ram,0x00494ff2) */
/* WARNING: Type propagation algorithm not settling */
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __thiscall FUN_00494ab0(undefined2 *param_1,undefined4 param_2)

{
  ushort *puVar1;
  short sVar2;
  ushort uVar3;
  int iVar4;
  uint uVar5;
  undefined4 uVar6;
  undefined4 extraout_ECX;
  int iVar7;
  int *piVar8;
  int iVar9;
  undefined4 *puVar10;
  int *piVar11;
  int *piVar12;
  float10 fVar13;
  undefined8 uVar14;
  int *local_14c;
  undefined2 *local_148;
  char local_141;
  undefined4 *local_140;
  undefined2 *local_13c;
  undefined2 *local_138;
  undefined2 *local_134;
  undefined2 *local_130 [13];
  int local_fc [63];
  
  *(undefined4 *)(param_1 + 0x14) = param_2;
  switch(param_2) {
  case 0:
    FUN_004df230();
    FUN_004df3a0(*(undefined4 *)(param_1 + 0x16),*(undefined4 *)(param_1 + 0x18),
                 *(undefined4 *)(param_1 + 0x1a),*(undefined4 *)(param_1 + 0x1c),
                 *(undefined4 *)(param_1 + 0x1e),*(undefined4 *)(param_1 + 0x20),param_1 + 0xe7,0x20
                );
    FUN_00487d60();
    uVar5 = *(uint *)(param_1 + 0xd3);
    goto LAB_00494b22;
  case 1:
  case 2:
    goto switchD_00494acc_caseD_1;
  case 3:
    if (*(int *)(param_1 + 0x26) == 0x49) {
      FUN_00571310(0x2c6,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                   *(undefined4 *)(param_1 + 8),0,param_1 + 0xce,1);
    }
    goto switchD_00494acc_caseD_1;
  case 4:
    fVar13 = (float10)fpatan((float10)*(float *)(param_1 + 0x36),
                             -(float10)*(float *)(param_1 + 0x38));
    if (fVar13 < (float10)_DAT_005c5380) {
      fVar13 = (float10)_DAT_005c6250 + (float10)_DAT_005c6250 + fVar13;
    }
    FUN_0059d53d((double)((fVar13 * (float10)_DAT_005c6260) / (float10)_DAT_005c6250 +
                         (float10)_DAT_005c5a88));
    uVar5 = __ftol();
    uVar5 = uVar5 & 0x8000000f;
    if ((int)uVar5 < 0) {
      uVar5 = (uVar5 - 1 | 0xfffffff0) + 1;
    }
    *(uint *)(param_1 + 0xd3) = uVar5;
LAB_00494b22:
    *(uint *)(param_1 + 0xd1) = uVar5 + DAT_005fed78 * -4 & 0xf;
    iVar4 = FUN_00489200();
    piVar11 = (int *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(int *)(param_1 + 0x10f) = piVar11[2];
    FUN_00464ca0();
    FUN_00464cb0(*(int *)(param_1 + 0xd1) + *piVar11,0);
    return;
  case 5:
    iVar4 = FUN_00489200();
    piVar11 = (int *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    break;
  case 6:
    iVar4 = FUN_00489200();
    puVar10 = (undefined4 *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(undefined4 *)(param_1 + 0x10f) = puVar10[2];
    FUN_00464ca0();
    FUN_00464cb0(*puVar10,0);
    uVar5 = *(uint *)(param_1 + 0x22);
    if (uVar5 == 0xffffffff) {
      return;
    }
    if (DAT_006def5c <= uVar5) {
      return;
    }
    iVar4 = uVar5 * 0xe4b + DAT_006def58;
    if (iVar4 == 0) {
      return;
    }
    if (DAT_00644520 == -1) {
      return;
    }
    if ((DAT_00644520 < 0) || (7 < DAT_00644520)) goto LAB_00494f79;
    iVar4 = *(int *)(iVar4 + 0x24 + DAT_00644520 * 4);
    goto LAB_00494f73;
  case 7:
    FUN_004df230();
    FUN_004df3a0(*(undefined4 *)(param_1 + 0x16),*(undefined4 *)(param_1 + 0x18),
                 *(undefined4 *)(param_1 + 0x1a),*(undefined4 *)(param_1 + 0x1c),
                 *(undefined4 *)(param_1 + 0x1e),*(undefined4 *)(param_1 + 0x20),param_1 + 0xe7,0x20
                );
  case 0x2d:
  case 0x42:
  case 0x48:
  case 0x49:
  case 0x4a:
  case 0x4c:
  case 0x4d:
  case 0x50:
    iVar4 = FUN_00489200();
    puVar10 = (undefined4 *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    goto LAB_00496c54;
  case 8:
    iVar4 = FUN_00489200();
    puVar10 = (undefined4 *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(undefined4 *)(param_1 + 0x10f) = puVar10[2];
    FUN_00464ca0();
    FUN_00464cb0(*puVar10,0);
    if ((DAT_005e18e8 <
         *(char *)((&DAT_006cb8c2)[*(uint *)(param_1 + 8) >> 1] +
                   (&DAT_006cb942)[*(int *)(param_1 + 6)] + *(int *)(param_1 + 4) + DAT_006c5c5c))
       || (*(int *)(param_1 + 0x109) == DAT_00644520)) {
      FUN_00571310(0x2d6,*(int *)(param_1 + 4),*(int *)(param_1 + 6),*(uint *)(param_1 + 8),0,0,0);
    }
    uVar5 = *(uint *)(param_1 + 0x22);
    if (uVar5 == 0xffffffff) {
      return;
    }
    if (DAT_006def5c <= uVar5) {
      return;
    }
    iVar4 = uVar5 * 0xe4b + DAT_006def58;
    if (iVar4 == 0) {
      return;
    }
    if (DAT_00644520 == -1) {
      return;
    }
    if ((DAT_00644520 < 0) || (7 < DAT_00644520)) goto LAB_00494f79;
    iVar4 = *(int *)(iVar4 + 0x24 + DAT_00644520 * 4);
LAB_00494f73:
    if (iVar4 != 0) {
      return;
    }
LAB_00494f79:
    *(undefined4 *)(param_1 + 0x115) = 0;
    return;
  case 9:
  case 0x1f:
  case 0x20:
    iVar4 = *(int *)(param_1 + 0x26);
    if (iVar4 == 0x4b) {
      FUN_00571310(0x2d9,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                   *(undefined4 *)(param_1 + 8),0,param_1 + 0xce,1);
      uVar14 = CONCAT44(&local_13c,*DAT_00689908);
      local_13c = param_1;
    }
    else if (iVar4 == 0x58) {
      uVar14 = CONCAT44(&local_134,*DAT_006898f0);
      local_134 = param_1;
    }
    else {
      if (iVar4 != 0x22) goto LAB_0049578f;
      uVar14 = CONCAT44(&local_138,*DAT_006898fc);
      local_138 = param_1;
    }
    FUN_0049e310(&local_14c,uVar14);
LAB_0049578f:
    FUN_00464c80();
    *(undefined4 *)(param_1 + 0xe9) =
         *(undefined4 *)(&DAT_006b13c9 + *(int *)(param_1 + 0x26) * 0x2d1);
    return;
  case 10:
    iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 0x348);
    FUN_00464ca0();
    FUN_00464cb0(*(undefined4 *)(iVar4 + 0x340),0);
    uVar5 = FUN_0054e200();
    uVar5 = uVar5 & 7;
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x1a9c);
    }
    for (; uVar5 != 0; uVar5 = uVar5 - 1) {
      FUN_00465000();
    }
    *(undefined4 *)(param_1 + 0xe9) = DAT_006beca6;
    return;
  case 0xb:
    FUN_00464c80();
    if (*(int *)(param_1 + 0x26) != 0x3a) {
      return;
    }
    FUN_00494ab0();
    return;
  case 0xc:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0xc0);
    goto LAB_004968a6;
  case 0xd:
    puVar10 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x10);
    *(int *)(param_1 + 10) = *(int *)(param_1 + 4) * 0x20 + 0x10;
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 6) * 0x20 + 0x10;
    *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10 + 8;
    goto LAB_00496c54;
  case 0xe:
    iVar4 = FUN_0048a770();
    if (iVar4 == 0) goto LAB_00495b51;
    iVar4 = *(int *)(param_1 + 4);
    iVar7 = *(int *)(param_1 + 6);
    *(int *)(param_1 + 10) = *(int *)(*(int *)(param_1 + 200) + 0xb8) + iVar4 * 0x20;
    *(int *)(param_1 + 0xc) = *(int *)(*(int *)(param_1 + 200) + 0xbc) + iVar7 * 0x20;
    iVar9 = *(int *)(DAT_0065660c + 0x94 + (uint)**(ushort **)(param_1 + 0xca) * 0x164);
    *(int *)(param_1 + 10) = iVar4 * 0x20 + 0x10;
    *(int *)(param_1 + 0xc) = iVar7 * 0x20 + 0x10;
    *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) << 4;
    if ((((iVar9 == 0x10) && (-1 < iVar4)) &&
        ((iVar4 < (int)DAT_006c5494 && ((-1 < iVar7 && (iVar7 < (int)DAT_006c5498)))))) &&
       ((-1 < *(int *)(param_1 + 8) + 1 && (*(int *)(param_1 + 8) + 1 < (int)DAT_006c549c)))) {
      *(ushort *)(*(int *)(param_1 + 0xca) + 10) = *(ushort *)(*(int *)(param_1 + 0xca) + 10) | 0x10
      ;
      iVar4 = (&DAT_006cb8c6)[*(int *)(param_1 + 8)] + (&DAT_006cb942)[*(int *)(param_1 + 6)] +
              *(int *)(param_1 + 4);
      uVar3 = *(ushort *)(DAT_006c54dc + iVar4 * 0xc);
      if ((uVar3 == 0) || (*(int *)(DAT_0065660c + 0x94 + (uint)uVar3 * 0x164) != 0x10)) {
        puVar1 = (ushort *)(DAT_006c54dc + iVar4 * 0xc + 10);
        *puVar1 = *puVar1 | 0x10;
      }
    }
    else {
      *(ushort *)(*(int *)(param_1 + 0xca) + 10) = *(ushort *)(*(int *)(param_1 + 0xca) + 10) | 0x10
      ;
    }
    *(undefined4 *)(param_1 + 0xe9) = *(undefined4 *)(*(int *)(param_1 + 200) + 0xc4);
    *(undefined4 *)(param_1 + 0xeb) =
         *(undefined4 *)(&DAT_006b1274 + *(int *)(param_1 + 0x26) * 0x2d1);
    iVar7 = *(int *)(param_1 + 4);
    iVar9 = *(int *)(param_1 + 6);
    *(undefined4 *)(param_1 + 0xed) =
         *(undefined4 *)(&DAT_006b13c5 + *(int *)(param_1 + 0x26) * 0x2d1);
    iVar4 = *(int *)(param_1 + 8) + 1;
    if (((((-1 < iVar7) && (iVar7 < (int)DAT_006c5494)) && (-1 < iVar9)) &&
        ((iVar9 < (int)DAT_006c5498 && (-1 < iVar4)))) &&
       ((iVar4 < (int)DAT_006c549c &&
        (*(short *)(DAT_006c54dc +
                    ((&DAT_006cb942)[iVar9] + (&DAT_006cb8c6)[*(int *)(param_1 + 8)] + iVar7) * 0xc
                   + 4) != -1)))) {
      FUN_0050d9f0();
    }
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x60);
    *(undefined4 *)(param_1 + 0x10f) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x68);
    FUN_00464ca0();
    uVar5 = FUN_004e1110();
    if (uVar5 < 5) {
      FUN_00464cb0(*piVar11 + 3,0);
      return;
    }
    if (uVar5 < 0x14) {
      FUN_00464cb0(*piVar11 + 2,0);
      return;
    }
    if (0x2f < uVar5) goto LAB_004968d6;
    iVar4 = *piVar11 + 1;
    goto LAB_00496eec;
  default:
    goto switchD_00494acc_caseD_f;
  case 0x11:
    FUN_0048a0c0();
    FUN_00571310(0x405,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,param_1 + 0xce,1);
    puVar10 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x3e0);
    goto LAB_00496c54;
  case 0x12:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 800);
    break;
  case 0x13:
    switch(*(undefined4 *)(param_1 + 0x26)) {
    case 0x44:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x590);
      break;
    default:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x1d0);
      break;
    case 0x5d:
    case 0x62:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x280);
      break;
    case 0x5e:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x4c0);
      break;
    case 0x60:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x4d0);
      break;
    case 99:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0xb0);
      break;
    case 0x66:
      piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x610);
    }
    break;
  case 0x14:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x400);
    goto LAB_0049675d;
  case 0x15:
    FUN_00464c80();
    FUN_00571310(0x387,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    *(undefined4 *)(param_1 + 0xe9) = 0;
    *(undefined4 *)(param_1 + 0xed) = DAT_006bdd3c;
    uVar5 = *(uint *)(param_1 + 0x24);
    if (uVar5 == 0xffffffff) {
      FUN_00493e20();
      return;
    }
    if (uVar5 < DAT_006def5c) {
      iVar4 = uVar5 * 0xe4b + DAT_006def58;
      goto LAB_00495476;
    }
    goto LAB_00495474;
  case 0x16:
    FUN_004896b0(*(undefined4 *)(param_1 + 0x24),0);
    puVar10 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x270);
    goto LAB_00496c54;
  case 0x17:
    goto switchD_00494acc_caseD_17;
  case 0x18:
    FUN_00464c80();
    *(undefined4 *)(param_1 + 0xe9) = 4;
    *(undefined4 *)(param_1 + 0xed) = 3;
    FUN_00492090();
    return;
  case 0x19:
    if ((*(byte *)(*(int *)(param_1 + 0xca) + 10) & 0x20) != 0) goto LAB_00495b51;
    uVar3 = *(ushort *)(*(int *)(param_1 + 0xca) + 4);
    if (uVar3 != 0xffff) {
      if (uVar3 < DAT_006def5c) {
        iVar4 = (uint)uVar3 * 0xe4b + DAT_006def58;
      }
      else {
        iVar4 = 0;
      }
      *(int *)(param_1 + 0xcc) = iVar4;
      FUN_0050cf30();
      if (*(int *)(*(int *)(*(int *)(param_1 + 0xcc) + 0xac) + 8) == 2) {
        if (*(int *)(param_1 + 0x8e) != 3) {
          FUN_0048ad20();
        }
      }
      else if (*(int *)(*(int *)(param_1 + 0xcc) + 0x618) != 0) {
        FUN_00488330();
        iVar4 = *(int *)(param_1 + 0xcc);
        *(undefined4 *)(param_1 + 4) =
             *(undefined4 *)(iVar4 + 0x97f + *(int *)(iVar4 + 0x977) * 0x1c);
        *(undefined4 *)(param_1 + 6) =
             *(undefined4 *)(iVar4 + 0x983 + *(int *)(iVar4 + 0x977) * 0x1c);
        uVar6 = *(undefined4 *)(iVar4 + 0x987 + *(int *)(iVar4 + 0x977) * 0x1c);
        *(undefined4 *)(param_1 + 8) = uVar6;
        uVar6 = FUN_00490ea0(*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),uVar6);
        *(undefined4 *)(param_1 + 0xca) = uVar6;
        FUN_004882b0();
        *(uint *)(param_1 + 200) = DAT_0065660c + (uint)**(ushort **)(param_1 + 0xca) * 0x164;
      }
    }
    *(ushort *)(*(int *)(param_1 + 0xca) + 10) = *(ushort *)(*(int *)(param_1 + 0xca) + 10) | 0x20;
    *(int *)(param_1 + 10) = *(int *)(param_1 + 4) * 0x20 + 0x10;
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 6) * 0x20 + 0x10;
    *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10;
    if (**(short **)(param_1 + 0xca) != 0) {
      *(int *)(param_1 + 0xe) =
           *(int *)(*(int *)(param_1 + 200) + 0x94) + *(int *)(param_1 + 8) * 0x10;
    }
    *(undefined4 *)(param_1 + 0xe9) = *(undefined4 *)(param_1 + 0x28);
    *(undefined4 *)(param_1 + 0xeb) =
         *(undefined4 *)(&DAT_006b13e9 + *(int *)(param_1 + 0x26) * 0x2d1);
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x290);
    goto LAB_004968a6;
  case 0x1a:
    iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 0x2b8);
    FUN_00464ca0();
    uVar5 = FUN_0054e200();
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x1b78);
    }
    iVar4 = *(int *)(iVar4 + 0x2b0) + (uVar5 & 0x7fffffff) % 3;
    goto LAB_00496eec;
  case 0x1b:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x2a0);
    goto LAB_004968a6;
  case 0x1c:
  case 0x1d:
    piVar11 = (int *)(*(int *)(param_1 + 0x26) * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(int *)(param_1 + 0x10f) = piVar11[2];
    FUN_00464ca0();
    iVar4 = *piVar11;
    goto LAB_004959ba;
  case 0x1e:
    FUN_00464c80();
    uVar5 = *(uint *)(param_1 + 0x24);
    if (uVar5 != 0xffffffff) {
      if (uVar5 < DAT_006def5c) {
        iVar4 = uVar5 * 0xe4b + DAT_006def58;
      }
      else {
        iVar4 = 0;
      }
      *(uint *)(param_1 + 0xe9) =
           ((*(int *)(*(int *)(iVar4 + 0xac) + 0x198) - *(int *)(param_1 + 0x1a)) +
           DAT_006c549c * 0x10) / 0xc + DAT_005e147c;
      return;
    }
    FUN_004943f0();
    return;
  case 0x21:
    FUN_00464c80();
    *(undefined4 *)(param_1 + 0xe9) = 0;
    local_130[0] = param_1;
    FUN_0049e310(&local_14c,*DAT_006898e4,local_130);
    *(undefined4 *)(param_1 + 0x10f) = 0;
    *(uint *)(param_1 + 0x115) = (uint)(*(int *)(param_1 + 0x109) != DAT_00644520);
    return;
  case 0x22:
    FUN_00464c80();
    return;
  case 0x23:
    FUN_00571310(0x2d0,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    FUN_0048a950(*(undefined4 *)(param_1 + 0x16),*(undefined4 *)(param_1 + 0x18),
                 *(undefined4 *)(param_1 + 0x1a),*(undefined4 *)(param_1 + 0x1c),
                 *(undefined4 *)(param_1 + 0x1e),*(undefined4 *)(param_1 + 0x20));
    iVar4 = FUN_00489200();
    puVar10 = (undefined4 *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(undefined4 *)(param_1 + 0x10f) = puVar10[2];
    FUN_00464ca0();
    FUN_00464cb0(*puVar10,0);
    *(undefined4 *)(param_1 + 0xe9) =
         *(undefined4 *)(&DAT_006b13c9 + *(int *)(param_1 + 0x26) * 0x2d1);
    return;
  case 0x24:
    FUN_00571310(0x3f5,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    puVar10 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x380);
    goto LAB_00496c54;
  case 0x25:
    FUN_00571310(0x3f5,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x390);
    goto LAB_004968a6;
  case 0x26:
    iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 0x3d8);
    FUN_00464ca0();
    iVar4 = *(int *)(param_1 + 0x26) + *(int *)(iVar4 + 0x3d0);
LAB_004959ba:
    FUN_00464cb0(iVar4);
    if (DAT_006def5c <= *(uint *)(param_1 + 0x22)) {
      *(undefined4 *)(param_1 + 0xcc) = 0;
      return;
    }
    *(uint *)(param_1 + 0xcc) = *(uint *)(param_1 + 0x22) * 0xe4b + DAT_006def58;
    return;
  case 0x27:
    if (*(uint *)(param_1 + 0x26) < DAT_006b1254) {
      iVar4 = *(uint *)(param_1 + 0x26) * 0x20 + DAT_006b1250;
    }
    else {
      iVar4 = 0;
    }
    iVar4 = *(int *)(iVar4 + 0x18) * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 8);
    FUN_00464ca0();
    FUN_00464cb0(*(undefined4 *)(iVar4 + 0xc),0);
    return;
  case 0x28:
  case 0x3a:
  case 0x57:
    goto switchD_00494acc_caseD_28;
  case 0x29:
    iVar4 = FUN_0048f490();
    if ((iVar4 != 0) && (iVar4 = FUN_0048f690(*(undefined4 *)(param_1 + 0x26),0), iVar4 != 0)) {
LAB_00495b65:
      FUN_00494ab0();
      return;
    }
LAB_00495a13:
    FUN_00571310(0x3f3,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    uVar5 = *(uint *)(param_1 + 0x22);
    if ((uVar5 == 0xffffffff) || (*(int *)(param_1 + 0x2e) == 4)) goto LAB_00495b51;
    if (uVar5 < DAT_006def5c) {
      iVar4 = uVar5 * 0xe4b + DAT_006def58;
    }
    else {
      iVar4 = 0;
    }
    *(int *)(param_1 + 0xcc) = iVar4;
    iVar4 = **(int **)(iVar4 + 0xac);
    goto joined_r0x00495b24;
  case 0x2a:
    FUN_00464c80();
    FUN_00571310(0x2f2,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    return;
  case 0x2b:
    if (**(short **)(param_1 + 0xca) != 0) {
      *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10;
      *(int *)(param_1 + 0xe) =
           *(int *)(*(int *)(param_1 + 200) + 0x94) + *(int *)(param_1 + 8) * 0x10;
    }
    local_148 = param_1 + 0xb0;
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x3c0);
    *(undefined4 *)(param_1 + 0x10f) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x3c8);
    FUN_00464ca0();
    if (*(uint *)(param_1 + 0x22) < DAT_006def5c) {
      iVar4 = *(uint *)(param_1 + 0x22) * 0xe4b + DAT_006def58;
    }
    else {
      iVar4 = 0;
    }
    *(int *)(param_1 + 0xcc) = iVar4;
    if (*(int *)(iVar4 + 0x104) != 0) {
      uVar14 = CONCAT44(*(undefined4 *)(iVar4 + 0x973),*(undefined4 *)(iVar4 + 0x973));
      FUN_0040e8a0(*(undefined4 *)(iVar4 + 0x96f));
      uVar6 = extraout_ECX;
      FUN_0040e290(*(undefined4 *)(iVar4 + 0x96b));
      iVar4 = FUN_00414890(uVar6,uVar14);
      if (iVar4 == 0) {
        *(undefined4 *)(param_1 + 0x2c) = 0x42;
        uVar5 = 0;
        puVar10 = DAT_00689498;
        do {
          if ((puVar10[1] == 0) && (*(int *)((int)puVar10 + 0x20e) == 0)) goto LAB_00495d50;
          uVar5 = uVar5 + 1;
          puVar10 = (undefined4 *)((int)puVar10 + 0x22e);
        } while (uVar5 < 0x7d);
        puVar10 = (undefined4 *)0x0;
LAB_00495d50:
        local_140 = puVar10;
        FUN_00461550();
        if (puVar10 != (undefined4 *)0x0) {
          piVar8 = (int *)(param_1 + 0x16);
          piVar12 = local_fc;
          for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
            *piVar12 = *piVar8;
            piVar8 = piVar8 + 1;
            piVar12 = piVar12 + 1;
          }
          FUN_00493f20(0x57,local_fc);
          *(undefined4 *)(param_1 + 0x2e) = 2;
          uVar6 = *local_140;
          *(undefined4 *)(param_1 + 0x8c) = 1;
          *(undefined4 *)(param_1 + 0x30) = uVar6;
        }
        FUN_0048a0c0();
        FUN_00464cb0(*piVar11 + 1,0);
        return;
      }
    }
    iVar4 = *piVar11;
    goto LAB_00496eec;
  case 0x2c:
    FUN_00464c80();
    *(undefined4 *)(param_1 + 0xe9) = *(undefined4 *)(param_1 + 0x34);
    return;
  case 0x2e:
    FUN_00464c80();
    FUN_00571310(0x38e,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    uVar5 = *(uint *)(param_1 + 0x24);
    if (uVar5 == 0xffffffff) {
      return;
    }
    if (uVar5 < DAT_006def5c) {
      iVar4 = uVar5 * 0xe4b + DAT_006def58;
      goto LAB_00495476;
    }
    goto LAB_00495474;
  case 0x2f:
    FUN_00571310(0x2db,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    FUN_004898a0();
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x170);
    break;
  case 0x30:
    FUN_00464c80();
    FUN_00571310(0x387,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    uVar5 = *(uint *)(param_1 + 0x24);
    *(undefined4 *)(param_1 + 0xe9) = 0;
    *(undefined4 *)(param_1 + 0xed) = DAT_006bdd3c;
    if (uVar5 == 0xffffffff) {
      return;
    }
    if (uVar5 < DAT_006def5c) {
      iVar4 = uVar5 * 0xe4b + DAT_006def58;
      goto LAB_00495476;
    }
LAB_00495474:
    iVar4 = 0;
LAB_00495476:
    *(int *)(param_1 + 0xcc) = iVar4;
    if (iVar4 == 0) {
      return;
    }
    if (*(int *)(iVar4 + 0xe4) == 0) {
      return;
    }
    FUN_005227e0();
    return;
  case 0x31:
    FUN_00464c80();
    *(undefined4 *)(param_1 + 0xe9) = DAT_006b765c;
    return;
  case 0x33:
    if (*(uint *)(param_1 + 0x24) < DAT_006def5c) {
      iVar4 = *(uint *)(param_1 + 0x24) * 0xe4b + DAT_006def58;
    }
    else {
      iVar4 = 0;
    }
    *(int *)(param_1 + 0xcc) = iVar4;
    if (iVar4 == 0) {
      FUN_004943f0();
      return;
    }
    FUN_004896b0(*(undefined4 *)(param_1 + 0x22),0);
    iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 0x2c8);
    FUN_00464ca0();
    uVar6 = FUN_00488010();
    *(undefined4 *)(param_1 + 0xed) = uVar6;
    FUN_0040e290();
    FUN_0040e290(*(undefined4 *)(*(int *)(param_1 + 0xcc) + 8));
    local_148 = (undefined2 *)FUN_0040e4e0();
    FUN_0040e8a0();
    FUN_0040e8a0(*(undefined4 *)(*(int *)(param_1 + 0xcc) + 0xc));
    local_14c = (int *)FUN_0040eb70();
    iVar7 = FUN_004eacc0((float)(int)local_148,(float)(int)local_14c);
    *(int *)(param_1 + 0xd3) = iVar7;
    FUN_00464cb0((iVar7 + DAT_005fed78 * -2 & 7U) + *(int *)(iVar4 + 0x2c0),0);
    uVar5 = *(uint *)(param_1 + 0x22);
    *(undefined4 *)(param_1 + 0x115) = 1;
    if (uVar5 == 0xffffffff) {
      return;
    }
    if (DAT_006def5c <= uVar5) {
      return;
    }
    if (uVar5 * 0xe4b + DAT_006def58 == 0) {
      return;
    }
    FUN_00508ca0(1,*(undefined4 *)(*(int *)(param_1 + 0xcc) + 0x174),*(int *)(param_1 + 0xcc));
    return;
  case 0x34:
    uVar5 = *(uint *)(param_1 + 0x22);
    if (uVar5 < DAT_006def5c) {
      iVar4 = uVar5 * 0xe4b + DAT_006def58;
    }
    else {
      iVar4 = 0;
    }
    *(int *)(param_1 + 0xcc) = iVar4;
    if (iVar4 == 0) {
      FUN_004943f0();
      return;
    }
    FUN_004896b0(uVar5,0);
    iVar7 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar7 + 0x2d8);
    FUN_00464ca0();
    iVar4 = *(int *)(*(int *)(param_1 + 0xcc) + 0x60c);
    *(undefined4 *)(param_1 + 0xe9) = 1;
    *(int *)(param_1 + 0xed) = iVar4;
    iVar4 = *(int *)(iVar7 + 0x2d0) + iVar4;
    goto LAB_00496eec;
  case 0x36:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x90);
    break;
  case 0x37:
    if (*(uint *)(param_1 + 0x22) < DAT_006def5c) {
      iVar4 = *(uint *)(param_1 + 0x22) * 0xe4b + DAT_006def58;
    }
    else {
      iVar4 = 0;
    }
    *(int *)(param_1 + 0xcc) = iVar4;
    if (iVar4 != 0) {
      FUN_00571310(0x3fe,*(undefined4 *)(iVar4 + 8),*(undefined4 *)(iVar4 + 0xc),
                   *(undefined4 *)(iVar4 + 0x10),0,0,0);
    }
    puVar10 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0xa0);
    goto LAB_00496c54;
  case 0x38:
    FUN_00464c80();
    *(undefined4 *)(param_1 + 0xe9) = 0;
    *(undefined4 *)(param_1 + 0xed) = 0;
    return;
  case 0x39:
    FUN_0048a950(*(undefined4 *)(param_1 + 0x16),*(undefined4 *)(param_1 + 0x18),
                 *(undefined4 *)(param_1 + 0x1a),*(undefined4 *)(param_1 + 0x1c),
                 *(undefined4 *)(param_1 + 0x1e),*(undefined4 *)(param_1 + 0x20));
    iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 0x418);
    FUN_00464ca0();
    FUN_00464cb0(*(undefined4 *)(iVar4 + 0x410),0);
    FUN_00571310(0x2c6,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,param_1 + 0xce,1);
    return;
  case 0x3b:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x430);
    break;
  case 0x3c:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x80);
    goto LAB_004968a6;
  case 0x3d:
    if (*(uint *)(param_1 + 0x26) < DAT_006b1254) {
      iVar4 = *(uint *)(param_1 + 0x26) * 0x20 + DAT_006b1250;
    }
    else {
      iVar4 = 0;
    }
    puVar10 = (undefined4 *)
              (*(int *)(iVar4 + 0x18) * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(undefined4 *)(param_1 + 0x10f) = puVar10[2];
    FUN_00464ca0();
    FUN_00464cb0(*puVar10,0);
    goto switchD_00494acc_caseD_17;
  case 0x3e:
    local_148 = *(undefined2 **)(param_1 + 4);
    if ((int)local_148 < 0) {
      do {
        local_148 = (undefined2 *)((int)local_148 + (int)DAT_006c5494);
      } while ((int)local_148 < 0);
    }
    else {
      for (; DAT_006c5494 <= local_148;
          local_148 = (undefined2 *)((int)local_148 - (int)DAT_006c5494)) {
      }
    }
    local_140 = *(undefined4 **)(param_1 + 6);
    if ((int)local_140 < 0) {
      do {
        local_140 = (undefined4 *)((int)local_140 + (int)DAT_006c5498);
      } while ((int)local_140 < 0);
    }
    else {
      for (; DAT_006c5498 <= local_140;
          local_140 = (undefined4 *)((int)local_140 - (int)DAT_006c5498)) {
      }
    }
    local_14c = *(int **)(param_1 + 8);
    if (*(int *)(param_1 + 0x28) != 0) {
      FUN_004f5b00(&local_148,&local_140,&local_14c);
    }
    FUN_00488330();
    *(undefined2 **)(param_1 + 4) = local_148;
    *(undefined4 **)(param_1 + 6) = local_140;
    *(int **)(param_1 + 8) = local_14c;
    iVar4 = DAT_006c54dc +
            ((&DAT_006cb942)[(int)local_140] + (&DAT_006cb8c2)[(int)local_14c] + (int)local_148) *
            0xc;
    *(int *)(param_1 + 0xca) = iVar4;
    if ((*(byte *)(iVar4 + 0xb) & 0x40) == 0) {
      uVar3 = *(ushort *)(iVar4 + 2);
      if (uVar3 == 0xffff) {
        *(undefined2 *)(iVar4 + 2) = *param_1;
      }
      else {
        do {
          if ((uint)uVar3 < (uint)(*(int **)(param_1 + 0x12))[1]) {
            iVar7 = **(int **)(param_1 + 0x12) + (uint)uVar3 * 0x22e;
          }
          else {
            iVar7 = 0;
          }
          uVar3 = *(ushort *)(iVar7 + 0x1a0);
        } while (uVar3 != 0xffff);
        *(undefined2 *)(iVar7 + 0x1a0) = *param_1;
        param_1[0xd0] = 0xffff;
      }
      *(ushort *)(iVar4 + 8) = *(ushort *)(iVar4 + 8) & 0xff7f;
    }
    *(int *)(param_1 + 10) = *(int *)(param_1 + 4) * 0x20 + 0x10;
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 6) * 0x20 + 0x10;
    *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) << 4;
  case 0x41:
    sVar2 = **(short **)(param_1 + 0xca);
    if ((sVar2 != 0) &&
       ((((sVar2 == 0x14d || (sVar2 == 0x153)) || (sVar2 == 0x159)) ||
        ((sVar2 == 0x160 || (sVar2 == 0x167)))))) {
      iVar9 = *(int *)(param_1 + 8) + 1;
      iVar7 = (&DAT_006cb942)[*(int *)(param_1 + 6)] + (&DAT_006cb8c2)[iVar9] +
              *(int *)(param_1 + 4);
      iVar4 = DAT_006c54dc + iVar7 * 0xc;
      sVar2 = *(short *)(DAT_006c54dc + iVar7 * 0xc);
      if (((sVar2 != 0) &&
          ((((((0x14c < sVar2 && (sVar2 < 0x154)) || ((0x152 < sVar2 && (sVar2 < 0x15a)))) ||
             ((0x158 < sVar2 && (sVar2 < 0x161)))) || ((0x15f < sVar2 && (sVar2 < 0x168)))) ||
           ((0x166 < sVar2 && (sVar2 < 0x16f)))))) &&
         (local_141 = '\0', puVar10 = DAT_00689498, uVar5 = DAT_0068949c, iVar9 < DAT_005e1780)) {
        do {
          if (local_141 != '\0') break;
          uVar3 = *(ushort *)(iVar4 + 2);
          if (((uVar3 != 0) && (uVar3 < uVar5)) &&
             (iVar7 = (int)puVar10 + (uint)uVar3 * 0x22e, iVar7 != 0)) {
            while (*(int *)(iVar7 + 0x28) != 0x21) {
              if ((uVar5 <= *(ushort *)(iVar7 + 0x1a0)) ||
                 (iVar7 = (int)puVar10 + (uint)*(ushort *)(iVar7 + 0x1a0) * 0x22e, iVar7 == 0))
              goto LAB_00496587;
            }
            local_141 = '\x01';
            FUN_00490260();
            puVar10 = DAT_00689498;
            uVar5 = DAT_0068949c;
          }
LAB_00496587:
          iVar9 = iVar9 + 1;
          iVar4 = iVar4 + DAT_006c54a0 * 0xc;
        } while (iVar9 < DAT_005e1780);
      }
    }
    *(undefined4 *)(param_1 + 0x2e) = 2;
    puVar10 = (undefined4 *)FUN_0049cdb0();
    if (puVar10 == (undefined4 *)0x0) {
      if (DAT_0068949c < 0x7d) {
        puVar10 = (undefined4 *)0x0;
      }
      else {
        puVar10 = DAT_00689498 + 0x4392;
      }
      if (puVar10[1] != 0) {
        FUN_004943f0();
      }
    }
    iVar4 = *(int *)(param_1 + 8);
    piVar11 = local_fc;
    for (iVar7 = 0x3f; iVar7 != 0; iVar7 = iVar7 + -1) {
      *piVar11 = 0;
      piVar11 = piVar11 + 1;
    }
    local_fc[1] = *(int *)(param_1 + 0xc);
    local_fc[0] = *(int *)(param_1 + 10);
    local_fc[2] = (iVar4 + 1) * 0x10;
    local_fc[5] = 0xffffffff;
    local_fc[4] = 0xffffffff;
    local_fc[3] = 0xffffffff;
    local_fc[9] = 0xffffffff;
    local_fc[8] = 0xffffffff;
    local_fc[7] = 0xffffffff;
    local_fc[6] = 0xffffffff;
    local_fc[0xc] = 7;
    local_fc[10] = 1;
    local_fc[0xb] = 0xffffffff;
    *(undefined4 *)(param_1 + 0x8c) = 1;
    *(undefined4 *)(param_1 + 0x32) = *puVar10;
    FUN_00493f20(0x4e,local_fc);
    puVar10 = *(undefined4 **)(param_1 + 6);
    if ((int)puVar10 < 0) {
      do {
        puVar10 = (undefined4 *)((int)puVar10 + (int)DAT_006c5498);
      } while ((int)puVar10 < 0);
    }
    else {
      for (; DAT_006c5498 <= puVar10; puVar10 = (undefined4 *)((int)puVar10 - (int)DAT_006c5498)) {
      }
    }
    uVar14 = CONCAT44(puVar10,DAT_006c5498);
    FUN_0040e290(*(undefined4 *)(param_1 + 4));
    FUN_00556820(uVar14);
    FUN_00464c80();
    FUN_004eb820();
    *(int *)(param_1 + 0xe9) = *(int *)(param_1 + 0x2c);
    if (*(int *)(param_1 + 0x2c) != -1) {
      FUN_004dde90();
      FUN_004de490();
      FUN_004de5a0((-(uint)(DAT_006de6d5 != 0) & 0xa0) + 0x1e0,
                   (-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x158);
      FUN_004de610();
      DAT_006905ac = 1;
    }
    *(undefined4 *)(param_1 + 0xeb) = 0;
    FUN_0056f000(0x414,0,0,param_1 + 0xce,1,0xffffffff,0xffffffff);
    return;
  case 0x3f:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x440);
LAB_0049675d:
    *(int *)(param_1 + 0x10f) = piVar11[2];
    FUN_00464ca0();
    iVar4 = *(int *)(param_1 + 0x2c) + *piVar11;
    goto LAB_00496eec;
  case 0x40:
    iVar4 = FUN_0048f490();
    if (iVar4 == 0) goto LAB_00495a13;
    iVar4 = FUN_0048f930();
    if (iVar4 != 0) goto LAB_00495b65;
    FUN_00571310(0x3f3,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    uVar5 = *(uint *)(param_1 + 0x22);
    if ((uVar5 == 0xffffffff) || (*(int *)(param_1 + 0x2e) == 4)) goto LAB_00495b51;
    if (uVar5 < DAT_006def5c) {
      iVar4 = uVar5 * 0xe4b + DAT_006def58;
    }
    else {
      iVar4 = 0;
    }
    *(int *)(param_1 + 0xcc) = iVar4;
    iVar4 = **(int **)(iVar4 + 0xac);
joined_r0x00495b24:
    if ((((iVar4 == 0) || (iVar4 == 0x18)) || (iVar4 == 0x19)) || (iVar4 == 0x1a)) {
      FUN_00507f00();
    }
LAB_00495b51:
    FUN_004943f0();
    return;
  case 0x43:
    FUN_004896b0(*(undefined4 *)(param_1 + 0x24),0);
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x200);
    goto LAB_004968a6;
  case 0x44:
    FUN_004896b0(*(undefined4 *)(param_1 + 0x24),0xfffffffe);
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x210);
    break;
  case 0x45:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x3a0);
    goto LAB_004968a6;
  case 0x46:
    if (**(short **)(param_1 + 0xca) != 0) {
      *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10;
      *(int *)(param_1 + 0xe) =
           *(int *)(*(int *)(param_1 + 200) + 0x94) + *(int *)(param_1 + 8) * 0x10;
    }
    local_148 = param_1 + 0xb0;
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x500);
    *(undefined4 *)(param_1 + 0x10f) =
         *(undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x508);
    local_14c = piVar11;
    FUN_00464ca0();
    if (*(int *)(param_1 + 0x2c) != 0) {
      *(undefined4 *)(param_1 + 0x2c) = 0x51;
      uVar5 = 0;
      puVar10 = DAT_00689498;
      while ((puVar10[1] != 0 || (*(int *)((int)puVar10 + 0x20e) != 0))) {
        uVar5 = uVar5 + 1;
        puVar10 = (undefined4 *)((int)puVar10 + 0x22e);
        if (0x7c < uVar5) {
LAB_004969c7:
          FUN_0048a0c0();
          FUN_00464cb0(*local_14c + 1,0);
          return;
        }
      }
      if (puVar10 != (undefined4 *)0x0) {
        piVar11 = (int *)(param_1 + 0x16);
        piVar8 = local_fc;
        for (iVar4 = 0x3f; iVar4 != 0; iVar4 = iVar4 + -1) {
          *piVar8 = *piVar11;
          piVar11 = piVar11 + 1;
          piVar8 = piVar8 + 1;
        }
        FUN_00493f20(0x57,local_fc);
        *(undefined4 *)(param_1 + 0x2e) = 2;
        *(undefined4 *)(param_1 + 0x30) = *puVar10;
        *(undefined4 *)(param_1 + 0x8c) = 1;
      }
      goto LAB_004969c7;
    }
    goto LAB_004968d6;
  case 0x47:
    uVar5 = FUN_0054e200();
    uVar5 = (uVar5 & 0x7fffffff) % 3;
    if (DAT_006e1f30 != 0) {
      FUN_00485b10(s_c__Projects_Chaos_effects_cpp_005e0138,0x1e41);
    }
    if (uVar5 == 0) {
      uVar5 = 0x14;
      *(undefined4 *)(param_1 + 0x2c) = 0;
LAB_00496a77:
      iVar7 = uVar5 << 4;
      iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    }
    else if (uVar5 == 1) {
      *(undefined4 *)(param_1 + 0x2c) = 1;
      iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
      iVar7 = 0x150;
    }
    else {
      if (uVar5 != 2) goto LAB_00496a77;
      *(undefined4 *)(param_1 + 0x2c) = 2;
      iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
      iVar7 = 0xd0;
    }
    goto LAB_00496eb8;
  case 0x4b:
    if (**(short **)(param_1 + 0xca) == 0) {
      return;
    }
    *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10;
    *(int *)(param_1 + 0xe) =
         *(int *)(*(int *)(param_1 + 200) + 0x94) + *(int *)(param_1 + 8) * 0x10;
    return;
  case 0x4e:
    piVar11 = (int *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x5f0);
LAB_004968a6:
    *(int *)(param_1 + 0x10f) = piVar11[2];
    FUN_00464ca0();
LAB_004968d6:
    FUN_00464cb0(*piVar11,0);
    return;
  case 0x4f:
    FUN_0048a950(*(undefined4 *)(param_1 + 0x16),*(undefined4 *)(param_1 + 0x18),
                 *(undefined4 *)(param_1 + 0x1a),*(undefined4 *)(param_1 + 0x1c),
                 *(undefined4 *)(param_1 + 0x1e),*(undefined4 *)(param_1 + 0x20));
    FUN_00464c80();
    FUN_00571310(0x2d9,*(undefined4 *)(param_1 + 4),*(undefined4 *)(param_1 + 6),
                 *(undefined4 *)(param_1 + 8),0,0,0);
    return;
  case 0x51:
    uVar5 = *(uint *)(param_1 + 0x22);
    if (uVar5 != 0xffffffff) {
      if (uVar5 < DAT_006def5c) {
        iVar4 = uVar5 * 0xe4b + DAT_006def58;
      }
      else {
        iVar4 = 0;
      }
      *(int *)(param_1 + 0xcc) = iVar4;
    }
    if (*(int *)(param_1 + 0x2a) == 4) {
      FUN_004114c0();
      FUN_004114c0(*(undefined4 *)(param_1 + 0x1c));
      local_148 = (undefined2 *)FUN_0040f7a0();
    }
    else {
      FUN_0040e290();
      FUN_0040e290(*(undefined4 *)(param_1 + 4));
      local_148 = (undefined2 *)FUN_0040e4e0();
    }
    if (*(int *)(param_1 + 0x2a) == 4) {
      FUN_00411500();
      FUN_00411500(*(undefined4 *)(param_1 + 0x1e));
      local_14c = (int *)FUN_0040fb00();
    }
    else {
      FUN_0040e8a0();
      FUN_0040e8a0(*(undefined4 *)(param_1 + 6));
      local_14c = (int *)FUN_0040eb70();
    }
    uVar6 = FUN_004eacc0((float)(int)local_148,(float)(int)local_14c);
    *(undefined4 *)(param_1 + 0xd3) = uVar6;
    iVar4 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 0x488);
    FUN_00464ca0();
    FUN_00464cb0((*(int *)(param_1 + 0xd3) + DAT_005fed78 * -2 & 7U) + *(int *)(iVar4 + 0x480),0);
    return;
  case 0x52:
    if (**(short **)(param_1 + 0xca) != 0) {
      *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10;
      *(int *)(param_1 + 0xe) =
           *(int *)(*(int *)(param_1 + 200) + 0x94) + *(int *)(param_1 + 8) * 0x10;
    }
    piVar11 = (int *)(*(int *)(param_1 + 0x26) * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
    *(int *)(param_1 + 0x10f) = piVar11[2];
    FUN_00464ca0();
    FUN_00464cb0(*(int *)(param_1 + 0x2c) + *piVar11,0);
    return;
  case 0x54:
    if (*(int *)(param_1 + 8) + 1U < DAT_006c549c) {
      FUN_00488330();
      iVar4 = *(int *)(param_1 + 8) + 1;
      *(int *)(param_1 + 8) = iVar4;
      *(int *)(param_1 + 0xe) = iVar4 * 0x10;
      puVar1 = (ushort *)
               (DAT_006c54dc +
               ((&DAT_006cb942)[*(int *)(param_1 + 6)] + (&DAT_006cb8c2)[iVar4] +
               *(int *)(param_1 + 4)) * 0xc);
      *(ushort **)(param_1 + 0xca) = puVar1;
      iVar4 = *(int *)(param_1 + 0xca);
      *(uint *)(param_1 + 200) = DAT_0065660c + (uint)*puVar1 * 0x164;
      if ((*(byte *)(iVar4 + 0xb) & 0x40) == 0) {
        uVar3 = *(ushort *)(iVar4 + 2);
        if (uVar3 == 0xffff) {
          *(undefined2 *)(iVar4 + 2) = *param_1;
        }
        else {
          do {
            if ((uint)uVar3 < (uint)(*(int **)(param_1 + 0x12))[1]) {
              iVar7 = **(int **)(param_1 + 0x12) + (uint)uVar3 * 0x22e;
            }
            else {
              iVar7 = 0;
            }
            uVar3 = *(ushort *)(iVar7 + 0x1a0);
          } while (uVar3 != 0xffff);
          *(undefined2 *)(iVar7 + 0x1a0) = *param_1;
          param_1[0xd0] = 0xffff;
        }
        *(ushort *)(iVar4 + 8) = *(ushort *)(iVar4 + 8) & 0xff7f;
      }
    }
    puVar10 = (undefined4 *)(*(int *)(*(int *)(param_1 + 0x12) + 0x438) + 0x580);
LAB_00496c54:
    *(undefined4 *)(param_1 + 0x10f) = puVar10[2];
    FUN_00464ca0();
    FUN_00464cb0(*puVar10,0);
    return;
  case 0x55:
    *(int *)(param_1 + 10) = *(int *)(param_1 + 4) * 0x20 + 0x10;
    *(int *)(param_1 + 0xc) = *(int *)(param_1 + 6) * 0x20 + 0x10;
    if (**(short **)(param_1 + 0xca) != 0) {
      *(int *)(param_1 + 0xe) = *(int *)(param_1 + 8) * 0x10;
      *(int *)(param_1 + 0xe) =
           *(int *)(*(int *)(param_1 + 200) + 0x94) + *(int *)(param_1 + 8) * 0x10;
    }
    goto switchD_00494acc_caseD_28;
  case 0x58:
    if (*(uint *)(param_1 + 0x26) < DAT_006b1254) {
      iVar4 = *(uint *)(param_1 + 0x26) * 0x20 + DAT_006b1250;
    }
    else {
      iVar4 = 0;
    }
    iVar4 = *(int *)(iVar4 + 0x18) * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438);
    *(undefined4 *)(param_1 + 0x10f) = *(undefined4 *)(iVar4 + 8);
    FUN_00464ca0();
    iVar4 = *(int *)(iVar4 + 0xc);
    goto LAB_00496eec;
  }
LAB_00496eba:
  *(int *)(param_1 + 0x10f) = piVar11[2];
  FUN_00464ca0();
  iVar4 = *piVar11;
LAB_00496eec:
  FUN_00464cb0(iVar4,0);
switchD_00494acc_caseD_f:
  return;
switchD_00494acc_caseD_28:
  iVar4 = *(int *)(param_1 + 0x2c) << 4;
  iVar7 = *(int *)(*(int *)(param_1 + 0x12) + 0x438);
LAB_00496eb8:
  piVar11 = (int *)(iVar4 + iVar7);
  goto LAB_00496eba;
switchD_00494acc_caseD_1:
  FUN_0048a950(*(undefined4 *)(param_1 + 0x16),*(undefined4 *)(param_1 + 0x18),
               *(undefined4 *)(param_1 + 0x1a),*(undefined4 *)(param_1 + 0x1c),
               *(undefined4 *)(param_1 + 0x1e),*(undefined4 *)(param_1 + 0x20));
  iVar4 = FUN_00489200();
  puVar10 = (undefined4 *)(iVar4 * 0x10 + *(int *)(*(int *)(param_1 + 0x12) + 0x438));
  *(undefined4 *)(param_1 + 0x10f) = puVar10[2];
  FUN_00464ca0();
  FUN_00464cb0(*puVar10,0);
  *(undefined4 *)(param_1 + 0xe9) =
       *(undefined4 *)(&DAT_006b13c9 + *(int *)(param_1 + 0x26) * 0x2d1);
  return;
switchD_00494acc_caseD_17:
  if (*(int *)(param_1 + 0x111) == 1) {
    return;
  }
  *(undefined4 *)(param_1 + 0x111) = 1;
  *(undefined4 *)(param_1 + 0x113) = 0;
  return;
}

