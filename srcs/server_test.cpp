
bool    Server::bind_socket()
{
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1)
    {
        perror("Error making socket.");
        return -1;
    }
    fcntl(server_fd, F_SETFL, O_NONBLOCK); //prevent blocking

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int server_bind = bind(server_fd, reinterpret_cast<struct sockaddr*>(&server_addr), sizeof(server_addr));

    if (server_bind == -1)
    {
        perror("Error server binding");
        close(server_fd);
        return 0;
    }
    return 1;
}

bool    Server::server_listen( const int server_fd )
{
    int ret_code = listen(server_fd, 128);
    if (ret_code == -1)
    {
        std::cerr << "server listening error." << std::endl;
        close(server_fd);
        return 0;
    }
    return 1;
}

void    Server::wait_poll( const )
{
    while (true)
    {
        int ret = poll(fds.data(), fds.size(), -1);
        if (ret == -1)
        {
            perror("poll");
            break;
        }
        for (size_t i = 0; i < fds.size();)
        {
            if (fds[i].revents & POLLIN)
            {
                if (fds[i]->pollfd.fd == server_fd)
                {
                    create_new_client();
                }
                else
                {
                    send_recv_msg();
                    disconnect_delete_client();
                }
            }
            else
            {
                ++i;
            }
        }

    }
    
}