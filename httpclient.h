#ifndef HTTPCLIENT_H
#define HTTPCLIENT_H

#include <functional>

class QString;
class QByteArray;
class QNetworkRequest;
class QNetworkReply;
class QNetworkAccessManager;
class RequestDispatcher;

typedef std::function<void (const QString &)> DeliverHandler;

/**
 * 对 QNetworkAccessManager 进行简单封装的 HTTP 访问客户端，简化 GET、POST、PUT、DELETE、上传、下载等操作。
 * 执行请求可调用 get(), post(), put(), remove(), download(), upload()。
 * 在执行请求前可调用 header() 设置请求头，参数使用 Form 表单的方式传递则调用 param()，如果参数使用 request body
 * 传递则调用 json() 设置参数(当然也可以不是 JSON 格式，使用 request body 的情况多数是 RESTful 时，大家都是用 JSON 格式，故命名为 json)。
 * 默认 HttpClient 会创建一个 QNetworkAccessManager，如果不想使用默认的，调用 manager() 传入即可。
 * 默认不输出请求的网址参数等调试信息，如果需要输出，调用 debug(true) 即可。
 */
class HttpClient {
public:
    HttpClient(const QString &url);
    HttpClient(const QString &url, int port);
    ~HttpClient();

    /**
     * @brief 每创建一个 QNetworkAccessManager 对象都会创建一个线程，当频繁的访问网络时，为了节省线程资源，
     *     可以传入 QNetworkAccessManager 给多个请求共享(它不会被 HttpClient 删除，用户需要自己手动删除)。
     *     如果没有使用 useManager() 传入一个 QNetworkAccessManager，则 HttpClient 会自动的创建一个，并且在网络访问完成后删除它。
     * @param  manager QNetworkAccessManager 对象
     * @return 返回 HttpClient 的引用，可以用于链式调用
     */
    HttpClient& manager(QNetworkAccessManager *manager);

    /**
     * @brief  参数 debug 为 true 则使用 debug 模式，请求执行时输出请求的 URL 和参数等
     * @param  debug 是否启用调试模式
     * @return 返回 HttpClient 的引用，可以用于链式调用
     */
    HttpClient& debug(bool debug);

    /**
     * @brief 添加请求的参数
     * @param name  参数的名字
     * @param value 参数的值
     * @return 返回 HttpClient 的引用，可以用于链式调用
     */
    HttpClient& param(const QString &name, const QString &value);

    /**
     * @brief 添加请求的参数，使用 Json 格式，例如 "{\"name\": \"Alice\"}"
     * @param json Json 格式的参数字符串
     * @return
     */
    HttpClient& json(const QString &json);

    HttpClient& header(const QString &header, const QString &value);

    void get(DeliverHandler successHandler,
             DeliverHandler errorHandler = NULL,
             const char *encoding = "UTF-8");

    void post(DeliverHandler successHandler,
              DeliverHandler errorHandler = NULL,
              const char *encoding = "UTF-8");

    void put(DeliverHandler successHandler,
             DeliverHandler errorHandler = NULL,
             const char *encoding = "UTF-8");

    // 执行 DELETE 请求
    void remove(DeliverHandler successHandler,
                DeliverHandler errorHandler = NULL,
                const char *encoding = "UTF-8");

    // 下载
    void download(const QString &savePath,
                  DeliverHandler successHandler = NULL,
                  DeliverHandler errorHandler = NULL);

    // 当有数据可读取时回调 readyRead
    void download(std::function<void (const QByteArray &)> readyRead,
                  DeliverHandler successHandler = NULL,
                  DeliverHandler errorHandler = NULL);

    // 上传文件
    void upload(const QString &path, DeliverHandler successHandler = NULL,
                DeliverHandler errorHandler = NULL,
                const char *encoding = "UTF-8");

    // 上传数据
    void upload(const QByteArray &data, DeliverHandler successHandler = NULL,
                DeliverHandler errorHandler = NULL,
                const char *encoding = "UTF-8");
private:
    RequestDispatcher *_dispatcher;
};
#endif // HTTPCLIENT_H
