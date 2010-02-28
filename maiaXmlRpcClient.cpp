/*
 * libMaia - maiaXmlRpcClient.cpp
 * Copyright (c) 2007 Sebastian Wiedenroth <wiedi@frubar.net>
 *                and Karl Glatz
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "maiaXmlRpcClient.h"
#include "maiaFault.h"

MaiaXmlRpcClient::MaiaXmlRpcClient(QObject* parent) : QObject(parent),
	manager(this), request() 
{

	request.setRawHeader("User-Agent", "libmaia 0.2");
	request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

	connect(&manager, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replyFinished(QNetworkReply*)));
}

MaiaXmlRpcClient::MaiaXmlRpcClient(QUrl url, QObject* parent) : QObject(parent),
	manager(this), request(url)
{
	request.setRawHeader("User-Agent", "libmaia 0.2");
	request.setHeader(QNetworkRequest::ContentTypeHeader, "text/xml");

	connect(&manager, SIGNAL(finished(QNetworkReply*)),
		this, SLOT(replyFinished(QNetworkReply*)));

	setUrl(url);
}

void MaiaXmlRpcClient::setUrl(QUrl url) {
	if(!url.isValid())
		return;
	
	request.setUrl(url);
}

QNetworkReply* MaiaXmlRpcClient::call(QString method, QList<QVariant> args,
							QObject* responseObject, const char* responseSlot,
							QObject* faultObject, const char* faultSlot) {
	MaiaObject* call = new MaiaObject(this);
	connect(call, SIGNAL(aresponse(QVariant &, QNetworkReply *)), responseObject, responseSlot);
	connect(call, SIGNAL(fault(int, const QString &, QNetworkReply *)), faultObject, faultSlot);

	QNetworkReply* reply = manager.post( request,
		call->prepareCall(method, args).toUtf8() );

	callmap[reply] = call;
	return reply;
}

void MaiaXmlRpcClient::replyFinished(QNetworkReply* reply) {
	QString response;
	if(!callmap.contains(reply))
		return;
	if(reply->error() != QNetworkReply::NoError) {
		MaiaFault fault(-32300, reply->errorString());
		response = fault.toString();
	} else {
		response = QString::fromUtf8(reply->readAll());
	}
	
	// parseResponse deletes the MaiaObject
	callmap[reply]->parseResponse(response, reply);
	delete reply;
	callmap.remove(reply);
}
