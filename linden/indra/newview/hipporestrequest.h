#ifndef __HIPPO_REST_REQUEST_H__
#define __HIPPO_REST_REQUEST_H__


#include <map>
#include <string>

#include <boost/shared_ptr.hpp>

class LLBufferArray;
class LLChannelDescriptors;
class LLXmlTree;


#define HIPPO_REST_TIMEOUT 60.f


// ********************************************************************
 

class HippoRestHandler
{
	public:
		virtual ~HippoRestHandler() { }
 
		virtual const char *getAcceptMimeType() const = 0;

		bool hasHeader(const std::string &header) const;
		const std::string &getHeader(const std::string &header) const;

	private:
		// These functions are called by the request engine
		void addHeader(const std::string &header, const std::string &content);
		virtual void handle(int status, const std::string &reason,
							const LLChannelDescriptors &channels,
							const boost::shared_ptr<LLBufferArray> &body) = 0;

		std::map<std::string, std::string> mHeaders;
		
		friend class HippoRestComplete;
};

 
class HippoRestHandlerRaw : public HippoRestHandler
{
	public:
		virtual ~HippoRestHandlerRaw() { }

		const char *getAcceptMimeType() const { return "application/octet-stream"; }

	private:
		// This function must be implemented to receive the content
		// it is executed on (status == 200) only
		virtual void result(const std::string &content) = 0;

		// This function is called by the request engine
		void handle(int status, const std::string &reason,
					const LLChannelDescriptors &channels,
					const boost::shared_ptr<LLBufferArray> &body);
};


class HippoRestHandlerXml : public HippoRestHandler
{
	public:
		virtual ~HippoRestHandlerXml() { }

		const char *getAcceptMimeType() const { return "application/xml"; }
		
	private:
		// This function must be implemented to receive the content
		virtual void result(LLXmlTree *content) = 0;

		// This function is called by the request engine
		void handle(int status, const std::string &reason,
					const LLChannelDescriptors &channels,
					const boost::shared_ptr<LLBufferArray> &body);
};


// ********************************************************************


class HippoRestRequest
{
	public:
		// asynchronous interface
		static void get(const std::string &url,
						HippoRestHandler *handler, float timeout=HIPPO_REST_TIMEOUT);
		static void put(const std::string &url, const std::string &body,
						HippoRestHandler *handler, float timeout=HIPPO_REST_TIMEOUT);
		static void put(const std::string &url, const LLXmlTree *body,
						HippoRestHandler *handler, float timeout=HIPPO_REST_TIMEOUT);
		static void post(const std::string &url, const std::string &body,
						 HippoRestHandler *handler, float timeout=HIPPO_REST_TIMEOUT);
		static void post(const std::string &url, const LLXmlTree *body,
						 HippoRestHandler *handler, float timeout=HIPPO_REST_TIMEOUT);
 
		// synchronous interface
		static int getBlocking(const std::string &url, std::string *result);
};


#endif
