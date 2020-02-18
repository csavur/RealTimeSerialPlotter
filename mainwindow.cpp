#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtDebug>
#include <QFileDialog>
#include <QJsonDocument>

int MainWindow::x = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //! [1] Creating thread
    thread = new QThread();
    manager = new SerialManager();
    manager->moveToThread(thread);

    connect(thread, &QThread::finished, manager, &SerialManager::deleteLater);
    connect(this, &MainWindow::updateSerialPort, manager, &SerialManager::open);
    connect(manager, &SerialManager::finished, thread, &QThread::quit);
    connect(manager, &SerialManager::resultReady, this, &MainWindow::handleResults);
    thread->start();
    //! [/2]

    initUI();
}

MainWindow::~MainWindow()
{
    delete chart;
    foreach (QLineSeries *p, series) {
        delete  p;
    }
    thread->quit();
    manager->deleteLater();

    qDebug() << "~MainWindow()";
    delete ui;
}

void MainWindow::initUI()
{
    QList<qint32> baudrates = QSerialPortInfo::standardBaudRates();
    foreach (qint32 baudrate, baudrates) {
        ui->comboBoxBaudRate->addItem(QString::number(baudrate));
    }
    ui->comboBoxBaudRate->setCurrentText("115200");

    ui->comboBoxADCRes->blockSignals(true);
    ui->comboBoxADCRes->addItems(QStringList() << "20" << "1023" << "2047" << "4095" << "8191" );
    ui->comboBoxADCRes->setCurrentText("2047");
    ui->comboBoxADCRes->blockSignals(false);

    ui->comboBoxChannel->blockSignals(true);
    ui->comboBoxChannel->addItems(QStringList() << "1" << "2" << "3" << "4" << "5" << "6" );
    ui->comboBoxChannel->setCurrentText("1");
    ui->comboBoxChannel->blockSignals(false);

    QLineSeries *ser = new QLineSeries();
    series.append(ser);

    chart = new QChart();
    chart->addSeries(ser);
    chart->legend()->hide();
    chart->createDefaultAxes();
    chart->setTitle("Real Time Serial Plotter");

    QChartView *chartView = new QChartView(chart);

    chartView->chart()->setTheme(QChart::ChartThemeDark);

    chartView->setRenderHint(QPainter::Antialiasing);

    ui->gridLayout->addWidget(chartView, 2, 0, 2, 10);

    updateAxis();

    series[0]->setColor(QColor(255, 128, 0));
}

void MainWindow::updateAxis()
{
    chart->axisX()->setRange(0, 100);
    chart->axisY()->setRange(0, ui->comboBoxADCRes->currentText().toInt());
}

void MainWindow::on_pushButtonStart_clicked()
{
    if(ui->pushButtonStart->text() == "Start") {

        QString interval = ui->lineEditInterval->text();
        QString serialPort = ui->lineEditPortName->text();
        QString baudRate = ui->comboBoxBaudRate->currentText();

        if(serialPort.isEmpty()) {
            QMessageBox::warning(this, tr("warming!"), tr("Port cannot be empty!"));
            ui->lineEditPortName->setFocus();
            return;
        }

        if(baudRate.isEmpty()) {
            QMessageBox::warning(this, tr("warming!"), tr("Baud Rate cannot be empty!"));
            ui->comboBoxBaudRate->setFocus();
            return;
        }

        if(!interval.isEmpty()) {
            QTimer::singleShot(1000*interval.toInt(), this, SLOT(on_pushButtonStart_clicked()));
        }

        manager->setBaudrate(baudRate.toInt());
        manager->setPortName(serialPort);
        ui->pushButtonStart->setText("Stop");

        emit updateSerialPort(1);

        mod = 500;
        x = 0;
        chart->axisX()->setRange(0, 500);

        foreach (QLineSeries *p, series) {
            p->clear();
        }

    } else {

        ui->pushButtonStart->setText("Start");
        emit updateSerialPort(0);

        saveToCSV();
    }
}

void MainWindow::handleResults(const QString &result)
{
    x++;



    QStringList fields = result.split(",");

    if (fields.count() < 5) {
        return;
    }

    int num_ch = fields[0].toInt();
    QString type = fields[1];


    qint64 ts = QDateTime::currentSecsSinceEpoch();

    if (type ==  "ppg") {
        //    qDebug() << num_ch << type <<fields[2] << fields[3] << fields[4];
        series[0]->append(x, fields[2].toInt());
        series[1]->append(x, fields[3].toInt());
        series[2]->append(x, fields[4].toInt());
    }

    if(x%mod == 0) {
        chart->axisX()->setRange(x-500, x);
        mod = 1;

        for (int i = 0; i < 3; ++i) {
            series[i]->removePoints(0, 400);
        }
    }



// old code
//    qDebug() << result;
//    QJsonDocument doc = QJsonDocument::fromJson(result.toUtf8());
//    QJsonObject root = doc.object();
//    QJsonValue value = root.value(QString("num_ch"));
//    QJsonValue timestamp = root.value(QString("timestamp"));
//    qDebug() << timestamp.toString();
//    QJsonValue data = root.value(QString("data"));
//    QJsonArray array = data.toArray();

//    qDebug() << "Array Size :" << array.size() << ":" << array[0].toInt() << ", " << array[1].toInt();

//    for (int i = 0; i < value.toInt(); ++i) {
//        //qDebug() << "Celal " <<  x << " : " <<array[i].toInt();
//        series[i]->append(x, array[i].toInt());
//    }

//    if(x%mod == 0) {
//        chart->axisX()->setRange(x-500, x);
//        mod = 1;
//    }
}

void MainWindow::saveToCSV()
{
    if(!ui->checkBoxSave->isChecked())
        return;

        QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "filename.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", nullptr, nullptr); // getting the filename (full path)
        QFile data(filename);
        if(data.open(QFile::WriteOnly |QFile::Truncate))
        {
            QTextStream output(&data);
            for (int i = 0; i < series[0]->count(); ++i) { // num reading
                output << series[0]->at(i).x() << ",";
                for (int j = 0; j < series.count(); ++j) { // num channel
                    output << series[j]->at(i).y() << ", ";
                }
                output << "\n";
            }
        }

        data.close();
}

void MainWindow::on_comboBoxADCRes_currentIndexChanged(const QString &arg1)
{
    chart->axisY()->setRange(0, arg1.toInt());
}


void MainWindow::on_comboBoxChannel_currentIndexChanged(const QString &arg1)
{
    Q_UNUSED(arg1);
    foreach (QLineSeries *p, series) {
        chart->removeSeries(p);
        delete  p;
    }
    series.clear();

    int num_ch = arg1.toInt();
    for (int i = 0; i < num_ch; ++i) {
        QLineSeries *ser = new QLineSeries();
        chart->addSeries(ser);
        series.append(ser);
    }

    chart->createDefaultAxes();

    updateAxis();
}
