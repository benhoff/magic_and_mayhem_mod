/* Tiny Linux i386 host for the address-space fixture on ARM/FEX machines.
 * No libc, game installation, Wine prefix or compiler sysroot is required. */
typedef __SIZE_TYPE__ size_t;
static void output(const char* p,unsigned n) {
    __asm__ volatile("int $0x80"::"a"(4),"b"(1),"c"(p),"d"(n):"memory");
}
static void finish(unsigned code) __attribute__((noreturn));
static void finish(unsigned code) {
    __asm__ volatile("int $0x80"::"a"(1),"b"(code):"memory");
    __builtin_unreachable();
}
static void number(unsigned n) {
    char b[16];unsigned i=sizeof(b);do{b[--i]=(char)('0'+n%10);n/=10;}while(n);
    output(b+i,sizeof(b)-i);
}
static void failed(unsigned line) {output("assertion at line ",18);number(line);output("\n",1);finish(1);}
#define assert(v) do{if(!(v))failed(__LINE__);}while(0)
void* memcpy(void* out,const void* in,size_t n) {
    unsigned char* d=out;const unsigned char* s=in;for(size_t i=0;i<n;++i)d[i]=s[i];return out;
}
void* memset(void* out,int value,size_t n) {
    unsigned char* d=out;for(size_t i=0;i<n;++i)d[i]=(unsigned char)value;return out;
}
int memcmp(const void* a,const void* b,size_t n) {
    const unsigned char* x=a;const unsigned char* y=b;
    for(size_t i=0;i<n;++i)if(x[i]!=y[i])return x[i]-y[i];return 0;
}
static int strcmp(const char* a,const char* b) {
    while(*a&&*a==*b){++a;++b;}return (unsigned char)*a-(unsigned char)*b;
}
static int printf(const char* format,unsigned n) {
    const char* p=format;while(*p){if(p[0]=='%'&&p[1]=='u'){number(n);p+=2;}else output(p++,1);}return 0;
}
#define PROT_READ 1
#define PROT_WRITE 2
#define PROT_EXEC 4
#define MAP_PRIVATE 2
#define MAP_ANONYMOUS 32
#define MAP_FIXED_NOREPLACE 0x100000
static void* mmap(void* p,unsigned n,unsigned prot,unsigned flags,int fd,unsigned offset) {
    unsigned args[]={(unsigned)p,n,prot,flags,(unsigned)fd,offset};unsigned result;
    __asm__ volatile("int $0x80":"=a"(result):"a"(90),"b"(args):"memory");return (void*)result;
}
int main(void);
void _start(void) {finish((unsigned)main());}
