
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef DISPERSION_H
#define DISPERSION_H

#include <cmath>
#include "tls.h"

struct Dispersion {

    double su = 0;
    double su_pow2 = 0;
    int    cnt_values = 0;

    Dispersion() {}
    Dispersion(double s, double s2, int c)
        : su(s), su_pow2(s2), cnt_values(c) {}

    Dispersion(const ArNums& ar) { add_data(ar); }

    bool empty() const { return cnt_values == 0; }
    void clear();
    void add_value(double v);
    void add_value_if_valid(double v);

    // !!! не корректирует ma & mi на случай вычитания этих крайних значений
    void   subtraction_value(double v);

    double unbiased_sample_variance() const;

    // !!! использует unbiased_sample_variance => делитель cnt-1
    double stddev() const { return sqrt(unbiased_sample_variance()); }
    double stddev_safe() const { return cnt_values<=1?0:stddev(); }

    double stddev_biased() const { return sqrt(sample_variance()); }

    double mean() const;
    double mean_safe() const { return cnt_values<=0?0:mean(); }

    // средне квадратичное отклонение от нуля
    double dev_zero() const;
    double dev_zero_pow2() const;

    // mean() +/- 2*stddev() - 95% значений
    double mi_2s() const { return mean() - stddev() * 2.0; }
    double ma_2s() const { return mean() + stddev() * 2.0; }

    double sample_variance() const;

    void add_data(const double* ar, int c);
    void add_data(const ArNums& ar) { add_data(ar.constData(),ar.size()); }

    void add_dispersion(Dispersion& oth);
    void substraction_dispersion(Dispersion& oth) { add_dispersion(oth); }
};


struct MeanAdapt {
    int    cnt_adapt = 0;
    int    cnt_vals = 0;
    double su = 0;

    bool   has_full() const {
        if (cnt_adapt == 0) return false;
        if (cnt_adapt < 0) return cnt_vals > 0;
        return cnt_vals >= cnt_adapt; }

    MeanAdapt() {}
    MeanAdapt(int c_r) { init(c_r); }

    void   clear() { *this = MeanAdapt(); }
    void   init(int c_r);
    double mean() const;
    void   add_value(double v);
};

struct DispAdapt {

    int    cnt_adapt = 0;
    int    cnt_vals = 0;
    double su = 0;
    double su_pow2 = 0;

    bool   has_full() const { return cnt_vals >= cnt_adapt; }

    DispAdapt() {}
    DispAdapt(int c_r) { init(c_r); }

    void   clear() { *this = DispAdapt(); }
    void   init(int c_r);
    double mean() const;
    double stddev() const;
    double sample_variance() const;
    double unbiased_sample_variance() const;
    void   add_value(double v);
    double dev_zero_pow2() const;
    double dev_zero() const { return sqrt(dev_zero_pow2()); }
};

#endif // DISPERSION_H
