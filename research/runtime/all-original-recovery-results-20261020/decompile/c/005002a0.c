
void __fastcall FUN_005002a0(int *param_1)

{
  undefined4 uVar1;
  short sVar2;
  undefined2 uVar3;
  short sVar4;
  uint uVar5;
  int iVar6;
  int iVar7;
  int iVar8;
  int iVar9;
  undefined4 *puVar10;
  undefined4 uVar11;
  undefined4 uVar12;
  uint local_18;
  int local_10;
  int local_c;
  
  iVar9 = *param_1;
  local_10 = -2000000000;
  local_c = -2000000000;
  iVar8 = param_1[2];
  if (iVar8 != 0) {
    local_18 = 0;
    if (DAT_005e1404 == 0) {
      if (iVar8 != 0) {
        puVar10 = (undefined4 *)(iVar9 + 0xc);
        do {
          switch(puVar10[3]) {
          case 0:
            iVar9 = puVar10[-2];
            uVar12 = *puVar10;
            sVar4 = *(short *)((int)puVar10 + 6);
            uVar11 = puVar10[-1];
            if (*(int *)(iVar9 + 0x1c) == -1) goto LAB_00501119;
            sVar2 = (*DAT_006c4e3c)(iVar9,uVar11,uVar12,0,(int)sVar4);
            if (sVar2 == 1) {
              sVar2 = FUN_0057de00(iVar9,uVar11,uVar12,0,(int)sVar4);
            }
            break;
          case 1:
            sVar2 = FUN_0057e540(*puVar10);
            break;
          case 2:
            sVar2 = FUN_0057e8e0(*puVar10,0,(int)*(short *)((int)puVar10 + 6));
            break;
          case 3:
            sVar2 = FUN_0057f5f0(*puVar10,0,(int)*(short *)((int)puVar10 + 6));
            break;
          case 4:
            sVar2 = FUN_0057ec90(*puVar10,0,(int)*(short *)((int)puVar10 + 6));
            break;
          case 5:
            sVar2 = FUN_0057f0f0(*puVar10,0,(int)*(short *)((int)puVar10 + 6));
            break;
          case 6:
            sVar2 = (*DAT_00689b78)(puVar10[-2],puVar10[-1],*puVar10);
            break;
          default:
            goto switchD_00500d69_caseD_7;
          case 8:
            sVar2 = (**(code **)(*(int *)puVar10[-2] + 0xc))(puVar10[-1],*puVar10);
            break;
          case 9:
            *(undefined2 *)((int)puVar10 + 0x12) = 0;
            FUN_004df840(*(undefined4 *)(puVar10[-2] + 8),*(undefined4 *)(puVar10[-2] + 0xc),
                         puVar10[-1],0,(int)*(short *)((int)puVar10 + 6));
            goto switchD_00500d69_caseD_7;
          case 10:
            sVar2 = FUN_00580a90(*puVar10,0,(int)*(short *)((int)puVar10 + 6));
            break;
          case 0xb:
            sVar2 = FUN_00580260(*puVar10,0,(int)*(short *)((int)puVar10 + 6));
            break;
          case 0xc:
            sVar2 = FUN_00581200(*puVar10,0,0xf4,0xc6,0xd);
            break;
          case 0xd:
            sVar2 = FUN_00581200(*puVar10,0,10,10,0xff);
            break;
          case 0xe:
            sVar2 = FUN_00581200(*puVar10,0,0xd6,0x5f,0xac);
            break;
          case 0xf:
            sVar2 = FUN_005806f0(*puVar10,1);
            break;
          case 0x10:
            sVar2 = FUN_005806f0(*puVar10,2);
            break;
          case 0x11:
            sVar2 = FUN_005806f0(*puVar10,3);
            break;
          case 0x12:
            sVar2 = FUN_005806f0(*puVar10,4);
            break;
          case 0x13:
            sVar2 = FUN_005806f0(*puVar10,5);
            break;
          case 0x14:
            sVar2 = FUN_005806f0(*puVar10,6);
            break;
          case 0x15:
            sVar2 = FUN_005806f0(*puVar10,7);
            break;
          case 0x16:
            sVar2 = FUN_005806f0(*puVar10,8);
            break;
          case 0x17:
            sVar2 = FUN_005806f0(*puVar10,9);
            break;
          case 0x18:
            sVar2 = FUN_005806f0(*puVar10,10);
            break;
          case 0x19:
            sVar2 = FUN_005806f0(*puVar10,0xb);
            break;
          case 0x1a:
            sVar2 = FUN_005806f0(*puVar10,0xc);
            break;
          case 0x1b:
            sVar2 = FUN_005806f0(*puVar10,0xd);
            break;
          case 0x1c:
            sVar2 = FUN_005806f0(*puVar10,0xe);
            break;
          case 0x1d:
            sVar2 = FUN_005806f0(*puVar10,0xf);
            break;
          case 0x1e:
            sVar2 = FUN_005806f0(*puVar10,0x10);
            break;
          case 0x1f:
            iVar9 = puVar10[-2];
            uVar12 = *puVar10;
            sVar4 = *(short *)((int)puVar10 + 6);
            uVar11 = puVar10[-1];
            if (*(int *)(iVar9 + 0x1c) == -1) goto LAB_00501119;
            sVar2 = (*DAT_006c4e3c)(iVar9,uVar11,uVar12,0,(int)sVar4);
            if (sVar2 == 1) {
              sVar2 = FUN_0057de00(iVar9,uVar11,uVar12,0,(int)sVar4);
            }
            break;
          case 0x21:
            iVar9 = puVar10[-2];
            uVar12 = *puVar10;
            sVar4 = *(short *)((int)puVar10 + 6);
            uVar11 = puVar10[-1];
            if (*(int *)(iVar9 + 0x1c) == -1) goto LAB_00501119;
            sVar2 = (*DAT_006c4e3c)(iVar9,uVar11,uVar12,0,(int)sVar4);
            if (sVar2 == 1) {
              sVar2 = FUN_0057de00(iVar9,uVar11,uVar12,0,(int)sVar4);
            }
            break;
          case 0x22:
            uVar12 = *puVar10;
            uVar11 = puVar10[-1];
            iVar9 = puVar10[-2];
LAB_00501119:
            sVar2 = (*DAT_006903dc)(iVar9,uVar11,uVar12);
          }
          *(short *)((int)puVar10 + 0x12) = sVar2;
switchD_00500d69_caseD_7:
          local_18 = local_18 + 1;
          puVar10 = puVar10 + 9;
        } while (local_18 < (uint)param_1[2]);
      }
    }
    else if (iVar8 != 0) {
      do {
        if (*(short *)(iVar9 + 0x1c) != -30000) {
          local_10 = (*(int *)(*(int *)(iVar9 + 4) + 8) - *(int *)(*(int *)(iVar9 + 4) + 0x10)) +
                     *(int *)(iVar9 + 0xc);
          uVar5 = local_10 + *(short *)(iVar9 + 0x1c);
          if (((int)uVar5 < *(int *)(DAT_006c482c + 0xc)) &&
             (*(int *)(DAT_006c482c + 4) < (int)uVar5)) {
            FUN_0057dc60(*(int *)(DAT_006c482c + 8) + DAT_006e1f70 * 2,uVar5 & ((int)uVar5 < 1) - 1)
            ;
          }
        }
        if (*(short *)(iVar9 + 0x1e) != -30000) {
          iVar8 = *(int *)(*(int *)(iVar9 + 4) + 8);
          local_c = (*(int *)(iVar9 + 0xc) - *(int *)(*(int *)(iVar9 + 4) + 0x10)) + iVar8;
          *(int *)(iVar9 + 0xc) = (*(int *)(iVar9 + 0xc) - (int)*(short *)(iVar9 + 0x1e)) + iVar8;
          if ((local_c < *(int *)(DAT_006c482c + 0xc)) && (*(int *)(DAT_006c482c + 4) < local_c)) {
            FUN_0057dc60(*(int *)(DAT_006c482c + 8) + DAT_006e1f70 * 2,local_c);
          }
        }
        if (*(int *)(iVar9 + 0x18) == 0x21) {
          sVar4 = *(short *)(iVar9 + 0x12);
          iVar8 = (int)*(short *)(iVar9 + 0x10);
          uVar12 = *(undefined4 *)(iVar9 + 8);
          uVar11 = *(undefined4 *)(iVar9 + 4);
          uVar1 = *(undefined4 *)(iVar9 + 0xc);
          if (iVar8 == -0x7f) {
            sVar2 = FUN_00595677(uVar11,uVar12,uVar1);
          }
          else {
            sVar2 = (*DAT_00689b7c)(uVar11,uVar12,uVar1,iVar8,sVar4 + 2);
          }
          if (sVar2 == 1) {
            sVar2 = FUN_0057de00(uVar11,uVar12,uVar1,iVar8,(int)sVar4);
          }
          goto LAB_00500848;
        }
        switch(*(int *)(iVar9 + 0x18)) {
        case 0:
          iVar8 = *(int *)(iVar9 + 4);
          uVar12 = *(undefined4 *)(iVar9 + 0xc);
          iVar7 = (int)*(short *)(iVar9 + 0x12);
          iVar6 = (int)*(short *)(iVar9 + 0x10);
          uVar11 = *(undefined4 *)(iVar9 + 8);
          if (*(int *)(iVar8 + 0x1c) == -1) goto LAB_0050083f;
          if (iVar6 == -0x7f) {
            sVar4 = FUN_00595677(iVar8,uVar11,uVar12);
            sVar2 = 0;
            if (sVar4 == 1) {
              sVar2 = FUN_0057de00(iVar8,uVar11,uVar12,0xffffff81,iVar7);
            }
          }
          else {
            sVar2 = (*DAT_006c4e3c)(iVar8,uVar11,uVar12,iVar6,iVar7);
            if (sVar2 == 1) {
              sVar2 = FUN_0057de00(iVar8,uVar11,uVar12,iVar6,iVar7);
            }
          }
          break;
        case 1:
          sVar2 = FUN_0057e540(*(undefined4 *)(iVar9 + 0xc));
          break;
        case 2:
          sVar2 = FUN_0057e8e0(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 3:
          sVar2 = FUN_0057f5f0(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 4:
          sVar2 = FUN_0057ec90(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 5:
          sVar2 = FUN_0057f0f0(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 6:
          sVar2 = (*DAT_00689b78)(*(undefined4 *)(iVar9 + 4),*(undefined4 *)(iVar9 + 8),
                                  *(undefined4 *)(iVar9 + 0xc));
          break;
        case 7:
          sVar2 = FUN_0057fe10(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 8:
          sVar2 = (**(code **)(**(int **)(iVar9 + 4) + 0xc))
                            (*(undefined4 *)(iVar9 + 8),*(undefined4 *)(iVar9 + 0xc));
          break;
        case 9:
          *(undefined2 *)(iVar9 + 0x1e) = 0;
          FUN_004df840(*(undefined4 *)(*(int *)(iVar9 + 4) + 8),
                       *(undefined4 *)(*(int *)(iVar9 + 4) + 0xc),*(undefined4 *)(iVar9 + 8),
                       (int)*(short *)(iVar9 + 0x10),(int)*(short *)(iVar9 + 0x12));
          goto switchD_005003fc_caseD_21;
        case 10:
          sVar2 = FUN_00580a90(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 0xb:
          sVar2 = FUN_00580260(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          break;
        case 0xc:
          sVar2 = FUN_00581200(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),0xf4,0xc6,
                               0xd);
          break;
        case 0xd:
          sVar2 = FUN_00581200(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),10,10,0xff
                              );
          break;
        case 0xe:
          sVar2 = FUN_00581200(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),0xd6,0x5f,
                               0xac);
          break;
        case 0xf:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),1);
          break;
        case 0x10:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),2);
          break;
        case 0x11:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),3);
          break;
        case 0x12:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),4);
          break;
        case 0x13:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),5);
          break;
        case 0x14:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),6);
          break;
        case 0x15:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),7);
          break;
        case 0x16:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),8);
          break;
        case 0x17:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),9);
          break;
        case 0x18:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),10);
          break;
        case 0x19:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xb);
          break;
        case 0x1a:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xc);
          break;
        case 0x1b:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xd);
          break;
        case 0x1c:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xe);
          break;
        case 0x1d:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xf);
          break;
        case 0x1e:
          sVar2 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0x10);
          break;
        case 0x1f:
          iVar8 = *(int *)(iVar9 + 4);
          uVar12 = *(undefined4 *)(iVar9 + 0xc);
          iVar7 = (int)*(short *)(iVar9 + 0x12);
          iVar6 = (int)*(short *)(iVar9 + 0x10);
          uVar11 = *(undefined4 *)(iVar9 + 8);
          if (*(int *)(iVar8 + 0x1c) == -1) goto LAB_0050083f;
          if (iVar6 == -0x7f) {
            sVar4 = FUN_00595677(iVar8,uVar11,uVar12);
            sVar2 = 0;
            if (sVar4 == 1) {
              sVar2 = FUN_0057de00(iVar8,uVar11,uVar12,0xffffff81,iVar7);
            }
          }
          else {
            sVar2 = (*DAT_006c4e3c)(iVar8,uVar11,uVar12,iVar6,iVar7);
            if (sVar2 == 1) {
              sVar2 = FUN_0057de00(iVar8,uVar11,uVar12,iVar6,iVar7);
            }
          }
          break;
        case 0x20:
        case 0x22:
          uVar12 = *(undefined4 *)(iVar9 + 0xc);
          uVar11 = *(undefined4 *)(iVar9 + 8);
          iVar8 = *(int *)(iVar9 + 4);
LAB_0050083f:
          sVar2 = (*DAT_006903dc)(iVar8,uVar11,uVar12);
          break;
        default:
          goto switchD_005003fc_caseD_21;
        }
LAB_00500848:
        *(short *)(iVar9 + 0x1e) = sVar2;
switchD_005003fc_caseD_21:
        if (local_10 == -2000000000) {
          if (local_c != -2000000000) goto switchD_0050093d_caseD_1;
          goto LAB_00500d32;
        }
        if (((*(short *)(iVar9 + 0x1c) == -30000) ||
            (local_10 = *(short *)(iVar9 + 0x1c) + local_10,
            (int)((-(uint)(DAT_006de6d5 != 0) & 0x78) + 0x1e0) <= local_10)) ||
           (*(int *)(DAT_006c482c + 0xc) <= local_10)) goto switchD_0050093d_caseD_1;
        FUN_0057dc60(*(int *)(DAT_006c482c + 8) + DAT_006e1f70 * 2,*(int *)(DAT_006c482c + 0xc));
        if (*(int *)(iVar9 + 0x18) == 0x21) {
          sVar4 = *(short *)(iVar9 + 0x12);
          iVar8 = (int)*(short *)(iVar9 + 0x10);
          uVar12 = *(undefined4 *)(iVar9 + 8);
          uVar11 = *(undefined4 *)(iVar9 + 4);
          uVar1 = *(undefined4 *)(iVar9 + 0xc);
          if (iVar8 == -0x7f) {
            sVar2 = FUN_00595677(uVar11,uVar12,uVar1);
          }
          else {
            sVar2 = (*DAT_00689b7c)(uVar11,uVar12,uVar1,iVar8,sVar4 + 2);
          }
          if (sVar2 != 1) goto LAB_005009d2;
          uVar3 = FUN_0057de00(uVar11,uVar12,uVar1,iVar8,(int)sVar4);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          goto switchD_0050093d_caseD_1;
        }
        switch(*(int *)(iVar9 + 0x18)) {
        case 0:
          iVar8 = *(int *)(iVar9 + 4);
          uVar12 = *(undefined4 *)(iVar9 + 0xc);
          iVar6 = (int)*(short *)(iVar9 + 0x10);
          uVar11 = *(undefined4 *)(iVar9 + 8);
          if (*(int *)(iVar8 + 0x1c) == -1) {
            uVar3 = (*DAT_006903dc)(iVar8,uVar11,uVar12);
            *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          }
          else {
            if (iVar6 == -0x7f) {
              sVar4 = FUN_00595677(iVar8,uVar11,uVar12);
              sVar2 = 0;
              if (sVar4 == 1) {
                uVar3 = FUN_0057de00(iVar8,uVar11,uVar12,0xffffff81,1);
                *(undefined2 *)(iVar9 + 0x1e) = uVar3;
                break;
              }
            }
            else {
              sVar2 = (*DAT_006c4e3c)(iVar8,uVar11,uVar12,iVar6,1);
              if (sVar2 == 1) {
                sVar2 = FUN_0057de00(iVar8,uVar11,uVar12,iVar6,1);
              }
            }
LAB_005009d2:
            *(short *)(iVar9 + 0x1e) = sVar2;
          }
          break;
        case 2:
          uVar3 = FUN_0057e8e0(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),1);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 3:
          uVar3 = FUN_0057f5f0(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),1);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 4:
          uVar3 = FUN_0057ec90(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),1);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 5:
          uVar3 = FUN_0057f0f0(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),1);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 10:
          uVar3 = FUN_00580a90(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0xb:
          uVar3 = FUN_00580260(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),
                               (int)*(short *)(iVar9 + 0x12));
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0xc:
          uVar3 = FUN_00581200(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),0xf4,0xc6,
                               0xd);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0xd:
          uVar3 = FUN_00581200(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),10,10,0xff
                              );
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0xe:
          uVar3 = FUN_00581200(*(undefined4 *)(iVar9 + 0xc),(int)*(short *)(iVar9 + 0x10),0xd6,0x5f,
                               0xac);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0xf:
          uVar12 = 1;
          goto LAB_00500bb0;
        case 0x10:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),2);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x11:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),3);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x12:
          uVar12 = 4;
          goto LAB_00500bb0;
        case 0x13:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),5);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x14:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),6);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x15:
          uVar12 = 7;
          goto LAB_00500bb0;
        case 0x16:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),8);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x17:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),9);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x18:
          uVar12 = 10;
          goto LAB_00500bb0;
        case 0x19:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xb);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x1a:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xc);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x1b:
          uVar12 = 0xd;
          goto LAB_00500bb0;
        case 0x1c:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xe);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x1d:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),0xf);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x1e:
          uVar12 = 0x10;
LAB_00500bb0:
          uVar3 = FUN_005806f0(*(undefined4 *)(iVar9 + 0xc),uVar12);
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          break;
        case 0x1f:
          iVar8 = *(int *)(iVar9 + 4);
          uVar12 = *(undefined4 *)(iVar9 + 0xc);
          iVar6 = (int)*(short *)(iVar9 + 0x10);
          uVar11 = *(undefined4 *)(iVar9 + 8);
          if (*(int *)(iVar8 + 0x1c) == -1) {
            uVar3 = (*DAT_006903dc)(iVar8,uVar11,uVar12);
            *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          }
          else if (iVar6 == -0x7f) {
            sVar4 = FUN_00595677(iVar8,uVar11,uVar12);
            sVar2 = 0;
            if (sVar4 != 1) goto LAB_005009d2;
            uVar3 = FUN_0057de00(iVar8,uVar11,uVar12,0xffffff81,1);
            *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          }
          else {
            sVar2 = (*DAT_006c4e3c)(iVar8,uVar11,uVar12,iVar6,1);
            if (sVar2 != 1) goto LAB_005009d2;
            uVar3 = FUN_0057de00(iVar8,uVar11,uVar12,iVar6,1);
            *(undefined2 *)(iVar9 + 0x1e) = uVar3;
          }
          break;
        case 0x20:
        case 0x22:
          uVar3 = (*DAT_006903dc)(*(undefined4 *)(iVar9 + 4),*(undefined4 *)(iVar9 + 8),
                                  *(undefined4 *)(iVar9 + 0xc));
          *(undefined2 *)(iVar9 + 0x1e) = uVar3;
        }
switchD_0050093d_caseD_1:
        FUN_0057dc60(*(int *)(DAT_006c482c + 8) + DAT_006e1f70 * 2,
                     *(undefined4 *)(DAT_006c482c + 0xc));
        local_10 = -2000000000;
        local_c = -2000000000;
LAB_00500d32:
        local_18 = local_18 + 1;
        iVar9 = iVar9 + 0x24;
      } while (local_18 < (uint)param_1[2]);
    }
  }
  if (DAT_006e1f24 != 0) {
    FUN_004d8ab0(DAT_006e8db0,DAT_006e8db4,DAT_006e8db8);
    DAT_006e8db0 = 0;
    DAT_006e8db4 = 0;
    DAT_006e8db8 = 0;
  }
  return;
}

