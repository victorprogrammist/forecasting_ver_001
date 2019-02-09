
/*
 * Author Telnov Victor, v-telnov@yandex.ru
 *
 */

#include <QFileDialog>

#include <QMessageBox>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tls.h"

QString app_path() {
    QString sp = QDir::separator();
    QString s = QApplication::applicationDirPath().replace("/", sp).replace("\\", sp);
    if (s.right(1) != sp) s += sp;
    return s;
}

Cfg MainWindow::make_params(bool mk_out_params) {

    QSettings settings(app_path()+"cfg.ini", QSettings::IniFormat);
    settings.setIniCodec("Windows-1251");
    settings.setValue("window_size", size());
    settings.setValue("window_pos", pos());

    auto param = [&settings,&mk_out_params](auto v, const QString& name) {
        if (mk_out_params)
            qDebug() << "param[ " + name + " ]=" << v;
        settings.setValue(name, v);
        return v; };

    Cfg cfg;
    cfg.file_with_data_or_dir = param(ui->ed_file_data->text(), "ФайлИлиКаталогДанных");
    cfg.col_for_forecast = param(ui->ed_col_forecast->text(), "ПрогнозируемаяКолонка");
    cfg.col_date_time = param(ui->ed_cols_date_time->text(), "КолонкаДатаВремя");
    cfg.date_time_for_forecast_or_position = param(ui->ed_start_date_time->text(), "ПозицияДляПрогноза");

    cfg.detail_out_info = param(ui->spin_level_out_info->value(), "ДетализацияВыводаИнформации");

    cfg.st_len_corr = param(ui->ed_len_first_corr->text().toInt(), "ПерваяДлинаКорреляций");
    cfg.cnt_len_corr = param(ui->ed_qty_corr->text().toInt(), "КоличествоДлинКорреляций");

    cfg.cnt_best = param(ui->ed_count_best->text().toInt(), "КоличествоЛучшихКорреляций");
    cfg.cnt_outlier = param(ui->ed_cnt_outlier->text().toInt(), "КоличествоИсключаемыхВыбросов");

    cfg.cnt_steps_forecast = param(ui->ed_steps_forecast->text().toInt(), "КоличествоПрогнозируемыхШагов");
    cfg.cnt_one_step_forecast = param(ui->ed_qty_one_forecast->text().toInt(), "КоличествоШаговВпередОтПозицииПрогноза");

    cfg.ar_periods_cicling = string_to_ar_ints(param(ui->ed_periods_cicling->text(), "ПериодыЦикличности"));

    QString s_meth;
    if (ui->rb_1_pirsona->isChecked()) {
        cfg.meth_corr = CorrSlide::corr_Pearson;
        s_meth = "КорреляцияПрисона";

    } else if (ui->rb_2_diff->isChecked()) {
        cfg.meth_corr = CorrSlide::simple_diff;
         s_meth = "ПростоеКвадратичноеОтклонение";

    } else if (ui->rb_3_diff_sub_mean->isChecked()) {
        cfg.meth_corr = CorrSlide::diff_sub_mean;
        s_meth = "КвадратичноеОтклонениеВыровненноеПоСреднейОтрезков";

    } else if (ui->rb_5_diff_zero->isChecked()) {
        cfg.meth_corr = CorrSlide::diff_sub_zero;
        s_meth = "КвадратичноеОтклонениеВыровненноеПоПоследнемуЗначениюОтрезков";

    } else if (ui->rb_6_only_cicling->isChecked()) {
        cfg.meth_corr = CorrSlide::without;
        s_meth = "БезСравненияОтрезковТолькоЦикличность";

    } else if (ui->rb_4_diff_norm->isChecked()) {
        cfg.meth_corr = CorrSlide::diff_div_mean;
        s_meth = "КвадратичноеОтклонениеДелительСредняяОтрезков";
    }

    param(s_meth, "МетодКорреляции");

    QString s_period;
    if (ui->rb_1_days->isChecked()) {
        cfg.period = 24*60*60;
        s_period = "Дни";
    } if (ui->rb_2_hours->isChecked()) {
        cfg.period = 60*60;
        s_period = "Часы";
    } else if (ui->rb_3_minutes->isChecked()) {
        cfg.period = 60;
        s_period = "Минуты";
    } else if (ui->rb_4_without_period->isChecked()) {
        cfg.period = 0;
        s_period = "БезВыравнивания";
    }

    param(s_period, "ПериодВыравнивания");

    cfg.use_comma_instead_dot = ui->checkBox_UseComma->checkState() == Qt::Checked;
    param( (cfg.use_comma_instead_dot?"true":"false"), "ИспользоватьЗапятуюДляВыводаЧисел");

    return cfg;
}

void MainWindow::load_settings(bool do_clear) {

    QSettings settings(app_path()+"cfg.ini", QSettings::IniFormat);
    settings.setIniCodec("Windows-1251");

    if (do_clear) settings.clear();

    auto get_param = [&settings](const QString& name, const QString& def) {
        if (settings.contains(name) == false)
            return def;
        return settings.value(name).toString();
    };

    auto load = [&get_param](QLineEdit* ed, const QString& name, const QString& def) {
        ed->setText(get_param(name,def));
    };

    ui->spin_level_out_info->setValue(get_param("ДетализацияВыводаИнформации", "0").toInt());

    load(ui->ed_file_data, "ФайлИлиКаталогДанных", "");

    load(ui->ed_col_forecast, "ПрогнозируемаяКолонка", "Value");
    load(ui->ed_cols_date_time, "КолонкаДатаВремя", "DateTime");
    load(ui->ed_start_date_time, "ПозицияДляПрогноза", "9/1/2012 23:00");

    load(ui->ed_len_first_corr, "ПерваяДлинаКорреляций", "6");
    load(ui->ed_qty_corr, "КоличествоДлинКорреляций", "8");
    load(ui->ed_count_best, "КоличествоЛучшихКорреляций", "100");
    load(ui->ed_cnt_outlier, "КоличествоИсключаемыхВыбросов", "100");

    load(ui->ed_steps_forecast, "КоличествоПрогнозируемыхШагов", "100");
    load(ui->ed_qty_one_forecast, "КоличествоШаговВпередОтПозицииПрогноза", "1");

    load(ui->ed_periods_cicling, "ПериодыЦикличности", "1, 24, 168");

    bool fl = get_param("ИспользоватьЗапятуюДляВыводаЧисел", "false")!="false";
    ui->checkBox_UseComma->setCheckState( fl ? Qt::Checked : Qt::Unchecked );


    auto set = [&settings](QRadioButton* rb, const QString& name, const QString& val, bool def) {
        if (settings.contains(name)==false)
            rb->setChecked(def);
        else
            rb->setChecked(settings.value(name).toString()==val);
    };

    set(ui->rb_1_days, "ПериодВыравнивания", "Дни", false);
    set(ui->rb_2_hours, "ПериодВыравнивания", "Часы", true);
    set(ui->rb_3_minutes, "ПериодВыравнивания", "Минуты", false);
    set(ui->rb_4_without_period, "ПериодВыравнивания", "БезВыравнивания", false);

    set(ui->rb_1_pirsona, "МетодКорреляции", "КорреляцияПрисона", false);
    set(ui->rb_2_diff, "МетодКорреляции", "ПростоеКвадратичноеОтклонение", true);
    set(ui->rb_3_diff_sub_mean, "МетодКорреляции", "КвадратичноеОтклонениеВыровненноеПоСреднейОтрезков", false);
    set(ui->rb_5_diff_zero, "МетодКорреляции", "КвадратичноеОтклонениеВыровненноеПоПоследнемуЗначениюОтрезков", false);
    set(ui->rb_6_only_cicling, "МетодКорреляции", "БезСравненияОтрезковТолькоЦикличность", false);
    set(ui->rb_4_diff_norm, "МетодКорреляции", "КвадратичноеОтклонениеДелительСредняяОтрезков", false);

//    //+++
//    ui->ed_file_data->setText("/ai/eurusd/hour");
//    ui->ed_cols_date_time->setText("<DATE>,<TIME>");
//    ui->ed_col_forecast->setText("<CLOSE>");
//    ui->ed_start_date_time->setText("20181001 230000");

    if (settings.contains("window_size"))
        resize(settings.value("window_size").toSize());
    if (settings.contains("window_pos"))
        move(settings.value("window_pos").toPoint());

    QString file_cotir = ui->ed_file_data->text();
    if (file_cotir.isEmpty()) {
        file_cotir = "/ai/eurusd/emmsp.txt";
        if (QFile(file_cotir).exists()==false)
            file_cotir = app_path() + "data" + QDir::separator();
        ui->ed_file_data->setText(file_cotir);
    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    original_message_handler = qInstallMessageHandler(myMessageOutput);

    validator_int_from_zero.setBottom(0);
    validator_int_from_one.setRegExp(QRegExp("[1-9][0-9]*"));
    validator_int_from_one.set_fixup_value("1");
    validator_int_from_four.setRegExp(QRegExp("[4-9]|[1-9][0-9]+"));
    validator_int_from_four.set_fixup_value("4");

    ui->ed_cnt_outlier->setValidator(&validator_int_from_zero);
    ui->ed_count_best->setValidator(&validator_int_from_one);
    ui->ed_qty_one_forecast->setValidator(&validator_int_from_one);
    ui->ed_steps_forecast->setValidator(&validator_int_from_zero);
    ui->ed_len_first_corr->setValidator(&validator_int_from_four);
    ui->ed_qty_corr->setValidator(&validator_int_from_one);

    validator_periods_of_cicling.setRegExp(QRegExp("[1-9][0-9]*([;,][1-9][0-9]*)*"));
    ui->ed_periods_cicling->setValidator(&validator_periods_of_cicling);

    ui->lb_about->setOpenExternalLinks(true);

    ui->bt_stop->setEnabled(false);

    load_settings(false);

    for (QWidget* w : QApplication::allWidgets()) {
        QPushButton* b = qobject_cast<QPushButton*>(w);
        if (b == nullptr) continue;
        if (b->text() != "?") continue;
        if (b->accessibleName().isEmpty()) continue;
        connect(b, SIGNAL(clicked()), this, SLOT(on_help_clicked()));
    }
}

MainWindow::~MainWindow()
{
    make_params(false);
    delete ui;
}

void MainWindow::on_bt_start_clicked()
{ start_process(ForecastingMain::calc_forecast); }

void MainWindow::on_bt_scan_cicling_clicked()
{ start_process(ForecastingMain::calc_cicling); }

void MainWindow::start_process(ForecastingMain::ProcessMode mode_scan_cicling) {
    ui->txt_log->clear();

    ui->wgt_params->setEnabled(false);
    ui->bt_start->setEnabled(false);
    ui->bt_scan_cicling->setEnabled(false);
    ui->bt_stop->setEnabled(true);

    ForecastingMain* f = new ForecastingMain;
    f->mode_scan_cicling = mode_scan_cicling;
    f->cfg = make_params(true);
    QThread* th = new QThread;
    f->moveToThread(th);
    connect(th, SIGNAL(started()), f, SLOT(run_forecasting()));
    connect(th, SIGNAL(finished()), f, SLOT(deleteLater()));
    connect(f, SIGNAL(destroyed()), th, SLOT(deleteLater()));
    connect(th, SIGNAL(destroyed()), this, SLOT(after_finish_forecasting()));
    connect(ui->bt_stop, SIGNAL(clicked()), f, SLOT(stop_process()), Qt::DirectConnection);
    th->start();
}

void MainWindow::after_finish_forecasting() {
    qDebug() << "=== готово ================";
    ui->wgt_params->setEnabled(true);
    ui->bt_start->setEnabled(true);
    ui->bt_scan_cicling->setEnabled(true);
    ui->bt_stop->setEnabled(false);
}

void MainWindow::myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg) {

    MainWindow* w = nullptr;
    for(QWidget* pWidget : QApplication::topLevelWidgets()) {
        w = qobject_cast<MainWindow*>(pWidget);
        if (w) break;
    }

    QMetaObject::invokeMethod(
                w, "add_msg_log",
                Qt::AutoConnection,
                Q_ARG(QString,msg));

    w->original_message_handler(type, context, msg);
}

void MainWindow::add_msg_log(QString msg) {

    QString s = msg;
    s = s.replace("\"", "");

    bool has_comma = ui->checkBox_UseComma->checkState() == Qt::Checked;
    if (has_comma)
        s = s.replace(QRegExp("([0-9])\\.([0-9])"), "\\1,\\2");
    else
        s = s.replace(QRegExp("([0-9])\\,([0-9])"), "\\1.\\2");

    ui->txt_log->appendPlainText(s);
    ui->txt_log->moveCursor(QTextCursor::End);
}

void MainWindow::on_bt_select_file_clicked()
{
    QString fn = ui->ed_file_data->text();
    QString fn_res = QFileDialog::getOpenFileName(ui->ed_file_data, "Open file", fn, "*.* (*.*)");
    if (fn_res.isEmpty()==false)
        ui->ed_file_data->setText(fn_res);
}

void MainWindow::on_bt_exit_clicked()
{ QApplication::closeAllWindows(); }

void MainWindow::on_bt_reset_settings_clicked()
{ load_settings(true); }

void MainWindow::on_help_clicked() {
    ui->txt_log->clear();

    QPushButton* b = qobject_cast<QPushButton*>(sender());
    if (b == nullptr) return;
    QString name = b->accessibleName().trimmed();
    if (name.isEmpty()) return;

    QStringList ss = ui->txt_help->toPlainText().split("===");

    for (QString s : ss) {
        QStringList ss_out = s.split(QRegExp("\r?\n"));
        if (ss_out.at(0).trimmed() != name) continue;

        for (int i = 1; i < ss_out.size(); ++i)
            qDebug() << ss_out.at(i);
        break;
    }
}
