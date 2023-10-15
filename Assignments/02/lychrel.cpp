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

#include <barrier>
#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>


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

    // Iterate across all available data values, processing them using the 
    //   reverse-digits and sum technique described in class.
	//do i do the parallelization here? do i replace this entire loop or add the new one that does threading?
	//so i make a new thread? but {} is expecting a function?
	//std::thread{hello}.join()
    // all the threads share the same barrier amd mutex
    std::barrier barrier{MaxThreads};
    //is how the threads communicate with eachother to tell when the variables they share are allowed to be changed by them.
    std::mutex mutex;

       //here is where we can parallelize the code
        

        int LastId = MaxThreads - 1;

        // ok so first we need to loop through he threads at each pass
        for (int id = 0; id < MaxThreads; ++id){
        
        // need to divide the the data into chunks that will be given to the threads.
        size_t chunkSize = int(data.size() / MaxThreads + 1);

        //so lets make a thread. we pass it the a function diredtly like so. the closure gives acess to the variables like so[=,&iter].
        // = gives it all the variables within but the variables it doesnt have acess too need to be put in with a &(varname)
        //
        
        std::thread t{[&,id]() {
            //need to chunk out data to each thread.
            //so we create the starting and end point of the chunk given to the thread
            //then we loop through that chunk for each thread.
            auto start = id * chunkSize;
            auto end = std::min(data.size(), start + chunkSize);
            for(auto i = start; i < end; ++i) {

                size_t iter = 0;
                Number number = data[i];
                Number n = number;
                while (!n.is_palindrome() && ++iter < MaxIterations) {
                //here is where we can parallelize the code
                Number sum(n.size());   // Value used to store current sum of digits
                Number r = n.reverse(); // reverse the digits of the value

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

                //problem, n is a refrence to a number object, not a number object. derefrence it? make a new container for the sums that are palindromes?

                n = sum;
            }
            
            {//remeber that you can create scope with {}. this is stylistic choice to make it clear how long the lock needs to live to protect the variables we need to be protected.
                std::lock_guard lock{mutex}; 
                //when the lock is created it engages the mutex and activates the lock until it is out of scope.
                // when the lock is out of scope the destructor of the lock unlocks the mutex.
                //creates a syncrnoization of the threads which puts them in an order allowing each of them to finish without interupting eachother.
                
                //when determining all the places that need to be protected by a lock, look at all the places its used.
                
                if (!(iter < maxIter || iter == MaxIterations)){
                    Record record{number, n};
                    if (iter > maxIter) {
                        //data that the threads share need to be protected by a lock otherwise we get a race condition
                        // std::lock_guard lock{mutex}; only protects the valuse within the scope of this block of data.
                        //meaning everything in this if statement is protected but nothing outside of it is.
                        records.clear();
                        maxIter = iter;
                    }

                    records.push_back(record);
                }
            }
        }

        barrier.arrive_and_wait();
        }};
        //this makes sure we dont go over the range of the values passed in. the range being the number of threads - 1.
        (id < LastId) ? t.detach() : t.join();

        //worker.join();
        }

        

        

        // Update our records.  First, determine if we have a new
        //   maximum number of iterations that isn't the control limit
        //   (MaxIterations) for a particular number.  If we're less
        //   tha the current maximum (maxIter) or we've exceeded the number
        //   of permissible iterations, ignore the current result and move
        //   onto the next number.
        //if (iter < maxIter || iter == MaxIterations) { continue; }

        // Otherwise update our records, which possibly means discarding
        //   our current maximum and rebuilding our records list.
        //Record record{number, n};
        //if (iter > maxIter) {
        //    records.clear();
        //    maxIter = iter;
       // }

        //records.push_back(record);
    
    // Output our final results
    std::cout << "\nmaximum number of iterations = " << maxIter << "\n";
    for (auto& [number, palindrome] : records) {
        //here is where we can parallelize the code 
        std::cout 
            << "\t" << number 
            << " : [" << palindrome.size() << "] "
            << palindrome << "\n";
    }
}
