#include "HttpServer.h"
#include <string>
#include <pthread.h>

// Bitcask
#include "HttpBitcaskService.h"

using namespace std;
// http_server ************************************

int HTTPServer::start(int port) {

    
    this->http_thread = std::thread(this->http_server, port);
    
    return 0;
}

int HTTPServer::stop() {
    this->http_thread.detach();
    pthread_cancel(this->http_thread.native_handle());
    //this->http_thread.join();

    return 0;
}

int HTTPServer::http_server(int port){

    printf("hello\n");
    // HTTP
    httplib::Server svr;
    svr.set_base_dir("./");
    svr.set_file_extension_and_mimetype_mapping("cc", "text/x-c");
    svr.set_file_extension_and_mimetype_mapping("cpp", "text/x-c");
    svr.set_file_extension_and_mimetype_mapping("hh", "text/x-h");
    svr.set_file_extension_and_mimetype_mapping("h", "text/x-h");
    svr.set_file_extension_and_mimetype_mapping("mp3", "audio/mpeg");
    svr.set_file_extension_and_mimetype_mapping("mp4", "video/mpeg");
    svr.set_file_extension_and_mimetype_mapping("avi", "video/x-msvideo");
    // 访问http://localhost:8080/index.html 就是访问本地./index.html
    auto ret = svr.set_mount_point("/", "./");
    // HTTPS
    //httplib::SSLServer svr;

    // search接口

//    svr.Post("/search", search_callback);
//    svr.Post("/get_list", get_list_callback);


    // 测试接口
    svr.Get("/hi", [](const httplib::Request &, httplib::Response &res) {
      res.set_content("Hello World!", "text/plain");
    });
//    svr.Get("/add", add_callback);
//    svr.Get("/login", login_callback);
//    svr.Get("/",index_callback);

    // svr.Get("/index2", [](const httplib::Request &req , httplib::Response &res) {
    //   (void)req;
    //   std::string body = "<html>linux so easy</html>";
    //   res.set_content(body.c_str(),body.size(),"text/html");
    // });

    // Bitcask
    svr.Post("/bitcask_set", bitcask_set_callback);
    svr.Post("/bitcask_get", bitcask_get_callback);


    svr.listen("0.0.0.0", port);

    return 0;
}
