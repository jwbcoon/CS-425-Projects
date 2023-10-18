/////////////////////////////////////////////////////////////////////////////
//
//  lychrel.cpp
//
//  A program that searches for the largest (in terms of number of
//    iterations without exceeding a specified maximum) palindrome value.
//
//  The program reports a list of numbers that share the maximum number of
//    iterations, along with the size and final palindrome number
//

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <mutex>
#include <cmath>

#include "LychrelData.h"

// A structure recording the starting number, and its final palindrome.
//   An vector of these records (typedefed to "Records") is built during
//   program execution, and output before the program terminates.
struct Record {
    Number n;
    Number palindrome;
};
using Records = std::vector<Record>;

// Application specific constants
const size_t MaxIterations = 7500;
const size_t MaxThreads = 10;

//
// --- main ---
//
int main() {
    LychrelData data;

    std::cerr << "Processing " << data.size() << " values ...\n";

    size_t maxIter = 0;  // Records the current maximum number of iterations
    Records records; // list of values that took maxIter iterations
    std::barrier barrier{MaxThreads};
    std::mutex mutex;
    int lastThreadID = MaxThreads - 1;
    std::atomic<size_t> consumed = 0;
    uint16_t chunkSize = [&]() { uint16_t result = 1; while (result << 1 < data.size() / MaxThreads) result <<= 1; return result; } ();
    std::vector<uint16_t> decays(MaxThreads, chunkSize);
    int chunkSum = 0;

    auto consume = [&](const size_t &tid) {
        if (consumed > data.size() / 4 && decays[tid] > chunkSize >> 1) decays[tid] >>= 1;
        if (consumed > data.size() / 2 && decays[tid] > chunkSize >> 2) decays[tid] >>= 1;
        if (consumed > (3 * data.size()) / 4 && decays[tid] > chunkSize >> 3) decays[tid] >>= 1;
        auto bite = decays[tid];
        if (consumed + bite >= data.size())
            bite = data.size() - consumed - 1;
        consumed += bite;
        return bite;
    };



    // Iterate across all available data values, processing them using the 
    //   reverse-digits and sum technique described in class.
    for (auto tid = 0; tid < MaxThreads; tid++) {
        std::thread t{[&, tid]() {
            do {
                size_t start, end;
                {
                    std::lock_guard lock{mutex};
                    start = consumed; // Add 1 if after 1st iteration to avoid overlap
                    end = consume(tid) + start;
                    chunkSum += end - start;
                    std::cout << "Thread " << tid << " beginning task with Chunk Size " << end - start << std::endl;
                    std::cout << "Total processed: " << chunkSum << std::endl;
                }

                for (int i = start; i < end; i++) {
                    Number number = data[i];
                    
                    size_t iter = 0;
                    Number n = number;

                    // The Lychrel loop - for any iteration, take the number, reverse
                    //   its digits, and sum those values together.  If that sum
                    //   is a palindrome, stop processing
                    while (!n.is_palindrome() && ++iter < MaxIterations) {
                        Number sum(n.size());   // Value used to store current sum of digits
                        n.reverse(); // reverse the digits of the value

                        // An iterator pointing to the first digit of the reversed
                        //   value.  This iterator will be incremented to basically
                        //   traverse the digits of the main number in reverse
                        auto rd = n.begin(); 
                        
                        bool carry = false;  // flag to indicate if we had a carry

                        // Sum the digits using the "transform" algorithm.  This
                        //   algorithm traverses a range of values (in our case,
                        //   starting with the least-siginificant [i.e., right most]
                        //   digit) of the original number, adding each digit to its
                        //   matching digit (by position) in the reversed number.
                        //
                        // The result is stored in the sum variable, which is
                        //   built up one digit at a time, respecting if a carry
                        //   digit was necessary for any iteration. 
                        std::transform(n.rbegin(), n.rend(), sum.rbegin(), 
                            [&](auto d) {
                                auto v = d + *rd++ + carry;
                
                                carry = v > 9;
                                if (carry) { v -= 10; }
                
                                return v;
                            }
                        );

                        // If there's a final carry value, prepend that to the sum
                        if (carry) { sum.push_front(1); }

                        // Transfer the sum making it the next number to be processed
                        //   (i.e., reversed, summed, and checked if it's a
                        //   palindrome)
                        n = sum;
                    }

                    // Update our records.  First, determine if we have a new
                    //   maximum number of iterations that isn't the control limit
                    //   (MaxIterations) for a particular number.  If we're less
                    //   tha the current maximum (maxIter) or we've exceeded the number
                    //   of permissible iterations, ignore the current result and move
                    //   onto the next number.
                    if (iter <= maxIter || iter == MaxIterations) { continue; }

                    // Otherwise update our records, which possibly means discarding
                    //   our current maximum and rebuilding our records list.
                    Record record{number, n};
                    if (iter > maxIter) {
                        std::lock_guard lock{mutex};
                        records.clear();
                        maxIter = iter;
                    }

                    records.push_back(record);
                }
            } while (consumed < data.size() - 1);

            barrier.arrive_and_wait();
        }};
        (tid < lastThreadID) ? t.detach() : t.join();
    }

    // Output our final results
    std::cout << "\nmaximum number of iterations = " << maxIter << "\n";
    for (auto& [number, palindrome] : records) {
        std::cout 
            << "\t" << number 
            << " : [" << palindrome.size() << "] "
            << palindrome << "\n";
    }
}
