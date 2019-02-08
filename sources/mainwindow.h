
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <QIntValidator>

#include "f_main.h"

namespace Ui {
class MainWindow;
}

class MyValidator : public QRegExpValidator {
    Q_OBJECT

    QString fixup_value;

public:
    explicit MyValidator(QObject* parent= nullptr)
        : QRegExpValidator(parent){}

    MyValidator(const QRegExp &rx, const QString& fxv, QObject* parent=nullptr)
        : QRegExpValidator(rx, parent), fixup_value(fxv) {}

    virtual void fixup(QString& input) const override {
        input = fixup_value; }

    void set_fixup_value(const QString& fxv) { fixup_value = fxv; }
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    MyValidator validator_int_from_one;
    MyValidator validator_int_from_four;
    QIntValidator validator_int_from_zero;

    MyValidator validator_periods_of_cicling;

    QtMessageHandler original_message_handler;
    static void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

    Cfg make_params(bool mk_out_params);
    void load_settings(bool do_clear);
    void start_process(ForecastingMain::ProcessMode mode_scan_cicling);

private slots:
    void on_bt_start_clicked();
    void after_finish_forecasting();
    void add_msg_log(QString msg);

    void on_bt_select_file_clicked();

    void on_bt_exit_clicked();

    void on_bt_reset_settings_clicked();

    void on_bt_scan_cicling_clicked();

    void on_help_clicked();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
