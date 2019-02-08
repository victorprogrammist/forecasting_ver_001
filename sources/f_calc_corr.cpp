
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "f_main.h"
#include "outlier.h"

void init_forecast_nodes(
        ForecastCalc& f,
        ArInts& ar_len_corr) {

    int c = ar_len_corr.size();
    int cc = c*2 - 1;

    QVector<ForecastNode>& ar_n = f.ar_nodes;
    ar_n.fill(ForecastNode(), cc);

    for (ForecastNode& n : ar_n)
        n.ar_pos.fill(OrdNode(), f.i_forecast+1);

    for (int i = 0; i < c; ++i) {
        int len = ar_len_corr.at(i);
        ar_n[i].present = "="+QString::number(len);
        ar_n[i].len_corr = len;
        if (i > 0) {
            ar_n[c+i-1].present = "+"+QString::number(len);
            ar_n[c+i-1].len_corr = len;
        }
    }
}

void forecast_init(
        ForecastCalc& f,
        ForecastData& dat,
        ArInts& ar_len_corr,
        int ma_len_forecast) {

    myassert( f.has_init == false );

    f.i_forecast = dat.i_forecast();

    f.ma_len_corr = ar_len_corr.last();
    if (f.ma_len_corr > f.i_forecast) return;

    f.ma_len_forecast = ma_len_forecast;
    f.ar_len_corr = ar_len_corr;

    init_forecast_nodes(f, ar_len_corr);

    f.has_init = true;
}

void forecast_calc_corr(
        ForecastCalc& f,
        ForecastData& dat,
        CorrSlide::KindCorr kc) {

    myassert( f.has_init && f.has_calc == false );
    myassert( f.i_forecast == dat.i_forecast() );

    double* rev_data = dat.ar_rev_data_test.data();

    CorrSlide corr;
    corr.init(f.ar_len_corr, rev_data);

    int i_rev_pos = f.ma_len_forecast;

    // цикл по всем доступным точкам для подсчета подобия
    //  в обратном порядки. Но т.к. массив реверсивен, то как будто бы в прямом порядке.
    for (; i_rev_pos <= dat.c_test_data()-f.ma_len_corr; ++i_rev_pos) {
        corr.calc_next(rev_data+i_rev_pos);

        int i_pos = (dat.c_test_data()-1) - i_rev_pos;

        // цикл по всем вариантам корреляций в пределах одной точки
        for (int i_corr = 0; i_corr < corr.count_res(); ++i_corr) {
            CorrSlide::Res res = corr.get_res(kc, i_corr);
            if (res.fail) continue;

            OrdNode& nn = f.ar_nodes[i_corr].ar_pos[i_pos];
            nn.i_pos = i_pos;
            nn.koef = res.res_koef;
            nn.a = res.res_a;
            nn.b = res.res_b;
        }
    }

    f.has_calc = true;
}

void forecast_process_node(
        ForecastCalc& f,
        ForecastNode& fn,
        int cnt_best,
        int cnt_outlier) {

    myassert( fn.has_calc == false && fn.has_fail == false );

    fn.c_true_pos = 0;

    // исключаем позиции выбросов
    int c_outlier = 0;
    const char* ar_out = f.ar_outlier.constData();
    for (int i_pos = 0; i_pos <= f.i_forecast; ++i_pos) {

        // суммируем к-во выбросов в отрезке
        if (ar_out[i_pos]>0) ++c_outlier;

        if (i_pos >= fn.len_corr)
            if (ar_out[i_pos-fn.len_corr]>0) --c_outlier;

        //****
        OrdNode& nn = fn.ar_pos[i_pos];

        if (c_outlier > 0 && nn.i_pos >= 0) nn.i_pos = -2;

        if (nn.i_pos >= 0)
            ++fn.c_true_pos;
    }

    //***
    if (fn.c_true_pos < cnt_best) {
        fn.has_fail = true;
        return;
    }

    // пачка лучших будет из к-ва лучших плюс к-ва выбросов,
    //   что бы если потом выбросы оказались в пачке лучших, то оставшися хватило на нормальную пачку
    OrdPack<OrdNode*> ord_best(cnt_best+cnt_outlier);

    for (OrdNode& nn : fn.ar_pos) {
        if (nn.i_pos < 0) continue;
        ord_best.add_value(nn.koef, &nn);
    }

    fn.ar_best_pos = ord_best.get_values();

    fn.has_calc = true;
}

// инициализировать массив пометок выбросов
void forecast_init_outlier(
        ForecastCalc& calc, ForecastData& dat, int cnt_outlier) {

    assert( calc.i_forecast == dat.i_forecast() );

    calc.ar_outlier.fill(0, calc.i_forecast+1);

    if (cnt_outlier==0)
        return;

    Outlier<int> out(cnt_outlier);
    for (int i = 0; i <= calc.i_forecast; ++i)
        out.add_value(dat.ar_data_test.at(i), i);

    for (int i : out.get_values())
        calc.ar_outlier[i] = 1;
}

void ForecastCalc::calc_corr(
        ForecastData& dat,
        int cnt_best,
        int cnt_outlier,
        ArInts& ar_len_corr,
        int ma_len_forecast,
        CorrSlide::KindCorr kc) {

    forecast_init(*this, dat, ar_len_corr, ma_len_forecast);

    forecast_init_outlier(*this, dat, cnt_outlier);

    forecast_calc_corr(*this, dat, kc);

    for (ForecastNode& fn : ar_nodes)
        forecast_process_node(*this, fn, cnt_best, cnt_outlier);
}

