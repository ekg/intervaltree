#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <time.h>
#include <assert.h>
#include "IntervalTree.h"
#define CATCH_CONFIG_RUNNER // Mark this as file as the test-runner for catch
#include "catch.hpp"        // Include the catch unit test framework

using namespace std;

typedef Interval<bool> interval;
typedef vector<interval> intervalVector;
typedef IntervalTree<bool> intervalTree;

TEST_CASE( "Empty tree" ) {
    IntervalTree<int> t;
    REQUIRE( t.findOverlapping(-1,1).size() == 0 );
}

TEST_CASE( "Singleton tree" ) {
    vector<Interval<double>> values{{1,3,5.5}};
    IntervalTree<double> t{values};

    SECTION ("Point query on left") {
	auto v = t.findOverlapping(1,1);
	REQUIRE( v.size() == 1);
	REQUIRE( v.front().start == 1 );
	REQUIRE( v.front().stop == 3 );
	REQUIRE( v.front().value == 5.5 );
    }

    SECTION ("Point query in middle") {
	auto v = t.findOverlapping(2,2);
	REQUIRE( v.size() == 1);
	REQUIRE( v.front().start == 1 );
	REQUIRE( v.front().stop == 3 );
	REQUIRE( v.front().value == 5.5 );
    }

    SECTION ("Point query on right") {
	auto v = t.findOverlapping(3,3);
	REQUIRE( v.size() == 1);
	REQUIRE( v.front().start == 1 );
	REQUIRE( v.front().stop == 3 );
	REQUIRE( v.front().value == 5.5 );
    }

    SECTION ("Non-overlapping queries") {
	REQUIRE( t.findOverlapping(4,4).size() == 0);
	REQUIRE( t.findOverlapping(0,0).size() == 0);
    }
}

TEST_CASE( "Two identical intervals with different contents" ) {
    vector<Interval<double>> values{{5,10,10.5},{5,10,5.5}};
    IntervalTree<double> t{values};

    auto v = t.findOverlapping(6,6);
    REQUIRE( v.size() == 2);
    REQUIRE( v.front().start == 5 );
    REQUIRE( v.front().stop == 10 );
    REQUIRE( v.back().start == 5 );
    REQUIRE( v.back().stop == 10 );
    set<double> expected{5.5, 10.5};
    set<double> actual{v.front().value, v.back().value};
    REQUIRE( actual == expected);
}

template<typename K>
K randKey(K floor, K ceiling) {
    K range = ceiling - floor;
    return floor + range * ((double) rand() / (double) (RAND_MAX + 1.0));
}

template<class T, typename K>
Interval<T,K> randomInterval(K maxStart, K maxLength, K maxStop, const T& value) {
    K start = randKey<K>(0, maxStart);
    K stop = min<K>(randKey<K>(start, start + maxLength), maxStop);
    return Interval<T,K>(start, stop, value);
}

int main(int argc, char**argv) {
    typedef vector<std::size_t> countsVector;

    // a simple sanity check
    intervalVector sanityIntervals;
    sanityIntervals.push_back(interval(60, 80, true));
    sanityIntervals.push_back(interval(20, 40, true));
    intervalTree sanityTree(sanityIntervals);

    intervalVector sanityResults;
    sanityTree.findOverlapping(30, 50, sanityResults);
    assert(sanityResults.size() == 1);
    sanityResults.clear();
    sanityTree.findContained(15, 45, sanityResults);
    assert(sanityResults.size() == 1);


    srand((unsigned)time(NULL));

    intervalVector intervals;
    intervalVector queries;

    // generate a test set of target intervals
    for (int i = 0; i < 10000; ++i) {
        intervals.push_back(randomInterval<bool, unsigned long>(100000, 1000, 100000 + 1, true));
    }
    // and queries
    for (int i = 0; i < 5000; ++i) {
        queries.push_back(randomInterval<bool, unsigned long>(100000, 1000, 100000 + 1, true));
    }

    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;

    // using brute-force search
    countsVector bruteforcecounts;
    Clock::time_point t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        for (intervalVector::iterator i = intervals.begin(); i != intervals.end(); ++i) {
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
    intervalTree tree = intervalTree(intervals);
    countsVector treecounts;
    t0 = Clock::now();
    for (intervalVector::iterator q = queries.begin(); q != queries.end(); ++q) {
        intervalVector results;
        tree.findContained(q->start, q->stop, results);
        treecounts.push_back(results.size());
    }
    t1 = Clock::now();
    ms = std::chrono::duration_cast<milliseconds>(t1 - t0);
    cout << "interval tree:\t" << ms.count() << "ms" << endl;

    // check that the same number of results are returned
    countsVector::iterator b = bruteforcecounts.begin();
    for (countsVector::iterator t = treecounts.begin(); t != treecounts.end(); ++t, ++b) {
        assert(*b == *t);
    }

    return Catch::Session().run( argc, argv );
}
