#include <winsock2.h>
#include <ws2tcpip.h>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
using namespace std;
using namespace cv;

// Говорим линковщику подключить библиотеку сокетов Windows
// (Если будете компилировать через MinGW/g++, эта строка игнорируется, 
// и нужно будет добавить флаг -lws2_32 при компиляции)
#pragma comment(lib, "ws2_32.lib")

int main() {
    // 1. Инициализация сокетов Windows
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ошибка инициализации Winsock\n";
        return -1;
    }

    // 2. Создаем сокет
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        cerr << "Ошибка создания сокета\n";
        WSACleanup();
        return -1;
    }

    // 3. Настраиваем адрес сервера (bore.pub)
    sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    
    // ВАЖНО: Сюда вставьте порт, который выдаст bore на Linux! (например, 34521)
    int BORE_PORT = 18837;
    serv_addr.sin_port = htons(BORE_PORT);

    // Разрешаем доменное имя bore.pub в IP-адрес
    cout << "Поиск IP-адреса для bore.pub...\n";
    struct hostent *he = gethostbyname("bore.pub");
    if (he == NULL) {
        cerr << "Ошибка: не удалось разрешить домен bore.pub\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    // Копируем полученный IP в структуру адреса
    memcpy(&serv_addr.sin_addr, he->h_addr_list[0], he->h_length);

    // 4. Подключаемся к серверу
    cout << "Подключение к bore.pub:" << BORE_PORT << "...\n";
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        cerr << "Ошибка подключения! Сервер на Linux (receiver) и bore запущены?\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }
    cout << "Успешно подключено! Запуск камеры...\n";

    // 5. Включаем камеру
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cerr << "Ошибка: Камера не найдена или занята другим приложением!\n";
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    Mat frame;
    vector<uchar> buffer;
    vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 70}; // Качество 70%

    cout << "Отправка видео начата. Для выхода закройте это окно.\n";

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        // Сжимаем кадр в JPEG в памяти
        cv::imencode(".jpg", frame, buffer, params);
        
        // Отправляем размер кадра (4 байта), чтобы Linux знал, сколько читать
        uint32_t size = buffer.size();
        if (send(sock, (char*)&size, sizeof(size), 0) == SOCKET_ERROR) break;
        
        // Отправляем сам сжатый кадр
        if (send(sock, (char*)buffer.data(), buffer.size(), 0) == SOCKET_ERROR) break;
        
        // Небольшая пауза (30 мс), чтобы не грузить процессор на 100%
        // Если нажать ESC в консоли (хотя окна нет), можно прервать цикл
        if (cv::waitKey(30) == 27) break; 
    }

    cout << "Завершение работы...\n";
    cap.release();
    closesocket(sock);
    WSACleanup();
    return 0;
}
