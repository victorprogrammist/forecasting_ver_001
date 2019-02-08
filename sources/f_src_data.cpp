
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "f_main.h"

ForecastData::ForecastData(const ArNums& ar_src, const Cicling& conv, int i_first_forecast) {

    myassert( conv.c_values == 0 );

    convertor = conv;
    ar_data_real = ar_src;

    int c = i_first_forecast + 1;
    if (c > ar_src.size())
        do_throw("Позиция прогноза за пределами доступных данных");

    ar_data_test = convertor.add_values( ar_src.mid(0, c), true );
    ar_rev_data_test.resize(c);

    for (int i = 0; i < c; ++i)
        ar_rev_data_test[i] = ar_data_test.at(c-1-i);
}

void ForecastData::move_forecast_point() {

    int i_new_f = i_forecast() + 1;
    myassert( i_new_f < ar_data_real.size() );

    double v = ar_data_real.at(i_new_f);
    double v_converted = convertor.add_conv(v);

    ar_data_test.append(v_converted);
    ar_rev_data_test.insert(0, v_converted);
}

bool   ForecastData::has_fact(int i_forward, int i_forecast_correction) const {

    int i_f = i_forecast();

    myassert( i_forward > 0 );
    myassert( i_forecast_correction <= 0 && i_f + i_forecast_correction >= 0 );

    return i_f + i_forecast_correction + i_forward < ar_data_real.size();
}

double ForecastData::get_fact(int i_forward, int i_forecast_correction) const {

    myassert( has_fact(i_forward,i_forecast_correction) );

    return ar_data_real.at( i_forecast() + i_forecast_correction + i_forward );
}

// выдает значения из массива тестовых данных
//   следует учитывать, что в текущей версии эти знаения обработанные через convertor
// i_pos - это позиция взятия прогноза для старых тестовых данных
double ForecastData::get_old_fact(int i_pos, int i_forward) const {

    myassert( i_forward > 0 );
    // i_forecast() - позиция прогноза, позиция последних известных данных (тестовые)
    myassert( i_pos >= 0 && i_pos + i_forward <= i_forecast() );

    return ar_data_test.at(i_pos + i_forward);
}

