#ifndef SERVER_UTILH_H
#define SERVER_UTILH_H

#include <utility>
#include <chrono>

/* -------------------------- */
/* Utilities                  */
/* -------------------------- */

#define m_max(a, b) (a > b ? a : b)
#define m_min(a, b) (a < b ? a : b)
#define println(s) std::cout << s << std::endl

using Clock = std::chrono::high_resolution_clock;

#endif //SERVER_UTILH_H
