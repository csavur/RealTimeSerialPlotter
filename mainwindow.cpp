#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QtDebug>
#include <QFileDialog>

int MainWindow::x = 0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setAttribute(Qt::WA_DeleteOnClose);

    //! [1] Creating thread
    worker = new QThread;
    manager = new SerialManager;
    manager->moveToThread(worker);

    connect(worker, &QThread::finished, manager, &SerialManager::deleteLater);
    //connect(worker, &QThread::started, manager, &SerialManager::open);
    connect(this, &MainWindow::updateSerialPort, manager, &SerialManager::open);
    connect(manager, &SerialManager::resultReady, this, &MainWindow::handleResults);
    worker->start();
    //! [/2]

    initUI();
}

MainWindow::~MainWindow()
{
    worker->exit();
    delete manager;
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
    ui->comboBoxADCRes->addItems(QStringList() << "1023" << "2047" << "4095" << "8191" );
    ui->comboBoxADCRes->setCurrentText("2047");
    ui->comboBoxADCRes->blockSignals(false);

    series = new QLineSeries();

    chart = new QChart();
    chart->legend()->hide();
    chart->addSeries(series);
    chart->createDefaultAxes();
    chart->setTitle("Real Time Serial Plotter");

    QChartView *chartView = new QChartView(chart);

    chartView->chart()->setTheme(QChart::ChartThemeDark);

    chartView->setRenderHint(QPainter::Antialiasing);

    ui->gridLayout->addWidget(chartView, 2, 0, 2, 10);

    chart->axisX()->setRange(0, 100);
    chart->axisY()->setRange(0, 2047);

    series->setColor(QColor(255, 128, 0));
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

        series->clear();
        mod = 500;
        x = 0;
        chart->axisX()->setRange(0, 500);

    } else {

        ui->pushButtonStart->setText("Start");
        emit updateSerialPort(0);

        saveToCSV();
    }

}


void MainWindow::handleResults(const QString &result)
{
    x++;
    qDebug() << "x : " << x << " mod :" << x%100 << " div :" << int(x/100.0);
    series->append(x, result.toInt());

    if(x%mod == 0) {
        chart->axisX()->setRange(x-500, x);
        mod = 1;
    }
}

void MainWindow::saveToCSV()
{
    if(!ui->checkBoxSave->isChecked())
        return;

    QString filename = QFileDialog::getSaveFileName(this, "DialogTitle", "filename.csv", "CSV files (.csv);;Zip files (.zip, *.7z)", 0, 0); // getting the filename (full path)
    QFile data(filename);
    if(data.open(QFile::WriteOnly |QFile::Truncate))
    {
        QTextStream output(&data);
        for (int i = 0; i < series->count(); ++i) {
            output << series->at(i).x() << "," << series->at(i).y() << "\n";
        }

    }

    data.close();
}

void MainWindow::on_comboBoxADCRes_currentIndexChanged(const QString &arg1)
{
    chart->axisY()->setRange(0, arg1.toInt());
}
