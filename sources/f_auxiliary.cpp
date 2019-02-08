
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "f_main.h"



void ForecastingMain::scan_best_cicling(const ArNums& ar) {

    qDebug() << "====================================================";

    Dispersion disp(ar);

    double mi_d = 0;
    int mi_i_cic = 0;

    for (int i_cic = 1; i_cic < MA_TEST_C_CICLING; ++i_cic) {
        if (has_signal_stop()) break;

        double d;
        int c_avg = cicling_sel_best_avg(ar, i_cic, &d);

        if (cfg.detail_out_info == 0)
            qDebug() << "цикличность" << i_cic << "ошибка" << d;
        else
            qDebug() << "цикличность" << i_cic << "ошибка" << d << "лучшее усреднение" << c_avg;

        if (mi_i_cic == 0 || d < mi_d) {
            mi_d = d;
            mi_i_cic = i_cic;
        }
    }

    qDebug() << "====================================================";
    qDebug() << "лучшая цикличность" << mi_i_cic;
    qDebug() << "ошибка от ее применения" << mi_d;
    qDebug() << "ошибка без этой цикличности но с теми, которые уже применены";
    qDebug() << "...составляет" << Cicling::avg_er_by_vector(ar, {});
}

// выбор наилучшего периода затухания/перестраивания для цикличности
//   отрицательные обозначают, что от предыдущего шага остается всего лишь 1/н значений
//     и основную долю составляет текущее значение
//   положительные обозначают, что доля текущего значения будет 1/н,
//     т.е. почти как средняя только остальные не убираются, а затухают с
//       со скоростью 1/n, 1/n*(1-1/n), 1/n*(1-1/n)*(1-1/n), ... (см.MeanAdapt)
int ForecastingMain::cicling_sel_best_avg(const ArNums& ar, int c_cic, double* res_dev) {

    double mi_dev = 0;
    int mi_c_avg = 0;

    for (int i = MI_TEST_C_AVG_FOR_CICLING; i < MA_TEST_C_AVG_FOR_CICLING; ++i) {
        if (i == 0) continue;

        double dev = Cicling::avg_er_by_vector(ar, {c_cic, i});

        if (mi_c_avg == 0 || dev < mi_dev) {
            mi_dev = dev;
            mi_c_avg = i;
        }
    }

    if (res_dev != nullptr)
        *res_dev = mi_dev;

    return mi_c_avg;
}

// рекурсивный, комбинаторный перебор всех возможных вариантов
//   периодов цикличности из resi
pair<double,ArInts> ForecastingMain::recu_sel_best_seq_cicling(const ArNums& ar, const ArInts& has, const ArInts& resi) {

    pair<double,ArInts> best = {Dispersion(ar).dev_zero_pow2(), has};

    if (has_signal_stop()) return best;

    if (resi.size() == 0)
        return best;

    for (int i = 0; i < resi.size(); ++i) {
        int c_cic = resi.at(i);
        int c_avg = cicling_sel_best_avg(ar, c_cic, nullptr);
        ArNums new_ar = Cicling::process_vector(ar, false, {c_cic,c_avg});

        ArInts ar_new_has = has + ArInts({c_cic,c_avg});

        ArInts ar_new_resi = resi;
        ar_new_resi.remove(i);

        pair<double,ArInts> lev = recu_sel_best_seq_cicling(new_ar, ar_new_has, ar_new_resi);

        if (lev.first < best.first)
            best = lev;
    }

    return best;
}

// здесь тестируются данные только до первой прогнозируемой позиции,
//   сюда не попадают данные за границу прогноза (см.точку вызова)
Cicling ForecastingMain::sel_best_seq_cicl(const ArNums& ar_data) {

    qDebug() << "====================================================";

    Dispersion disp(ar_data);
    qDebug() << "Исходный график, сред.квадр.откл от нуля" << disp.dev_zero()
             << "средняя" << disp.mean()
             << "станд.откл" << disp.stddev();

    ArNums ar_1 = ar_data;
    if (ar_data.size() > MA_LEN_TEST_DATA)
        ar_1 = ar_data.mid(ar_data.size()-MA_LEN_TEST_DATA);

    ArInts ar_cic_for_test = cfg.ar_periods_cicling;

    pair<double,ArInts> p_best = recu_sel_best_seq_cicling(ar_1, {}, ar_cic_for_test);

    Cicling res(p_best.second);

    Cicling cpy = res;
    cpy.add_values(ar_data, false);
    cpy.present();

    return res;
}

