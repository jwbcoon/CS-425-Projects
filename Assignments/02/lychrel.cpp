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
    bool threadBiteOrderFlag = true;

    auto consume = [&data, &consumed, &threadBiteOrderFlag](const size_t &tid) {
        auto bite = 2 << ((threadBiteOrderFlag ? MaxThreads - tid : tid) + tid % 2 + 1);
        if (consumed < data.size() / 4 && consumed + bite >= data.size() / 4) std::cout << "Consumed is " << consumed << std::endl;
        if (consumed < data.size() / 2 && consumed + bite >= data.size() / 2) std::cout << "Consumed is " << consumed << std::endl;
        if (consumed < (3 * data.size()) / 4 && consumed + bite >= (3 * data.size()) / 4) std::cout << "Consumed is " << consumed << std::endl;
        if (consumed + bite < data.size())
           consumed += bite;
        else consumed += data.size() - consumed - 1;
        return bite;
    };


    /*for (int tid = 0; tid < MaxThreads; tid++) {
        std::cout << "Consumed: " << consumed << std::endl;
        std::cout << "Bite: " << consume(tid) << std::endl;
    }*/



    // Iterate across all available data values, processing them using the 
    //   reverse-digits and sum technique described in class.
    for (auto tid = 0; tid < MaxThreads; tid++) {
        std::thread t{[&, tid]() {
            do {
                size_t start, end;
                threadBiteOrderFlag = !threadBiteOrderFlag;
                {
                    std::lock_guard lock{mutex};
                    start = consumed + (consumed > 0); // Add 1 if after 1st iteration to avoid overlap
                    std::cout << "Thread " << tid << " beginning task with Chunk Size " << consume(tid) << std::endl;
                    end = consumed + 1;
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
            } while (consumed < data.size());

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
