#include <atomic>

int main() {
   std::atomic<int64_t> i(0);
   i++;
   return 0;
}
