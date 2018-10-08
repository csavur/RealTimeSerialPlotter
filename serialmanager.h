#ifndef SERIALMANAGER_H
#define SERIALMANAGER_H

#include <QObject>
#include <QtSerialPort>
#include <QSerialPortInfo>
#include <QTextStream>

class SerialManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialManager(QObject *parent = nullptr);
    ~SerialManager();

    QString getPortName() const;
    void setPortName(const QString &value);

    int getBaudrate() const;
    void setBaudrate(int value);
    void closeSafely();

signals:
    void finished();
    void resultReady(const QString &result);

public slots:
    void handleError(QSerialPort::SerialPortError serialPortError);
    void handleReadyRead();
    void open(const int &state);


private:
    QTextStream standardOutput;
    QSerialPort *serialPort;
    QString portName;
    int baudrate;
};

#endif // SERIALMANAGER_H
