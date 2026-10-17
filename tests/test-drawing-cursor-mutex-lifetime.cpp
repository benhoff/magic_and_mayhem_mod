#include "../runtime/cursor-mutex/diagnostic_lifetime.h"
#include <cassert>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
static void* handle(unsigned value) {return reinterpret_cast<void*>(uintptr_t(value));}
int main() {
    // Retirement while active cannot close; reservations against retired/reused
    // identities refuse. Returning each level is required before close claim.
    for (unsigned count=0;count<33;++count) {
        MnmCursorLifetime cell{};
        assert(mnm_cursor_lifetime_publish(&cell,handle(1),handle(2),7));
        assert(!mnm_cursor_lifetime_publish(&cell,handle(3),handle(4),8));
        assert(!mnm_cursor_lifetime_reserve(&cell,handle(3),7));
        assert(!mnm_cursor_lifetime_reserve(&cell,handle(1),8));
        assert(!mnm_cursor_lifetime_close_begin(&cell));
        for(unsigned i=0;i<count;++i)assert(mnm_cursor_lifetime_reserve(&cell,handle(1),7));
        assert(!mnm_cursor_lifetime_retire(&cell,handle(1),8));
        assert(mnm_cursor_lifetime_retire(&cell,handle(1),7));
        assert(!mnm_cursor_lifetime_reserve(&cell,handle(1),7));
        assert(!mnm_cursor_lifetime_return(&cell,8));
        for(unsigned i=count;i>0;--i) {
            assert(!mnm_cursor_lifetime_close_begin(&cell));
            assert(mnm_cursor_lifetime_return(&cell,7));
        }
        assert(!mnm_cursor_lifetime_return(&cell,7));
        assert(mnm_cursor_lifetime_close_begin(&cell)==handle(2));
        assert(!mnm_cursor_lifetime_close_begin(&cell));
        assert(!mnm_cursor_lifetime_close_end(&cell,handle(2),8,1));
        assert(!mnm_cursor_lifetime_close_end(&cell,handle(3),7,1));
        assert(mnm_cursor_lifetime_close_end(&cell,handle(2),7,0));
        assert(cell.duplicate==handle(2)&&cell.retired&&!cell.closing);
        assert(!mnm_cursor_lifetime_publish(&cell,handle(1),handle(3),8));
        assert(mnm_cursor_lifetime_close_begin(&cell)==handle(2));
        assert(mnm_cursor_lifetime_close_end(&cell,handle(2),7,1));
        assert(!cell.duplicate&&!cell.original&&!cell.identity);
        assert(mnm_cursor_lifetime_publish(&cell,handle(1),handle(3),8));
        assert(!mnm_cursor_lifetime_reserve(&cell,handle(1),7));
        assert(mnm_cursor_lifetime_reserve(&cell,handle(1),8));
    }
    // Two identities retire independently; closing one cannot retire the other.
    MnmCursorLifetime outer{},inner{};
    assert(mnm_cursor_lifetime_publish(&outer,handle(1),handle(2),1));
    assert(mnm_cursor_lifetime_publish(&inner,handle(3),handle(4),2));
    assert(mnm_cursor_lifetime_reserve(&outer,handle(1),1));
    assert(mnm_cursor_lifetime_reserve(&inner,handle(3),2));
    assert(mnm_cursor_lifetime_retire(&outer,handle(1),1));
    assert(mnm_cursor_lifetime_reserve(&inner,handle(3),2));
    assert(!mnm_cursor_lifetime_reserve(&outer,handle(1),1));
    assert(mnm_cursor_lifetime_return(&outer,1));
    assert(mnm_cursor_lifetime_close_begin(&outer)==handle(2));
    assert(mnm_cursor_lifetime_close_end(&outer,handle(2),1,1));
    assert(inner.leases==2&&!inner.retired&&inner.duplicate==handle(4));
    // Real host threads exercise caller serialization without OS handle calls.
    MnmCursorLifetime shared{};std::mutex lock;
    assert(mnm_cursor_lifetime_publish(&shared,handle(10),handle(11),10));
    assert(mnm_cursor_lifetime_reserve(&shared,handle(10),10));
    std::vector<std::thread> workers;
    for(unsigned n=0;n<8;++n)workers.emplace_back([&] {
        for(unsigned i=0;i<1000;++i) {
            bool reserved;
            {std::lock_guard<std::mutex> held(lock);reserved=mnm_cursor_lifetime_reserve(&shared,handle(10),10);if(!reserved)assert(shared.retired);}
            std::this_thread::yield();
            if(reserved){std::lock_guard<std::mutex> held(lock);assert(mnm_cursor_lifetime_return(&shared,10));}
        }
    });
    {std::lock_guard<std::mutex> held(lock);assert(mnm_cursor_lifetime_retire(&shared,handle(10),10));assert(!mnm_cursor_lifetime_close_begin(&shared));assert(mnm_cursor_lifetime_return(&shared,10));}
    for(auto& thread:workers)thread.join();
    assert(shared.leases==0&&shared.retired);
    assert(mnm_cursor_lifetime_close_begin(&shared)==handle(11));
    assert(mnm_cursor_lifetime_close_end(&shared,handle(11),10,1));
    MnmCursorLifetime cell{};
    assert(!mnm_cursor_lifetime_publish(&cell,nullptr,handle(2),1));
    assert(!mnm_cursor_lifetime_publish(&cell,handle(1),nullptr,1));
    assert(!mnm_cursor_lifetime_publish(&cell,handle(1),handle(1),1));
    assert(!mnm_cursor_lifetime_publish(&cell,handle(1),handle(2),0));
    assert(mnm_cursor_lifetime_publish(&cell,handle(1),handle(2),1));
    assert(!mnm_cursor_lifetime_retire(&cell,handle(3),1));
    assert(!mnm_cursor_lifetime_close_end(&cell,handle(2),1,1));
    cell.leases=~0u;
    assert(!mnm_cursor_lifetime_reserve(&cell,handle(1),1));
}
