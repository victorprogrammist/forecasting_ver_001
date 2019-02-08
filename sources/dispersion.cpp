
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "dispersion.h"

void Dispersion::add_data(const double* ar, int c) {
    for (int i = 0; i < c; ++i)
        add_value(ar[i]);
}

void Dispersion::add_dispersion(Dispersion& oth) {
    su += oth.su;
    su_pow2 += oth.su_pow2;
    cnt_values += oth.cnt_values;
}

void Dispersion::clear() {
    su = 0;
    su_pow2 = 0;
    cnt_values = 0;
}

void Dispersion::add_value_if_valid(double v) {
    if (std::isfinite(v)) add_value(v);
}

void Dispersion::add_value(double v) {
    su += v;
    su_pow2 += v * v;
    cnt_values++;
}

// !!! не корректирует ma & mi на случай вычитания этих крайних значений
void Dispersion::subtraction_value(double v) {
    myassert( cnt_values > 0 );
    su -= v;
    su_pow2 -= v * v;
    cnt_values--;
}

double Dispersion::unbiased_sample_variance() const {
    myassert( cnt_values > 1 );
    return (su_pow2 - su * su / dbl(cnt_values)) / dbl(cnt_values-1);
}

double Dispersion::mean() const {
    myassert( cnt_values > 0 );
    return su / dbl(cnt_values);
}

double Dispersion::dev_zero_pow2() const {
    myassert( cnt_values > 1 );
    return su_pow2 / dbl(cnt_values);
}

double Dispersion::dev_zero() const {
    return sqrt(dev_zero_pow2());
}

double Dispersion::sample_variance() const {
    myassert( cnt_values > 0 );
    return (su_pow2 - su * su / dbl(cnt_values)) / dbl(cnt_values);
}

void MeanAdapt::init(int c_r) {
    cnt_adapt = c_r;
    su = 0;
    cnt_vals = 0;
}

double MeanAdapt::mean() const {
    myassert( cnt_vals > 0 );

    if (cnt_adapt < 0)
        return su;

    return su / dbl(std::min(cnt_vals, cnt_adapt));
}

void MeanAdapt::add_value(double v) {
    myassert( cnt_adapt != 0 );

    if (cnt_adapt < 0) {

        if (cnt_vals == 0)
            su = v;
        else {
            int cr = 1-cnt_adapt;
            su = (su + v * dbl(cr)) / dbl(1 + cr);
        }

    } else if (cnt_vals >= cnt_adapt)
        su = su - su / dbl(cnt_adapt) + v;
    else
        su = su + v;

    ++cnt_vals;
}

void DispAdapt::init(int c_r) {
    clear();
    if (c_r > 0) {
        cnt_adapt = c_r;
        su = 0;
        su_pow2 = 0;
        cnt_vals = 0;
    }
}

double DispAdapt::dev_zero_pow2() const {
    myassert( cnt_vals > 0 );
    return su_pow2 / dbl(std::min(cnt_vals,cnt_adapt));
}

double DispAdapt::mean() const {
    return su / dbl(std::min(cnt_vals,cnt_adapt));
}

double DispAdapt::stddev() const {
    return sqrt(unbiased_sample_variance());
}

double DispAdapt::sample_variance() const {

    myassert( cnt_vals > 0 );

    double n = dbl(std::min(cnt_vals,cnt_adapt));
    return (su_pow2 - su*su/ n) / n;
}

double DispAdapt::unbiased_sample_variance() const {

    myassert( cnt_vals > 1 );

    return (su_pow2 - su*su/ dbl(std::min(cnt_vals,cnt_adapt)))
            / dbl(std::min(cnt_vals-1,cnt_adapt));
}

void DispAdapt::add_value(double v) {
    myassert( cnt_adapt>0 );

    if (cnt_vals >= cnt_adapt) {
        double k = dbl(cnt_adapt - 1)/dbl(cnt_adapt);
        su = su * k + v;
        su_pow2 = su_pow2 * k + v*v;
    } else {
        su = su + v;
        su_pow2 = su_pow2 + v*v;
    }

    ++cnt_vals;
}
