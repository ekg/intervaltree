#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <time.h>
#include <assert.h>
#include "IntervalTree.h"

using namespace std;

int randInt(int floor, int ceiling) {
    int range = ceiling - floor;
    return floor + range * ((double) rand() / (double) (RAND_MAX + 1.0));
}

Interval randomInterval(int maxStart, int maxLength, int maxStop) {
    int start = randInt(0, maxStart);
    int stop = min(randInt(start, start + maxLength), maxStop);
    return Interval(start, stop);
}

int main() {

    srand((unsigned)time(NULL));

    vector<Interval> intervals;
    vector<Interval> queries;

    // generate a test set of target intervals
    for (int i = 0; i < 10000; ++i) {
        intervals.push_back(randomInterval(100000, 1000, 100000 + 1));
    }
    // and queries
    for (int i = 0; i < 5000; ++i) {
        queries.push_back(randomInterval(100000, 1000, 100000 + 1));
    }

    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;

    // using brute-force search
    vector<int> bruteforcecounts;
    Clock::time_point t0 = Clock::now();
    for (vector<Interval>::iterator q = queries.begin(); q != queries.end(); ++q) {
        vector<Interval> results;
        for (vector<Interval>::iterator i = intervals.begin(); i != intervals.end(); ++i) {
            if (i->start >= q->start && i->stop <= q->stop) {
                results.push_back(*i);
            }
        }
        bruteforcecounts.push_back(results.size());
    }
    Clock::time_point t1 = Clock::now();
    milliseconds ms = chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "brute force:\t" << ms.count() << "ms" << endl;

    // using the interval tree
    IntervalTree tree(intervals);
    vector<int> treecounts;
    t0 = Clock::now();
    for (vector<Interval>::iterator q = queries.begin(); q != queries.end(); ++q) {
        vector<Interval> results;
        tree.findContained(q->start, q->stop, results);
        treecounts.push_back(results.size());
    }
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "interval tree:\t" << ms.count() << "ms" << endl;

    // check that the same number of results are returned
    vector<int>::iterator b = bruteforcecounts.begin();
    for (vector<int>::iterator t = treecounts.begin(); t != treecounts.end(); ++t, ++b) {
        assert(*b == *t);
    }

    return 0;
}

