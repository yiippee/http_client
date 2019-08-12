#include "httpclient.h"
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QUrlQuery>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QHttpPart>
#include <QHttpMultiPart>

class RequestDispatcher {
public:
    RequestDispatcher(const QString &url);
    RequestDispatcher(const QString &url, int p);

    QString   url;    // 请求的 URL
    int       port;
    QUrlQuery params; // 请求的参数使用 Form 格式
    QString   json;   // 请求的参数使用 Json 格式

    QHash<QString, QString> headers; // 请求头
    QNetworkAccessManager  *manager;

    bool useJson; // 为 true 时请求使用 Json 格式传递参数，否则使用 Form 格式传递参数
    bool debug;   // 为 true 时输出请求的 URL 和参数

    enum HttpMethod {
        GET, POST, PUT, DELETE, UPLOAD /* UPLOAD 不是 HTTP Method，只是为了上传时特殊处理而定义的 */
    };

    // 获取 Manager
    static QNetworkAccessManager* getManager(RequestDispatcher *dispatcher, bool *internal);

    // 使用用户设定的 URL、请求头等创建 Request
    static QNetworkRequest createRequest(RequestDispatcher *dispatcher, HttpMethod method);

    // 执行请求的辅助函数
    static void executeQuery(RequestDispatcher *dispatcher, HttpMethod method,
                             DeliverHandler successHandler,
                             DeliverHandler errorHandler,
                             const char *encoding);

    // 上传文件或者数据
    static void upload(RequestDispatcher *dispatcher,
                       const QString &path, const QByteArray &data,
                       DeliverHandler successHandler,
                       DeliverHandler errorHandler,
                       const char *encoding);

    // 读取服务器响应的数据
    static QString readReply(QNetworkReply *reply, const char *encoding = "UTF-8");

    // 请求结束的处理函数
    static void handleFinish(bool debug,
                             const QString &successMessage,
                             const QString &errorMessage,
                             DeliverHandler successHandler,
                             DeliverHandler errorHandler,
                             QNetworkReply *reply, QNetworkAccessManager *manager);
};

RequestDispatcher::RequestDispatcher(const QString &url) :
    url(url),
    manager(NULL),
    useJson(false),
    debug(false) {

}

RequestDispatcher::RequestDispatcher(const QString &url, int p) :
    port(p),
    url(url),
    manager(NULL),
    useJson(false),
    debug(false) {

}

// 注意: 不要在回调函数中使用 d，因为回调函数被调用时 HttpClient 对象很可能已经被释放掉了。
HttpClient::HttpClient(const QString &url) :
    _dispatcher(new RequestDispatcher(url)) {

}

HttpClient::HttpClient(const QString &url, int port) :
    _dispatcher(new RequestDispatcher(url, port)) {

}

HttpClient::~HttpClient() {
    delete _dispatcher;
}

HttpClient &HttpClient::manager(QNetworkAccessManager *manager) {
    _dispatcher->manager = manager;
    return *this;
}

// 传入 debug 为 true 则使用 debug 模式，请求执行时输出请求的 URL 和参数等
HttpClient &HttpClient::debug(bool debug) {
    _dispatcher->debug = debug;
    return *this;
}

// 添加 Form 格式参数
HttpClient &HttpClient::param(const QString &name, const QString &value) {
    _dispatcher->params.addQueryItem(name, value);
    return *this;
}

// 添加 Json 格式参数
HttpClient &HttpClient::json(const QString &json) {
    _dispatcher->useJson  = true;
    _dispatcher->json = json;
    return *this;
}

// 添加访问头
HttpClient &HttpClient::header(const QString &header, const QString &value) {
    _dispatcher->headers[header] = value;
    return *this;
}

// 执行 GET 请求
void HttpClient::get(DeliverHandler successHandler,
                     DeliverHandler errorHandler,
                     const char *encoding) {
    RequestDispatcher::executeQuery(_dispatcher, RequestDispatcher::GET, successHandler, errorHandler, encoding);
}

// 执行 POST 请求
void HttpClient::post(DeliverHandler successHandler,
                      DeliverHandler errorHandler,
                      const char *encoding) {
    RequestDispatcher::executeQuery(_dispatcher, RequestDispatcher::POST, successHandler, errorHandler, encoding);
}

// 执行 PUT 请求
void HttpClient::put(DeliverHandler successHandler,
                     DeliverHandler errorHandler,
                     const char *encoding) {
    RequestDispatcher::executeQuery(_dispatcher, RequestDispatcher::PUT, successHandler, errorHandler, encoding);
}

// 执行 DELETE 请求
void HttpClient::remove(DeliverHandler successHandler,
                        DeliverHandler errorHandler,
                        const char *encoding) {
    RequestDispatcher::executeQuery(_dispatcher, RequestDispatcher::DELETE, successHandler, errorHandler, encoding);
}

void HttpClient::download(const QString &destinationPath,
                          DeliverHandler successHandler,
                          DeliverHandler errorHandler) {
    bool  debug = _dispatcher->debug;
    QFile *file = new QFile(destinationPath);
    if (file->open(QIODevice::WriteOnly)) {
        download([=](const QByteArray &data) {
            file->write(data);
        }, [=](const QString &) {
            // 请求结束后释放文件对象
            file->flush();
            file->close();
            file->deleteLater();
            if (debug) {
                qDebug().noquote() << QStringLiteral("下载完成，保存到: %1").arg(destinationPath);
            }
            if (NULL != successHandler) {
                successHandler(QStringLiteral("下载完成，保存到: %1").arg(destinationPath));
            }
        }, errorHandler);
    } else {
        // 打开文件出错
        if (debug) {
            qDebug().noquote() << QStringLiteral("打开文件出错: %1").arg(destinationPath);
        }
        if (NULL != errorHandler) {
            errorHandler(QStringLiteral("打开文件出错: %1").arg(destinationPath));
        }
    }
}

// 使用 GET 进行下载，当有数据可读取时回调 readyRead(), 大多数情况下应该在 readyRead() 里把数据保存到文件
void HttpClient::download(std::function<void (const QByteArray &)> readyRead,
                          DeliverHandler successHandler,
                          DeliverHandler errorHandler) {
    bool debug = _dispatcher->debug;
    bool internal;

    QNetworkAccessManager *manager = RequestDispatcher::getManager(_dispatcher, &internal);
    QNetworkRequest request = RequestDispatcher::createRequest(_dispatcher, RequestDispatcher::GET);
    QNetworkReply *reply = manager->get(request);

    // 有数据可读取时回调 readyRead()
    QObject::connect(reply, &QNetworkReply::readyRead, [=] {
        readyRead(reply->readAll());
    });

    // 请求结束
    QObject::connect(reply, &QNetworkReply::finished, [=] {
        QString successMessage = "下载完成"; // 请求结束时一次性读取所有响应数据
        QString errorMessage   = reply->errorString();
        RequestDispatcher::handleFinish(debug, successMessage, errorMessage, successHandler, errorHandler,
                                        reply, internal ? manager : NULL);
    });
}

// 上传文件
void HttpClient::upload(const QString &path,
                        DeliverHandler successHandler,
                        DeliverHandler errorHandler,
                        const char *encoding) {
    RequestDispatcher::upload(_dispatcher, path, QByteArray(), successHandler, errorHandler, encoding);
}

// 上传数据
void HttpClient::upload(const QByteArray &data,
                        DeliverHandler successHandler,
                        DeliverHandler errorHandler,
                        const char *encoding) {
    RequestDispatcher::upload(_dispatcher, QString(), data, successHandler, errorHandler, encoding);
}

// 上传文件或者数据的实现
void RequestDispatcher::upload(RequestDispatcher *dispatcher,
                               const QString &path, const QByteArray &data,
                               DeliverHandler successHandler,
                               DeliverHandler errorHandler,
                               const char *encoding) {
    bool debug = dispatcher->debug;
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    // 创建 Form 表单的参数 Text Part
    QList<QPair<QString, QString> > paramItems = dispatcher->params.queryItems();
    for (int i = 0; i < paramItems.size(); ++i) {
        QHttpPart textPart;
        QString name  = paramItems.at(i).first;
        QString value = paramItems.at(i).second;
        textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data; name=\"%1\"").arg(name));
        textPart.setBody(value.toUtf8());
        multiPart->append(textPart);
    }

    if (!path.isEmpty()) {
        // path 不为空时，上传文件
        QFile *file = new QFile(path);
        file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
        // 如果文件打开失败，则释放资源返回
        if(!file->open(QIODevice::ReadOnly)) {
            QString errorMessage = QStringLiteral("打开文件失败[%2]: %1").arg(path).arg(file->errorString());
            if (debug) {
                qDebug().noquote() << errorMessage;
            }
            if (NULL != errorHandler) {
                errorHandler(errorMessage);
            }
            multiPart->deleteLater();
            return;
        }
        // 文件上传的参数名为 file，值为文件名
        QString   disposition = QString("form-data; name=\"file\"; filename=\"%1\"").arg(file->fileName());
        QHttpPart filePart;
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
        filePart.setBodyDevice(file);
        multiPart->append(filePart);
    } else {
        // 上传数据
        QString   disposition = QString("form-data; name=\"file\"; filename=\"no-name\"");
        QHttpPart dataPart;
        dataPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(disposition));
        dataPart.setBody(data);
        multiPart->append(dataPart);
    }

    bool internal;
    QNetworkAccessManager *manager = RequestDispatcher::getManager(dispatcher, &internal);
    QNetworkRequest        request = RequestDispatcher::createRequest(dispatcher, RequestDispatcher::UPLOAD);
    QNetworkReply           *reply = manager->post(request, multiPart);

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        multiPart->deleteLater(); // 释放资源: multiPart + file
        QString successMessage = RequestDispatcher::readReply(reply, encoding); // 请求结束时一次性读取所有响应数据
        QString errorMessage   = reply->errorString();
        RequestDispatcher::handleFinish(debug, successMessage, errorMessage, successHandler, errorHandler,
                                        reply, internal ? manager : NULL);
    });
}

// 执行请求的辅助函数
void RequestDispatcher::executeQuery(RequestDispatcher *dispatcher, HttpMethod method,
                                     DeliverHandler successHandler,
                                     DeliverHandler errorHandler,
                                     const char *encoding) {
    // 如果不使用外部的 manager 则创建一个新的，在访问完成后会自动删除掉
    bool debug = dispatcher->debug;
    bool internal;

    QNetworkAccessManager *manager = RequestDispatcher::getManager(dispatcher, &internal);
    QNetworkRequest request = RequestDispatcher::createRequest(dispatcher, method);
    QNetworkReply *reply = NULL;

    switch (method) {
    case RequestDispatcher::GET:
        reply = manager->get(request);
        break;
    case RequestDispatcher::POST:
        reply = manager->post(request, dispatcher->useJson ? dispatcher->json.toUtf8() : dispatcher->params.toString(QUrl::FullyEncoded).toUtf8());
        break;
    case RequestDispatcher::PUT:
        reply = manager->put(request, dispatcher->useJson ? dispatcher->json.toUtf8() : dispatcher->params.toString(QUrl::FullyEncoded).toUtf8());
        break;
    case RequestDispatcher::DELETE:
        reply = manager->deleteResource(request);
        break;
    default:
        break;
    }

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        QString successMessage = RequestDispatcher::readReply(reply, encoding); // 请求结束时一次性读取所有响应数据
        QString errorMessage   = reply->errorString();
        RequestDispatcher::handleFinish(debug, successMessage, errorMessage, successHandler, errorHandler,
                                        reply, internal ? manager : NULL);
    });

    QObject::connect(reply, &QNetworkReply::finished, [=] {
        QString successMessage = RequestDispatcher::readReply(reply, encoding); // 请求结束时一次性读取所有响应数据
        QString errorMessage   = reply->errorString();
        RequestDispatcher::handleFinish(debug, successMessage, errorMessage, successHandler, errorHandler,
                                        reply, internal ? manager : NULL);

        });
}

QNetworkAccessManager* RequestDispatcher::getManager(RequestDispatcher *dispatcher, bool *internal) {
    *internal = dispatcher->manager == NULL;
    return *internal ? new QNetworkAccessManager() : dispatcher->manager;
}

QNetworkRequest RequestDispatcher::createRequest(RequestDispatcher *dispatcher, HttpMethod method) {
    bool get = method == HttpMethod::GET;
    bool upload = method == RequestDispatcher::UPLOAD;
    bool postForm = !get && !upload && !dispatcher->useJson;
    bool postJson = !get && !upload &&  dispatcher->useJson;

    // 如果是 GET 请求，并且参数不为空，则编码请求的参数，放到 URL 后面
    if (get && !dispatcher->params.isEmpty()) {
        dispatcher->url += "?" + dispatcher->params.toString(QUrl::FullyEncoded);
    }

    // 调试时输出网址和参数
    if (dispatcher->debug) {
        qDebug().noquote() << QStringLiteral("网址:") << dispatcher->url;
        if (postJson) {
            qDebug().noquote() << QStringLiteral("参数:") << dispatcher->json;
        } else if (postForm || upload) {
            QList<QPair<QString, QString> > paramItems = dispatcher->params.queryItems();
            // 按键值对的方式输出参数
            for (int i = 0; i < paramItems.size(); ++i) {
                QString name  = paramItems.at(i).first;
                QString value = paramItems.at(i).second;
                if (0 == i) {
                    qDebug().noquote() << QStringLiteral("参数: %1=%2").arg(name).arg(value);
                } else {
                    qDebug().noquote() << QString("     %1=%2").arg(name).arg(value);
                }
            }
        }
    }

    // 如果是 POST 请求，useJson 为 true 时添加 Json 的请求头，useJson 为 false 时添加 Form 的请求头
    if (postForm) {
        dispatcher->headers["Content-Type"] = "application/x-www-form-urlencoded";
    } else if (postJson) {
        dispatcher->headers["Accept"]       = "application/json; charset=utf-8";
        dispatcher->headers["Content-Type"] = "application/json";
    }

    // 把请求的头添加到 request 中
    QUrl url(dispatcher->url);
    url.setPort(dispatcher->port);
    //QNetworkRequest request(QUrl(dispatcher->url));
    QNetworkRequest request(url);
    QHashIterator<QString, QString> iter(dispatcher->headers);
    while (iter.hasNext()) {
        iter.next();
        request.setRawHeader(iter.key().toUtf8(), iter.value().toUtf8());
    }
    return request;
}

QString RequestDispatcher::readReply(QNetworkReply *reply, const char *encoding) {
    QTextStream in(reply);
    QString result;
    in.setCodec(encoding);
    while (!in.atEnd()) {
        result += in.readLine();
    }
    return result;
}

void RequestDispatcher::handleFinish(bool debug,
                                     const QString &successMessage,
                                     const QString &errorMessage,
                                     DeliverHandler successHandler,
                                     DeliverHandler errorHandler,
                                     QNetworkReply *reply, QNetworkAccessManager *manager) {
    if (reply->error() == QNetworkReply::NoError) {
        // 请求成功
        if (debug) {
            qDebug().noquote() << QStringLiteral("[成功]请求结束: %1").arg(successMessage);
        }
        if (NULL != successHandler) {
            successHandler(successMessage);
        }
    } else {
        // 请求失败
        if (debug) {
            qDebug().noquote() << QStringLiteral("[失败]请求结束: %1").arg(errorMessage);
        }
        if (NULL != errorHandler) {
            errorHandler(errorMessage);
        }
    }
    // 释放资源
    reply->deleteLater();
    if (NULL != manager) {
        manager->deleteLater();
    }
}
