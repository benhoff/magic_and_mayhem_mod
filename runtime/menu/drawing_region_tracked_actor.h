/* Optional diagnostic identity/fields for one requested pool slot. Never writes
 * original state; no allocation generation or complete selection claim. */
static u32 region_input_track_enabled,region_input_track_slot;
static int region_input_track_configure(void){
 region_input_track_enabled=0;region_input_track_slot=0;
 char value[4];SetLastError(0);u32 n=GetEnvironmentVariableA("MNM_DRAW_REGION_TRACK_SLOT",value,sizeof(value));
 if(!n)return GetLastError()==203;
 if(n>=sizeof(value))return 0;
 u32 slot=0;for(u32 i=0;i<n;++i){if(value[i]<'0'||value[i]>'9')return 0;slot=slot*10+(u32)(value[i]-'0');}
 if(slot>255)return 0;region_input_track_slot=slot;region_input_track_enabled=1;return 1;
}
static void region_input_track_snapshot(u32* row){
 if(!region_input_track_enabled)return;row[45]=4;row[55]=region_input_track_slot;
 if(!readable((void*)MNM_REGION_INPUT_POOL,8))return;
 u32 base=get((void*)MNM_REGION_INPUT_POOL),capacity=get((void*)(MNM_REGION_INPUT_POOL+4));
 if(!base||!capacity||capacity>256||region_input_track_slot>=capacity||base>~0u-region_input_track_slot*0xe4bu)return;
 if(base!=region_input_pool_base){region_input_pool_base=base;++region_input_pool_id;}
 row[45]|=1;row[46]=region_input_pool_id;row[47]=capacity;
 u32 actor=base+region_input_track_slot*0xe4bu;
 if(!readable((void*)actor,0x178)||get((void*)actor)!=region_input_track_slot)return;
 row[45]|=2;row[48]=region_input_track_slot+1;row[49]=get((void*)(actor+0xa8));row[50]=get((void*)(actor+0x174));row[51]=get((void*)(actor+4));row[52]=get((void*)(actor+0xe4));row[53]=get((void*)(actor+0x68));row[54]=get((void*)(actor+0x70));
}
