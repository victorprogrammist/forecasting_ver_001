
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef OUTLIER_H
#define OUTLIER_H

#include "ord_pack.h"

template <class T>
struct Outlier {

    OrdPack<T> hi_bound;
    OrdPack<T> lo_bound;

    Outlier(int cnt) : hi_bound(cnt), lo_bound(cnt) {}


    int count_normal() const {
        int c = hi_bound.cnt_values - hi_bound.c_invalid - hi_bound.c_numbers - lo_bound.c_numbers;
        return std::max(c, 0);
    }

    double get_hi_bound() const { return hi_bound.mi_koef; }
    double get_lo_bound() const { return -lo_bound.mi_koef; }

    bool check_is_outlier(double v) const {
        if (std::isfinite(v)==false) return true;
        return v <= get_lo_bound() || v >= get_hi_bound(); }

    void add_value(double v, const T& data) {
        hi_bound.add_value(v, data);
        lo_bound.add_value(-v, data);
    }

    QVector<T> get_values() const { return lo_bound.get_values()+hi_bound.get_values(); }
};



#endif // OUTLIER_H
