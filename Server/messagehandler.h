#ifndef MESSAGEHANDLER_H
#define MESSAGEHANDLER_H

#include <QObject>
#include <QTextStream>

class MessageHandler : public QObject
{
    Q_OBJECT
public:
    explicit MessageHandler(QObject *parent = nullptr);

signals:

public slots:
    void errorMessage(const QString &message);
    void warningMessage(const QString &message);
    void infoMessage(const QString &message);

    void addClientMessage(const QString &peerInfo, const QByteArray &datagram,
                          const QByteArray &plainText);
};

#endif // MESSAGEHANDLER_H