#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <vector>
#include <cstring>
#include <stack>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/poll.h>

void safe_write(int fd, const char *buf, size_t len);
int safe_read(int fd, char *buf, size_t len);
char* safe_malloc(size_t size);
int safe_poll(pollfd fds[], int nfds, int timeout);
int check_buf(char *buf, size_t length);
int find(char *buf, size_t pos, size_t length, char ch);
int check_command(std::string command);
bool check_path(std::string command);
int check_tree(std::string s_tree);
std::vector<std::string> get_command(const std::string);
int safe_check_input(int fd, char *buf, size_t len);

class Node
{
    std::string key;
    Node* left;
    Node* right;
    Node* parent;
public:
    Node(std::string key)
        : key(key)
        , left(nullptr)
        , right(nullptr)
        , parent(nullptr)
    {}
    void set_key(std::string new_key)
        { key = new_key; }
    void set_left(Node* new_left)
        { left = new_left; }
    void set_right(Node* new_right)
        { right = new_right; }
    void set_parent(Node* new_parent)
        { parent = new_parent; }
    std::string get_key()
        { return key; }
    Node* get_left()
        { return left; }
    Node* get_right()
        { return right; }
    Node* get_parent()
        { return parent; }
};

class Speak_Tree
{
    Node* root;
    const std::string success = "Success\n";
    const std::string fail_path = "Fail: Incorrect path\n";
    const std::string fail_root = "Fail: You can't delete empty root\n";
public:
    Speak_Tree();
    ~Speak_Tree();
    std::string add(std::string path, std::string tree);
    std::string print(std::string path);
    std::string del(std::string path);
private:
    void free_node(Node* node);
    std::string trace(Node* curr);
    Node* find(std::string path);
    Node* build_tree(std::string tree);
};

Speak_Tree::Speak_Tree()
{
    root = new Node("()");
}

Speak_Tree::~Speak_Tree()
{
    free_node(root);
}

void Speak_Tree::free_node(Node* node)
{
    if (node != nullptr)
    {
        free_node(node->get_left());
        free_node(node->get_right());
        delete node;
    }
}

std::string Speak_Tree::trace(Node* curr)
{
    std::string dump("");
    if (curr != nullptr)
    {
        dump += "|";
        if (curr->get_left())
        {
            dump += "<" + trace(curr->get_left()) + ">";
        }
        dump += curr->get_key();
        if (curr->get_right())
        {
            dump += "<" + trace(curr->get_right()) + ">";
        }
        dump += "|";
    }
    return dump;
}

Node* Speak_Tree::find(std::string path)
{
    Node* curr = root;
    for (size_t i = 0; i < path.length() && curr != nullptr; i++)
    {
        curr = (path[i] == 'l' ? curr->get_left() : curr->get_right());
    }
    return curr;
}

Node* Speak_Tree::build_tree(std::string t)
{
    Node* new_tree = new Node("()");
    size_t fst_right_br = 0, scn_left_br = 0;
    size_t i = 0;
    int k = 0;
    for (i = 0; i < t.length(); i++)
    {
        if (t[i] == '<') { k += 1; }
        if (t[i] == '>') { k -= 1; }
        if (k == 0) { break; }
    }
    fst_right_br = i;
    for (i = fst_right_br; t[i] != '<' && i < t.length(); i++);
    scn_left_br = i;
    std::string left = t.substr(1, fst_right_br - 1);
    std::string val = t.substr(fst_right_br + 1, scn_left_br - fst_right_br - 1);
    std::string right = t.substr(scn_left_br + 1, t.length() - scn_left_br - 2);
    if (!right.empty())
    {
        if (right == "()")
        {
            new_tree->set_right(new Node(right));
        }
        else
        {
            new_tree->set_right(build_tree(right));
        }
        new_tree->get_right()->set_parent(new_tree);
    }
    if (!left.empty())
    {
        if (left == "()")
        {
            new_tree->set_left(new Node(left));
        }
        else
        {
            new_tree->set_left(build_tree(left));
        }
        new_tree->get_left()->set_parent(new_tree);
    }
    new_tree->set_key(val);
    return new_tree;
}

std::string Speak_Tree::add(std::string path, std::string tree)
{
    Node* curr = find(path);
    if (curr == nullptr)
    {
        return fail_path;
    }
    Node* new_tree = build_tree(tree);
    new_tree->set_parent(curr->get_parent());
    if (curr->get_parent())
    {
        if (curr->get_parent()->get_left() == curr) {
            curr->get_parent()->set_left(new_tree);
            new_tree->set_parent(curr->get_parent());
        } else {
            curr->get_parent()->set_right(new_tree);
        }
        free_node(curr);
    } else { //root
        delete curr;
        root = new_tree;
    }
    return success;
}

std::string Speak_Tree::print(std::string path)
{
    Node* curr = find(path);
    if (curr == nullptr)
    {
        return fail_path;
    }
    std::string result = success + " " + trace(curr) + "\n";
    return result;
}

std::string Speak_Tree::del(std::string path)
{
    Node* curr = find(path);
    if (curr == nullptr)
    {
        return fail_path;
    }
    if (curr == root && curr->get_key() == "()")
    {
        return fail_root;
    }
    if (curr->get_parent())
    {
        if (curr->get_parent()->get_left() == curr) {
            curr->get_parent()->set_left(nullptr);
        } else {
            curr->get_parent()->set_right(nullptr);
        }
        free_node(curr);
    } else {
        delete curr;
        root = new Node("()");
    }
    return success;
}


const std::string error_msg = "tear off your hands\n";
const std::string com_add("add");
const std::string com_add2("a");
const std::string com_print("print");
const std::string com_print2("p");
const std::string com_del("del");
const std::string com_del2("d");
pid_t pid;
char *buf;

void handler(int)
{
    if (buf != nullptr)
    {
        free(buf);
    }
    kill(pid, SIGINT);
}


int main()
{
    pid = fork();
    if (pid)
    {
        signal(SIGINT, handler);
        std::cout << "Server starter with pid = " << pid << std::endl;
        int status;
        waitpid(pid, &status, 0);
        return 0;
    }
    setsid();
    addrinfo hints;
    
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;

    addrinfo *result;
    if (getaddrinfo(nullptr, "8442", &hints, &result) != 0)
    {
        std::exit(EXIT_FAILURE);
    }
    if (result == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }

    int socket_fd;
    socket_fd = socket(result->ai_family, result->ai_socktype,
                        result->ai_protocol);
    if (socket_fd == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    int sso_status = 1;
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR,
                &sso_status, sizeof(int)) == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    if (bind(socket_fd, result->ai_addr, result->ai_addrlen) == -1)
    {
        std::exit(EXIT_FAILURE);
    }
    
    const int backlog = 5;
    if (listen(socket_fd, backlog) == -1)
    {
        std::exit(EXIT_FAILURE);
    }

    pollfd fd[backlog + 1];
    fd[0].fd = socket_fd;
    fd[0].events = POLLIN;
   
    const int timeout = -1;
    int clients = 1;

    Speak_Tree head;
    const size_t buf_len = 1024 + 1;
    buf = safe_malloc(buf_len);
    std::string message;
    while (true)
    {
        //sigpipe =(
        safe_poll(fd, clients, timeout);
        for (int i = 1; i < clients; i++)
        {
            if (fd[i].revents & (POLLERR | POLLHUP))
            {
                fd[i] = fd[clients - 1];
                clients -= 1;
                continue;
            }
            if (fd[i].revents & POLLIN)
            {
                int ret = safe_check_input(fd[i].fd, buf, buf_len);
                if (ret == -1)
                {
                    safe_write(fd[i].fd, error_msg.data(), error_msg.size());
                    continue;
                }
                if (ret == -2)
                {
                    if (fd[i].events & POLLOUT)
                    {
                        fd[i].events = POLLOUT;
                    }
                    else
                    {
                        fd[i].events = 0;
                    }
                }
                std::vector<std::string> cm = get_command(buf);
                if (cm[0] == com_add || cm[0] == com_add2)
                {
                    message = head.add(cm[1], cm[2]);
                }
                else if (cm[0] == com_print || cm[0] == com_print2)
                {
                    message = head.print(cm[1]);
                }
                else if (cm[0] == com_del || cm[0] == com_del2)
                {
                    message = head.del(cm[1]);
                }
                safe_write(fd[i].fd, message.data(), message.size());
            }
        }

        if (fd[0].revents && POLLIN)
        {
            int fd_acc = accept(socket_fd, result->ai_addr, &result->ai_addrlen);
            if (fd_acc == -1)
            {
                std::exit(EXIT_FAILURE);
            }
            fd[clients].fd = fd_acc;
            fd[clients].events = POLLIN | POLLERR | POLLHUP;
            clients += 1;
        }
    }
}

std::vector<std::string> get_command(const std::string line)
{
    //heeeeeell
    std::vector<std::string> tmp;
    size_t pos_fst_wspace = line.find(" ", 0);
    std::string command = line.substr(0, pos_fst_wspace);
    tmp.push_back(command);
    int ret = check_command(command);
    if (ret == 2)
    {
        std::string path = line.substr(pos_fst_wspace + 1,
                            line.length() - pos_fst_wspace - 3);
        tmp.push_back(path);
    }
    if (ret == 1)
    {
        size_t pos_scn_wspace = line.find(" ", pos_fst_wspace + 1);
        std::string path = line.substr(pos_fst_wspace + 1,
                        pos_scn_wspace - pos_fst_wspace - 2);
        tmp.push_back(path);
        std::string s_tree = line.substr(pos_scn_wspace + 1,
                        line.length() - pos_scn_wspace - 2);
        tmp.push_back(s_tree);
    }
    return tmp;
}

bool check_path(std::string path)
{
    for (size_t i = 0; i < path.length() - 1; i++)
    {
        if (path[i] != 'l' && path[i] != 'r')
        {
            return false;
        }
    }
    return path.back() == 'h';
}

int check_command(std::string command)
{
    if (command == com_add || command == com_add2)
    {
        return 1;
    }
    if (command == com_print || command == com_del
        || command == com_print2 || command == com_del2)
    {
        return 2;
    }
    return 0;
}

int check_tree(std::string s_tree)
{
    //heeeeeeell
    bool left = false;
    bool right = false;
    size_t i;
    std::stack<char> stack;
    for (i = 0; i < s_tree.length(); i++)
    {
        char c = s_tree[i];
        if (c == '<')
        {
            stack.push(c);
            right = left;
        } else
        if (c == '(')
        {
            if (stack.empty())
            {
                return -1;
            }
            stack.push(c);
        } else
        if (c == ')')
        {
            if (stack.empty() || stack.top() != '(')
            {
                return -1;
            }
            stack.pop();
        } else
        if (c == '>')
        {
            if (stack.empty())
            {
                return -1;
            }
            if (stack.top() == '<')
            {
                stack.pop();
                if (stack.empty())
                {
                    left |= true;
                }
            } else {
                return -1;
            }
        }
    }
    if (right && stack.empty())
    {
        return 0;
    }
    return (right) ? -1 : -2;
}

int check_buf(std::string line)
{
    ///aaaaaaaa
    size_t pos_fst_wspace = line.find(" ", 0);
    if (pos_fst_wspace == std::string::npos)
    {
        return -1;
    }
    std::string command = line.substr(0, pos_fst_wspace);
    int ret = check_command(command);
    if (!ret)
    {
        return -1;
    }
    if (ret == 2)
    {
        std::string path = line.substr(pos_fst_wspace + 1,
                            line.length() - pos_fst_wspace - 2);
        if (!check_path(path))
        {
            return -1;
        }
    }
    if (ret == 1)
    {
        size_t pos_scn_wspace = line.find(" ", pos_fst_wspace + 1);
        if (pos_scn_wspace == std::string::npos)
        {
            return -1;
        }
        std::string path = line.substr(pos_fst_wspace + 1,
                        pos_scn_wspace - pos_fst_wspace - 1);
        if (!check_path(path))
        {
            return -1;
        }
        std::string s_tree = line.substr(pos_scn_wspace + 1,
                        line.length() - pos_scn_wspace - 2);
        ret = check_tree(s_tree);
        return ret;
    }
    return 0;
}

int safe_check_input(int fd, char *buf, size_t len)
{
    size_t read_count = 0;
    int fail = 1;
    int curr_read = -1;
    while (curr_read && fail)
    {
        curr_read = safe_read(fd, buf + read_count, len - read_count);
        if (curr_read == 0)
        {
            return -2;
        }
        read_count += curr_read;
        buf[read_count] = '\0';
        //-1 super fail; -2 waiting; 0 - super ok
        fail = check_buf(buf);
        if (fail == -1)
        {
            return -1;
        }
    }
    return 0;
}

int safe_poll(pollfd fds[], int nfds, int timeout)
{
    int count = poll(fds, nfds, timeout);
    if (count == -1)
    {
        perror("Error");
        std::exit(EXIT_FAILURE);
    }
    return count;
}

void safe_write(int fd, const char *buf, size_t len)
{
    size_t write_count = 0;
    while (write_count < len)
    {
        int curr_write = write(fd, buf + write_count, len - write_count);
        if (curr_write == -1)
        {
            std::exit(EXIT_FAILURE);
        }
        write_count += curr_write;
    }
}

int safe_read(int fd, char *buf, size_t len)
{
    int read_count = read(fd, buf, len);
    if (read_count < 0)
    {
        perror("Error");
        std::exit(EXIT_FAILURE);
    }
    return read_count;
}

char* safe_malloc(size_t size)
{
    void *ptr = malloc(size);
    if (ptr == nullptr)
    {
        std::exit(EXIT_FAILURE);
    }
    return static_cast<char*>(ptr);
}

