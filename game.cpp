#include "game.hpp"
#include "limit.hpp"

#include <QTime>

#define WIDTH 1000
#define HEIGHT 640

Game::Game()
{
    //create timer
    timer = new QTimer();

    //create netork element : socket
    socket = new QTcpSocket(this);

    //connect socket to slots
    QObject::connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
    QObject::connect(socket, SIGNAL(disconnected()), this, SLOT(disconnect()));
    QObject::connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(errorSocket(QAbstractSocket::SocketError)));
    //QObject::connect(timer, SIGNAL(timeout()), this, SLOT(sendPosition()));


    sizeMessage = 0;

    connect();
    //playing();

    player1 = false;
}

void Game::connect() {
    socket->abort();
    socket->connectToHost("127.0.0.1", 50885);
    std::cout << "Successful to connect to host !" << std::endl;
}

void Game::disconnect() {
    std::cout << "Disconnected" << std::endl;
}

void Game::receiveData() {
    //qDebug() << "Données reçues à :" + QTime::currentTime().toString("hh:mm:ss");
    std::cout << "Une donnée a été reçue" << std::endl;
    QDataStream in(socket);

    if (sizeMessage == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
            return;

        in >> sizeMessage;
    }

    if (socket->bytesAvailable() < sizeMessage)
        return;


    //if we are at this line of code, we have received the full message
    QString messageReceived;
    in >> messageReceived;

    std::cout << messageReceived.toStdString().c_str() << std::endl;

    if(messageReceived == "first") {
        player1 = true;
    } else if(messageReceived == "start") {
        playing();
    } else {
        QStringList fSplit(messageReceived.split(":"));
        if(player1) {
            QString pos(fSplit.at(1));
            rightPaddle->setPosY(pos.toInt());
        } else {
            QString pos(fSplit.at(0));
            leftPaddle->setPosY(pos.toInt());
        }
        QString posX(fSplit.at(2));
        QString posY(fSplit.at(3));
        ball->setPosition(posX.toInt(), posY.toInt());
        std::cout << "Ball maj" << std::endl;
        sendPosition();
    }

    //we reset this var for the next message
    sizeMessage = 0;

    //qDebug() << "Fin du traitement à :" + QTime::currentTime().toString("hh:mm:ss");

}

void Game::errorSocket(QAbstractSocket::SocketError error) {
    //we print message in function of type of error
    std::cout << "!!! ERROR !!! ";
    switch (error) {

    case QAbstractSocket::HostNotFoundError:
        std::cout << "Server not found !" << std::endl;
        break;

    case QAbstractSocket::ConnectionRefusedError:
        std::cout << "Server refused your connection !" << std::endl;
        break;

    case QAbstractSocket::RemoteHostClosedError:
        std::cout << "Server closed connection !" << std::endl;
        break;

    default:
        std::cout << socket->errorString().toStdString();
        break;
    }
}

void Game::sendData(QString &message) {
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    //we write size of message
    out << (quint16) 0;

    //we write message in byte array
    out << message;

    //we select the position 0 of message
    out.device()->seek(0);
    //we write the real size of message in the beginning
    out << (quint16) (paquet.size() - sizeof(quint16));

    //we send data
    socket->write(paquet);
}

void Game::playing() {
    //timer->start(100);

    //create scene & view
    scene = new QGraphicsScene();

    //setting scene & view
    scene->setBackgroundBrush(Qt::white);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setFixedSize(WIDTH, HEIGHT);
    scene->setSceneRect(0, 0, WIDTH, HEIGHT);

    //create element
    int heightPaddle(150), widthPaddle(15);
    leftPaddle = new Paddle(0, 0, widthPaddle, heightPaddle, player1, this->size());
    rightPaddle = new Paddle(this->width() - widthPaddle, 0, widthPaddle, heightPaddle, !player1, this->size());
    ball = new Ball(100, 100, 40, 40);
    top = new Limit(0, 0, this->size().width(), 0);
    bottom = new Limit(0, this->size().height(), this->size().width(), this->size().height());

    //add element to scene
    scene->addItem(leftPaddle);
    scene->addItem(rightPaddle);
    scene->addItem(top);
    scene->addItem(bottom);
    scene->addItem(ball);

    //add scene to view and show view
    setScene(scene);
}

void Game::sendPosition() {
    QString msg;
    int pos;

    if(player1) {
        msg.append("l:");
        pos = leftPaddle->getPos();
    } else {
        msg.append("r:");
        pos = rightPaddle->getPos();
    }

    msg.append(QString::number(pos));
    std::cout << msg.toStdString().c_str() << std::endl;
    sendData(msg);
}
