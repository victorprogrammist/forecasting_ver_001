
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "corr_slide.h"
#include "tls.h"

CorrSlide::Res CorrSlide::get_res(KindCorr kc, int i_res) {

    if (i_res < c_lens)
        return get_res(simple_dist, kc, i_res);

    return get_res(accum_dist, kc, i_res - c_lens);
}

CorrSlide::Res CorrSlide::get_res(KindDist kd, KindCorr kc, int i_dist) {

    Calc* calc;
    if (kd == simple_dist) {
        calc = &ar_calc[i_dist];
    } else if (kd == accum_dist) {
        calc = &ar_calc_2[i_dist];
    } else { myassert(false); }

    Res res;

    double su_x_2 = calc->su_m_pow2;
    double su_y_2 = calc->su_s_pow2;
    double su_y_x = calc->su_s_m;
    double su_x = calc->su_m;
    double su_y = calc->su_s;
    double n = dbl(calc->cnt);

    if (kc == simple_diff) {

        // sqrt( sum( (x-y)^2 ) / n )

        double er_pow2 = (su_x_2 + su_y_2 - 2.0 * su_y_x) / n;

        res.res_koef = -sqrt(er_pow2);
        res.res_a = 1;
        res.res_b = 0;

    } else if (kc == diff_sub_zero) {

        // sqrt( sum( ( (x-x_last) - (y-y_last) )^2 ) / n )

        double A = last_first_m;
        double B = ar_static.at(0);
        double d1 = su_x_2 + su_y_2 - 2.0 * su_y_x + 2.0 * (- A*su_x - B*su_y + B*su_x + A*su_y);
        double er_pow2 = d1/n + pow2(A-B);

        res.res_koef = -sqrt(er_pow2);
        res.res_a = 1.0;
        res.res_b = B - A;

    } else if (kc == diff_sub_mean) {

        // sqrt( sum( ( (x-sum(x)/n) - (y-sum(y)/n) )^2 ) / n )

        double d2 = pow2(su_y - su_x) / n;
        double er_pow2 = (su_x_2 + su_y_2 - 2.0 * su_y_x - d2) / n;

        res.res_koef = - sqrt(er_pow2);
        res.res_a = 1.0;
        res.res_b = (su_y - su_x) / n;

    } else if (kc == diff_div_mean) {

        // sqrt( sum( (x/sum(x)/n - y/sum(y)/n)^2 ) / n )

        double d1 = su_x_2 / (su_x*su_x) + su_y_2 / (su_y*su_y) - 2.0 * su_y_x / (su_x * su_y);

        res.res_koef = - sqrt(d1/n);
        res.res_a = su_y / su_x;
        res.res_b = 0;

    } else if (kc == corr_Pearson) {

        // корреляция Пирсона

        double d1 = n * su_y_x - su_y * su_x;
        double d2 = n * su_x_2 - su_x * su_x;
        double d4 = calc->div_s * d2;

        // в квадрате, но с сохранением знака
        res.res_koef = (d1 < 0 ? -d1 : d1) *d1 / d4;
        res.res_a = d1 / d2;
        res.res_b = (su_y - res.res_a * su_x) / n;

    } else if (kc == without) {

        // не используется
        res.res_koef = 1.0;
        res.res_a = 0;
        res.res_b = 0;
    }

    if (std::isfinite(res.res_koef)==false ||
            std::isfinite(res.res_a)==false ||
            std::isfinite(res.res_b)==false ) {
        res.fail = true;
    }

    return res;
}

void CorrSlide::make_res_2() {

    for (int i_len = 0; i_len < c_lens-1; ++i_len) {

        Calc& calc = ar_calc_2[i_len];
        if (i_len == 0)
            calc = ar_calc[i_len];
        else
            calc = ar_calc_2[i_len-1];

        Calc& calc_plus = ar_calc[i_len+1];

        calc.cnt += calc_plus.cnt;
        calc.su_s += calc_plus.su_s;
        calc.su_s_pow2 += calc_plus.su_s_pow2;
        calc.su_m += calc_plus.su_m;
        calc.su_m_pow2 += calc_plus.su_m_pow2;
        calc.su_s_m += calc_plus.su_s_m;
        calc.div_s = dbl(calc.cnt) * calc.su_s_pow2 - calc.su_s * calc.su_s;
    }
}

void CorrSlide::init(const ArInts& _ar_lens, const double* ar_s) {

    c_lens = _ar_lens.size();
    ar_lens = _ar_lens;

    for (int i = 1; i < ar_lens.size(); ++i)
        myassert( ar_lens.at(i-1) < ar_lens.at(i) );

    int ma_len = ar_lens.at(c_lens-1);
    ar_static.resize(ma_len);
    for (int i = 0; i < ma_len; ++i)
        ar_static[i] = ar_s[i];

    i_step = 0;
}

void CorrSlide::private_calc_first(const double* ar_m) {

    ar_calc.fill(Calc(), c_lens);
    ar_calc_2.fill(Calc(), c_lens-1);

    const double* ar_s = ar_static.constData();

    int i_pos = 0;
    for (int i_len = 0; i_len < c_lens; ++i_len) {

        int c_pos = ar_lens.at(i_len);
        Calc& calc = ar_calc[i_len];

        if (i_len > 0)
            calc = ar_calc.at(i_len-1);

        for (; i_pos < c_pos; ++i_pos) {
            double s = ar_s[i_pos];
            double m = ar_m[i_pos];

            calc.su_s += s;
            calc.su_s_pow2 += s*s;
            calc.su_m += m;
            calc.su_m_pow2 += m*m;
            calc.su_s_m += s*m;
        }

        calc.cnt = c_pos;
        calc.div_s = dbl(c_pos) * calc.su_s_pow2 - calc.su_s * calc.su_s;
    }

    make_res_2();

    last_first_m = ar_m[0];
}

void CorrSlide::calc_next(const double* ar_m) {

    if (i_step-- == 0) {
        i_step = 1000; // иначе накапливается ошибка
        private_calc_first(ar_m);
        return;
    }

    double su_s_m = 0;
    const double* ar_s = ar_static.constData();

    int i_pos = 0;
    for (int i_len = 0; i_len < c_lens; ++i_len) {

        int c_pos = ar_lens.at(i_len);
        double last_m = ar_m[c_pos-1];

        Calc& calc = ar_calc[i_len];
        calc.su_m += last_m - last_first_m;
        calc.su_m_pow2 += last_m*last_m - last_first_m*last_first_m;

        for (; i_pos < c_pos; ++i_pos)
            su_s_m += ar_m[i_pos] * ar_s[i_pos];

        calc.su_s_m = su_s_m;
    }

    make_res_2();

    last_first_m = ar_m[0];
}
