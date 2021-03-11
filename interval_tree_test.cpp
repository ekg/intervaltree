#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <limits>
#include <time.h>
#include <assert.h>
#include "IntervalTree.h"
#define CATCH_CONFIG_RUNNER // Mark this as file as the test-runner for catch
#include "catch.hpp"        // Include the catch unit test framework

using namespace std;

typedef IntervalTree<std::size_t, bool> intervalTree;
typedef intervalTree::interval interval;
typedef intervalTree::interval_vector intervalVector;

TEST_CASE( "Empty tree" ) {
    IntervalTree<std::size_t, int> t;
    REQUIRE( t.findOverlapping(-1,1).size() == 0 );
}

TEST_CASE( "Singleton tree" ) {
  IntervalTree<std::size_t, double> t{ {{1,3,5.5}},
                                       1, 64, 512};

    SECTION ("Point query on left") {
	auto v = t.findOverlapping(1,1);
	REQUIRE( v.size() == 1);
	REQUIRE( v.front().start == 1 );
	REQUIRE( v.front().stop == 3 );
	REQUIRE( v.front().value == 5.5 );
    }

    SECTION ("Wild search values") {
        typedef IntervalTree<double, std::size_t> IT;
        IT t { {{0.0, 1.0, 0}} };
        const auto inf = std::numeric_limits<double>::infinity();
        const auto nan = std::numeric_limits<double>::quiet_NaN();
        auto sanityResults = t.findOverlapping(inf, inf);
        assert(sanityResults.size() == 0);
        sanityResults = t.findOverlapping(-inf, inf);
        assert(sanityResults.size() == 1);
        sanityResults = t.findOverlapping(0, inf);
        assert(sanityResults.size() == 1);
        sanityResults = t.findOverlapping(0.5, inf);
        assert(sanityResults.size() == 1);
        sanityResults = t.findOverlapping(1.1, inf);
        assert(sanityResults.size() == 0);
        sanityResults = t.findOverlapping(-inf, 1.0);
        assert(sanityResults.size() == 1);
        sanityResults = t.findOverlapping(-inf, 0.5);
        assert(sanityResults.size() == 1);
        sanityResults = t.findOverlapping(-inf, 0.0);
        assert(sanityResults.size() == 1);
        sanityResults = t.findOverlapping(-inf, -0.1);
        assert(sanityResults.size() == 0);
        sanityResults = t.findOverlapping(nan, nan);
        assert(sanityResults.size() == 0);
        sanityResults = t.findOverlapping(-nan, nan);
        assert(sanityResults.size() == 0);
        sanityResults = t.findOverlapping(nan, 1);
        assert(sanityResults.size() == 0);
        sanityResults = t.findOverlapping(0, nan);
        assert(sanityResults.size() == 0);
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
    IntervalTree<std::size_t, double> t{{{5,10,10.5},{5,10,5.5}}};

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

template<typename Scalar>
Scalar randKey(Scalar floor, Scalar ceiling) {
    Scalar range = ceiling - floor;
    return floor + range * ((double) rand() / (double) (RAND_MAX + 1.0));
}

template<class Scalar, typename Value>
Interval<Scalar, Value> randomInterval(Scalar maxStart,  Scalar maxLength, Scalar maxStop, 
                                       const Value& value) {
    Scalar start = randKey<Scalar>(0, maxStart);
    Scalar stop = min<Scalar>(randKey<Scalar>(start, start + maxLength), maxStop);
    return Interval<Scalar, Value>(start, stop, value);
}

int main(int argc, char**argv) {
    typedef vector<std::size_t> countsVector;

    // a simple sanity check
    typedef IntervalTree<int, bool> ITree;
    ITree::interval_vector sanityIntervals;
    sanityIntervals.push_back(ITree::interval(60, 80, true));
    sanityIntervals.push_back(ITree::interval(20, 40, true));
    ITree sanityTree(std::move(sanityIntervals), 16, 1);

    ITree::interval_vector sanityResults;
    sanityResults = sanityTree.findOverlapping(30, 50);
    assert(sanityResults.size() == 1);

    sanityResults = sanityTree.findContained(15, 45);
    assert(sanityResults.size() == 1);


    srand((unsigned)time(NULL));

    ITree::interval_vector intervals;
    ITree::interval_vector queries;

    // generate a test set of target intervals
    for (int i = 0; i < 10000; ++i) {
        intervals.push_back(randomInterval<int, bool>(100000, 1000, 100000 + 1, true));
    }
    // and queries
    for (int i = 0; i < 5000; ++i) {
        queries.push_back(randomInterval<int, bool>(100000, 1000, 100000 + 1, true));
    }

    typedef chrono::high_resolution_clock Clock;
    typedef chrono::milliseconds milliseconds;

    // using brute-force search
    countsVector bruteforcecounts;
    Clock::time_point t0 = Clock::now();
    for (auto q = queries.begin(); q != queries.end(); ++q) {
        ITree::interval_vector results;
        for (auto i = intervals.begin(); i != intervals.end(); ++i) {
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
    cout << intervals[0];
    ITree tree = ITree(std::move(intervals), 16, 1);
    countsVector treecounts;
    t0 = Clock::now();
    for (auto q = queries.begin(); q != queries.end(); ++q) {
        auto results = tree.findContained(q->start, q->stop);
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
