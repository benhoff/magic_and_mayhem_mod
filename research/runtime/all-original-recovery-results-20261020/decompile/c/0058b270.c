
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */

bool FUN_0058b270(void)

{
  HMODULE hModule;
  
  if (DAT_006f98bc != 0) {
    return DAT_006f98b0 != (FARPROC)0x0;
  }
  hModule = GetModuleHandleA(s_USER32_005f1a5c);
  if (hModule != (HMODULE)0x0) {
    DAT_006f98a0 = GetProcAddress(hModule,s_GetSystemMetrics_005f1a48);
    if (DAT_006f98a0 != (FARPROC)0x0) {
      _DAT_006f98a4 = GetProcAddress(hModule,s_MonitorFromWindow_005f1a34);
      if (_DAT_006f98a4 != (FARPROC)0x0) {
        _DAT_006f98a8 = GetProcAddress(hModule,s_MonitorFromRect_005f1a24);
        if (_DAT_006f98a8 != (FARPROC)0x0) {
          _DAT_006f98ac = GetProcAddress(hModule,s_MonitorFromPoint_005f1a10);
          if (_DAT_006f98ac != (FARPROC)0x0) {
            _DAT_006f98b4 = GetProcAddress(hModule,s_EnumDisplayMonitors_005f19fc);
            if (_DAT_006f98b4 != (FARPROC)0x0) {
              DAT_006f98b0 = GetProcAddress(hModule,s_GetMonitorInfoA_005f19ec);
              if (DAT_006f98b0 != (FARPROC)0x0) {
                _DAT_006f98b8 = GetProcAddress(hModule,s_EnumDisplayDevicesA_005f19d8);
                if (_DAT_006f98b8 != (FARPROC)0x0) {
                  DAT_006f98bc = 1;
                  return true;
                }
              }
            }
          }
        }
      }
    }
  }
  DAT_006f98a0 = (FARPROC)0x0;
  _DAT_006f98a4 = (FARPROC)0x0;
  _DAT_006f98a8 = (FARPROC)0x0;
  _DAT_006f98ac = (FARPROC)0x0;
  DAT_006f98b0 = (FARPROC)0x0;
  _DAT_006f98b4 = (FARPROC)0x0;
  _DAT_006f98b8 = (FARPROC)0x0;
  DAT_006f98bc = 1;
  return false;
}

