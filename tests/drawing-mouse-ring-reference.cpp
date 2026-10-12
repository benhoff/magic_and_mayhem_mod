// Execute unchanged original mouse ring writer in an owned private image.
#define main retry_reference_unused_main
#include "surface-retry-reference.cpp"
#undef main
#include <algorithm>

static U& mouse_global(U address) { return *reinterpret_cast<U*>(address); }
static U alt_result, key_calls;
static I __attribute__((stdcall)) mouse_key(U key) {
 if (key != 18) throw std::runtime_error("Unexpected original key poll");
 ++key_calls;
 return alt_result;
}
static void require_mouse(bool ok, const char* reason) {
 if (!ok) throw std::runtime_error(reason);
}

int main(int argc, char** argv) try {
 require_mouse(argc == 2, "Expected pinned PE");
 mapImage(argv[1]);
 const auto* writer = reinterpret_cast<const unsigned char*>(0x4a3800);
 const std::vector<unsigned char> original(writer, writer + 0x10d);
 mouse_global(0x5c5204) = reinterpret_cast<U>(&mouse_key);
 using Write = void(__attribute__((fastcall))*)(U, U, U);
 std::array<U, 320> before, expected;
 const std::array<U, 3> coordinates{{0, 400, 0xffff}};
 U cases = 0, collisions = 0, write_wraps = 0, current_wraps = 0, tail_wraps = 0;
 for (U write = 0; write < 32; ++write) {
  U per_write = 0;
  for (U current : {write, (write + 31) % 32})
  for (U flags = 0; flags < 16; ++flags)
  for (U previous = 0; previous < 3; ++previous)
  for (U xy : coordinates)
  for (U alt : {0u, 0x8000u, 0xffffu})
  for (U collision : {0u, 1u}) {
   for (U i = 0; i < before.size(); ++i) before[i] = 0x23450000u + i;
   const U last = (write + 31) % 32;
   before[last * 10] = xy;
   before[last * 10 + 1] = previous == 1 ? (xy ^ 1u) : xy;
   before[last * 10 + 3] = previous == 2 ? (flags & 1u) ^ 1u : flags & 1u;
   before[last * 10 + 4] = previous == 2 ? ((flags >> 1) & 1u) ^ 1u : (flags >> 1) & 1u;
   expected = before;
   const U tail = collision ? (write + 1) % 32 : (write + 2) % 32;
   auto* record = expected.data() + write * 10;
   record[0] = record[1] = xy;
   record[2] = previous == 1;
   record[3] = flags & 1u;
   record[4] = (flags >> 1) & 1u;
   record[5] = record[6] = previous == 2;
   record[7] = (flags >> 2) & 1u;
   record[8] = (flags >> 3) & 1u;
   record[9] = (alt & 0x8000u) != 0;
   std::memcpy(reinterpret_cast<void*>(0x641ab8), before.data(), sizeof(before));
   mouse_global(0x641a88) = write;
   mouse_global(0x641fbc) = current;
   mouse_global(0x641a74) = tail;
   alt_result = alt; key_calls = 0;
   // Upper coordinate/flag bits must be ignored by the original instructions.
   reinterpret_cast<Write>(0x4a3800)(flags | 0xabcd0000u, xy | 0x56780000u, xy | 0x9abc0000u);
   require_mouse(key_calls == 1, "Original Alt poll count");
   require_mouse(std::memcmp(reinterpret_cast<void*>(0x641ab8), expected.data(), sizeof(expected)) == 0, "Exact40byte record/unrelated ring cells");
   require_mouse(mouse_global(0x641a88) == (write + 1) % 32, "Write wrap");
   require_mouse(mouse_global(0x641fbc) == (current + 1) % 32, "Current wrap");
   require_mouse(mouse_global(0x641a74) == (collision ? (tail + 1) % 32 : tail), "Collision tail advancement");
   require_mouse(std::equal(original.begin(), original.end(), writer), "Original writer instructions changed");
   ++cases; ++per_write; collisions += collision; write_wraps += write == 31;
   current_wraps += current == 31; tail_wraps += collision && tail == 31;
  }
  require_mouse(per_write == 1728, "Per-index matrix cardinality");
  std::cout << "{\"write\":" << write << ",\"cases\":" << per_write << "}\n";
 }
 require_mouse(cases == 55296 && collisions == 27648 && write_wraps == 1728 && current_wraps == 1728 && tail_wraps == 864, "Complete matrix/wrap boundaries");
 std::cout << "{\"complete\":true,\"cases\":" << cases << ",\"collision_cases\":" << collisions << ",\"write_wraps\":" << write_wraps << ",\"current_wraps\":" << current_wraps << ",\"tail_wraps\":" << tail_wraps << "}\n";
 return 0;
} catch (const std::exception& error) {
 std::cerr << error.what() << '\n';
 return 1;
}
