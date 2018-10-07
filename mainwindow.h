#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <serialmanager.h>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QSerialPortInfo>

QT_CHARTS_USE_NAMESPACE


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void updateSerialPort(const int &state);

private slots:
    void on_pushButtonStart_clicked();
    void handleResults(const QString &);

    void on_comboBoxADCRes_currentIndexChanged(const QString &arg1);

private:
    void saveToCSV();
    void initUI();

private:
    Ui::MainWindow *ui;
    QThread *worker;
    SerialManager *manager;
    QLineSeries *series;
    QChart *chart;
    static int x;
    int mod = 500;
};

#endif // MAINWINDOW_H
