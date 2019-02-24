
#include <iostream>
#include <sstream>
#include <vector>
#include <zim/writer/zimcreator.h>
#include <zim/blob.h>
#include <zim/article.h>

#include "nbind/api.h"
#include "nbind/v8/External.h"


#include <nan.h>
#include <node_version.h>

class ZimArticle : public zim::writer::Article
{

  public:
    explicit ZimArticle(char ns,
                        std::string aid,
                        std::string url,
                        std::string title,
                        std::string mimeType,
                        std::string redirectAid,
                        std::string fileName,
                        char *bufferData,
                        size_t bufferLength) : ns(ns),
                                               aid(aid),
                                               url(url),
                                               title(title),
                                               mimeType(mimeType),
                                               redirectAid(redirectAid),
                                               fileName(fileName),
                                               bufferData(bufferData),
                                               bufferLength(bufferLength)
    {
        _data = zim::Blob(bufferData, bufferLength);
    }

    char ns;
    std::string aid;
    std::string url;
    std::string title;
    std::string mimeType;
    std::string redirectAid;
    std::string fileName;
    char *bufferData;
    size_t bufferLength;
    zim::Blob _data;

    zim::Blob getData() const
    {
        return _data;
    }

    std::string getFilename() const
    {
        // return fileName;
        return "";
    }

    virtual zim::size_type getSize() const { return _data.size(); }

    bool shouldIndex() const { return false; }

    std::string getAid() const
    {
        // return aid;
        return aid;
    }

    char getNamespace() const
    {
        return ns;
    }

    std::string getUrl() const
    {
        return url;
    }

    std::string getTitle() const
    {
        return title;
        // return title;
    }

    bool isRedirect() const
    {
        return !redirectAid.empty();
        // return false;
    }

    std::string getMimeType() const
    {
        return mimeType;
    }

    std::string getRedirectAid() const
    {
        return redirectAid;
        // return "";
    }

    bool shouldCompress() const
    {
        return getMimeType().find("text") == 0 || getMimeType() == "application/javascript" || getMimeType() == "application/json" || getMimeType() == "image/svg+xml";
        // return true;
    }
};

class Article
{
  public:
    char ns;
    std::string aid;
    std::string url;
    std::string title;
    std::string mimeType;
    std::string redirectAid;
    std::string fileName;
    nbind::Buffer buf;
    char *bufferData;
    size_t bufferLength;

    Article(std::string _ns,
            std::string aid,
            std::string url,
            std::string title,
            std::string mimeType,
            std::string redirectAid,
            std::string fileName,
            nbind::Buffer buf) : aid(aid),
                                 url(url),
                                 title(title),
                                 mimeType(mimeType),
                                 redirectAid(redirectAid),
                                 fileName(fileName),
                                 buf(buf)
    {
        ns = _ns[0];
        bufferLength = buf.length();
        bufferData = (char *)buf.data();
    }

    ZimArticle toZimArticle()
    {
        ZimArticle za(ns,
                      aid,
                      url,
                      title,
                      mimeType,
                      redirectAid,
                      fileName,
                      bufferData,
                      bufferLength);
        return za;
    }

    void toJS(nbind::cbOutput output)
    {
        output(url,
               NULL,
               std::string(1, ns),
               mimeType,
               title,
               redirectAid,
               aid,
               fileName);
    }
};

#include "nbind/nbind.h"

#ifdef NBIND_CLASS

NBIND_CLASS(Article)
{

    construct<std::string,
              std::string,
              std::string,
              std::string,
              std::string,
              std::string,
              std::string,
              nbind::Buffer>();
}

#endif
