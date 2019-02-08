/*
 * Author Telnov Victor, v-telnov@yandex.ru
 *
 */

#ifndef CICLING_H
#define CICLING_H

#include "dispersion.h"

struct Cicling {

    // вектор для накопления средних, размерностью size_cicling
    QVector<MeanAdapt> ar_means;

    int    size_cicling = 0;

    // продолжительность подстраивания MeanAdapt в ar_means
    int    c_adapt = 0;

    // количество добавленных значений
    int    c_values = 0;

    // сумма накопленной ошибки, прежде чем добавить новое значение к средне,
    //   расчитывается разница между новым значением и текущей средней.
    double er_su_pow2 = 0;

    // следующий уровень обработки цикличностей, если конструировали многоуровневую цикличность
    Cicling* ne_level = nullptr;

    void present() const { private_recu_present(1); }

    // накопленная ошибка, считающаяся от последнего уровня цикличностей
    double avg_er() const { return (ne_level ? ne_level->avg_er() : private_avg_er()); }

    const Cicling& operator=(const Cicling& oth);

    // если уже есть хотя бы один цикл значений
    bool has_cicle() const { return c_values >= size_cicling; }

    // если уже есть циклы значений больше чем продолжительность подстраивания
    bool has_full() const { return c_values >= size_cicling*c_adapt; }

    // начальная инициализация, или переинициализация
    void init(int c_stp, int c_avg);

    // разные варианты добавления значений, все они в итоге ведут в private_recu_add_value
    void add_value(double v) { double dummy; add_value(v,dummy); }
    bool add_value(double v, double& res) { return private_recu_add_value(v, v, res); }
    ArNums add_values(const ArNums& ar_data, bool add_not_ready);
    double add_conv(double v); // возвращает ошибку

    // обратная конвертация из ошибки в значение,
    //   из которого она могла быть получена
    double conv_back(double v) const;

    // обработка вектора без создания объекта в пользовательском коде
    static ArNums process_vector(const ArNums& ar_data, bool add_not_ready, const ArInts& ar_init);
    static double avg_er_by_vector(const ArNums& ar_data, const ArInts& ar_init);

    //***************************
    Cicling() {}
    Cicling(const Cicling& oth) { *this = oth; }
    Cicling(int c_stp, int c_avg) { init(c_stp, c_avg); }
    Cicling(const ArInts& ar_init);
    ~Cicling() { if (ne_level) delete ne_level; }

private:
    bool private_recu_add_value(double v, double src_v, double& res);

    double private_avg_er() const { return sqrt(private_avg_er_pow2()); }

    double private_avg_er_pow2() const {
        if (c_values < size_cicling) return 0;
        return er_su_pow2 / dbl(c_values - size_cicling);
    }

    void private_recu_present(int lev) const;

};

#endif // CICLING_H
