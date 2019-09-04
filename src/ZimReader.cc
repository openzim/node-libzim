#include <string>
#include <iostream>
#include <zim/file.h>
#include <zim/search.h>

#include "Article.cc"
#include "nbind/api.h"
#include "ExternalBuffer.hpp"

static Article getArticleFromZimArticle(zim::Article _article)
{
    unsigned char *bufferData = (unsigned char *)_article.getData().data();
    size_t bufferLength = size_t(_article.getData().size());
    nbind::Buffer buf(bufferData, bufferLength);

    std::string ns = std::string(1, _article.getNamespace());

    std::string mimeType = "";
    std::string redirectUrl = "";

    if (!_article.isRedirect())
    {
        mimeType = _article.getMimeType();
    }
    else
    {
        auto rArticle = _article.getRedirectArticle();
        redirectUrl = rArticle.getLongUrl();
    }

    Article article(
        ns,
        "", // _article.getIndex(),
        _article.getUrl(),
        _article.getTitle(),
        mimeType,
        redirectUrl,
        "", // _article.getFilename(),
        false,
        buf);

    return article;
}

class ZimReaderWrapper
{
public:
    ZimReaderWrapper(zim::File *file) : _reader(file)
    {
    }

    ~ZimReaderWrapper()
    {
        delete _reader;
    }

    static ZimReaderWrapper *create(std::string fileName)
    {
        auto file = new zim::File(fileName);
        return new ZimReaderWrapper(file);
    }

    void destroy()
    {
        delete this;
    }

    void getCountArticles(nbind::cbFunction &callback)
    {
        callback(NULL, _reader->getCountArticles());
    }

    void getArticleById(uint32_t id, nbind::cbFunction &callback)
    {
        zim::Article article = _reader->getArticle(id);

        if (!article.good())
        {
            callback("Failed to find article");
            return;
        }

        Article _article = getArticleFromZimArticle(article);

        if (article.isRedirect())
        {
            callback(NULL, _article, NULL);
        }
        else
        {
            zim::Blob data = article.getData();

            ExternalBuffer eBuf((unsigned char *)data.data(), data.size());

            callback(NULL, _article, eBuf);
        }
    }

    void getArticleByUrl(std::string url, nbind::cbFunction &callback)
    {
        zim::Article article = _reader->getArticleByUrl(url);

        if (!article.good())
        {
            callback("Failed to find article");
            return;
        }

        Article _article = getArticleFromZimArticle(article);

        if (article.isRedirect())
        {
            callback(NULL, _article, NULL);
        }
        else
        {
            zim::Blob data = article.getData();

            ExternalBuffer eBuf((unsigned char *)data.data(), data.size());

            callback(NULL, _article, eBuf);
        }
    }

    void suggest(std::string query, nbind::cbFunction &callback)
    {
        try
        {
            std::vector<std::string> results;
            auto search = _reader->suggestions(query, 0, 10);
            for (auto it = search->begin(); it != search->end(); it++)
            {
                results.push_back(it->getLongUrl());
            }
            callback(NULL, results);
        }
        catch (...)
        {
            callback("Failed to search");
        }
    }

    void search(std::string query, nbind::cbFunction &callback)
    {
        try
        {
            std::vector<std::string> results;
            auto search = _reader->search(query, 0, 10);
            for (auto it = search->begin(); it != search->end(); it++)
            {
                results.push_back(it->getLongUrl());
            }
            // std::string url = it.get_snippet();
            // int numResults = search->get_matches_estimated();
            callback(NULL, results);
        }
        catch (...)
        {
            callback("Failed to search");
            throw;
        }
    }

    zim::File *_reader;
};

#include "nbind/nbind.h"

#ifdef NBIND_CLASS

NBIND_CLASS(ZimReaderWrapper)
{
    construct<zim::File *>();
    method(create);
    method(destroy);
    method(getArticleByUrl);
    method(getArticleById);
    method(getCountArticles);
    method(suggest);
    method(search);
}

#endif
