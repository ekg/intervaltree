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

template<class T>
Interval<T> randomInterval(int maxStart, int maxLength, int maxStop, const T& value) {
    int start = randInt(0, maxStart);
    int stop = min(randInt(start, start + maxLength), maxStop);
    return Interval<T>(start, stop, value);
}

int main() {

    srand((unsigned)time(NULL));

    vector<Interval<bool> > intervals;
    vector<Interval<bool> > queries;

    // generate a test set of target intervals
    for (int i = 0; i < 10000; ++i) {
        intervals.push_back(randomInterval(100000, 1000, 100000 + 1, true));
    }
    // and queries
    for (int i = 0; i < 5000; ++i) {
        queries.push_back(randomInterval(100000, 1000, 100000 + 1, true));
    }

    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;

    // using brute-force search
    vector<int> bruteforcecounts;
    Clock::time_point t0 = Clock::now();
    for (vector<Interval<bool> >::iterator q = queries.begin(); q != queries.end(); ++q) {
        vector<Interval<bool> > results;
        for (vector<Interval<bool> >::iterator i = intervals.begin(); i != intervals.end(); ++i) {
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
    //IntervalTree<bool> tree(intervals);
    IntervalTree<bool> tree;
    tree = IntervalTree<bool>(intervals);
    vector<int> treecounts;
    t0 = Clock::now();
    for (vector<Interval<bool> >::iterator q = queries.begin(); q != queries.end(); ++q) {
        vector<Interval<bool> > results;
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

