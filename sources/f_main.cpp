
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "f_main.h"

#include "corr_slide.h"
#include "file_loader.h"

void ForecastingMain::run_forecasting() {

    qint64 t1 = QDateTime::currentMSecsSinceEpoch();

    try {
        to_do_main_process();
    } catch (std::exception& e) {
        qDebug() << "Exception:" << e.what();
    } catch (const char* s) {
        qDebug() << "Exception:" << s;
    } catch (...) {
        qDebug() << "Exception";
    }

    qint64 t2 = QDateTime::currentMSecsSinceEpoch();

    qDebug() << "=============================================";
    qDebug() << "Время выполнения, сек" << dbl(t2-t1)/1000.0;

    thread()->quit();
}

void ForecastingMain::to_do_main_process() {

    //******************************************************************
    // длины отрезков замеряемой корреляции, cnt штук, каждый последующий больше в два раза
    ArInts ar_len_corr;
    for (int i = 0, len = cfg.st_len_corr; i < cfg.cnt_len_corr; ++i, len *= 2)
        ar_len_corr.append(len);

    //******************************************************************
    // считывание данных из файла котировок
    FileLoader file_loader;
    file_loader.load_files(cfg.file_with_data_or_dir);

    if (file_loader.count_loaded_data == 0) {
        qDebug() << "ОТСУТСТВУЮТ ДАННЫЕ ДЛЯ ПРОГНОЗИРОВАНИЯ";
        return;
    }

    PrepareByDate prepare_data;

    prepare_data.ar_raw_cotir = file_loader.get_col_nums(cfg.col_for_forecast);
    if (cfg.col_date_time.isEmpty()==false)
        prepare_data.ar_raw_date_time = file_loader.get_cat_cols(cfg.col_date_time, " ");

    //******************************************************************
    // ОПРЕДЕЛЕНИЕ ПОЗИЦИИ ПРОГНОЗА ПО ВРЕМЕННОЙ МЕТКЕ, ИЛИ ПОЗИЦИЯ, ЕСЛИ ЗАДАНО ЧИСЛОМ
    //   или прогноз самой последней позиции, если поле не заполнено
    // результат = 0..ar_date_time.size()-1
    prepare_data.raw_st_forecast = find_pos_st_forecast(
                cfg.date_time_for_forecast_or_position,
                cfg.col_date_time,
                prepare_data.ar_raw_cotir,
                prepare_data.ar_raw_date_time);

    //******************************************************************
    // ОБРАБОТКА КОРРЕКТНОСТИ ПОСЛЕДОВАТЕЛЬНОСТИ ДАТЫ-ВРЕМЕНИ
    if (cfg.col_date_time.isEmpty()==false && cfg.period != 0)
        prepare_data.prepare_by_date_time(cfg.period);

    //******************************************************************
    // определение лучших цикличностей
    Cicling conv = sel_best_seq_cicl(prepare_data.get_ar_cotir_for_test());

    if (mode_scan_cicling == calc_cicling) {
        scan_best_cicling(conv.add_values(prepare_data.get_ar_cotir_for_test(), false));
        return;
    }

    //******************************************************************
    // объект исходных данных для прогнозирования
    ForecastData dat(prepare_data.get_ar_cotir(), conv, prepare_data.get_st_forecast());

    qDebug() << "================================";

    auto fn_mean_if = [](Dispersion& d) -> QString {
        if (d.cnt_values==0) return "---";
        return procent(d.mean()); };

    Dispersion disp_avg_mape;
    Dispersion disp_avg_mape_by_cicl;
    Dispersion disp_avg_mape_by_prev;

    // ОСНОВНОЙ ЦИКЛ ПРОГНОЗИРОВАНИЯ
    for (int i_stp = 0; i_stp < cfg.cnt_steps_forecast || cfg.cnt_steps_forecast==0; ++i_stp) {
        if (has_signal_stop()) break;

        if (i_stp > 0) {
            if (dat.i_forecast() == dat.ma_i_forecast())
                break;
            dat.move_forecast_point();
        }

        // прогноз с коррекцией позиции -1
        //   т.е. последнее известное значение
        double fact_1 = dat.get_fact(1, -1);

        Cicling conv_cpy = dat.get_copy_convertor();
        Cicling conv_cpy_0 = dat.get_copy_convertor();

        ForecastCalc forecasting;
        forecasting.cfg = cfg;

        if (cfg.meth_corr != CorrSlide::without)
            forecasting.calc_corr(dat, cfg.cnt_best, cfg.cnt_outlier, ar_len_corr, cfg.cnt_one_step_forecast, cfg.meth_corr);

        for (int i_forward = 1; i_forward <= cfg.cnt_one_step_forecast; ++i_forward) {

            double fore_minus_cicling = 0;
            double dev_minus_cicling = 0;

            // расчет прогноза по корреляциям
            ForecastRes res;
            if (cfg.meth_corr != CorrSlide::without) {
                res = forecasting.get_res(dat, i_forward);
                fore_minus_cicling = res.res_forecast;
                dev_minus_cicling = res.dev_forecast;
            }

            double fore = conv_cpy.conv_back(fore_minus_cicling);
            double f_ma_99 = conv_cpy.conv_back(fore_minus_cicling + dev_minus_cicling*3.0);
            double f_mi_99 = conv_cpy.conv_back(fore_minus_cicling - dev_minus_cicling*3.0);

            // это нужно если cnt_one_step_forecast больше единицы
            conv_cpy.add_value(fore);

            // расчет прогноза только по цикличности
            double fore_cicl = conv_cpy_0.conv_back(0);
            conv_cpy_0.add_value(fore_cicl);

            // получение фактического значения
            bool has_fact = dat.has_fact(i_forward);
            double fact = 0;
            QString s_fact = "---";
            if (has_fact) {
                fact = dat.get_fact(i_forward);
                s_fact = QString::number(fact);
            }

            // расчет и учет процентных ошибок
            double ape=0, ape_cicl=0, ape_1=0;
            if (has_fact) {
                // ошибка прогноза absolute percentage error
                ape = safe_div(std::abs(fore - fact), fact);
                disp_avg_mape.add_value(ape);
                // ошибка прогноза только по цикличности
                ape_cicl = safe_div(std::abs(fore_cicl - fact), fact);
                disp_avg_mape_by_cicl.add_value(ape_cicl);
                // ошибка прогноза по последней известной позиции
                ape_1 = safe_div(std::abs(fact_1 - fact), fact);
                disp_avg_mape_by_prev.add_value(ape_1);
            }

            QString dt = prepare_data.get_date_time_by_pos(dat.i_forecast());
            if (cfg.cnt_one_step_forecast>1)
                dt = dt+"+"+QString::number(i_forward);
            dt = "("+dt+")";


            if (cfg.detail_out_info == 0) {
                // минимальный вывод:
                // позиция,дата,последний_известный_факт,прогноз,ми99,ма99,факт
                qDebug()
                        << i_stp << dt
                        << "last-known-fact" << fact_1
                        << "forecast" << fore
                        << "mi-ma-99%" << f_mi_99 << f_ma_99
                        << "fact" << s_fact;
            } else {
                qDebug()
                        << i_stp << dt
                        << "last-known-fact" << fact_1
                        << "forecast" << fore
                        << "mi-ma-99%" << f_mi_99 << f_ma_99
                        << "fact" << s_fact
                        << "koef_forecast" << res.koef_forecast
                        << "node" << res.node_present
                        << "c_best" << res.c_best_pos
                        << "c_all" << res.c_all_pos
                        << "ape" << procent(ape)
                        << "ape_cicl" << procent(ape)
                        << "ape_prev" << procent(ape_1)
                        << "mape" << fn_mean_if(disp_avg_mape)
                        << "mape_cicl" << fn_mean_if(disp_avg_mape_by_cicl)
                        << "mape_prev" << fn_mean_if(disp_avg_mape_by_prev);
            }
        }
    }

    //****************************************************************
    // ВЫВОД ИТОГОВОЙ ИНФОРМАЦИИ

    qDebug() << "=============================================";
    qDebug() << "средняя ошибка за выбранный период" << fn_mean_if(disp_avg_mape);
    qDebug() << "сред.ошиб только по цикличности" << fn_mean_if(disp_avg_mape_by_cicl);
    qDebug() << "сред.ошиб при условии что прогноз это последняя известная позиция"
             << fn_mean_if(disp_avg_mape_by_prev);

    auto fn_devzero_if = [](Dispersion& d) -> QString {
        if (d.cnt_values <= 1) return "---";
        return procent(d.dev_zero());};

    qDebug() << "=============================================";
    qDebug() << "среднее квадратичное отклонение процентов ошибок от нуля подчеркивает максимальные ошибки";
    qDebug() << "сред.квадр.ошибка за выбранный период" << fn_devzero_if(disp_avg_mape);
    qDebug() << "сред.квадр.ошиб только по цикличности" << fn_devzero_if(disp_avg_mape_by_cicl);
    qDebug() << "сред.квадр.ошиб при условии что прогноз это последняя известная позиция"
             << fn_devzero_if(disp_avg_mape_by_prev);
}

