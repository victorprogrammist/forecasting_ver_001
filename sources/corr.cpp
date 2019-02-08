
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "corr.h"

Corr::Corr(const double* ar_x, const double* ar_y, int c) {
    for (int i = 0; i < c; ++i)
        add_value(ar_x[i], ar_y[i]);
}

Corr::ABE Corr::get_a_b_e() const {

    ABE res;
    if (cnt < 2) {
        res.fail = true;
        return res;
    }

    double n = dbl(cnt);

    double d1 = n * su_x_y - su_x * su_y;
    double d2 = n * su_x_pow2 - su_x * su_x;

    double a = d1 / d2;
    double b = (su_y - a * su_x) / n;

    double d3 = su_y_pow2 - 2.0*a*su_x_y - 2*b*su_y + a*a*su_x_pow2 + 2.0*a*b*su_x + b*b*n;

    res.a = a;
    res.b = b;

    res.er_pow2 = d3 / n;
    res.er_pow2_unbiased = d3 / (n-1.0);

    return res;
}

pair<double,double> Corr::get_a_b() const {

    double n = dbl(cnt);

    double d1 = n * su_x_y - su_x * su_y;
    double d2 = n * su_x_pow2 - su_x * su_x;

    double a = d1 / d2;
    double b = (su_y - a * su_x) / n;

    return {a,b};
}

double Corr::corr_pow2() const {

    double n = dbl(cnt);

    double d1 = n * su_x_y - su_x * su_y;

    double d2 = n * su_x_pow2 - su_x * su_x;

    double d3 = n * su_y_pow2 - su_y * su_y;

    if (d2 == 0.0 || d3 == 0.0)
        return -100;

    if (d1 < 0)
        return - d1 * d1 / (d2 * d3);

    return d1 * d1 / (d2 * d3);
}

void Corr::add_value(double x, double y) {

    ++cnt;
    su_x_y += x*y;
    su_x += x;
    su_y += y;
    su_x_pow2 += x*x;
    su_y_pow2 += y*y;
}
