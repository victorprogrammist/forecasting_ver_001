

/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#include "cicling.h"

void Cicling::private_recu_present(int lev) const {
    qDebug() << "=== цикличность уровень" << lev << "===";

    if (size_cicling == 0) {
        qDebug() << "пустой объект, без обработки";

    } else {

        qDebug() << "длина цикла:" << size_cicling;
        qDebug() << "длина угасания/перестраивания в циклах:" << c_adapt;
        qDebug() << "количество примененных циклов:" << c_values/size_cicling;

        if (c_values >= size_cicling) {

            double su = 0;
            for (auto& m : ar_means)
                su += std::abs(m.mean());

            qDebug() << "последняя корректировка ошибки средняя за цикл" << su/dbl(size_cicling);
            qDebug() << "(при перестраивании близких или меньших 1, это скорее последняя корректировка, а не средняя)";
            qDebug() << "остающаяся сред.квадр.ошибка" << private_avg_er();

        } else
            qDebug() << "недостаточно статистики по цикличности для полной информации";
    }

    if (ne_level)
        ne_level->private_recu_present(lev+1);
}

const Cicling& Cicling::operator=(const Cicling& oth) {
    if (ne_level) delete ne_level;
    ne_level = nullptr;

    size_cicling = oth.size_cicling;
    c_adapt = oth.c_adapt;
    ar_means = oth.ar_means;
    c_values = oth.c_values;

    er_su_pow2 = oth.er_su_pow2;

    if (oth.ne_level)
        ne_level = new Cicling(*oth.ne_level);

    return *this;
}

void Cicling::init(int c_stp, int c_avg) {
    myassert( (c_stp > 0 && c_avg != 0) || (c_stp == 0 && c_avg == 0) );

    // !!! ne_level не изменяет

    size_cicling = c_stp;
    c_adapt = c_avg;

    ar_means.fill(MeanAdapt(c_avg), c_stp);

    c_values = 0;

    er_su_pow2 = 0;
}

// добавляет значение,
//   в res возвращает оставшуюся ошибку,
//   return bool истина, если уже есть хотя бы один цикл,
//   и res можно посчитать, иначе res = 0;
bool Cicling::private_recu_add_value(double v, double src_v, double& res) {

    double res_res = 0;

    bool has_res = true;

    // если нулевая цикличность, то это пустой объект - заглушка,
    //   и на выход выдается то, что поступает на вход
    if (size_cicling == 0) {
        res_res = v;
        er_su_pow2 += v*v;
    } else {

        MeanAdapt& d = ar_means[ c_values % size_cicling ];
        has_res = d.cnt_vals > 0;

        if (has_res) {
            double m = d.mean();
            res_res = v - m;
            er_su_pow2 += res_res * res_res;
        }

        d.add_value(v);
    }

    ++c_values;

    if (ne_level && has_res)
        return ne_level->private_recu_add_value(res_res, src_v, res);
    else
        res = res_res;

    return has_res;
}

double Cicling::conv_back(double v) const {
    myassert( has_cicle() );

    if (ne_level)
        v = ne_level->conv_back(v);

    if (size_cicling == 0)
        return v;

    return v + ar_means.at( c_values % size_cicling ).mean();
}

ArNums Cicling::add_values(const ArNums& ar_data, bool add_not_ready) {

    ArNums ar_res;
    for (double v : ar_data) {

        double v_res;
        if (add_value(v, v_res) == false && add_not_ready == false)
            continue;

        ar_res.append(v_res);
    }

    ar_res.squeeze();
    return ar_res;
}

double Cicling::add_conv(double v) {
    double res;
    add_value(v, res);
    return res;
}

double Cicling::avg_er_by_vector(const ArNums& ar_data, const ArInts& ar_init) {
    Cicling cic(ar_init);
    for (double v : ar_data) cic.add_value(v);
    return cic.avg_er();
}

ArNums Cicling::process_vector(const ArNums& ar_data, bool add_not_ready, const ArInts& ar_init) {
    Cicling cic(ar_init);
    return cic.add_values(ar_data, add_not_ready);
}

Cicling::Cicling(const ArInts& ar_init) {
    if (ar_init.empty()) return;

    myassert( ar_init.size() >= 2 );

    init(ar_init.at(0), ar_init.at(1));

    if (ar_init.size()>2) ne_level = new Cicling(ar_init.mid(2));
}
