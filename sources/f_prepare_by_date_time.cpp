
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "f_main.h"

int ForecastingMain::find_pos_st_forecast(
        const QString& pos_dt,
        const QString& col_date_time,
        const ArNums& ar_data_cotir,
        const QVector<QString>& ar_date_time
        ) {

    QString tm_present;

    int pos_st_forecast = 0;
    if (pos_dt.isEmpty() || pos_dt == "0")
        // позиция прогноза - самая последняя позиция
        pos_st_forecast = ar_data_cotir.size();

    else if (pos_dt.contains(QRegExp("^[1-9][0-9]*$")))
        // позиция указана числом
        pos_st_forecast = pos_dt.toInt();

    else if (col_date_time.isEmpty()==false) {
        // поиск по колонке даты
        myassert( ar_date_time.size() == ar_data_cotir.size() );
        pos_st_forecast = ar_date_time.indexOf(pos_dt)+1;
        if (pos_st_forecast == 0) do_throw("Дата/время не найдено: "+pos_dt);

        tm_present = "("+pos_dt+")";

    } else
        // попытка конвертировать то что указали к числу, типа позиция
        pos_st_forecast = pos_dt.toInt();

    if (pos_st_forecast <= 0 || pos_st_forecast > ar_data_cotir.size())
        do_throw("Неверная позиция прогнозирования: "+QString::number(pos_st_forecast));

    qDebug() << "Прогнозирование позиции" << pos_st_forecast << tm_present << "из" << ar_data_cotir.size();

    return pos_st_forecast - 1;
}

QString PrepareByDate::get_date_time_by_pos(int i_pos) const {
    if (ar_raw_date_time.size() == 0) {
        myassert( i_pos >= 0 && i_pos < ar_raw_cotir.size() );
        return QString::number(i_pos+1);
    }
    if (st_forecast<0)
        return ar_raw_date_time.at(i_pos);
    return ar_date_time.at(i_pos).toString("dd.MM.yyyy hh:mm"); }

const ArNums& PrepareByDate::get_ar_cotir() const {
    // если корректировка по дате-времени не производилась, то исходный массив
    if (st_forecast<0) return ar_raw_cotir;
    // иначе откорректированный
    return ar_cotir; }

const ArNums& PrepareByDate::get_ar_cotir_for_test() const {
    if (ar_cotir_for_test.size()==0)
        ar_cotir_for_test = get_ar_cotir().mid(0, get_st_forecast()+1);
    return ar_cotir_for_test;
}

// проверяет и корректирует данные по колонке дата,
//   последовательность, полноту, и не дублирование
//   это важно для замера цикличностей
//   в случае дублирования или преобразования из мелкой периодичности в более крупную,
//      в результатирующий массив пойдет среднее от значений
//   в случае отсутствия, в рез массив пойдет значение предшествующее отсутствующему
//   prepare.ar_pos_conv является массивом конвертации, по индекусу старой позиции из него получаем позиции нового массива
void PrepareByDate::prepare_by_date_time(PrepareByDate& prepare, int period) {

    myassert( prepare.ar_raw_cotir.size() == prepare.ar_raw_date_time.size() );

    qDebug() << "====================================================";
    qDebug() << "обработка колонки даты, дополнение и убирание лишних";

    QString fmt = "yyyyMMdd hhmmss";
    QString s = prepare.ar_raw_date_time.at(0);
    if (s.indexOf("/") >= 0)
        fmt = "M/d/yyyy h:mm";
    else if (s.indexOf(".") >= 0)
        fmt = "d.M.yyyy h:mm";

    QVector<QDateTime> ar_dt(prepare.ar_raw_date_time.size());

    //*********
    // сконвертировали строку даты-время к бинарному представлению
    for (int i = 0; i < prepare.ar_raw_date_time.size(); ++i) {
        QDateTime dd = QDateTime::fromString(prepare.ar_raw_date_time.at(i), fmt);
        dd.setTimeSpec(Qt::UTC);
        ar_dt[i] = dd;
    }

    //*********
    // составляет сортированный индекс по всем датам
    std::multimap<QDateTime,int> mmap_pos;

    for (int i = 0; i < ar_dt.size(); ++i) {
        QDateTime d = ar_dt.at(i);

        QDateTime dd;
        if (period == 24*60*60)
            dd = QDateTime(d.date(), QTime(0,0,0));
        else if (period == 60*60)
            dd = QDateTime(d.date(), QTime(d.time().hour(),0,0));
        else if (period == 60)
            dd = QDateTime(d.date(), QTime(d.time().hour(),d.time().minute(),0));
        else
            do_throw("Не предусмотренная периодичность: "+QString::number(period));

        dd.setTimeSpec(Qt::UTC);
        mmap_pos.insert({dd,i});
    }

    //*********
    // сам процесс конвертации
    QDateTime mi_dt = mmap_pos.begin()->first;

    prepare.ar_pos_conv.fill(-1, prepare.ar_raw_date_time.size());
    prepare.ar_cotir.clear();
    prepare.ar_date_time.clear();

    double pre_v = 0;
    int len_without = 0;
    int ma_len_without = 0;
    int ma_len_avg = 0;
    while (mmap_pos.empty()==false) {

        Dispersion current;
        while (mmap_pos.empty()==false) {
            QDateTime d = mmap_pos.begin()->first;
            myassert( d >= mi_dt );
            if (d > mi_dt) break;
            // суммирует котировки, если на одну результатирующую строку приходится несколько исходных
            int i_pos = mmap_pos.begin()->second;
            current.add_value(prepare.ar_raw_cotir.at(i_pos));
            mmap_pos.erase(mmap_pos.begin());
            prepare.ar_pos_conv[i_pos] = prepare.ar_cotir.size();
        }

        double v = pre_v;
        if (current.empty() == false) {
            v = current.mean();
            len_without = 0;
        } else
            // использует предыдущее значени котировки, если на эту дату-время не было
            ++len_without;

        ma_len_without = std::max(ma_len_without, len_without);
        ma_len_avg = std::max(ma_len_avg, current.cnt_values);

        prepare.ar_cotir.append(v);
        prepare.ar_date_time.append(mi_dt);
        pre_v = v;
        mi_dt = mi_dt.addSecs(period);
    }

    myassert( prepare.ar_cotir.size() == prepare.ar_date_time.size() );

    qDebug() << "строк до обработки" << prepare.ar_raw_date_time.size() << "после" << prepare.ar_cotir.size();
    qDebug() << "максимальный интервал без котировок" << ma_len_without;
    qDebug() << "максимальное усреднение по идентичным датам" << ma_len_avg;

    prepare.ar_cotir.squeeze();
    prepare.ar_date_time.squeeze();
    prepare.st_forecast = prepare.ar_pos_conv.at(prepare.raw_st_forecast);
}
