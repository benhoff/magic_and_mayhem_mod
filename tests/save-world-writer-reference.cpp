// Isolated original serializer execution. Python patches only a disposable PE copy.
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <sys/mman.h>

using Bytes = std::vector<unsigned char>;
static Bytes read(const char* path) {
    std::ifstream stream(path, std::ios::binary);
    if (!stream) throw std::runtime_error("input open failed");
    return Bytes(std::istreambuf_iterator<char>(stream), {});
}
static std::uint32_t u32(const Bytes& b, std::size_t at) {
    if (at + 4 > b.size()) throw std::runtime_error("input extent");
    std::uint32_t n; std::memcpy(&n, b.data() + at, 4); return n;
}

static Bytes captured;
extern "C" unsigned capture_write(const void* data,unsigned size,unsigned count,void*) {
    const std::uint64_t length=std::uint64_t(size)*count;
    if(length>1024*1024-captured.size()) return 0;
    const auto* begin=static_cast<const unsigned char*>(data);
    captured.insert(captured.end(),begin,begin+length);return count;
}
static void put(Bytes& b,std::size_t offset,std::uint32_t n) {std::memcpy(b.data()+offset,&n,4);}
static void map_image(const Bytes& b) {
    const auto pe = u32(b, 0x3c), opt = pe + 24;
    if (u32(b, opt + 28) != 0x400000) throw std::runtime_error("image base");
    const auto length = u32(b, opt + 56);
    auto* memory = static_cast<unsigned char*>(mmap(reinterpret_cast<void*>(0x400000), length,
        PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0));
    if (memory == MAP_FAILED) throw std::runtime_error("PE map failed");
    const auto count = static_cast<unsigned>(b.at(pe + 6) | (b.at(pe + 7) << 8));
    const auto opt_length = static_cast<unsigned>(b.at(pe + 20) | (b.at(pe + 21) << 8));
    for (unsigned i = 0; i < count; ++i) {
        const auto section = opt + opt_length + i * 40;
        const auto size = u32(b, section + 16), offset = u32(b, section + 20), rva = u32(b, section + 12);
        if (offset + size > b.size() || rva + size > length) throw std::runtime_error("PE section");
        std::memcpy(memory + rva, b.data() + offset, size);
    }
    if (*reinterpret_cast<unsigned char*>(0x59c24f) != 0xe9)
        throw std::runtime_error("missing capture shim");
}

int main(int argc,char** argv) try {
    if(argc!=5) throw std::runtime_error("expected PE, address, case, output");
    map_image(read(argv[1]));
    const auto address=std::stoul(argv[2],nullptr,16);
    const auto mode=std::stoul(argv[3]);
    Bytes object(8192,0);std::uint32_t type=31;
    if(address==0x524df0) {
        put(object,0,42);put(object,4,mode!=0);
        if(mode==2) type=24;
        put(object,0xa8,type);put(object,0xac,reinterpret_cast<std::uintptr_t>(&type));
        if(mode==2) {
            put(object,0xe4,1);put(object,499*4,1);
            put(object,0x855,1);put(object,0x889,1);put(object,0x8bd,1);put(object,0xd03,1);
            put(object,0xc73,2);put(object,0xcef,1); // Counts in nested word lists (base 0xbf3).
        }
    } else if(address==0x49aa90) {put(object,0,42);put(object,4,mode!=0);put(object,0x168,mode==2);
        *reinterpret_cast<std::uint32_t*>(0x5fed74)=0;}
    else if(address==0x544020) {put(object,0,42);put(object,4,mode!=0);put(object,0x2a,mode==2?50:mode==3?61:0);}
    else throw std::runtime_error("unsupported writer");
    using Writer=int (__attribute__((thiscall)) *)(void*,void*);
    const auto accounting=reinterpret_cast<Writer>(address)(object.data(),nullptr);
    if(accounting<0) throw std::runtime_error("original writer failure");
    std::ofstream output(argv[4],std::ios::binary);
    output.write(reinterpret_cast<const char*>(captured.data()),captured.size());
    if(!output) throw std::runtime_error("capture write failure");
    std::cout<<"{\"physical_bytes\":"<<captured.size()<<",\"accounting_value\":"<<accounting<<"}\n";
    return 0;
} catch(const std::exception& e) {std::cerr<<e.what()<<'\n';return 1;}
