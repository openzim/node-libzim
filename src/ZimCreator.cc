#include <string>
#include <iostream>
#include <zim/writer/creator.h>

#include "Article.cc"
#include "nbind/api.h"

class OverriddenZimCreator : public zim::writer::Creator
{
public:
	OverriddenZimCreator(std::string mainPage)
		: zim::writer::Creator(true),
		  mainPage(mainPage) {}

	virtual zim::writer::Url getMainUrl()
	{
		return zim::writer::Url('A', mainPage);
	}

	std::string mainPage;
};

class ZimCreatorWrapper
{
public:
	ZimCreatorWrapper(OverriddenZimCreator *creator) : _creator(creator)
	{
	}

	~ZimCreatorWrapper()
	{
		delete _creator;
	}

	static ZimCreatorWrapper *create(std::string fileName, std::string mainPage, std::string fullTextIndexLanguage, int minChunkSize)
	{
		bool shouldIndex = !fullTextIndexLanguage.empty();

		OverriddenZimCreator *c = new OverriddenZimCreator(mainPage); // TODO: consider when to delete this
		c->setIndexing(shouldIndex, fullTextIndexLanguage);
		c->setMinChunkSize(minChunkSize);
		c->startZimCreation(fileName);
		return (new ZimCreatorWrapper(c));
	}

	void addArticle(Article article, nbind::cbFunction &callback)
	{
		try
		{
			ZimArticle a = article.toZimArticle();
			_creator->addArticle(a);
			callback();
		}
		catch (...)
		{
			callback("Failed to add article");
			throw;
		}
	}

	void finalise(nbind::cbFunction &callback)
	{
		_creator->finishZimCreation();
		callback();
		delete this;
	}

	OverriddenZimCreator *_creator;
};

#include "nbind/nbind.h"

#ifdef NBIND_CLASS

NBIND_CLASS(ZimCreatorWrapper)
{
	construct<OverriddenZimCreator *>();
	method(create);
	method(addArticle);
	method(finalise);
}

#endif
