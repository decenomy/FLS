// Copyright (c) 2009-2014 The Bitcoin developers
// Copyright (c) 2017-2019 The PIVX developers
// Copyright (c) 2021-2022 The DECENOMY Core Developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "uritests.h"

#include "guiutil.h"
#include "walletmodel.h"

#include <QUrl>

void URITests::uriTests()
{
    SendCoinsRecipient rv;
    QUrl uri;

    //TODO: Fix and uncomment below tests
    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?req-dontexist="));
    // QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?dontexist="));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.label == QString());
    // QVERIFY(rv.amount == 0);

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?label=Some Example Address"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.label == QString("Some Example Address"));
    // QVERIFY(rv.amount == 0);

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?amount=0.001"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.label == QString());
    // QVERIFY(rv.amount == 100000);

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?amount=1.001"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.label == QString());
    // QVERIFY(rv.amount == 100100000);

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?amount=100&label=Some Example"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.amount == 10000000000LL);
    // QVERIFY(rv.label == QString("Some Example"));

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?message=Some Example Address"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.label == QString());

    // QVERIFY(GUIUtil::parseBitcoinURI("flits://79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?message=Some Example Address", &rv));
    // QVERIFY(rv.address == QString("79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU"));
    // QVERIFY(rv.label == QString());

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?req-message=Some Example Address"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?amount=1,000&label=Some Example"));
    // QVERIFY(GUIUtil::parseBitcoinURI(uri, &rv));

    // uri.setUrl(QString("flits:79xmgc7SzcCSmVBigS6HCa9oS35K2QJAvU?amount=1,000.0&label=Some Example"));
    // QVERIFY(!GUIUtil::parseBitcoinURI(uri, &rv));
}
