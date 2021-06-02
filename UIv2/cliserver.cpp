#include "cliserver.h"

CliServer::CliServer(int argc, char ** argv)
    : CLI(argc, argv), server(nullptr)
{

}

void CliServer::update()
{
    /* With the NEW marvelous design, this function doesn't seem necessary. */
    //timer->start(1000); //reset the timer
    QCoreApplication::processEvents();
    QCoreApplication::processEvents();
    qout.flush();
}

bool CliServer::parseSpec(const QStringList &commandParts)
{
    if(commandParts.length() > 0)
    {
        QString primary = commandParts[0];

        // aliases
        QMap<QString, QString> aliases;

        if(aliases.contains(primary))
        {
            primary = aliases[primary];
        }
        // end aliases

        if(primary.compare("start", Qt::CaseInsensitive) == 0)
        {
            if(commandParts.length() < 3)
            {
                qout << tr("Usage: start [ip] [port]") << Qt::endl;
                return true; // if false, then the above message and invalidCommand becomes redundant
            }
            else
            {
                server = new QProcess();
                bool success = QObject::connect(server, &QProcess::errorOccurred,
                                                this, &CliServer::processError)
                        && QObject::connect(server, &QProcess::started,
                                            this, &CliServer::serverStarted)
                        && QObject::connect(server, &QProcess::stateChanged,
                                            this, &CliServer::serverChanged)
                        && QObject::connect(server, &QProcess::finished,
                                            this, &CliServer::processFinished)
                        && QObject::connect(server, &QProcess::readyReadStandardError,
                                            this, &CliServer::serverStderr)
                        && QObject::connect(server, &QProcess::readyReadStandardOutput,
                                            this, &CliServer::serverStdout);
                if(!success)
                {
                    qFatal("Communication with server process can't be established.");
                }
                server->start("build-Server-Desktop_Qt_6_1_0_MinGW_64_bit-Debug/debug/Server.exe",
                              {commandParts[1], commandParts[2]}, QIODevice::ReadWrite);
                return true;
            }
        }
        else if(primary.compare("stop", Qt::CaseInsensitive) == 0)
        {
            shutdownServer();
            return true;
        }
    }
    return false;
}

void CliServer::processError(QProcess::ProcessError error)
{
    switch(error)
    {
    case QProcess::FailedToStart:
        qCritical("Server failed to start."); break;
    case QProcess::Crashed:
        qCritical("Server crashed."); break;
    case QProcess::Timedout:
        qCritical("Server function timed out."); break;
    case QProcess::WriteError:
        qCritical("Error writing to the server process."); break;
    case QProcess::ReadError:
        qCritical("Error reading from the server process."); break;
    case QProcess::UnknownError:
        qCritical("Server had unknown error."); break;
    }
}

void CliServer::processFinished(int exitcode, QProcess::ExitStatus exitst)
{
    switch(exitst)
    {
    case QProcess::NormalExit:
        qInfo("Server finished normally with exit code %d", exitcode);
        break;
    case QProcess::CrashExit:
        qCritical("Server crashed."); break;
    }
}

void CliServer::serverStderr()
{
    QByteArray output = server->readAllStandardError();
    const char * output_str = output.constData();
    switch(output_str[7])
    {
    case 'E': qCritical("%s", output.constData()); break;
    case 'W': qWarning("%s", output.constData()); break;
    case 'I': qInfo("%s", output.constData()); break;
    default: qCritical("%s", output.constData()); break;
    }
}

void CliServer::serverStdout()
{
    QByteArray output = server->readAllStandardOutput();
    qInfo("%s", output.constData());
}

void CliServer::serverStarted()
{
    qout << tr("Server started and running.") << Qt::endl;
}

void CliServer::serverChanged(QProcess::ProcessState newstate)
{
    switch(newstate)
    {
    case QProcess::NotRunning:
        qCritical("Server->NotRunning."); break;
    case QProcess::Starting:
        qInfo("Server->Starting."); break;
    case QProcess::Running:
        qInfo("Server->Running."); break;
    }
}

void CliServer::shutdownServer()
{
    int waitformsec = 12000;
    if(server && server->state()) // QProcess::NotRunning = 0
    {
        server->write("SIGTERM\n");
        qout << tr("Waiting for server finish...") << Qt::endl;
        server->terminate();
        if(!server->waitForFinished(waitformsec))
        {
            qout << (tr("Server isn't responding after %1 msecs, killing.")).arg(QString::number(waitformsec))
                 << Qt::endl;
            server->kill();
        }
    }
    else
    {
        qout << tr("Server isn't running.") << Qt::endl;
    }
}

const QStringList CliServer::getCommandsSpec()
{
    QStringList result = QStringList();
    result.append(getCommands());
    result.append({"start", "stop"});
    result.sort(Qt::CaseInsensitive);
    return result;
}

const QStringList CliServer::getValidCommands()
{
    QStringList result = QStringList();
    result.append(getCommands());
    if(server && server->state())
    {
        result.append("stop");
    }
    else
    {
        result.append("start");
    }
    result.sort(Qt::CaseInsensitive);
    return result;
}

void CliServer::exitGraceSpec()
{
    shutdownServer();
}