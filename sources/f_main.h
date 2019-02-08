
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef F_MAIN_H
#define F_MAIN_H

#include <QtCore>

#include "cicling.h"

#include "tls.h"
#include "corr_slide.h"

// !!! при компиляции с этим дефайном
//  при выполнении
//  необходимо увеличивать значение "Количество лучших подобий"
//  в два-три раза.
// #define MAKE_ONLY_DOUBLE_BEST

// при тестировании цикличностей,
//   от тестовых данных берется не более чем это к-во,
//    в случае если этих тестовых данных больше
#define MA_LEN_TEST_DATA 50000

// диапазон тестирования периода адаптации цикличностей
#define MI_TEST_C_AVG_FOR_CICLING -32
#define MA_TEST_C_AVG_FOR_CICLING 128

// максимальный период сканируемых цикличностей
//   при поиске цикличностей
#define MA_TEST_C_CICLING 2000

//************************************************
struct Cfg {
    QString file_with_data_or_dir;

    QString col_for_forecast;
    QString col_date_time;

    QString date_time_for_forecast_or_position;

    CorrSlide::KindCorr meth_corr;

    int detail_out_info = 0;
    int st_len_corr = 0;
    int cnt_len_corr = 0;

    int cnt_best = 0;
    int cnt_outlier = 0;
    int cnt_steps_forecast = 0;
    int cnt_one_step_forecast = 0;

    ArInts ar_periods_cicling;

    int period = 0;

    bool use_comma_instead_dot = false;
};

class ForecastingMain : public QObject {

    Q_OBJECT

public:
    QSemaphore signal_stop;

    Cfg cfg;

    enum ProcessMode {
        calc_forecast,
        calc_cicling,
    };

    ProcessMode mode_scan_cicling = calc_forecast;

    bool has_signal_stop() const { return signal_stop.available()>0; }

    void to_do_main_process();

    static int find_pos_st_forecast(
            const QString& pos_dt,
            const QString& col_date_time,
            const ArNums& ar_data_cotir,
            const QVector<QString>& ar_date_time);

    void scan_best_cicling(const ArNums& ar);
    int cicling_sel_best_avg(const ArNums& ar, int c_cic, double* res_dev);

    pair<double,ArInts> recu_sel_best_seq_cicling(const ArNums& ar, const ArInts& has, const ArInts& resi);

    Cicling sel_best_seq_cicl(const ArNums& ar);

public slots:
    void run_forecasting();
    void stop_process() { signal_stop.release(1); }
};

// для корректировки под правильную последовательность по датам и не друблирование и не пропуски
struct PrepareByDate {

    // до обработки на корректность дат
    int raw_st_forecast = 0;
    ArNums ar_raw_cotir;
    QVector<QString> ar_raw_date_time;

    // после обработки
    int st_forecast = -1;
    ArNums ar_cotir;
    QVector<QDateTime> ar_date_time;

    // массив конвертации позиций из старых позиций в новые
    ArInts ar_pos_conv;

    // массив прогнозируемых данных, но обрезанный до первой позиции прогноза
    mutable ArNums ar_cotir_for_test; // см. get_ar_cotir_for_test()

    //******
    void prepare_by_date_time(int period) { prepare_by_date_time(*this, period); }
    static void prepare_by_date_time(PrepareByDate& prepare, int period);

    QString get_date_time_by_pos(int i_pos) const;
    int get_st_forecast() const { return (st_forecast<0?raw_st_forecast:st_forecast); }

    const ArNums& get_ar_cotir() const;
    const ArNums& get_ar_cotir_for_test() const;
};

// управление данными для прогнозирования
struct ForecastData {

    // Данные до позиции прогноза. Не включают в себя прогнозируемое значение.
    //  значения за вычетом цикличности.
    ArNums ar_data_test;
    ArNums ar_rev_data_test; // реверсированный массив ar_data_test

    //+++
    //double dbg_get_any(int i_pos) const { return ar_data_real.at(i_pos); }

    int c_test_data() const { return ar_data_test.size(); }
    int i_forecast() const { return c_test_data()-1; }
    int ma_i_forecast() const { return ar_data_real.size()-1; }

    ForecastData(const ArNums& ar_src, const Cicling& conv, int i_first_forecast);
    void   move_forecast_point();

    bool   has_fact(int i_forward) const { return has_fact(i_forward,0); }
    bool   has_fact(int i_forward, int i_forecast_correction) const;

    double get_fact(int i_forward) const { return get_fact(i_forward,0); }
    double get_fact(int i_forward, int i_forecast_correction) const;

    double get_old_fact(int i_pos, int i_forward) const;

    const Cicling& get_copy_convertor() const { return convertor; }

private:
    // вектор данных со всеми значениями, обращение к нему через метод get_fact
    //   что гарантирует, что методы прогнозирования не могут залезть в эти данные
    //   по какой-либо случайности.
    ArNums ar_data_real;
    Cicling convertor;
};

//*************************************************************************
// процесс прогнозирования

struct OrdNode {
    double koef = 0;
    double a = 0;
    double b = 0;
    int i_pos = -1;
};

struct ForecastNode {

    bool has_calc = false;
    bool has_fail = false;

    int len_corr = 0;
    QString present;

    QVector<OrdNode> ar_pos;
    int c_true_pos = 0;

    QVector<OrdNode*> ar_best_pos;
};

struct ForecastRes {
    bool has_fail = false;
    QString node_present;
    int    c_best_pos = 0;
    int    c_all_pos = 0;
    double mean_all = 0;
    double dev_all = 0;
    double koef_forecast = 0;
    double res_forecast = 0;
    double dev_forecast = 0;
    int    i_forward;
};

struct ForecastCalc {

    Cfg cfg;

    bool has_init = false;
    bool has_calc = false;

    int i_forecast;

    int ma_len_forecast;

    QVector<char> ar_outlier;

    ArInts ar_len_corr;
    int ma_len_corr;

    // size == ar_len_corr.size()*2 - 1
    QVector<ForecastNode> ar_nodes;

    void calc_corr(
            ForecastData& dat,
            int cnt_best,
            int cnt_outlier,
            ArInts& ar_len_corr,
            int ma_len_forecast,
            CorrSlide::KindCorr kc);

    ForecastRes get_res(ForecastData& dat, int i_forward);
};

#endif // F_MAIN_H
