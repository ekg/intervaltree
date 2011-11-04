#ifndef __INTERVAL_TREE_H
#define __INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>

using namespace std;


template <class T>
class Interval {
public:
    int start;
    int stop;
    T value;
    Interval(int s, int e, const T& v)
        : start(s)
        , stop(e)
        , value(v)
    { }
};

template <class T>
int intervalStart(const Interval<T>& i) {
    return i.start;
}

template <class T>
int intervalStop(const Interval<T>& i) {
    return i.stop;
}

template <class T>
ostream& operator<<(ostream& out, Interval<T>& i) {
    out << "Interval(" << i.start << ", " << i.stop << "): " << i.value;
    return out;
}

template <class T>
class IntervalStartSorter {
public:
    bool operator() (const Interval<T>& a, const Interval<T>& b) {
        return a.start < b.start;
    }
};

template <class T>
class IntervalTree {

public:
    vector<Interval<T> > intervals;
    IntervalTree<T>* left;
    IntervalTree<T>* right;
    int center;

    IntervalTree<T>(void)
        : left(NULL)
        , right(NULL)
        , center(0)
    { }

    IntervalTree<T>(const IntervalTree<T>& other) {
        center = other.center;
        intervals = other.intervals;
        if (other.left) {
            left = (IntervalTree<T>*) malloc(sizeof(IntervalTree<T>));
            *left = *other.left;
        } else {
            left = NULL;
        }
        if (other.right) {
            right = new IntervalTree<T>();
            *right = *other.right;
        } else {
            right = NULL;
        }
    }

    IntervalTree<T>& operator=(const IntervalTree<T>& other) {
        center = other.center;
        intervals = other.intervals;
        if (other.left) {
            left = new IntervalTree<T>();
            *left = *other.left;
        } else {
            left = NULL;
        }
        if (other.right) {
            right = new IntervalTree<T>();
            *right = *other.right;
        } else {
            right = NULL;
        }
    }

    IntervalTree<T>(
            vector<Interval<T> >& ivals,
            int depth = 16,
            int minbucket = 64,
            int leftextent = 0,
            int rightextent = 0,
            int maxbucket = 512
            )
        : left(NULL)
        , right(NULL)
    {

        --depth;
        if (depth == 0 || ivals.size() < minbucket && ivals.size() < maxbucket) {
            intervals = ivals;
        } else {
            if (leftextent == 0 && rightextent == 0) {
                // sort intervals by start
                IntervalStartSorter<T> intervalStartSorter;
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
                transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop<T>);
                rightp = *max_element(stops.begin(), stops.end());
            }

            //centerp = ( leftp + rightp ) / 2;
            centerp = ivals.at(ivals.size() / 2).start;
            center = centerp;

            vector<Interval<T> > lefts;
            vector<Interval<T> > rights;

            for (typename vector<Interval<T> >::iterator i = ivals.begin(); i != ivals.end(); ++i) {
                Interval<T>& interval = *i;
                if (interval.stop < center) {
                    lefts.push_back(interval);
                } else if (interval.start > center) {
                    rights.push_back(interval);
                } else {
                    intervals.push_back(interval);
                }
            }

            if (!lefts.empty()) {
                left = new IntervalTree<T>(lefts, depth, minbucket, leftp, centerp);
            }
            if (!rights.empty()) {
                right = new IntervalTree<T>(rights, depth, minbucket, centerp, rightp);
            }
        }
    }

    void findOverlapping(int start, int stop, vector<Interval<T> >& overlapping) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (typename vector<Interval<T> >::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                Interval<T>& interval = *i;
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

    void findContained(int start, int stop, vector<Interval<T> >& contained) {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (typename vector<Interval<T> >::iterator i = intervals.begin(); i != intervals.end(); ++i) {
                Interval<T>& interval = *i;
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
