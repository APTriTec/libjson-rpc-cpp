/*************************************************************************
 * libjson-rpc-cpp
 *************************************************************************
 * @file    abstractserver.h
 * @date    30.12.2012
 * @author  Peter Spiess-Knafl <peter.knafl@gmail.com>
 * @license See attached LICENSE.txt
 ************************************************************************/

#ifndef JSONRPC_CPP_ABSTRACTSERVER_H_
#define JSONRPC_CPP_ABSTRACTSERVER_H_

#include <map>
#include <string>
#include <vector>
#include <jsonrpccpp/common/procedure.h>
#include "abstractserverconnector.h"
#include "iprocedureinvokationhandler.h"
#include "iclientconnectionhandler.h"
#include "requesthandlerfactory.h"

#include <memory>
#ifdef __INTIME__
# include <tr1/_smartptr.h>
  namespace mem = std::tr1;
#else
  namespace mem = std;
#endif

namespace jsonrpc
{

    template<class S>
    class AbstractServer : public IProcedureInvokationHandler
    {
        public:
            typedef void(S::*methodPointer_t)       (const Json::Value &parameter, Json::Value &result);
            typedef void(S::*notificationPointer_t) (const Json::Value &parameter);

            AbstractServer(mem::shared_ptr<AbstractServerConnector> connector, serverVersion_t type = JSONRPC_SERVER_V2) :
                connection(connector)
            {
                this->handler = RequestHandlerFactory::createProtocolHandler(type, *this);
                connector->SetHandler(this->handler);
            }

            virtual ~AbstractServer()
            {
                delete this->handler;
            }

            bool StartListening()
            {
                return connection->StartListening();
            }

            bool StopListening()
            {
                return connection->StopListening();
            }

            virtual void HandleMethodCall(Procedure &proc, const Json::Value& input, Json::Value& output)
            {
                S* instance = dynamic_cast<S*>(this);
                (instance->*methods[proc.GetProcedureName()])(input, output);
            }

            virtual void HandleNotificationCall(Procedure &proc, const Json::Value& input)
            {
                S* instance = dynamic_cast<S*>(this);
                (instance->*notifications[proc.GetProcedureName()])(input);
            }

        protected:
            virtual bool bindAndAddMethod(Procedure* proc, methodPointer_t pointer)
            {
                if(proc->GetProcedureType() == RPC_METHOD)
                {
                    this->handler->AddProcedure(*proc);
                    this->methods[proc->GetProcedureName()] = pointer;
                    return true;
                }
                return false;
            }

            virtual bool bindAndAddNotification(Procedure* proc, notificationPointer_t pointer)
            {
                if(proc->GetProcedureType() == RPC_NOTIFICATION)
                {
                    this->handler->AddProcedure(*proc);
                    this->notifications[proc->GetProcedureName()] = pointer;
                    delete proc;
                    return true;
                }
                return false;
            }

        private:
            mem::shared_ptr<AbstractServerConnector>        connection;
            IProtocolHandler                                *handler;
            std::map<std::string, methodPointer_t>          methods;
            std::map<std::string, notificationPointer_t>    notifications;
    };

} /* namespace jsonrpc */
#endif /* JSONRPC_CPP_ABSTRACTSERVER_H_ */
