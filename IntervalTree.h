#ifndef __INTERVAL_TREE_H
#define __INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;


class Interval {
    friend int intervalStart(const Interval& i);
    friend int intervalStop(const Interval& i);
    friend ostream& operator<<(ostream& out, Interval& i);
public:
    int start;
    int stop;
    Interval(int s, int e)
        : start(s)
        , stop(e)
    { }
};

int intervalStart(const Interval& i) {
    return i.start;
}

int intervalStop(const Interval& i) {
    return i.stop;
}

ostream& operator<<(ostream& out, Interval& i) {
    out << "Interval(" << i.start << ", " << i.stop << ")";
    return out;
}

class IntervalStartSorter {
public:
    bool operator() (const Interval& a, const Interval& b) {
        return a.start < b.start;
    }
};

class IntervalTree {

public:
    vector<Interval> intervals;
    IntervalTree* left;
    IntervalTree* right;
    int center;
    bool emptyTree;

    IntervalTree(
            vector<Interval>& ivals,
            int depth = 16,
            int minbucket = 64,
            int leftextent = 0,
            int rightextent = 0,
            int maxbucket = 512
            )
        : emptyTree(false)
        , left(NULL)
        , right(NULL)
    {

        --depth;
        if (depth == 0 || ivals.size() < minbucket && ivals.size() < maxbucket) {
            intervals = ivals;
        } else {
            if (leftextent == 0 && rightextent == 0) {
                // sort intervals by start
                IntervalStartSorter intervalStartSorter;
                sort(ivals.begin(), ivals.end(), intervalStartSorter);
            }

            int leftp = 0;
            int rightp = 0;
            int centerp = 0;
            
            if (leftextent || rightextent) {
                leftp = leftextent;
                rightp = rightextent;
            } else {
                leftp = ivals.front().start;
                vector<int> stops;
                stops.resize(ivals.size());
                transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop);
                rightp = *max_element(stops.begin(), stops.end());
            }

            //centerp = ( leftp + rightp ) / 2;
            centerp = ivals.at(ivals.size() / 2).start;
            center = centerp;

            vector<Interval> lefts;
            vector<Interval> rights;

            for (vector<Interval>::iterator i = ivals.begin(); i != ivals.end(); ++i) {
                Interval& interval = *i;
                if (interval.stop < center) {
                    lefts.push_back(interval);
                } else if (interval.start > center) {
                    rights.push_back(interval);
                } else {
                    intervals.push_back(interval);
                }
            }

            if (!lefts.empty()) {
                left = new IntervalTree(lefts, depth, minbucket, leftp, centerp);
            }
            if (!rights.empty()) {
                right = new IntervalTree(rights, depth, minbucket, centerp, rightp);
            }
        }
    }

    void findOverlapping(int start, int stop, vector<Interval>& overlapping) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (vector<Interval>::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                Interval& interval = *i;
                if (interval.stop >= start && interval.start <= stop) {
                    overlapping.push_back(interval);
                }
            }
        }

        if (left && start <= center) {
            left->findOverlapping(start, stop, overlapping);
        }

        if (right && stop >= center) {
            right->findOverlapping(start, stop, overlapping);
        }

    }

    void findContained(int start, int stop, vector<Interval>& contained) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (vector<Interval>::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                Interval& interval = *i;
                if (interval.start >= start && interval.stop <= stop) {
                    contained.push_back(interval);
                }
            }
        }

        if (left && start <= center) {
            left->findContained(start, stop, contained);
        }

        if (right && stop >= center) {
            right->findContained(start, stop, contained);
        }

    }

    ~IntervalTree(void) {
        // traverse the left and right
        // delete them all the way down
        if (left) {
            delete left;
        }
        if (right) {
            delete right;
        }
    }

};



#endif
