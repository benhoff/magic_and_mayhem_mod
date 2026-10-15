/* Native policy. menu_header/menu_host are nonblocking seqlock readers. */
enum { MENU_ADMISSION_OK=1, MENU_ADMISSION_BUSY=2, MENU_ADMISSION_HEADER=3,
       MENU_ADMISSION_DEAD=4, MENU_ADMISSION_REQUEST=5,
       MENU_ADMISSION_OWNED=6, MENU_ADMISSION_TIMEOUT=7 };
#define MENU_ADMISSION_LIMIT_MS 120000u
static u32 menu_admission_check(u32* host,u32 elapsed){
    if(!menu_header())return MENU_ADMISSION_HEADER;
    if(elapsed>=MENU_ADMISSION_LIMIT_MS)return MENU_ADMISSION_TIMEOUT;
    if(!menu_host(host))return MENU_ADMISSION_BUSY;
    if(!host[0])return MENU_ADMISSION_DEAD;
    if(host[2])return MENU_ADMISSION_REQUEST;
    if(menu_load(menu_words+(menu_version>=2?32:16)))return MENU_ADMISSION_OWNED;
    return MENU_ADMISSION_OK;
}
