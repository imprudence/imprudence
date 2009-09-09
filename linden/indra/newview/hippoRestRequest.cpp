

#include "hippoRestRequest.h"

#ifndef CURL_STATICLIB
#define CURL_STATICLIB 1
#endif
#include <curl/curl.h>

#include <stdtypes.h>
#include <llerror.h>


static size_t curlWrite(void *ptr, size_t size, size_t nmemb, void *userData)
{
	std::string *result = (std::string*)userData;
	size_t bytes = (size * nmemb);
	result->append((char*)ptr, bytes);
	return nmemb;
}


//static
int HippoRestRequest::getBlocking(const std::string &url, std::string *result)
{
	llinfos << "Requesting: " << url << llendl;

	char curlErrorBuffer[CURL_ERROR_SIZE];
	CURL* curlp = curl_easy_init();

	curl_easy_setopt(curlp, CURLOPT_NOSIGNAL, 1);	// don't use SIGALRM for timeouts
	curl_easy_setopt(curlp, CURLOPT_TIMEOUT, 5);	// seconds

	curl_easy_setopt(curlp, CURLOPT_WRITEFUNCTION, curlWrite);
	curl_easy_setopt(curlp, CURLOPT_WRITEDATA, result);
	curl_easy_setopt(curlp, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curlp, CURLOPT_ERRORBUFFER, curlErrorBuffer);
	curl_easy_setopt(curlp, CURLOPT_FAILONERROR, 1);

	*result = "";
	S32 curlSuccess = curl_easy_perform(curlp);
	S32 httpStatus = 499;
	curl_easy_getinfo(curlp, CURLINFO_RESPONSE_CODE, &httpStatus);

	if (curlSuccess != 0) {
		llwarns << "CURL ERROR (HTTP Status " << httpStatus << "): " << curlErrorBuffer << llendl;
	} else if (httpStatus != 200) {
		llwarns << "HTTP Error " << httpStatus << ", but no Curl error." << llendl;
	}
	
	curl_easy_cleanup(curlp);
	return httpStatus;
}

