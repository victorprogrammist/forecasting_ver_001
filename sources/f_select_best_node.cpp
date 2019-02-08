
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "f_main.h"
#include "outlier.h"

ForecastRes calc_node_for_pos_forward(
        ForecastNode& node,
        ForecastCalc& f,
        ForecastData& dat,
        int i_forward
        ) {

    myassert( node.has_calc );

    ArNums ar_res(node.ar_pos.size(), 0);
    Outlier<int> out(f.cfg.cnt_outlier);

    for (OrdNode& p : node.ar_pos) {
        if (p.i_pos < 0) continue;
        double v = dat.get_old_fact(p.i_pos, i_forward) * p.a + p.b;
        ar_res[p.i_pos] = v;
        out.add_value(v, p.i_pos);
    }

    QVector<char> ar_out(node.ar_pos.size(), 0);
    for (int i_pos : out.get_values())
        ar_out[i_pos] = 1;

    //****************************
    // подсчет дисперсии лучших по корреляции

#ifdef MAKE_ONLY_DOUBLE_BEST
    Dispersion disp_all_double;
#endif

    int i_dbg = 0;
    Dispersion disp_best;
    Dispersion disp_koef_best;
    for (OrdNode* pp : node.ar_best_pos) {
        if (pp->i_pos < 0) continue;
        if (ar_out.at(pp->i_pos) > 0) continue;
        double v = ar_res.at(pp->i_pos);

#ifdef MAKE_ONLY_DOUBLE_BEST
        disp_all_double.add_value(v);
        if (disp_best.cnt_values < f.cfg.cnt_best/2)
#endif
        disp_best.add_value(v);
        disp_koef_best.add_value(pp->koef);

        if (f.cfg.detail_out_info >= 3)
            qDebug() << "===pack_best" << ++i_dbg << v << pp->koef << pp->i_pos << pp->a << pp->b;

        if (disp_best.cnt_values >= f.cfg.cnt_best) break;

#ifdef MAKE_ONLY_DOUBLE_BEST
        if (disp_all_double.cnt_values >= f.cfg.cnt_best) break;
#endif
    }

    ForecastRes res;
#ifndef MAKE_ONLY_DOUBLE_BEST
    if (disp_best.cnt_values < f.cfg.cnt_best) {
#else
    if (disp_all_double.cnt_values < f.cfg.cnt_best) {
#endif
        res.has_fail = true;
        return res;
    }

    //****************************
    // подсчет дисперсии всех остальных
    Dispersion disp_all;
    Dispersion disp_koef_all;
    for (OrdNode& p : node.ar_pos) {
        if (p.i_pos < 0) continue;
        if (ar_out.at(p.i_pos) > 0) continue;
        disp_all.add_value(ar_res.at(p.i_pos));
        disp_koef_all.add_value(p.koef);
    }

    // прогноз это средняя по прогнозам от пачки лучших
    double res_forecast = disp_best.mean();
    // и стандартное отклонение в этой пачке
    double dev_forecast = disp_best.stddev();
    // коэффициент прогноза - насколько в пачке лучших вариация ответов меньше чем в выборке по всем
#ifndef MAKE_ONLY_DOUBLE_BEST
    double koef_forecast = 1.0 - dev_forecast / disp_all.stddev();
#else
    double koef_forecast = 1.0 - dev_forecast / disp_all_double.stddev();
#endif

    if (f.cfg.detail_out_info >= 2)
        qDebug()
            << "===sel_best"
            << "node" << node.present
            << "koef_forecast" << koef_forecast
            << "forecast" << res_forecast
            << "stdev" << dev_forecast
            << "count" << disp_best.cnt_values
            << "all c/m/d"
            << disp_all.cnt_values
            << disp_all.mean()
            << disp_all.stddev()
            << "koef_corr_best"
            << disp_koef_best.mean() << disp_koef_best.stddev()
            << "koef_corr_all"
            << disp_koef_all.mean() << disp_koef_all.stddev();

    res.i_forward = i_forward;
    res.node_present = node.present;
    res.koef_forecast = koef_forecast;
    res.res_forecast = res_forecast;
    res.dev_forecast = dev_forecast;
    res.c_best_pos = disp_best.cnt_values;
    res.c_all_pos = disp_all.cnt_values;
    res.mean_all = disp_all.mean();
    res.dev_all = disp_all.stddev();
    return res;
}

ForecastRes select_best_node_for_pos_forward(
        ForecastCalc& f,
        ForecastData& dat,
        int i_forward) {

    myassert( f.has_calc );
    myassert( f.i_forecast == dat.i_forecast() );

    ForecastRes res;
    res.has_fail = true;
    bool has_first = false;

    for (ForecastNode& node : f.ar_nodes) {
        if (node.has_fail) continue;

        ForecastRes sel_res = calc_node_for_pos_forward(node, f, dat, i_forward);
        if (sel_res.has_fail) continue;

        if (has_first==false || sel_res.koef_forecast > res.koef_forecast) {
            res = sel_res;
            has_first = true;
        }
    }

    return res;
}

ForecastRes ForecastCalc::get_res(ForecastData& dat, int i_forward) {
    return select_best_node_for_pos_forward(*this, dat, i_forward);
}
