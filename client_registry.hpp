#ifndef CLIENT_REGISTRY_HPP
#define CLIENT_REGISTRY_HPP

#include <vector>
#include <unordered_set>
#include <netinet/in.h>

#include "dispatcher.hpp"
#include "command_executor.hpp"
#include "client.hpp"

class ClientRegistry {
public:
    ClientRegistry(Dispatcher &dispatcher, CommandExecutor &executor);
    ~ClientRegistry();

    ClientRegistry(const ClientRegistry &) = delete;
    ClientRegistry &operator=(const ClientRegistry &) = delete;

    // To be called from TCPServer
    void handle_client(int fd,
            const sockaddr_in &client_address,
            const sockaddr_in &server_address);

    // To be called from Client
    void client_disconnected(Client *client);

    // To be called from Dispatcher
    void clear_disconnected_clients();

    // To be called from CommandExecutor
    void get_stats(long &connected, long &total);

private:
    Dispatcher &m_dispatcher;
    CommandExecutor &m_executor;
    std::unordered_set<Client *> m_clients;
    std::vector<Client *> m_disconnected_clients;
    long m_connected, m_total;
};

#endif // CLIENT_REGISTRY_HPP
