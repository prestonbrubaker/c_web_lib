// hello.c
#include "hello.h"

// Manually define structures (no external headers)
struct in_addr {
    unsigned int s_addr;
};

struct sockaddr_in {
    unsigned short sin_family;
    unsigned short sin_port;
    struct in_addr sin_addr;
    char sin_zero[8];
};

// Convert number to ASCII string (handles positive, negative, and zero; e.g., 3 -> "3\0", -14 -> "-14\0")
static void write_number(long num, char *buf, unsigned long *len) {
    unsigned long i = 0;
    if (num < 0) {
        buf[i++] = '-';
        num = -num;
    }
    char temp[20];
    unsigned long j = 0;
    if (num == 0) {
        temp[j++] = '0';
    } else {
        do {
            temp[j++] = '0' + (num % 10);
            num /= 10;
        } while (num > 0);
    }
    while (j > 0) {
        buf[i++] = temp[--j];
    }
    buf[i] = '\0';
    *len = i;
}

void tcp_server(unsigned short port) {
    long sockfd;

    // Syscall: socket(AF_INET=2, SOCK_STREAM=1, protocol=0)
    asm volatile (
        "mov $41, %%rax\n"     // Syscall number for socket (41)
        "mov $2, %%rdi\n"      // domain (AF_INET)
        "mov $1, %%rsi\n"      // type (SOCK_STREAM)
        "mov $0, %%rdx\n"      // protocol (0 = auto)
        "syscall\n"
        : "=a" (sockfd)        // Output: rax -> sockfd
        :                      // No inputs (immediates used)
        : "rcx", "r11", "rdi", "rsi", "rdx"  // Clobbers (no memory access)
    );

    // Debug: Print sockfd to stderr
    {
        char num_buf[20];
        unsigned long num_len;
        write_number(sockfd, num_buf, &num_len);
        if (num_len > 0) {
            asm volatile (
                "mov $1, %%rax\n"      // Syscall number for write (1)
                "mov $2, %%rdi\n"      // fd 2 (stderr)
                "mov %0, %%rsi\n"      // buffer
                "mov %1, %%rdx\n"      // length
                "syscall\n"
                :
                : "r" (num_buf), "r" (num_len)
                : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
            );
        }
        // Print newline
        asm volatile (
            "mov $1, %%rax\n"
            "mov $2, %%rdi\n"
            "mov $10, %%rsi\n"     // '\n'
            "mov $1, %%rdx\n"
            "syscall\n"
            :
            :
            : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
        );
    }

    if (sockfd < 0) {
        // Exit on error
        asm volatile (
            "mov $60, %%rax\n"     // Syscall number for exit (60)
            "mov $1, %%rdi\n"      // exit status 1
            "syscall\n"
            :
            :
            : "rax", "rdi"
        );
    }

    // Prepare address structure (static for data segment placement)
    static struct sockaddr_in addr;
    addr.sin_family = 2;  // AF_INET
    addr.sin_port = ((port & 0xFF) << 8) | ((port >> 8) & 0xFF);  // htons(port)
    addr.sin_addr.s_addr = 0;  // INADDR_ANY
    // Explicitly zero sin_zero (static initializes to zero, but for clarity)
    for (int i = 0; i < 8; ++i) {
        addr.sin_zero[i] = 0;
    }

    // Syscall: bind(sockfd, &addr, sizeof(addr))
    long bind_ret;
    asm volatile (
        "mov $49, %%rax\n"     // Syscall number for bind (49)
        "mov %1, %%rdi\n"      // sockfd
        "mov %2, %%rsi\n"      // &addr
        "mov $16, %%rdx\n"     // addrlen (16 bytes)
        "syscall\n"
        : "=a" (bind_ret)      // Output: rax -> bind_ret
        : "r" (sockfd), "r" (&addr)
        : "rcx", "r11", "rdi", "rsi", "rdx", "memory"  // Clobbers, including "memory" to flush stores to addr
    );

    // Debug: Print bind_ret to stderr
    {
        char num_buf[20];
        unsigned long num_len;
        write_number(bind_ret, num_buf, &num_len);
        if (num_len > 0) {
            asm volatile (
                "mov $1, %%rax\n"
                "mov $2, %%rdi\n"
                "mov %0, %%rsi\n"
                "mov %1, %%rdx\n"
                "syscall\n"
                :
                : "r" (num_buf), "r" (num_len)
                : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
            );
        }
        // Print newline
        asm volatile (
            "mov $1, %%rax\n"
            "mov $2, %%rdi\n"
            "mov $10, %%rsi\n"
            "mov $1, %%rdx\n"
            "syscall\n"
            :
            :
            : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
        );
    }

    if (bind_ret < 0) {
        // Exit on error
        asm volatile (
            "mov $60, %%rax\n"
            "mov $1, %%rdi\n"
            "syscall\n"
            :
            :
            : "rax", "rdi"
        );
    }

    // Syscall: listen(sockfd, backlog=5)
    long listen_ret;
    asm volatile (
        "mov $50, %%rax\n"     // Syscall number for listen (50)
        "mov %1, %%rdi\n"      // sockfd
        "mov $5, %%rsi\n"      // backlog
        "syscall\n"
        : "=a" (listen_ret)
        : "r" (sockfd)
        : "rcx", "r11", "rdi", "rsi"  // No memory access
    );

    // Debug: Print listen_ret to stderr
    {
        char num_buf[20];
        unsigned long num_len;
        write_number(listen_ret, num_buf, &num_len);
        if (num_len > 0) {
            asm volatile (
                "mov $1, %%rax\n"
                "mov $2, %%rdi\n"
                "mov %0, %%rsi\n"
                "mov %1, %%rdx\n"
                "syscall\n"
                :
                : "r" (num_buf), "r" (num_len)
                : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
            );
        }
        // Print newline
        asm volatile (
            "mov $1, %%rax\n"
            "mov $2, %%rdi\n"
            "mov $10, %%rsi\n"
            "mov $1, %%rdx\n"
            "syscall\n"
            :
            :
            : "rax", "rdi", "rsi", "rdx", "rcx", "r11", "memory"
        );
    }

    if (listen_ret < 0) {
        // Exit on error
        asm volatile (
            "mov $60, %%rax\n"
            "mov $1, %%rdi\n"
            "syscall\n"
            :
            :
            : "rax", "rdi"
        );
    }

    // Message to send ("hello", 5 bytes)
    static const char msg[] = "~~~~~~~~~~~~~~~~~~~~~" "\n" "hello weary traveler" "\n" "~~~~~~~~~~~~~~~~~~~~~" "\n";
    static const unsigned long msg_len = sizeof(msg) - 1;  // Exclude null terminator

    // Infinite loop: accept clients, send "hello", close
    while (1) {
        long client_fd;

        // Syscall: accept(sockfd, NULL, NULL) - blocks until client connects
        asm volatile (
            "mov $43, %%rax\n"     // Syscall number for accept (43)
            "mov %1, %%rdi\n"      // sockfd
            "mov $0, %%rsi\n"      // NULL (addr)
            "mov $0, %%rdx\n"      // NULL (addrlen)
            "syscall\n"
            : "=a" (client_fd)
            : "r" (sockfd)
            : "rcx", "r11", "rdi", "rsi", "rdx", "memory"  // "memory" for safety
        );

        if (client_fd < 0) continue;  // Skip errors (e.g., interrupted)

        // Syscall: sendto(client_fd, msg, msg_len, flags=0, NULL, 0) - acts as send
        long send_ret;
        asm volatile (
            "mov $44, %%rax\n"     // Syscall number for sendto (44)
            "mov %1, %%rdi\n"      // client_fd
            "mov %2, %%rsi\n"      // msg (memory read)
            "mov %3, %%rdx\n"      // msg_len
            "mov $0, %%r10\n"      // flags=0
            "mov $0, %%r8\n"       // NULL dest_addr
            "mov $0, %%r9\n"       // addrlen=0
            "syscall\n"
            : "=a" (send_ret)
            : "r" (client_fd), "r" (msg), "r" (msg_len)
            : "rcx", "r11", "rdi", "rsi", "rdx", "r10", "r8", "r9", "memory"  // "memory" for msg read
        );

        // Syscall: close(client_fd)
        long close_ret;
        asm volatile (
            "mov $3, %%rax\n"      // Syscall number for close (3)
            "mov %1, %%rdi\n"      // client_fd
            "syscall\n"
            : "=a" (close_ret)
            : "r" (client_fd)
            : "rcx", "r11", "rdi"  // No memory access
        );
    }
    // Server runs forever; stop with Ctrl+C
}
