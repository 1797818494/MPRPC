#include "mprpcchannel.h"
#include <string>
#include "rpcheader.pb.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include "mprpcapplication.h"
#include "mprpccontroller.h"
#include "zookeeperutil.h"

void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method, 
    google::protobuf::RpcController* controller, const google::protobuf::Message* request,
    google::protobuf::Message* response, google::protobuf::Closure* done) {
        const google::protobuf::ServiceDescriptor* sd = method->service();
        std::string service_name = sd->name();
        std::string method_name = method->name();

        // 获取参数的序列化字符串长度 args_size
        uint32_t args_size = 0;
        std::string args_str;
        if(request->SerializeToString(&args_str)) {
            args_size = args_str.size();
        } else {
            controller->SetFailed("serialize request error!");
            return;
        }
        mprpc::RpcHeader rpcHeader;
        rpcHeader.set_service_name(service_name);
        rpcHeader.set_method_name(method_name);
        rpcHeader.set_args_size(args_size);
        uint32_t header_size = 0;
        std::string rpc_header_str;
        if(rpcHeader.SerializePartialToString(&rpc_header_str)) {
            header_size = rpc_header_str.size();
        } else {
            controller->SetFailed("serialize rpc header error!");
            return;
        }
        std::string send_rpc_str;
        send_rpc_str.insert(0, (char*)&header_size, 4);
        send_rpc_str += rpc_header_str;
        send_rpc_str += args_str;
        // 打印调试信息
        std::cout << "============================================" << std::endl;
        std::cout << "header_size: " << header_size << std::endl; 
        std::cout << "rpc_header_str: " << rpc_header_str << std::endl; 
        std::cout << "service_name: " << service_name << std::endl; 
        std::cout << "method_name: " << method_name << std::endl; 
        std::cout << "args_str: " << args_str << std::endl; 
        std::cout << "============================================" << std::endl;
        int clientfd = socket(AF_INET, SOCK_STREAM, 0);
        if(-1 == clientfd) {
            char errtxt[512] = {0};
            sprintf(errtxt, "create socket error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }
        std::string ip;
        uint16_t port;
        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

        if(-1 == connect(clientfd, (struct sockaddr*)&server_addr, sizeof(server_addr))) {
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "connect error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }
        if(-1 == send(clientfd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "send error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }
        char recv_buf[1024] = {0};
        int recv_size = 0;
        if(-1 == (recv_size = recv(clientfd, recv_buf, 1024, 0))) {
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "recv error! errno:%d", errno);
            controller->SetFailed(errtxt);
            return;
        }

        if(!response->ParseFromArray(recv_buf, recv_size)) {
            close(clientfd);
            char errtxt[512] = {0};
            sprintf(errtxt, "parse error! response_str:%s", recv_buf);
            controller->SetFailed(errtxt);
            return;
        }

        close(clientfd);
    }