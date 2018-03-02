#ifndef __INTERVAL_TREE_H
#define __INTERVAL_TREE_H

#include <vector>
#include <algorithm>
#include <iostream>
#include <memory>

template <class Scalar, typename Value>
class Interval {
public:
    Scalar start;
    Scalar stop;
    Value value;
    Interval(const Scalar& s, const Scalar& e, const Value& v)
        : start(s)
        , stop(e)
        , value(v)
    { }
};

template <class Scalar, typename Value>
Value intervalStart(const Interval<Scalar,Value>& i) {
    return i.start;
}

template <class Scalar, typename Value>
Value intervalStop(const Interval<Scalar, Value>& i) {
    return i.stop;
}

template <class Scalar, typename Value>
  std::ostream& operator<<(std::ostream& out, Interval<Scalar, Value>& i) {
    out << "Interval(" << i.start << ", " << i.stop << "): " << i.value;
    return out;
}

class IntervalStartLessThan {
public:
  template <class Scalar, typename Value>
    bool operator()(const Interval<Scalar, Value>& a, const Interval<Scalar, Value>& b) {
        return a.start < b.start;
    }
};

template <class Scalar, typename Value, 
          typename IntervalVector = std::vector<Interval<Scalar, Value>>>
class IntervalTree {
public:
    typedef typename IntervalVector::value_type interval;
    typedef IntervalVector interval_vector;

    interval_vector intervals;
    std::unique_ptr<IntervalTree> left;
    std::unique_ptr<IntervalTree> right;
    Scalar center;

    IntervalTree()
        : left(nullptr)
        , right(nullptr)
        , center(0)
    {}

    std::unique_ptr<IntervalTree> clone() const {
        return std::unique_ptr<IntervalTree>(new IntervalTree(*this));
    }

    IntervalTree(const IntervalTree& other)
    :   intervals(other.intervals),
        left(other.left ? other.left->clone() : nullptr),
        right(other.right ? other.right->clone() : nullptr),
        center(other.center)
    {}

    IntervalTree& operator=(IntervalTree&&) = default;
    IntervalTree(IntervalTree&&) = default;

    IntervalTree& operator=(const IntervalTree& other) {
        center = other.center;
        intervals = other.intervals;
        left = other.left ? other.left->clone() : nullptr;
        right = other.right ? other.right->clone() : nullptr;
        return *this;
    }

    // Note: changes the order of ivals
    IntervalTree(
            interval_vector& ivals,
            std::size_t depth = 16,
            std::size_t minbucket = 64,
            Scalar leftextent = 0,
            Scalar rightextent = 0,
            std::size_t maxbucket = 512
            )
        : left(nullptr)
        , right(nullptr)
    {
        --depth;
        IntervalStartLessThan intervalStartLessThan;
        if (depth == 0 || (ivals.size() < minbucket && ivals.size() < maxbucket)) {
            std::sort(ivals.begin(), ivals.end(), intervalStartLessThan);
            intervals = ivals;
        } else {
            if (leftextent == 0 && rightextent == 0) {
                // sort intervals by start
              std::sort(ivals.begin(), ivals.end(), intervalStartLessThan);
            }

            Scalar leftp = 0;
            Scalar rightp = 0;
            Scalar centerp = 0;

            if (leftextent || rightextent) {
                leftp = leftextent;
                rightp = rightextent;
            } else {
                leftp = ivals.front().start;
                std::vector<Scalar> stops;
                stops.resize(ivals.size());
                transform(ivals.begin(), ivals.end(), stops.begin(), intervalStop<Scalar,Value>);
                rightp = *max_element(stops.begin(), stops.end());
            }

            //centerp = ( leftp + rightp ) / 2;
            centerp = ivals.at(ivals.size() / 2).start;
            center = centerp;

            interval_vector lefts;
            interval_vector rights;

            for (typename interval_vector::const_iterator i = ivals.begin(); i != ivals.end(); ++i) {
                const interval& interval = *i;
                if (interval.stop < center) {
                    lefts.push_back(interval);
                } else if (interval.start > center) {
                    rights.push_back(interval);
                } else {
                    intervals.push_back(interval);
                }
            }

            if (!lefts.empty()) {
                left.reset(new IntervalTree(lefts, depth, minbucket, leftp, centerp));
            }
            if (!rights.empty()) {
                right.reset(new IntervalTree(rights, depth, minbucket, centerp, rightp));
            }
        }
    }

    interval_vector findOverlapping(const Scalar start, const Scalar& stop) const {
        interval_vector ov;
        findOverlapping(start, stop, ov);
        return ov;
    }

    void findOverlapping(const Scalar& start, const Scalar& stop, interval_vector& overlapping) const {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (typename interval_vector::const_iterator i = intervals.begin(); i != intervals.end(); ++i) {
                const interval& interval = *i;
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

    interval_vector findContained(const Scalar& start, const Scalar& stop) const {
        interval_vector contained;
        findContained(start, stop, contained);
        return contained;
    }

    void findContained(const Scalar& start, const Scalar& stop, interval_vector& contained) const {
        if (!intervals.empty() && ! (stop < intervals.front().start)) {
            for (typename interval_vector::const_iterator i = intervals.begin(); i != intervals.end(); ++i) {
                const interval& interval = *i;
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

    ~IntervalTree() = default;
};

#endif
