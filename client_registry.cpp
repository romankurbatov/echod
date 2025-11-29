#include "client_registry.hpp"

#include <iostream>

ClientRegistry::ClientRegistry(
        Dispatcher &dispatcher, CommandExecutor &executor) :
    m_dispatcher(dispatcher),
    m_executor(executor),
    m_connected(0),
    m_total(0)
{
}

ClientRegistry::~ClientRegistry() {
    clear_disconnected_clients();

    for (Client *client : m_clients) {
        delete client;
    }
}

void ClientRegistry::handle_client(int fd,
        const sockaddr_in &client_address,
        const sockaddr_in &server_address)
{
    Client *client = new Client(fd,
            m_dispatcher, m_executor, *this,
            client_address, server_address);
    m_clients.insert(client);
    m_connected++;
    m_total++;
}

void ClientRegistry::client_disconnected(Client *client) {
    auto it = m_clients.find(client);
    if (it == m_clients.end()) {
        std::cerr << "ClientRegistry: disconnected client not found"
                  << std::endl;
        return;
    }

    m_clients.erase(it);
    m_disconnected_clients.push_back(client);
    m_connected--;
}

void ClientRegistry::clear_disconnected_clients() {
    for (Client *client : m_disconnected_clients) {
        delete client;
    }

    m_disconnected_clients.clear();
}

void ClientRegistry::get_stats(long &connected, long &total) {
    connected = m_connected;
    total = m_total;
}
