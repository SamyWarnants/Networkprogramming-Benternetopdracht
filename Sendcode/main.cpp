#include <iostream>
#include <nzmqt/nzmqt.hpp>
#include <nzmqt/impl.hpp>
#include <QCoreApplication>
#include <QString>
#include <QTimer>
#include <QThread>
#include <QDateTime>

/*void printMessages( const QList<QByteArray>& messages )
{
    for( const QByteArray & message : messages )
    {
        std::cout << message.toStdString() << std::endl;
    }
}*/

/*nzmqt::ZMQSocket *pusher;
void pingMessage( void )
{
    nzmqt::ZMQMessage message = nzmqt::ZMQMessage( QString("PING").toUtf8() );
    pusher->sendMessage( message );
    std::cout << "PING send !" << std::endl;
}*/

int main( int argc, char *argv[] )
{
    QCoreApplication a(argc, argv);
    std::cout << "Prep!" << std::endl;
    try
    {
        nzmqt::ZMQContext *context = nzmqt::createDefaultContext( &a );
        nzmqt::ZMQSocket *pusher = context->createSocket( nzmqt::ZMQSocket::TYP_PUSH, context );
        nzmqt::ZMQSocket *subscriber = context->createSocket( nzmqt::ZMQSocket::TYP_SUB, context );
        //QObject::connect( subscriber, &nzmqt::ZMQSocket::messageReceived, printMessages );
        QObject::connect( subscriber, &nzmqt::ZMQSocket::messageReceived, []( const QList<QByteArray>& messages )
                         {
                             if( messages.size() < 1 )
                             {
                                 std::cout << "Received empty message !" << std::endl;
                             }
                             else if( messages.size() == 1 )
                             {
                                 std::cout << "Received (" << messages.constFirst().size() << ") : " << messages.constFirst().toStdString() << std::endl;
                             }
                             else
                             {
                                 std::cout << "Received " << messages.size() << " parts" << std::endl;
                                 int part = 1;
                                 for( const QByteArray & message : messages )
                                 {
                                     std::cout << "Part " << part << " (" << message.size() << ") : " << message.toStdString() << std::endl;
                                     part++;
                                 }
                             }
                         } );

        //		QTimer * pinger = new QTimer( context );
        //QObject::connect( pinger, &QTimer::timeout, pingMessage );
        //		QObject::connect( pinger, &QTimer::timeout, [pusher](void)
        //		{
        //			//QDateTime::currentDateTime().toString( Qt::ISODate );
        //			nzmqt::ZMQMessage message = nzmqt::ZMQMessage( QDateTime::currentDateTime().toString( Qt::ISODate ).toUtf8() );//QString("PING").toUtf8() );
        //			//nzmqt::ZMQMessage message = nzmqt::ZMQMessage( QString("Discord!help").toUtf8() );
        //			pusher->sendMessage( message );
        //			std::cout << "PING send !" << std::endl;
        //		} );
        //		pinger->setInterval( 1000*2 );

        QThread *thread = QThread::create([pusher]{
            QTextStream s(stdin);
            while( 1 )
            {
                QString input = s.readLine();
                nzmqt::ZMQMessage message = nzmqt::ZMQMessage( input.toUtf8() );
                pusher->sendMessage( message );
                std::cout << "Message send !" << std::endl;
            }
        });


        //		pusher->connectTo( "tcp://localhost:24041" );
        //		subscriber->connectTo( "tcp://localhost:24042" );
        //		pusher->connectTo( "tcp://84.193.238.142:24041" );
        //		subscriber->connectTo( "tcp://84.193.238.142:24042" );
        pusher->connectTo( "tcp://benternet.pxl-ea-ict.be:24041" );
        subscriber->connectTo( "tcp://benternet.pxl-ea-ict.be:24042" );
        //		pusher->connectTo( "tcp://benternet.backup.pxl-ea-ict.be:24041" );
        //		subscriber->connectTo( "tcp://benternet.backup.pxl-ea-ict.be:24042" );
        //		pusher->connectTo( "tcp://np.pxl-ea-ict.be:24041" );
        //		subscriber->connectTo( "tcp://np.pxl-ea-ict.be:24042" );
        //		pusher->connectTo( "tcp://192.168.1.8:24041" );
        //		subscriber->connectTo( "tcp://192.168.1.8:24042" );


        if( argc > 1 )
        {
            for( int i = 1 ; i < argc ; i++ )
            {
                subscriber->subscribeTo( argv[i] );
            }
        }
        else
        {
            subscriber->subscribeTo( "" );
        }

        if( !pusher->isConnected() || !subscriber->isConnected() )
        {
            std::cerr << "NOT CONNECTED !!!" << std::endl;
        }

        context->start();
        //pinger->start();
        thread->start();

        /*nzmqt::ZMQMessage * incoming = new nzmqt::ZMQMessage();
        while( subscriber->isConnected() )
        {
            subscriber->receiveMessage( incoming, 0 );
            std::cout << incoming->toByteArray().toStdString() << std::endl;
        }*/
    }
    catch( nzmqt::ZMQException & ex )
    {
        std::cerr << "Caught an exception : " << ex.what();
    }
    std::cout << "Start!" << std::endl;
    return a.exec();
}
