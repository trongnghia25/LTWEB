// Bài 2: Lập trình ứng dụng time_server thực hiện chức năng sau:
// + Chấp nhận nhiều kết nối từ các client.
// + Cient gửi lệnh GET_TIME [format] để nhận thời gian từ server.
// + format là định dạng thời gian server cần trả về client. Các format cần hỗ trợ gồm:
//     dd/mm/yyyy – vd: 30/01/2023
//     dd/mm/yy – vd: 30/01/23
//     mm/dd/yyyy – vd: 01/30/2023
//     mm/dd/yy – vd: 01/30/23
// + Cần kiểm tra tính đúng đắn của lệnh client gửi lên server.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <time.h>
// Kiem tra cu phap
int idformat;
int isValidDateFormat(char *format) {
    // Các định dạng thời gian yêu cầu
    char *validFormats[] = {"[dd/mm/yyyy]", "[dd/mm/yy]", "[mm/dd/yyyy]", "[mm/dd/yy]"};

    for (int i = 0; i < sizeof(validFormats) / sizeof(validFormats[0]); i++) {
        if (strcmp(format, validFormats[i]) == 0) {
            idformat=i;
            return 1; // Định dạng hợp lệ
        }
    }

    return 0; // Định dạng không hợp lệ
}
int Test(char* input){
    // Tách từ "GET_TIME" và format
    char *token = strtok(input, " ");
    char *format = strtok(NULL, " ");
    printf("%s",token);
    // Kiểm tra xem có tồn  tại từ "GET_TIME" không
    if (token != NULL && strcmp(token, "GET_TIME") == 0) {
        if (format != NULL && isValidDateFormat(format)) {
            printf("Xâu \"%s\" là định dạng hợp lệ.\n", input);
            return 1;
        } else {
            printf("Xâu \"%s\" không hợp lệ hoặc không có format.\n", input);
        }
    } else {
        printf("Xâu \"%s\" không phải là dạng \"GET_TIME [format]\".\n", input);
    }
    return 0;
}
void signalHanlder(int signo) {
    pid_t pid = wait(NULL);
    printf("Child process terminated, pid = %d\n", pid);
}
// Get time
char formatted_time[20];
void GetTime(int idformat){
    time_t rawtime;
    struct tm *timeinfo;

    // Lấy thời gian hiện tại
    time(&rawtime);
    timeinfo = localtime(&rawtime);

    // Format thời gian theo các định dạng khác nhau

    // Định dạng dd/mm/yyyy
    if(idformat==0)
    strftime(formatted_time, sizeof(formatted_time), "%d/%m/%Y", timeinfo);

    // Định dạng dd/mm/yy
    if(idformat==1)
    strftime(formatted_time, sizeof(formatted_time), "%d/%m/%y", timeinfo);

    // Định dạng mm/dd/yyyy
    if(idformat==2)
    strftime(formatted_time, sizeof(formatted_time), "%m/%d/%Y", timeinfo);

    // Định dạng mm/dd/yy
    if(idformat==3)
    strftime(formatted_time, sizeof(formatted_time), "%m/%d/%y", timeinfo);
}
int main() {
    // Tao socket cho ket noi
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1) {
        perror("socket() failed");
        return 1;
    }

    // Khai bao dia chi server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);

    // Gan socket voi cau truc dia chi
    if (bind(listener, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("bind() failed");
        return 1;
    }

    // Chuyen socket sang trang thai cho ket noi
    if (listen(listener, 5)) {
        perror("listen() failed");
        return 1;
    }

    signal(SIGCHLD, signalHanlder);

    while (1) {
        int client = accept(listener, NULL, NULL);
        printf("New client accepted, client = %d\n", client);

        if (fork() == 0) {
            // Tien trinh con, xu ly yeu cau tu client
            // Dong socket listener
            close(listener);

            char buf[256];
            while (1) {
                int ret = recv(client, buf, sizeof(buf), 0);
                if (ret <= 0)
                    break;

                buf[strcspn(buf, "\n")] = '\0';
                printf("Received: %s", buf);
                if(Test(buf)==1){
                    GetTime(idformat);
                    send(client, formatted_time, strlen(formatted_time), 0);
                }else{
                    send(client, "Sai cu phap, vui long gui lai", 100, 0);

                }

            }

            // Ket thuc tien trinh con
            exit(0);
        }

        // Dong socket client o tien trinh cha
        close(client);
    }

    return 0;
}