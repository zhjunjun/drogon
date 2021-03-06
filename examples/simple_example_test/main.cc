/**
 *
 *  test.cc
 *  An Tao
 *
 *  Copyright 2018, An Tao.  All rights reserved.
 *  https://github.com/an-tao/drogon
 *  Use of this source code is governed by a MIT license
 *  that can be found in the License file.
 *
 *  Drogon
 *
 */

//Make a http client to test the example server app;

#include <drogon/drogon.h>
#include <trantor/net/EventLoopThread.h>
#include <trantor/net/TcpClient.h>

#include <mutex>
#include <future>
#include <unistd.h>

#define RESET "\033[0m"
#define RED "\033[31m"   /* Red */
#define GREEN "\033[32m" /* Green */

using namespace drogon;

void outputGood(const HttpRequestPtr &req, bool isHttps)
{
    static int i = 0;
    // auto start = req->creationDate();
    // auto end = trantor::Date::now();
    // int ms = end.microSecondsSinceEpoch() - start.microSecondsSinceEpoch();
    // ms = ms / 1000;
    // char str[32];
    // sprintf(str, "%6dms", ms);
    static std::mutex mtx;
    {
        std::lock_guard<std::mutex> lock(mtx);
        i++;
        std::cout << i << GREEN << '\t' << "Good" << '\t' << RED << req->methodString()
                  << " " << req->path();
        if (isHttps)
            std::cout << "\t(https)";
        std::cout << RESET << std::endl;
    }
}
void doTest(const HttpClientPtr &client, std::promise<int> &pro, bool isHttps = false)
{
    /// Test gzip
    auto req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->addHeader("accept-encoding", "gzip");
    req->setPath("/api/v1/apitest/get/111");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().length() == 5123)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody().length();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });
    /// Post json
    Json::Value json;
    json["request"] = "json";
    req = HttpRequest::newHttpJsonRequest(json);
    req->setMethod(drogon::Post);
    req->setPath("/api/v1/apitest/json");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            auto ret = resp->getJsonObject();
            if (ret && (*ret)["result"].asString() == "ok")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });
    /// 1 Get /
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            //LOG_DEBUG << resp->getBody();
            if (resp->getBody() == "<p>Hello, world!</p>")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });
    /// 2. Get /slow to test simple controller, session and filter (cookie is not supported by HttpClient now)
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/slow");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            //LOG_DEBUG << resp->getBody();
            if (resp->getBody() == "<p>Hello, world!</p>")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });
    /// 3. Post to /tpost to test Http Method constraint
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/tpost");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            //LOG_DEBUG << resp->getBody();
            if (resp->statusCode() == k405MethodNotAllowed)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/tpost");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody() == "<p>Hello, world!</p>")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// 4. Test HttpController
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/v1/apitest");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody() == "ROOT Post!!!")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/apitest");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody() == "ROOT Get!!!")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/apitest/get/abc/123");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().find("<td>p1</td>\n        <td>123</td>") != std::string::npos &&
                resp->getBody().find("<td>p2</td>\n        <td>abc</td>") != std::string::npos)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/apitest/3.14/List?P2=1234");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().find("<td>p1</td>\n        <td>3.140000</td>") != std::string::npos &&
                resp->getBody().find("<td>p2</td>\n        <td>1234</td>") != std::string::npos)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/apitest/static");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody() == "staticApi,hello!!")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    //auto loop = app().loop();

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Post);
    req->setPath("/api/v1/apitest/static");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody() == "staticApi,hello!!")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/apitest/get/111");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().length() == 5123)
            {
                outputGood(req, isHttps);
            }
            else
            {
                //LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test static function
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/handle11/11/22/?p3=33&p4=44");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().find("<td>int p1</td>\n        <td>11</td>") != std::string::npos &&
                resp->getBody().find("<td>int p4</td>\n        <td>44</td>") != std::string::npos &&
                resp->getBody().find("<td>string p2</td>\n        <td>22</td>") != std::string::npos &&
                resp->getBody().find("<td>string p3</td>\n        <td>33</td>") != std::string::npos)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test lambda
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/handle2/111/222");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().find("<td>a</td>\n        <td>111</td>") != std::string::npos &&
                resp->getBody().find("<td>b</td>\n        <td>222.000000</td>") != std::string::npos)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test std::bind and std::function
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/v1/handle4/444/333/111");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().find("<td>int p1</td>\n        <td>111</td>") != std::string::npos &&
                resp->getBody().find("<td>int p4</td>\n        <td>444</td>") != std::string::npos &&
                resp->getBody().find("<td>string p3</td>\n        <td>333</td>") != std::string::npos &&
                resp->getBody().find("<td>string p2</td>\n        <td></td>") != std::string::npos)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test file download
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/drogon.jpg");
    client->sendRequest(req, [=, &pro](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().length() == 52594)
            {
                outputGood(req, isHttps);
                auto lastModified = resp->getHeader("last-modified");
                //LOG_DEBUG << lastModified;
                // Test 'Not Modified'
                auto req = HttpRequest::newHttpRequest();
                req->setMethod(drogon::Get);
                req->setPath("/drogon.jpg");
                req->addHeader("If-Modified-Since", lastModified);
                client->sendRequest(req, [=, &pro](ReqResult result, const HttpResponsePtr &resp) {
                    if (result == ReqResult::Ok)
                    {
                        if (resp->statusCode() == k304NotModified)
                        {
                            outputGood(req, isHttps);
                            pro.set_value(1);
                        }
                        else
                        {
                            LOG_DEBUG << resp->getBody().length();
                            LOG_ERROR << "Error!";
                            exit(1);
                        }
                    }
                    else
                    {

                        LOG_ERROR << "Error!";
                        exit(1);
                    }
                });
            }
            else
            {
                LOG_DEBUG << resp->getBody().length();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test file download, It is forbidden to download files from the parent folder
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/../../drogon.jpg");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->statusCode() == k403Forbidden)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody().length();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test form post
    req = HttpRequest::newHttpFormPostRequest();
    req->setPath("/api/v1/apitest/form");
    req->setParameter("k1", "1");
    req->setParameter("k2", "安");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            auto ret = resp->getJsonObject();
            if (ret && (*ret)["result"].asString() == "ok")
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });

    /// Test attachment download
    req = HttpRequest::newHttpRequest();
    req->setMethod(drogon::Get);
    req->setPath("/api/attachment/download");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            if (resp->getBody().length() == 52594)
            {
                outputGood(req, isHttps);
            }
            else
            {
                LOG_DEBUG << resp->getBody().length();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });
    //return;
    // Test file upload
    UploadFile file1("./drogon.jpg");
    UploadFile file2("./config.example.json", "config.json", "file2");
    req = HttpRequest::newFileUploadRequest({file1, file2});
    req->setPath("/api/attachment/upload");
    req->setParameter("P1", "upload");
    req->setParameter("P2", "test");
    client->sendRequest(req, [=](ReqResult result, const HttpResponsePtr &resp) {
        if (result == ReqResult::Ok)
        {
            auto json = resp->getJsonObject();
            if (json && (*json)["result"].asString() == "ok" && (*json)["P1"] == "upload" && (*json)["P2"] == "test")
            {
                outputGood(req, isHttps);
                //std::cout << (*json) << std::endl;
            }
            else
            {
                LOG_DEBUG << resp->getBody().length();
                LOG_ERROR << "Error!";
                exit(1);
            }
        }
        else
        {
            LOG_ERROR << "Error!";
            exit(1);
        }
    });
}

int main(int argc, char *argv[])
{
    trantor::EventLoopThread loop[2];
    trantor::Logger::setLogLevel(trantor::Logger::LogLevel::DEBUG);
    bool ever = false;
    if (argc > 1 && std::string(argv[1]) == "-f")
        ever = true;
    loop[0].run();
    loop[1].run();

    do
    {
        std::promise<int> pro1;
        auto client = HttpClient::newHttpClient("::1", 8848, false, loop[0].getLoop());
        doTest(client, pro1);
#ifdef USE_OPENSSL
        std::promise<int> pro2;
        auto sslClient = HttpClient::newHttpClient("127.0.0.1", 8849, true, loop[1].getLoop());
        doTest(sslClient, pro2, true);
        auto f2 = pro2.get_future();
        f2.get();
#endif
        auto f1 = pro1.get_future();
        f1.get();
        //LOG_DEBUG << sslClient.use_count();
    } while (ever);
    //getchar();
    loop[0].getLoop()->quit();
    loop[1].getLoop()->quit();
}
