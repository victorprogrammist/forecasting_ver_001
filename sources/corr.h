/*
 * Author Telnov Victor, v-telnov@yandex.ru
 *
 */

#ifndef CORR_H
#define CORR_H

#include "tls.h"
#include "dispersion.h"

struct Corr {

    int cnt = 0;
    double su_x_y = 0;
    double su_x = 0;
    double su_y = 0;
    double su_x_pow2 = 0;
    double su_y_pow2 = 0;

    struct ABE {
        bool fail = false;
        double a = 0;
        double b = 0;
        double er_pow2 = 0;
        double er_pow2_unbiased = 0;

        double er() const { return sqrt(er_pow2); }
        double er_unbiased() const { return sqrt(er_pow2_unbiased); }
    };

    Corr() {}
    Corr(const double* ar_x, const double* ar_y, int c);

    Dispersion disp_x() const { return Dispersion(su_x, su_x_pow2, cnt); }
    Dispersion disp_y() const { return Dispersion(su_y, su_y_pow2, cnt); }

    void add_value(double x, double y);

    double corr_pow2() const;
    double corr() const { return sqrt_sign(corr_pow2()); }

    // коэффициенты пересчета из x в y
    pair<double,double> get_a_b() const;
    double get_a_b_error_pow2() const;
    ABE get_a_b_e() const;

    double calc_from_x(double x) const {
        pair<double,double> p = get_a_b();
        return x * p.first + p.second; }
};


#if 0
struct CorrAdapt {

    int cnt_vals = 0;
    int cnt_adapt = 0;

    double su_x_y = 0;
    double su_x = 0;
    double su_y = 0;
    double su_x_pow2 = 0;
    double su_y_pow2 = 0;

    CorrAdapt() {}
    CorrAdapt(int cr) : cnt_adapt(cr) {}

    double calc_from_x(double x) const {
        pair<double,double> p = get_a_b();
        return x * p.first + p.second; }

    pair<double,double> get_a_b() const {

        myassert(cnt_vals>0);
        if (cnt_vals == 1 || cnt_adapt == 1)
            return {0.0,su_y};

        double n = dbl(std::min(cnt_vals,cnt_adapt));

        double d1 = n * su_x_y - su_x * su_y;
        double d2 = n * su_x_pow2 - su_x * su_x;

        double a = d1 / d2;
        double b = (su_y - a * su_x) / n;

        return {a,b};
    }

    void add_value(double x, double y) {

        myassert( cnt_adapt>0 );

        if (cnt_vals >= cnt_adapt) {
            double k = dbl(cnt_adapt - 1)/dbl(cnt_adapt);
            su_x_y = su_x_y * k + x*y;
            su_x = su_x * k + x;
            su_y = su_y * k + y;
            su_x_pow2 = su_x_pow2 * k + x*x;
            su_y_pow2 = su_y_pow2 * k + y*y;
        } else {
            su_x_y += x*y;
            su_x += x;
            su_y += y;
            su_x_pow2 += x*x;
            su_y_pow2 += y*y;
        }

        ++cnt_vals;
    }
};
#endif

#endif // CORR_H
