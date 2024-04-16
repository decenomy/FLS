#ifndef BOOTSTRAP_H
#define BOOTSTRAP_H

#include <functional> 
#include <cerrno>  // For perror
#include <cstring>  // For strerror
#include <iostream>
#include <string>
#include <curl/curl.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/stat.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "minizip/unzip.h"

#ifndef TICKER
#define TICKER "FLS"
#endif

#define APPLE_CA_PATH "/etc/ssl/cert.pem"
//#define LINUX_CA_PATH "/etc/ssl/certs/ca-certificates.crt"
//#define WIN_CA_PATH ""

class Bootstrap{

public:
	// Define the function pointer type
    //using ProgressCallbackFunc = std::function<int(void*, double, double, double, double)>;
	static int ProgressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
	static bool init(const char* path);
	static bool rmDirectory(const std::string& path);
	static bool isDirectory(const std::string& path);
	static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
	static bool DownloadFile(const std::string& url, const std::string& outputFileName, curl_progress_callback);
	static bool extractZip(const std::string& inputPath, const std::string& outputPath);

private:

	static bool ensureOutputFolder(const std::string& outputPath);
	static bool endsWithSlash(const std::string& str);
};

#endif
