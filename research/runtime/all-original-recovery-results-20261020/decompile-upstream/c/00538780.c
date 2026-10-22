
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

void __fastcall FUN_00538780(int param_1)

{
  char cVar1;
  int *piVar2;
  int iVar3;
  undefined4 uVar4;
  undefined4 *puVar5;
  int iVar6;
  uint uVar7;
  uint uVar8;
  uint uVar9;
  int unaff_ESI;
  undefined4 *puVar10;
  char *pcVar11;
  int iVar12;
  undefined4 *puVar13;
  char *pcVar14;
  undefined4 local_b28;
  undefined1 auStack_b24 [4];
  undefined4 auStack_b20 [3];
  undefined4 uStack_b14;
  undefined1 auStack_b10 [4];
  char acStack_b0c [256];
  undefined1 auStack_a0c [8];
  char *pcStack_a04;
  undefined1 auStack_90c [256];
  undefined1 auStack_80c [256];
  undefined1 auStack_70c [256];
  undefined1 auStack_60c [256];
  undefined1 auStack_50c [8];
  char *pcStack_504;
  undefined1 auStack_40c [256];
  undefined1 auStack_30c [256];
  undefined1 auStack_20c [256];
  undefined1 auStack_10c [268];
  
  iVar3 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 8) + 0x44))
                    ((int *)**(undefined4 **)(param_1 + 8),*(undefined4 *)(param_1 + 0xc),&local_b28
                    );
  if (iVar3 != 0) {
    uVar4 = FUN_00534d10();
    FUN_0059c1fd(auStack_a0c,s__s_directplay_error_code____s_005eaba0,
                 s_failed_to_get_message_count_005ead3c,uVar4);
    unaff_ESI = 0;
  }
  if (0 < unaff_ESI) {
    while( true ) {
      unaff_ESI = unaff_ESI + -1;
      local_b28 = *(undefined4 *)(param_1 + 0xc);
      iVar3 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 8) + 100))
                        ((int *)**(undefined4 **)(param_1 + 8),auStack_b24,&local_b28,2,
                         &DAT_006e9d78,&stack0xfffff4d4);
      if (iVar3 == -0x7788ff42) break;
      if (iVar3 != 0) {
        uVar4 = FUN_00534d10();
        FUN_0059c1fd(auStack_60c,s__s_directplay_error_code____s_005eaba0,
                     s_client_message_receive_error_005eafc0,uVar4);
        return;
      }
      if (DAT_006e9d78 == 0) {
        switch(_DAT_006e9d7c) {
        case 0:
          puVar5 = *(undefined4 **)(param_1 + 0x24);
          iVar3 = 0;
          if (0 < *(int *)(param_1 + 0x28)) {
            do {
              if (puVar5[4] == DAT_006e9d84) {
                if (puVar5 != (undefined4 *)0x0) {
                  iVar3 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 8) + 0x68))
                                    ((int *)**(undefined4 **)(param_1 + 8),
                                     *(undefined4 *)(param_1 + 0xc),1,0,*puVar5,puVar5[3]);
                  if (iVar3 != 0) {
                    uVar4 = FUN_00534d10();
                    FUN_0059c1fd(auStack_10c,s__s_directplay_error_code____s_005eaba0,
                                 s_failed_to_send_client_frame_mess_005eaf10,uVar4);
                  }
                  goto switchD_0053885e_caseD_3;
                }
                break;
              }
              iVar3 = iVar3 + 1;
              puVar5 = (undefined4 *)((int)puVar5 + 0x15);
            } while (iVar3 < *(int *)(param_1 + 0x28));
          }
          if ((int)DAT_006e9d84 < *(int *)(param_1 + 0x3c)) {
            FUN_0059c1fd(auStack_90c,s_client_frame___d_resend_frame____005eae80,
                         *(int *)(param_1 + 0x3c),DAT_006e9d84);
            FUN_0049e6a0();
          }
          break;
        case 1:
          iVar3 = *(int *)(param_1 + 0x40);
          if ((int)DAT_006e9d84 < iVar3) {
            puVar5 = *(undefined4 **)(param_1 + 0x10);
            iVar3 = 0;
            if (0 < *(int *)(param_1 + 0x14)) {
              do {
                if (puVar5[4] == DAT_006e9d84) {
                  if ((puVar5 != (undefined4 *)0x0) && (*(char *)(puVar5 + 5) == '\0')) {
                    *(undefined1 *)(puVar5 + 5) = 1;
                    uVar8 = DAT_006e9d80;
                    puVar10 = &DAT_006e9d78;
                    puVar5 = (undefined4 *)*puVar5;
                    for (uVar9 = DAT_006e9d80 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
                      *puVar5 = *puVar10;
                      puVar10 = puVar10 + 1;
                      puVar5 = puVar5 + 1;
                    }
                    for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
                      *(undefined1 *)puVar5 = *(undefined1 *)puVar10;
                      puVar10 = (undefined4 *)((int)puVar10 + 1);
                      puVar5 = (undefined4 *)((int)puVar5 + 1);
                    }
                  }
                  break;
                }
                iVar3 = iVar3 + 1;
                puVar5 = (undefined4 *)((int)puVar5 + 0x15);
              } while (iVar3 < *(int *)(param_1 + 0x14));
            }
          }
          else if (iVar3 < (int)DAT_006e9d84) {
            iVar3 = DAT_006e9d84 - iVar3;
            iVar12 = 0;
            if (0 < iVar3) {
              do {
                uVar8 = *(uint *)(param_1 + 0x20);
                if (uVar8 == *(uint *)(param_1 + 0x14)) break;
                if (uVar8 != *(uint *)(param_1 + 0x14)) {
                  uVar9 = *(int *)(param_1 + 0x1c) + 1;
                  *(uint *)(param_1 + 0x1c) = -(uint)(uVar9 != *(uint *)(param_1 + 0x14)) & uVar9;
                  *(uint *)(param_1 + 0x20) = uVar8 + 1;
                }
                if (*(int *)(param_1 + 0x20) == 0) {
                  iVar6 = 0;
                }
                else {
                  iVar6 = *(int *)(param_1 + 0x1c);
                  if (iVar6 == 0) {
                    iVar6 = *(int *)(param_1 + 0x14);
                  }
                  iVar6 = (iVar6 + -1) * 0x15 + *(int *)(param_1 + 0x10);
                }
                *(undefined1 *)(iVar6 + 0x14) = 0;
                *(undefined4 *)(iVar6 + 0x10) = *(undefined4 *)(param_1 + 0x40);
                uVar4 = *(undefined4 *)(param_1 + 0x40);
                auStack_b20[0] = 0;
                FUN_005396d0();
                uStack_b14 = uVar4;
                iVar6 = (**(code **)(*(int *)**(undefined4 **)(param_1 + 8) + 0x68))
                                  ((int *)**(undefined4 **)(param_1 + 8),
                                   *(undefined4 *)(param_1 + 0xc),1,0,auStack_b20,0x10);
                if (iVar6 != 0) {
                  uVar4 = FUN_00534d10();
                  FUN_0059c1fd(auStack_40c,s__s_directplay_error_code____s_005eaba0,
                               s_client_send__resend_message__req_005eaf70,uVar4);
                }
                iVar12 = iVar12 + 1;
                *(int *)(param_1 + 0x40) = *(int *)(param_1 + 0x40) + 1;
              } while (iVar12 < iVar3);
            }
            uVar8 = *(uint *)(param_1 + 0x20);
            uVar9 = *(uint *)(param_1 + 0x14);
            if (uVar8 != uVar9) {
              if (uVar8 != uVar9) {
                uVar7 = *(int *)(param_1 + 0x1c) + 1;
                *(uint *)(param_1 + 0x1c) = -(uint)(uVar7 != uVar9) & uVar7;
                *(uint *)(param_1 + 0x20) = uVar8 + 1;
              }
              if (*(int *)(param_1 + 0x20) == 0) {
                puVar5 = (undefined4 *)0x0;
              }
              else {
                iVar3 = *(int *)(param_1 + 0x1c);
                if (iVar3 == 0) {
                  iVar3 = *(int *)(param_1 + 0x14);
                }
                puVar5 = (undefined4 *)((iVar3 + -1) * 0x15 + *(int *)(param_1 + 0x10));
              }
              *(undefined1 *)(puVar5 + 5) = 1;
              uVar8 = DAT_006e9d80;
              puVar10 = &DAT_006e9d78;
              puVar13 = (undefined4 *)*puVar5;
              for (uVar9 = DAT_006e9d80 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
                *puVar13 = *puVar10;
                puVar10 = puVar10 + 1;
                puVar13 = puVar13 + 1;
              }
              for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
                *(undefined1 *)puVar13 = *(undefined1 *)puVar10;
                puVar10 = (undefined4 *)((int)puVar10 + 1);
                puVar13 = (undefined4 *)((int)puVar13 + 1);
              }
              puVar5[4] = *(undefined4 *)(param_1 + 0x40);
              *(int *)(param_1 + 0x40) = *(int *)(param_1 + 0x40) + 1;
            }
          }
          else {
            uVar8 = *(uint *)(param_1 + 0x20);
            uVar9 = *(uint *)(param_1 + 0x14);
            if (uVar8 != uVar9) {
              if (uVar8 != uVar9) {
                uVar7 = *(int *)(param_1 + 0x1c) + 1;
                *(uint *)(param_1 + 0x1c) = -(uint)(uVar7 != uVar9) & uVar7;
                *(uint *)(param_1 + 0x20) = uVar8 + 1;
              }
              if (*(int *)(param_1 + 0x20) == 0) {
                puVar5 = (undefined4 *)0x0;
              }
              else {
                iVar3 = *(int *)(param_1 + 0x1c);
                if (iVar3 == 0) {
                  iVar3 = *(int *)(param_1 + 0x14);
                }
                puVar5 = (undefined4 *)((iVar3 + -1) * 0x15 + *(int *)(param_1 + 0x10));
              }
              *(undefined1 *)(puVar5 + 5) = 1;
              uVar8 = DAT_006e9d80;
              puVar10 = &DAT_006e9d78;
              puVar13 = (undefined4 *)*puVar5;
              for (uVar9 = DAT_006e9d80 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
                *puVar13 = *puVar10;
                puVar10 = puVar10 + 1;
                puVar13 = puVar13 + 1;
              }
              for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
                *(undefined1 *)puVar13 = *(undefined1 *)puVar10;
                puVar10 = (undefined4 *)((int)puVar10 + 1);
                puVar13 = (undefined4 *)((int)puVar13 + 1);
              }
              puVar5[4] = *(undefined4 *)(param_1 + 0x40);
              *(int *)(param_1 + 0x40) = *(int *)(param_1 + 0x40) + 1;
            }
          }
          break;
        case 2:
          if (DAT_006de6b4 == 1) {
            FUN_00487470(&DAT_006e9d78);
          }
          *(undefined4 *)(param_1 + 0x44) = 1;
          break;
        case 0x10:
          if (*(int *)(*(int *)(param_1 + 8) + 0x134) == 1) {
            FUN_00487370(&DAT_006e9d84);
          }
          break;
        case 0x11:
          if (*(int *)(*(int *)(param_1 + 8) + 0x134) == 1) {
            FUN_00487350(DAT_006e9eb7,&DAT_006e9d84);
          }
          break;
        case 0x12:
          if (*(int *)(*(int *)(param_1 + 8) + 0x134) == 2) {
            FUN_005925c0(&DAT_006e9d8c,DAT_006e9d88);
            FUN_0053a160();
          }
          break;
        case 0x17:
          if (*(int *)(*(int *)(param_1 + 8) + 0x134) == 1) {
            FUN_004873f0(DAT_006e9d84);
          }
          break;
        case 0x18:
          FUN_00487490(DAT_006e9d84 & 0xff);
        }
      }
      else if (DAT_006e9d78 < 0x32) {
        if (DAT_006e9d78 == 0x31) {
          FUN_0058a680(0x10);
          FUN_0058a680(0x11);
          FUN_0049e6a0();
        }
        else if (DAT_006e9d78 == 3) {
          if (*(int *)(*(int *)(param_1 + 8) + 300) != 0) {
            FUN_00536680();
          }
        }
        else if (DAT_006e9d78 == 5) {
          if (*(int *)(*(int *)(param_1 + 8) + 300) != 0) {
            FUN_00536680();
          }
          iVar3 = DAT_006e9d9c;
          if ((DAT_006de6b4 == 3) && (DAT_006e9d9c != 0)) {
            FUN_004dd040(DAT_006e9d9c);
            uVar4 = FUN_0058a680(1);
            FUN_0059c1fd(auStack_70c,s__s__s_005e0fc8,iVar3,uVar4);
            FUN_004da770(auStack_70c);
          }
        }
      }
      else if (DAT_006e9d78 == 0x101) {
        if (*(int *)(*(int *)(param_1 + 8) + 0x134) == 3) {
          DAT_006de6ac = 1;
          iVar3 = (**(code **)(*DAT_006de580 + 0x18))(DAT_006de580,auStack_b10,0,0,0,0,0x100);
          if (iVar3 != 0) {
            uVar4 = FUN_00534d10();
            FUN_0059c1fd(auStack_60c,s__s_directplay_error_code____s_005eaba0,
                         s_could_not_create_host_player_005ead1c,uVar4);
          }
          FUN_00536680();
          FUN_00472960();
          _DAT_006a29c4 = *(int *)(param_1 + 0x38) - DAT_006ead94;
          _DAT_006a29c4 = _DAT_006a29c4 & ((int)_DAT_006a29c4 < 1) - 1;
          _DAT_006a29c8 = _DAT_006a29c4;
          _DAT_006a29cc = _DAT_006a29c4;
          _DAT_006a29d0 = _DAT_006a29c4;
          _DAT_006a29d4 = _DAT_006a29c4;
          FUN_00537780();
        }
        else {
          FUN_00487490(0);
        }
      }
      else if (DAT_006e9d78 == 0x109) {
        if (DAT_006de6b4 == 3) {
          piVar2 = (int *)**(undefined4 **)(param_1 + 8);
          iVar3 = (**(code **)(*piVar2 + 0x54))(piVar2,DAT_006e9d80,auStack_50c,&stack0xfffff4d0);
          pcVar11 = pcStack_504;
          if (iVar3 != 0) {
            uVar4 = FUN_00534d10();
            FUN_0059c1fd(auStack_20c,s__s_directplay_error_code____s_005eaba0,
                         s_could_not_get_player_name_005ead58,uVar4);
            pcVar11 = &DAT_005faf28;
          }
          uVar8 = 0xffffffff;
          do {
            pcVar14 = pcVar11;
            if (uVar8 == 0) break;
            uVar8 = uVar8 - 1;
            pcVar14 = pcVar11 + 1;
            cVar1 = *pcVar11;
            pcVar11 = pcVar14;
          } while (cVar1 != '\0');
          uVar8 = ~uVar8;
          pcVar11 = pcVar14 + -uVar8;
          pcVar14 = acStack_b0c;
          for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
            *(undefined4 *)pcVar14 = *(undefined4 *)pcVar11;
            pcVar11 = pcVar11 + 4;
            pcVar14 = pcVar14 + 4;
          }
          for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
            *pcVar14 = *pcVar11;
            pcVar11 = pcVar11 + 1;
            pcVar14 = pcVar14 + 1;
          }
          FUN_0059c1fd(auStack_80c,s___s____s_005eafe0,acStack_b0c,*(undefined4 *)(DAT_006e9d8c + 8)
                      );
          FUN_004da770(auStack_80c);
        }
        else {
          piVar2 = (int *)**(undefined4 **)(param_1 + 8);
          iVar3 = (**(code **)(*piVar2 + 0x54))(piVar2,DAT_006e9d80,auStack_a0c,&stack0xfffff4d0);
          pcVar11 = pcStack_a04;
          if (iVar3 != 0) {
            uVar4 = FUN_00534d10();
            FUN_0059c1fd(auStack_30c,s__s_directplay_error_code____s_005eaba0,
                         s_could_not_get_player_name_005ead58,uVar4);
            pcVar11 = &DAT_005faf28;
          }
          uVar8 = 0xffffffff;
          do {
            pcVar14 = pcVar11;
            if (uVar8 == 0) break;
            uVar8 = uVar8 - 1;
            pcVar14 = pcVar11 + 1;
            cVar1 = *pcVar11;
            pcVar11 = pcVar14;
          } while (cVar1 != '\0');
          uVar8 = ~uVar8;
          pcVar11 = pcVar14 + -uVar8;
          pcVar14 = acStack_b0c;
          for (uVar9 = uVar8 >> 2; uVar9 != 0; uVar9 = uVar9 - 1) {
            *(undefined4 *)pcVar14 = *(undefined4 *)pcVar11;
            pcVar11 = pcVar11 + 4;
            pcVar14 = pcVar14 + 4;
          }
          for (uVar8 = uVar8 & 3; uVar8 != 0; uVar8 = uVar8 - 1) {
            *pcVar14 = *pcVar11;
            pcVar11 = pcVar11 + 1;
            pcVar14 = pcVar14 + 1;
          }
          FUN_00487390(acStack_b0c,*(undefined4 *)(DAT_006e9d8c + 8));
        }
      }
switchD_0053885e_caseD_3:
      if (unaff_ESI < 1) {
        return;
      }
    }
  }
  return;
}

