#include "serialmanager.h"
#include <QDebug>
#include <iostream>

SerialManager::SerialManager(QObject *parent) : QObject(parent), standardOutput(stdout)
{
    serialPort = new QSerialPort(this);

    connect(serialPort, &QSerialPort::readyRead, this, &SerialManager::handleReadyRead);
    connect(serialPort, &QSerialPort::errorOccurred, this, &SerialManager::handleError);
}

SerialManager::~SerialManager()
{
    qDebug() << "Closing port" << portName;
    serialPort->close();
    delete  serialPort;
}

QString SerialManager::getPortName() const
{
    return portName;
}

void SerialManager::setPortName(const QString &value)
{
    portName = value;
}

int SerialManager::getBaudrate() const
{
    return baudrate;
}

void SerialManager::setBaudrate(int value)
{
    baudrate = value;
}

void SerialManager::open(const int &state)
{
    if(state == 0) {
        qDebug() << "Closing..." << portName << ":" << baudrate;
        if(serialPort)
            serialPort->close();
    } else {
        qDebug() << "Opening..." << portName << ":" << baudrate;

        serialPort->setPortName(portName);
        serialPort->setBaudRate(static_cast<QSerialPort::BaudRate>(baudrate), QSerialPort::Input );

        if (!serialPort->open(QIODevice::ReadOnly)) {
            standardOutput << QObject::tr("Failed to open port %1, error: %2")
                              .arg(portName)
                              .arg(serialPort->errorString())
                           << endl;
            return;
        }
    }
}

void SerialManager::handleReadyRead()
{
    QByteArray data = serialPort->readAll();
    //std::cout << data.toStdString() << std::endl;
    emit resultReady(QString::fromStdString(data.toStdString()));
}

void SerialManager::handleError(QSerialPort::SerialPortError serialPortError)
{
    if (serialPortError == QSerialPort::ReadError) {
        standardOutput << QObject::tr("An I/O error occurred while reading the data from port %1, error: %2")
                            .arg(serialPort->portName()).arg(serialPort->errorString()) << endl;
        QCoreApplication::exit(1);
    }
}
