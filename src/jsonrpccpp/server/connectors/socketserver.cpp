#include "socketserver.h"

#include <cstring>
#include <vector>
#include <iostream>
#include <errno.h>
#ifdef __INTIME__
#include <EcOs.h>
#define usleep(usec) OsSleep(usec)
  #include <EcOs.h>
  #define usleep(usec) OsSleep(usec)
#else
	void usleep(__int64 usec)
	{
		HANDLE timer;
		LARGE_INTEGER ft;

		ft.QuadPart = -(10 * usec);

		timer = CreateWaitableTimer(NULL, TRUE, NULL);
		SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
		WaitForSingleObject(timer, INFINITE);
		CloseHandle(timer);
	}
 #endif

namespace jsonrpc {

    void CloseConnection(const SocketServer::Connection* connection) {
      shutdown(connection->socket, BOTH_DIRECTION);
      closesocket(connection->socket);
      threadJoin(connection->thread);
    }

    void CloseAllConnections(std::vector<SocketServer::Connection*>& clients) {
      for(std::vector<SocketServer::Connection*>::iterator it = clients.begin(); it != clients.end(); ++it ) {
        CloseConnection(*it);
        delete *it;
      }
      clients.clear();
    }

    void CloseFinishedConnections(std::vector<SocketServer::Connection*>& clients) {
      std::vector<SocketServer::Connection*>::iterator it = clients.begin();
      while (it != clients.end()) {
          if ((*it)->finished) {
            CloseConnection(*it);
            delete *it;
            it = clients.erase(it);
          }
          else
            ++it;
      }
    }

    void CloseOldestConnection(std::vector<SocketServer::Connection*>& clients) {
      CloseConnection(clients.front());
      clients.erase(clients.begin());
    }


  SocketServer::SocketServer(const std::string& port, const int type, const int pool) throw (JsonRpcException) :
    AbstractServerConnector(),
    host_info_(NULL),
    shutdown_(false),
    poolSize_(pool)
  {
    struct addrinfo hint;
    memset((char*)&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = type;
    hint.ai_flags = AI_PASSIVE;
    int status = getaddrinfo(NULL, port.c_str(), &hint, &host_info_);
    CHECK(status);
    return;
error:
    throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR);
  }

  SocketServer::~SocketServer()
  {
    if (host_info_ != NULL)
      freeaddrinfo(host_info_);
    host_info_ = NULL;

  }

  bool SocketServer::StartListening()
  {
    CreateSocket();
    shutdown_ = false;
    return threadCreate(&server_thread_, (ThreadStartRoutine)HandleConnections, this) == 0;
  }

  bool SocketServer::StopListening()
  {
    shutdown_ = true;
    CloseSocket();
    threadJoin(server_thread_);
    return true;
  }

  bool SocketServer::SendResponse( const std::string& response,
                                void* addInfo)
  {
    Connection* connection = (Connection*)addInfo;
    return send(connection->socket, response.c_str(), response.length(), 0) == response.length();
  }

 THREAD_ROUTINE_RETURN SocketServer::HandleConnections(void* data)
 {
    SocketServer* server = (SocketServer*) data;
    std::vector<SocketServer::Connection*> clients;
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof(client_addr);
    MutexHandle lock;
    mutexCreate(&lock);
    int client_socket;
    while (!server->shutdown_) {
      if ((client_socket = accept(server->socket_, (struct sockaddr*)&client_addr, &addr_size)) > 0) {
        clients.push_back(new jsonrpc::SocketServer::Connection());
        clients.back()->socket = client_socket;
        clients.back()->pserver = server;
        clients.back()->plock_server = &lock;
        CloseFinishedConnections(clients);
        if (clients.size() > server->poolSize_) {
          CloseOldestConnection(clients);
        }
        threadCreate(&clients.back()->thread, (ThreadStartRoutine)ConnectionHandler, clients.back(), 10*1024);
      }
    }
    CloseAllConnections(clients);
    mutexDestroy(&lock);
    return 0;
  }

  THREAD_ROUTINE_RETURN SocketServer::ConnectionHandler(void* data)
  {
    static const int KEEPALIVE_TIMEOUT_MS = 500; // time in ms to keep socket opened while inactive
    char EOT = 4;
    Connection* connection = (Connection*)data;
    const int MAX_SIZE = 5000;
    char client_message[MAX_SIZE];
    int read_size;
    connection->finished = false;
    std::string req;
    int keep_alive_timeout = KEEPALIVE_TIMEOUT_MS;
    do {
      req.clear();
      while ((read_size = recv(connection->socket, client_message, MAX_SIZE, 0)) > 0) {
        req.append(&client_message[0], read_size);
        if (client_message[read_size - 1] == EOT)
          break;
      }
      if ( read_size > 0 )
      {
        keep_alive_timeout = KEEPALIVE_TIMEOUT_MS;
        if ( client_message[read_size - 1] == EOT )
        {
          mutexLock(connection->plock_server);
          try {
            connection->pserver->OnRequest(req, connection);
          } catch (...) {}
          mutexUnlock(connection->plock_server);
        }
      }else if( read_size == 0 )
      {
        if ( keep_alive_timeout <= 0 )
        {
          read_size = -1; // timeout !
        } else {
          --keep_alive_timeout;
          usleep(1);
        }
      }
    } while( read_size != -1 );
    connection->finished = true;
    return 0;
  }

  void SocketServer::CreateSocket() throw (JsonRpcException) {
    int yes = 1;
    socket_ = socket(host_info_->ai_family, host_info_->ai_socktype, host_info_->ai_protocol);
    CHECK_SOCKET(socket_);
    CHECK_STATUS(setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (char*)(&yes), sizeof(yes)));
    CHECK_STATUS(bind(socket_, host_info_->ai_addr, host_info_->ai_addrlen));
    CHECK_STATUS(listen(socket_, poolSize_));
    return;
error:
    CloseSocket();
    throw JsonRpcException(Errors::ERROR_SERVER_CONNECTOR);
  }

  void SocketServer::CloseSocket() {
    if (socket_ != -1) {
      shutdown(socket_, BOTH_DIRECTION);
      closesocket(socket_);
    }
    socket_ = -1;
  }

};
